#include "ir_to_asm.h"

const x64asm::R64 IrInterpreter::callerSavedRegs[] = {
    // the first 6 are the arg regs
    x64asm::rdi,
    x64asm::rsi,
    x64asm::rdx,
    x64asm::rcx,
    x64asm::r8,
    x64asm::r9,
    // end arg regs
    x64asm::r10, 
    x64asm::r11,
    x64asm::rax 
};

const x64asm::R64 IrInterpreter::calleeSavedRegs[] = {
    // rbp and rsp are handled a little differently, so will not consider
    // callee-saved
    x64asm::rbx,
    x64asm::r12,
    x64asm::r13,
    x64asm::r14,
    x64asm::r15
};

IrInterpreter::IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer, vector<bool> isLocalRefVec) {
    vmPointer = vmInterpreterPointer;
    func = irFunction;
    isLocalRef = isLocalRefVec;
    // by convention, the first ir function is the main function
    instructionIndex = 0;
    finished = false;
}

void IrInterpreter::comparisonSetup(x64asm::R32 left, x64asm::R32 right, instptr_t inst) {
    // load right temp into a reg
    loadTemp(right, inst->tempIndices->at(1));
    // load left temp into a reg
    loadTemp(left, inst->tempIndices->at(2));
    // perform the sub; result stored in left
    assm.cmp(left, right); // this sets flags
    // load 0 and 1 into two diff regs
    assm.mov(right, x64asm::Imm32{1});
    assm.mov(left, x64asm::Imm32{0});
};

void IrInterpreter::installLocalVar(tempptr_t temp, uint32_t localIdx) {
    // IMPORTANT! make sure you order these calls in a way that you do not
    // overrwrite rdi or rsi before you finish needing them
    if (temp->reg) {
        // move directly to the reg
        assm.assemble({x64asm::MOV_R64_M64, {
             temp->reg.value(),
             x64asm::M64{
                 x64asm::rdi, 
                 x64asm::Scale::TIMES_1,
                 x64asm::Imm32{localIdx*8}
             }
        }});
    } else {
        // the arg is stored on the stack; use rdi as scratch space
        assm.push(x64asm::rdi); 
        // move the value into rdi 
        assm.assemble({x64asm::MOV_R64_M64, {
             x64asm::rdi,
             x64asm::M64{
                 x64asm::rdi, 
                 x64asm::Scale::TIMES_1,
                 x64asm::Imm32{localIdx*8}
             }
        }});
        // move rdi onto the right stack location 
        uint32_t offset = getTempOffset(temp);
        assm.assemble({x64asm::MOV_M64_R64, {
            x64asm::M64{
                x64asm::rbp, 
                x64asm::Scale::TIMES_1,
                x64asm::Imm32{-offset}
            }, 
            x64asm::rdi
        }});
        // restore rdi
        assm.pop(x64asm::rdi);
    }
}

void IrInterpreter::installLocalNone(tempptr_t temp) {
    if (temp->reg) {
        assm.mov(temp->reg.value(), x64asm::Imm64{vmPointer->NONE});
    } else {
        // put None in a scratch reg
        assm.push(x64asm::rdi); 
        assm.mov(x64asm::rdi, x64asm::Imm64{vmPointer->NONE});
        moveTemp(temp, x64asm::rdi); 
        assm.pop(x64asm::rdi);
    }
}

void IrInterpreter::installLocalRefVar(tempptr_t temp, uint32_t localIdx) { 
    x64asm::R64 reg = x64asm::rdi; 
    bool usedScratch = false;
    if (temp->reg) {
        reg = temp->reg.value(); 
    } else {
        // use rdi as scratch space
        assm.push(reg);
        usedScratch = true;
    }
    // move the value to the right reg
    assm.assemble({x64asm::MOV_R64_M64, {
         reg,
         x64asm::M64{
             x64asm::rdi, 
             x64asm::Scale::TIMES_1,
             x64asm::Imm32{8*localIdx}
         }
    }});
    // convert to a valwrapper
    vector<tempptr_t> noTempArgs;
    vector<x64asm::Imm64> args = {
        x64asm::Imm64{vmPointer},
    };
    tempptr_t returnTemp = temp;
    // this will put the result back inside the right temp
    callHelper((void*) &(helper_new_valwrapper), args, noTempArgs, reg, returnTemp);

    // cleanup
    if (usedScratch) {
        // restore rdi 
        assm.pop(reg);
    }
}

void IrInterpreter::installLocalRefNone(tempptr_t temp) {
    x64asm::R64 reg = x64asm::rdi; 
    bool usedScratch = false;
    if (temp->reg) {
        reg = temp->reg.value();
    } else {
        usedScratch = true;
        assm.push(reg);
    }
    assm.mov(reg, x64asm::Imm64{vmPointer->NONE});
    
    // convert to ValWrapper
    vector<tempptr_t> noTempArgs;
    vector<x64asm::Imm64> args = {
        x64asm::Imm64{vmPointer},
    };
    tempptr_t returnTemp = temp;
    // this will put the result back inside the right temp
    callHelper((void*) &(helper_new_valwrapper), args, noTempArgs, reg, returnTemp);

    if (usedScratch) {
        assm.pop(x64asm::rdi);
    }
}


void IrInterpreter::prolog() {
    // push old rbp 
    assm.push(x64asm::rbp);

    // move old rsp to rbp
    assm.mov(x64asm::rbp, x64asm::rsp);

    // push callee saved, including rbp
    for (int i = 0; i < numCalleeSaved; ++i) {
        assm.push(calleeSavedRegs[i]);
    }
    // allocate space for locals, refs, and temps on the stack 
    // by decrementing rsp 
    // and note that we are only storing on ref pointer by pushing the pointer to the array
    spaceToAllocate = 8*(1 + func->temps.size()); // locals are temps now
    assm.assemble({x64asm::SUB_R64_IMM32, {x64asm::rsp, x64asm::Imm32{spaceToAllocate}}});

    // put a pointer to the references onto the stack
    // ptr to ref array is the second arg
    uint32_t refArrayOffset = getRefArrayOffset();
    assm.assemble({
        x64asm::MOV_M64_R64, {
            x64asm::M64{
                 x64asm::rbp, 
                 x64asm::Scale::TIMES_1,
                 x64asm::Imm32{-refArrayOffset}
            },
            x64asm::rsi
        }
    });

    // put args in correct location
    int32_t rdiTemp = -1; // we need to do this one last so we don't clobber rdi

    // rdi stores arg0 which is the list of arguments to the MS func
    for (uint64_t i = 0; i < func->parameter_count_; i++) {
        tempptr_t localTemp = func->temps.at(i);
        if (localTemp->startInterval == -1 && localTemp->endInterval == -1) {
           continue;  // the arg is never used, don't bother
        }

        if (localTemp->reg && (localTemp->reg.value() == x64asm::rdi)) {
            // current arg is stored in a register; make sure it won't 
            // overwrite rdi 
            rdiTemp = i; 
            continue;
            // else, we are safe to move the reg into its arg 
        }

        if (isLocalRef.at(i)) {
            installLocalRefVar(localTemp, i);
        } else {
            installLocalVar(localTemp, i);
        }
    }
    // now do rdi, if applicable
    if (rdiTemp >= 0) {
        if (isLocalRef.at(rdiTemp)) {
            installLocalVar(func->temps.at(rdiTemp), rdiTemp);
        } else {
            installLocalRefVar(func->temps.at(rdiTemp), rdiTemp);
        }
    }

    // set all other locals to none
    for (uint64_t i = func->parameter_count_; i < func->local_count_; i++) {
        tempptr_t localTemp = func->temps.at(i);
        if (isLocalRef.at(i)) {
            installLocalNone(localTemp);             
        } else {
            installLocalRefNone(localTemp);
        }
    }
}

void IrInterpreter::epilog() {
    // TO USE THIS FUNCTION, FIRST MAKE SURE YOU HAVE CLEARED OUT THE STACK
    // AND PUT THE RETURN VALUE IN THE RIGHT REG

    // increment rsp again to deallocate locals, temps, etc
    assm.assemble({x64asm::ADD_R64_IMM32, {x64asm::rsp, x64asm::Imm32{spaceToAllocate}}});

    // in reverse order, restore the callee-save regs
    for (int i = 1; i <= numCalleeSaved; ++i) {
        assm.pop(calleeSavedRegs[numCalleeSaved - i]);
    }
    
    // restore rsp and rbp 
    assm.mov(x64asm::rsp, x64asm::rbp);
    assm.pop(x64asm::rbp);

    // return using the stored return address 
    assm.ret();
}

x64asm::Function IrInterpreter::run() {
    // start the assembler on the function
    assm.start(asmFunc);
	asmFunc.reserve(func->instructions.size() * 100); // TODO: figure out how to allocate the right amount of memory
    prolog();

    if (func->instructions.size() > 0) {
        // translate to asm
        while (!finished) {
            executeStep();
        }
    }
    
    // move None into rax
    assm.assemble({x64asm::MOV_R64_IMM64, {x64asm::rax, x64asm::Imm64{vmPointer->NONE}}});
    epilog();

    // finish compiling
    assm.finish();
    LOG("done compiling asm");
    return asmFunc;
}

void IrInterpreter::callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, opttemp_t returnTemp) {
    callHelper(fn, args, temps, nullopt, returnTemp);
}


void IrInterpreter::callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, optreg_t lastArg, opttemp_t returnTemp) {
    int totalArgs = args.size() + temps.size() + 1;
    assert (totalArgs <= numArgRegs);
    
    // IMPORTANT: the reg in lastArg should not be r10, rax, or any of the 6 argRegs
    // STEP 1: save caller-saved registers to stack, reverse order
    for (int i = 0; i < numCallerSaved; ++i) {
        assm.push(callerSavedRegs[numCallerSaved -1 - i]);
    }

    // STEP 2: push first 6 arguments to registers, rest to stack
    // args + temps are put in that order as arguments for the function to call
    // on the stack
    // args should contain pointers to values or immediate numbers
    // temps are pointers to Temps which store stack offsets
    int numArgs = args.size() + temps.size();
    int argIndex = 0;
    LOG("  calling helper w/ " + to_string(numArgs) + " total args");
    while (argIndex < args.size()) {
        // put args 1 - 6 into regs (direct imm values)
        assm.mov(callerSavedRegs[argIndex], args[argIndex]);
        argIndex++;
    }
    while (argIndex < numArgs) {
        tempptr_t tempToPush = temps[argIndex - args.size()];
        x64asm::R64 regToStore = callerSavedRegs[argIndex];
        // load differently depending on where the arg is
        if (tempToPush->reg) {
            // if the val is stored in an earlier arg reg, 
            // have to get off the stack 
            bool wasClobbered = false;
            for (uint32_t i = 0; i < argIndex; i++) {
                if (callerSavedRegs[i] == tempToPush->reg.value()) {
                   // the val got clobbered; get it from the stack
                   assm.assemble({x64asm::MOV_R64_M64, {
                        regToStore,
                        x64asm::M64{
                            x64asm::rsp, 
                            x64asm::Scale::TIMES_1,
                            x64asm::Imm32{i*8}
                        }
                   }});
                   wasClobbered = true;
                   break;
                 }
            } 
            if (!wasClobbered) {
                // the val was not clobbered; move directly 
                assm.mov(regToStore, tempToPush->reg.value()); 
            }
        } else { // the value is stored on the stack; move from mem
            // move to the right reg from mem
            uint32_t offset = getTempOffset(tempToPush);
            assm.assemble({x64asm::MOV_R64_M64, {
                regToStore,
                x64asm::M64{
                    x64asm::rbp, 
                    x64asm::Scale::TIMES_1,
                    x64asm::Imm32{-offset}
                }
            }});
        }
        argIndex++;
    }
    // if the optional register argument is defined, push it as an arg
    if (lastArg) {
        assm.mov(callerSavedRegs[argIndex], lastArg.value());
    }

    assm.mov(x64asm::r10, x64asm::Imm64{fn}); 
    assm.call(x64asm::r10);

    // assume we do not have args to pop from the stack 
    
    // restore caller-saved registers from stack, minus rax
    for (uint32_t i = 0; i < numCallerSaved - 1; ++i) {
        assm.pop(callerSavedRegs[i]);
    }
    
    // save return value
    if (returnTemp) {
        // move from rax to the temp 
        bool usesRax = (returnTemp.value()->reg) && (returnTemp.value()->reg.value() == x64asm::rax);
        if (!usesRax) {
            moveTemp(returnTemp.value(), x64asm::rax);
            assm.pop(x64asm::rax);
        } // if it does use rax, nothing to do! 
    } else {
        // restore rax
        assm.pop(x64asm::rax);
    }
}

uint32_t IrInterpreter::getRefArrayOffset() {
    return 8*(1 + numCalleeSaved);
}

uint32_t IrInterpreter::getTempOffset(tempptr_t temp) {
    return 8*(1 + numCalleeSaved + 1 + temp->index);
}

void IrInterpreter::getRbpOffset(uint32_t offset) {
    // put rbp in a reg
    assm.mov(x64asm::r10, x64asm::rbp);
    // subtract the offset from rbp 
    assm.assemble({x64asm::SUB_R64_IMM32, {x64asm::r10, x64asm::Imm32{offset}}});
}

// storeTemp puts a reg value into a temp
void IrInterpreter::storeTemp(x64asm::R32 reg, tempptr_t temp) {
    // right now, put everything on the stack
    // assign the temp an offset (from rbp)
    getRbpOffset(getTempOffset(temp)); // leaves correct address into r10
    // Move the val in reg into the mem address stored in r10
    assm.mov(x64asm::M64{x64asm::r10}, reg);
}

// storeTemp puts a reg value into a temp
void IrInterpreter::storeTemp(x64asm::R64 reg, tempptr_t temp) {
    // right now, put everything on the stack
    // assign the temp an offset (from rbp)
    getRbpOffset(getTempOffset(temp)); // leaves correct address into r10
    // Move the val in reg into the mem address stored in r10
    assm.mov(x64asm::M64{x64asm::r10}, reg);
}

// loadTemp takes a temp value and puts it in a register
void IrInterpreter::loadTemp(x64asm::R32 reg, tempptr_t temp) {
    getRbpOffset(getTempOffset(temp)); // location in r10
    // put the thing from that mem address into the reg
    assm.mov(reg, x64asm::M64{x64asm::r10});
}

// loadTemp takes a temp value and puts it in a register
void IrInterpreter::loadTemp(x64asm::R64 reg, tempptr_t temp) {
    getRbpOffset(getTempOffset(temp)); // location in r10
    // put the thing from that mem address into the reg
    assm.mov(reg, x64asm::M64{x64asm::r10});
}

/////////// REG ALLOCATION HELPERS ////////////

void IrInterpreter::updateFreeRegs(instptr_t inst) {
    // for each temp in the instruction
    for (int i = 0; i < inst->tempIndices->size(); i++) {
        tempptr_t temp = inst->tempIndices->at(i);
        if (!temp->reg) {
            return;
        }

        if (temp->startInterval == instructionIndex) {
            // if a temp started here, add its reg
            freeRegs.insert(temp->reg.value());
        }
        if (temp->endInterval -1 == instructionIndex) {
            // if the temp ended here, remove it from the set 
            freeRegs.erase(temp->reg.value());
        }
    }
}

x64asm::R64 IrInterpreter::getReg(tempptr_t temp) {
    if (temp->reg) {
        return temp->reg.value();
    } else {
        uint32_t offset = temp->stackOffset.value();
        // our temp is on the stack
        // get a temp to load it into 
        x64asm::R64 reg = getScratchReg();
        // mov it into the scratch reg
        assm.mov(reg, x64asm::rbp);
        assm.assemble({x64asm::SUB_R64_IMM32, {reg, x64asm::Imm32{offset}}});
        return reg;
    }
} 

void IrInterpreter::setReg(tempptr_t temp, x64asm::R64 reg) {
    if (reg == temp->reg.value()) {
        // it's already in the right reg
        return;
    }
    if (temp->reg) {
        // do a simple reg mov 
        assm.mov(temp->reg.value(), reg);
        return;
    } else {
        // gotta put it back onto the stack eww
        // TODO
    }
}

x64asm::R64 IrInterpreter::getScratchReg() {
    for (x64asm::R64 reg : freeRegs) {
        return reg; 
    };
    // there are no free regs avlb; we gotta dump something 
    // for now just randomly dump r10 
    // TODO
    LOG("spilling");
    spilled = true;
    x64asm::R64 toSpill = x64asm::r10; 
    assm.push(toSpill); 
    return toSpill;
};

void IrInterpreter::returnScratchReg(x64asm::R64 reg) {
    // release a scratch reg for other uses and 
    // restores the regiser to its previous value 
    freeRegs.insert(reg);
    // TODO: you should ONLY pop if you had to push!!!
    // TODO
    if (spilled) {
        assm.pop(reg);
        spilled = false;
        LOG("returned from spill");
    }
}

void IrInterpreter::moveTemp(x64asm::R64 dest, tempptr_t src) {
    if (src->reg) {
        assm.mov(dest, src->reg.value());
    } else {
        uint32_t offset = getTempOffset(src);
        assm.assemble({x64asm::MOV_R64_M64, {
            dest,
            x64asm::M64{
                x64asm::rbp, 
                x64asm::Scale::TIMES_1,
                x64asm::Imm32{-offset}
            }
        }});
    }
}

void IrInterpreter::moveTemp(tempptr_t dest, x64asm::R64 src) {
    if (dest->reg) {
        assm.mov(dest->reg.value(), src);
    } else {
        // dest in mem, src is a reg
        uint32_t destOffset = getTempOffset(dest);
        assm.assemble({x64asm::MOV_M64_R64, {
            x64asm::M64{
                x64asm::rbp, 
                x64asm::Scale::TIMES_1,
                x64asm::Imm32{-destOffset}
            }, 
            src
        }});
    }
}

void IrInterpreter::moveTemp(tempptr_t dest, tempptr_t src) {
    if (dest->reg) {
        moveTemp(dest->reg.value(), src);
    } else {
        if (src->reg) {
            // dest is in memory, src in a reg
            uint32_t offset = getTempOffset(dest);
            assm.assemble({x64asm::MOV_M64_R64, {
                x64asm::M64{
                    x64asm::rbp, 
                    x64asm::Scale::TIMES_1,
                    x64asm::Imm32{-offset}
                }, 
                src->reg.value()
            }});
        } else { // both are in memory :'( 
            // we need a scratch reg
            x64asm::R64 reg = getScratchReg();
            // move src into the reg
            uint32_t srcOffset = getTempOffset(src);
            assm.assemble({x64asm::MOV_R64_M64, {
                reg,
                x64asm::M64{
                    x64asm::rbp, 
                    x64asm::Scale::TIMES_1,
                    x64asm::Imm32{-srcOffset}
                }
            }});
            // move the reg into dest
            uint32_t destOffset = getTempOffset(dest);
            assm.assemble({x64asm::MOV_M64_R64, {
                x64asm::M64{
                    x64asm::rbp, 
                    x64asm::Scale::TIMES_1,
                    x64asm::Imm32{-destOffset}
                }, 
                reg
            }});
            // give back the reg
            returnScratchReg(reg);
        }
    }
}

/////////// END REG ALLOCATION HELPERS ////////////

void IrInterpreter::executeStep() {
    instptr_t inst = func->instructions.at(instructionIndex);
    int freeRegsStartSize = freeRegs.size();
    switch(inst->op) {
        case IrOp::LoadConst:
            {
                LOG(to_string(instructionIndex) + ": LoadConst");
                int constIndex = inst->op0.value();
                Constant* c = func->constants_.at(constIndex);
                tempptr_t t = inst->tempIndices->at(0);
                if (t->reg) {
                    assm.mov(t->reg.value(), x64asm::Imm64{(uint64_t)c});
                } else {
                    // get a scratch reg to store this 
                    x64asm::R64 reg = getScratchReg();
                    // move the val into the scratch reg
                    assm.mov(t->reg.value(), x64asm::Imm64{(uint64_t)c});
                    // get the offset of the temp on the stack
                    uint32_t offset = getTempOffset(t);
                    // this moves the val at reg into the right rbp offset
                    assm.assemble({x64asm::MOV_M64_R64, {
                        x64asm::M64{
                            x64asm::rbp,
                            x64asm::Scale::TIMES_1, 
                            x64asm::Imm32{-offset},
                        },
                        reg
                    }});
                    // give back the scratch reg
                    returnScratchReg(reg);
                }
                break;
            }
        case IrOp::LoadFunc:
            {
                LOG(to_string(instructionIndex) + ": LoadFunc");
                int funcIndex = inst->op0.value();
                Function* f = func->functions_.at(funcIndex);
                tempptr_t t = inst->tempIndices->at(0);
                if (t->reg) {
                    assm.mov(t->reg.value(), x64asm::Imm64{(uint64_t)f});
                } else {
                    // get a scratch reg to store this 
                    x64asm::R64 reg = getScratchReg();
                    // move the val into the scratch reg
                    assm.mov(reg, x64asm::Imm64{(uint64_t)f});
                    // get the offset of the temp on the stack
                    uint32_t offset = getTempOffset(t);
                    // this moves the val at reg into the right rbp offset
                    assm.assemble({x64asm::MOV_M64_R64, {
                        x64asm::M64{
                            x64asm::rbp,
                            x64asm::Scale::TIMES_1, 
                            x64asm::Imm32{-offset},
                        },
                        reg
                    }});
                    // give back the scratch reg
                    returnScratchReg(reg);
                }
                break;
            }
        case IrOp::LoadLocal:
            {
                LOG(to_string(instructionIndex) + ": LoadLocal");
                tempptr_t src = inst->tempIndices->at(1);
                tempptr_t dest = inst->tempIndices->at(0);
                moveTemp(dest, src); 
                break;
            }
        case IrOp::LoadGlobal:
            {
                LOG(to_string(instructionIndex) + ": LoadGlobal");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name},
                };
                vector<tempptr_t> temps;
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_load_global), args, temps, returnTemp);
                break;
            }
        case IrOp::StoreLocal:
            {
                LOG(to_string(instructionIndex) + ": StoreLocal");
                tempptr_t src = inst->tempIndices->at(1);
                tempptr_t dest = inst->tempIndices->at(0);
                moveTemp(dest, src);
                break;
            }
       case IrOp::StoreGlobal:
            {
                LOG(to_string(instructionIndex) + ": StoreGlobal");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name},
                };
                vector<tempptr_t> temps = {inst->tempIndices->at(0)};
                callHelper((void *) &(helper_store_global), args, temps, nullopt);
                break;
            }
		case IrOp::StoreLocalRef: 
			{
                // TODO
				LOG(to_string(instructionIndex) + ": StoreLocalRef");
                vector<x64asm::Imm64> args = {
				};
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1), // value to store
					inst->tempIndices->at(0) // local valwrapper
				};
                tempptr_t returnTemp = inst->tempIndices->at(1);
                callHelper((void *) &(helper_store_local_ref), args, temps, returnTemp);
                break;
			}
        case IrOp::PushLocalRef: 
            {
                // TODO
                LOG(to_string(instructionIndex) + ": PushLocalRef");
                tempptr_t localTemp = inst->tempIndices->at(1);
                loadTemp(x64asm::rdi, localTemp);
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::PushFreeRef: 
            {
                // TODO
                LOG(to_string(instructionIndex) + ": PushFreeRef");
                // the ValWrapper is already sitting in the refs array;
                // just move it into the temp 
                
                // get address of ref array
                getRbpOffset(getRefArrayOffset());
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10});
                // increment rdi to index into ref array
                uint32_t offset = 8*inst->op0.value();
                assm.assemble({x64asm::ADD_R64_IMM32, {x64asm::rdi, x64asm::Imm32{offset}}});
                // one deref gets us to the val wrapper
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::rdi});
                // store this in a temp 
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::LoadReference:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": LoadReference");
                // dereferences to get to the object itself
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {inst->tempIndices->at(1)};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_unbox_valwrapper), args, temps, returnTemp);
                break;
            }
        case IrOp::AllocRecord:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": AllocRecord");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
				};
                vector<tempptr_t> temps;
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_new_record), args, temps, returnTemp);
                break;
            };
        case IrOp::FieldLoad:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": FieldLoad");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name}
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1),
				};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_get_record_field), args, temps, returnTemp);
                break;
            };
        case IrOp::FieldStore:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": FieldStore");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0),
                    inst->tempIndices->at(1),
				};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_set_record_field), args, temps, returnTemp);
                break;
            };
        case IrOp::IndexLoad:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": IndexLoad");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(2),
                    inst->tempIndices->at(1),
				};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_get_record_index), args, temps, returnTemp);
                break;
            };
        case IrOp::IndexStore:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": IndexStore");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer}, // vm pointer
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(2), // index 
                    inst->tempIndices->at(0), // record
                    inst->tempIndices->at(1), // val
				};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_set_record_index), args, temps, returnTemp);
                break;
            };
        case IrOp::AllocClosure:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": AllocClosure");
                // the first two args to the helper are the 
                // interpreter pointer and the num refs
                int numRefs = inst->op0.value();
                vector<x64asm::Imm64> immArgs = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{(uint64_t)numRefs},
                };
                // the rest of the args are basically the temps minus temp0
                // create an "array" by pushing these to the stack 
                tempptr_t refTemp;
                for (int i = 0; i < numRefs; ++i) {
                    // push in reverse order, so first ref is lowest 
                    refTemp = inst->tempIndices->at(2 + numRefs - i - 1);
                    if (refTemp->reg) {
                        assm.push(refTemp->reg.value());
                    } else {
                        // TODO don't use r10 as a lazy reg pls
                        // on the stack; 
                        uint32_t offset = getTempOffset(refTemp);
                        assm.assemble({x64asm::MOV_R64_M64, {
                            x64asm::r10,
                            x64asm::M64{
                                x64asm::rbp, 
                                x64asm::Scale::TIMES_1,
                                x64asm::Imm32{-offset}
                            }
                        }});
                        assm.push(x64asm::r10);
                    }
                }

                // store the array pointer in a temp so callHelper can use it
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1), // function
                };
                x64asm::R64 refs = x64asm::r10; // array of regs
                assm.mov(x64asm::r10, x64asm::rsp);

                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_alloc_closure), immArgs, temps, refs, returnTemp);
                // clear the stack
                for (int i = 0; i < numRefs; i++) {
                    assm.pop(x64asm::r10);
                };
                break;
            };
        case IrOp::Call:
            {
                // TODO
                // we push all the MITScript function arguments to the stack,
                // then pass %rsp (which points to the first element of that
                // array) as an argument to helper_call
                LOG(to_string(instructionIndex) + ": Call");
                uint64_t numArgs = inst->op0.value();
                // push all the MITScript function arguments to the stack
                // to make a contiguous array in memory
                tempptr_t argTemp;
                for (int i = 0; i < numArgs; ++i) {
                    // push in reverse order, so first arg is lowest
                    argTemp = inst->tempIndices->at(2 + numArgs - i - 1);
                    if (argTemp->reg) {
                        assm.push(argTemp->reg.value());
                    } else {
                        // TODO don't use r10 as a lazy reg pls
                        // on the stack; 
                        uint32_t offset = getTempOffset(argTemp);
                        assm.assemble({x64asm::MOV_R64_M64, {
                            x64asm::r10,
                            x64asm::M64{
                                x64asm::rbp, 
                                x64asm::Scale::TIMES_1,
                                x64asm::Imm32{-offset}
                            }
                        }});
                        assm.push(x64asm::r10);
                    }
                }

                // helper_call takes args: vm pointer, numArgs, 
                // closure, array of args
                vector<x64asm::Imm64> immArgs = {
                    x64asm::Imm64{vmPointer}, // vm pointer
                    x64asm::Imm64{numArgs}   // numArgs
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1) // closure
                };
                x64asm::R64 argsArray = x64asm::r10; // args
                assm.mov(x64asm::r10, x64asm::rsp);
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_call), immArgs, temps, argsArray, returnTemp);

                // clear the stack
                for (int i = 0; i < numArgs; i++) {
                    assm.pop(x64asm::r10);
                };
                break;
            };
        case IrOp::Return:
            {
                // TODO
                LOG(to_string(instructionIndex) + ": Return");
                getRbpOffset(getTempOffset(inst->tempIndices->at(0)));
                assm.mov(x64asm::rax, x64asm::M64{x64asm::r10});
                epilog();
                break;
            };
        case IrOp::Add:
            {
                LOG(to_string(instructionIndex) + ": Add");
                // call a helper to do add
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(2),
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_add), args, temps, returnTemp);
                break;
            };
        case IrOp::Sub:
            {
                LOG(to_string(instructionIndex) + ": Sub");
                //rdi and rsi
                // load the left temp into a reg
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                loadTemp(left, inst->tempIndices->at(2));
                // load right temp into a reg
                loadTemp(right, inst->tempIndices->at(1));
                // perform the sub; result stored in left
                assm.sub(left, right);
                // put the value back in the temp
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Mul:
            {
                LOG(to_string(instructionIndex) + ": Mul");
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                loadTemp(left, inst->tempIndices->at(2));
                // load right temp into a reg
                loadTemp(right, inst->tempIndices->at(1));
                // perform the sub; result stored in left
                assm.imul(left, right);
                // put the value back in the temp
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Div:
            {
                LOG(to_string(instructionIndex) + ": Div");
                auto numerator_secondhalf = x64asm::eax;
                loadTemp(numerator_secondhalf, inst->tempIndices->at(2));
    			assm.cdq(); // weird asm thing to sign-extend rax into rdx
                vector<x64asm::Imm64> args = {};
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1),
                };
                callHelper((void *) &(helper_assert_nonzero), args, temps, nullopt);
                auto divisor = x64asm::ebx;
				loadTemp(divisor, inst->tempIndices->at(1));
                // perform the div; result stored in rax
                assm.idiv(divisor);
                // put the value back in the temp
                storeTemp(x64asm::eax, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Neg: {
                LOG(to_string(instructionIndex) + ": Neg");
                auto operand = x64asm::edi;
                loadTemp(operand, inst->tempIndices->at(1));
                assm.neg(operand);
                storeTemp(operand, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Gt:
            {
                LOG(to_string(instructionIndex) + ": Gt");
                // use a conditional move to put the bool in the right place
                // right(1) gets moved into left(0) if left was greater
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                comparisonSetup(left, right, inst);
                assm.cmovnle(left, right);
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Geq:
            {
                LOG(to_string(instructionIndex) + ": Geq");
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                // use a conditional move to put the bool in the right place
                // right(1) gets moved into left(0) if left was greater
                comparisonSetup(left, right, inst);
                assm.cmovnl(left, right);
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Eq:
            {
                LOG(to_string(instructionIndex) + ": Eq");
                // call a helper to do equality 
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(2),
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_eq), args, temps, returnTemp);
                break;
            };
        case IrOp::And:
            {
                LOG(to_string(instructionIndex) + ": And");
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                loadTemp(left, inst->tempIndices->at(2));
                // load right temp into a reg
                loadTemp(right, inst->tempIndices->at(1));
                // perform the sub; result stored in left
                assm.and_(left, right);
                // put the value back in the temp
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Or:
            {
                LOG(to_string(instructionIndex) + ": Or");
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                loadTemp(left, inst->tempIndices->at(2));
                // load right temp into a reg
                loadTemp(right, inst->tempIndices->at(1));
                // perform the sub; result stored in left
                assm.or_(left, right);
                // put the value back in the temp
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Not:
            {
                LOG(to_string(instructionIndex) + ": Not");
                auto operand = x64asm::edi;
				auto one = x64asm::esi;
                loadTemp(operand, inst->tempIndices->at(1));
                assm.mov(one, x64asm::Imm32{1});
				assm.xor_(operand, one);
                storeTemp(operand, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Goto:
            {
                LOG(to_string(instructionIndex) + ": Goto");
                string labelStr = to_string(inst->op0.value());
                LOG("jumping to label " + labelStr);
                assm.jmp_1(x64asm::Label{labelStr});
                break;
            };
        case IrOp::If:
            {
                LOG(to_string(instructionIndex) + ": If");
                string labelStr = to_string(inst->op0.value());
                // load the temp into a reg
                auto left = x64asm::edi;
                auto right = x64asm::esi;
                loadTemp(left, inst->tempIndices->at(0));
                assm.mov(right, x64asm::Imm32{1});
                assm.cmp(left, right);
                LOG("jumping if to label " + labelStr);
                assm.je_1(x64asm::Label{labelStr});
                break;
            };
       case IrOp::AssertInteger:
            {
                LOG(to_string(instructionIndex) + ": AssertInteger");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_int), args, temps, nullopt);
                break;
            };
        case IrOp::AssertBoolean:
            {
                LOG(to_string(instructionIndex) + ": AssertBool");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_bool), args, temps, nullopt);
                break;
            };
        case IrOp::AssertString:
            {
                LOG(to_string(instructionIndex) + ": AssertString");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_str), args, temps, nullopt);
                break;
            };
        case IrOp::AssertRecord:
            {
                LOG(to_string(instructionIndex) + ": AssertRecord");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_record), args, temps, nullopt);
                break;
            };
        case IrOp::AssertFunction:
            {
                LOG(to_string(instructionIndex) + ": AssertFunction");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_func), args, temps, nullopt);
                break;
            };
        case IrOp::AssertClosure:
            {
                LOG(to_string(instructionIndex) + ": AssertClosure");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_closure), args, temps, nullopt);
                break;
            };
        case IrOp::AssertValWrapper: 
            {
                LOG(to_string(instructionIndex) + ": AssertValWrapper");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_valwrapper), args, temps, nullopt);
                break;
            };
        case IrOp::UnboxInteger:
            {
                LOG(to_string(instructionIndex) + ": UnboxInteger");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_unbox_int), args, temps, returnTemp);
                break;
            };
        case IrOp::UnboxBoolean:
            {
                LOG(to_string(instructionIndex) + ": UnboxBoolean");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_unbox_bool), args, temps, returnTemp);
                break;
            };
        case IrOp::NewInteger:
            {
                LOG(to_string(instructionIndex) + ": NewInteger");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_new_integer), args, temps, returnTemp);
                break;
            };
        case IrOp::NewBoolean:
            {
                LOG(to_string(instructionIndex) + ": NewBoolean");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_new_boolean), args, temps, returnTemp);
                break;
            };
        case IrOp::CastString:
            {
                LOG(to_string(instructionIndex) + ": CastString");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_cast_string), args, temps, returnTemp);
                break;
            };
        case IrOp::AddLabel:
            {
                LOG(to_string(instructionIndex) + ": AddLabel");
                string labelStr = to_string(inst->op0.value());
                LOG("adding label " + labelStr);
                assm.bind(x64asm::Label{labelStr});
                break;
            };
        case IrOp::GarbageCollect:
            {
                LOG(to_string(instructionIndex) + ": GarbageCollect");
                vector<x64asm::Imm64> args = {x64asm::Imm64{vmPointer}};
                vector<tempptr_t> temps; 
                callHelper((void *) &(helper_gc), args, temps, nullopt);
                break;
            };
        default:
            throw RuntimeException("Should not get here: invalid ir inst");
    }
    LOG(inst->getInfo());
    int freeRegsEndSize = freeRegs.size();
    assert (freeRegsStartSize == freeRegsEndSize);
    // run maintenance to figure out if regs are avlb 
    updateFreeRegs(inst);

    instructionIndex += 1;
    if (instructionIndex >= func->instructions.size()) {
        finished = true;
    }
}
