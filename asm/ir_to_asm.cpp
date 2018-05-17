#include "ir_to_asm.h"

IrInterpreter::IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer, vector<bool> isLocalRefVec) {
    vmPointer = vmInterpreterPointer;
    func = irFunction;
    isLocalRef = isLocalRefVec;
    // by convention, the first ir function is the main function
    instructionIndex = 0;
    finished = false;
}

/************************
 * CONTROL FLOW
 ***********************/
void IrInterpreter::comparisonSetup(instptr_t inst, TempBoolOp tempBoolOp) {
    tempptr_t left = inst->tempIndices->at(2);
    tempptr_t right = inst->tempIndices->at(1);
    tempptr_t res = inst->tempIndices->at(0);
    // perform a comparison; not stored anywhere
    moveTemp32(left, right, TempOp::CMP);
    // get a scratch reg
    // would be better to get two free regs, but I haven't set up the
    // code to be able to do that yet
    x64asm::R64 reg = getScratchReg();
    if (res->reg) {
        x64asm::Opcode op;
        switch (tempBoolOp) {
            case TempBoolOp::CMOVNLE:
                op = x64asm::CMOVNLE_R64_R64;
                break;
            case TempBoolOp::CMOVNL:
                op = x64asm::CMOVNL_R64_R64;
                break;
        }
        // put 1 in the scratch reg
        assm.mov(reg, x64asm::Imm64{1});
        // put 0 in the return reg
        assm.mov(res->reg.value(), x64asm::Imm64{0});
        // do the operation, leaving val in the return reg
        assm.assemble({op, {res->reg.value(), reg}});
    } else {
        x64asm::Opcode op;
        switch (tempBoolOp) {
            case TempBoolOp::CMOVNLE:
                op = x64asm::CMOVNLE_R64_M64;
                break;
            case TempBoolOp::CMOVNL:
                op = x64asm::CMOVNL_R64_M64;
                break;
        }
        // move 0 into the scratch reg
        assm.mov(reg, x64asm::Imm64{0});
        // move 1 into the return temp
        uint32_t offset = getTempOffset(res);
        assm.mov(
            x64asm::M64{x64asm::rbp, x64asm::Imm32{-offset}},
            x64asm::Imm32{1}
        );
        // perform the comparison, leaving in the reg
        assm.assemble({op, {reg,
            x64asm::M64 {
                x64asm::rbp,
                x64asm::Imm32{-offset}
            }
        }});
        // move from reg into its home in the temp
        moveTemp(res, reg);
    }
    returnScratchReg(reg);
};

/************************
 * SETUP/TEARDOWN
 ***********************/
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
    assm.sub(x64asm::rsp, x64asm::Imm32{spaceToAllocate});

    // put a pointer to the references onto the stack
    // ptr to ref array is the second arg
    uint32_t refArrayOffset = getRefArrayOffset();
    assm.mov(
        x64asm::M64{x64asm::rbp, x64asm::Imm32{-refArrayOffset}},
        x64asm::rsi
    );

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

        // TODO: I would collect the following 3 occurences of this and put it in a single function to prevent bugs like switching installing locals vs refs
        if (isLocalRef.at(i)) {
            installLocalRefVar(localTemp, i);
        } else {
            installLocalVar(localTemp, i);
        }
    }
    // now do rdi, if applicable
    if (rdiTemp >= 0) {
        if (isLocalRef.at(rdiTemp)) {
            installLocalRefVar(func->temps.at(rdiTemp), rdiTemp);
        } else {
            installLocalVar(func->temps.at(rdiTemp), rdiTemp);
        }
    }

    // set all other locals to none
    for (uint64_t i = func->parameter_count_; i < func->local_count_; i++) {
        tempptr_t localTemp = func->temps.at(i);
        if (isLocalRef.at(i)) {
            installLocalRefNone(localTemp);
        } else {
            installLocalNone(localTemp);
        }
    }
}

void IrInterpreter::epilog() {
    // TO USE THIS FUNCTION, FIRST MAKE SURE YOU HAVE CLEARED OUT THE STACK
    // AND PUT THE RETURN VALUE IN THE RIGHT REG

    // increment rsp again to deallocate locals, temps, etc
    assm.add(x64asm::rsp, x64asm::Imm32{spaceToAllocate});

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
    asmFunc.reserve(func->instructions.size() * 1000); // TODO: figure out how to allocate the right amount of memory
    prolog();

    if (func->instructions.size() > 0) {
        // translate to asm
        while (!finished) {
            executeStep();
        }
    }

    assm.mov(x64asm::R64{x64asm::rax}, x64asm::Imm64{vmPointer->NONE});
    epilog();

    // finish compiling
    assm.finish();
    LOG("done compiling asm");
    return asmFunc;
}

/************************
 * MAIN EXECUTION
 ***********************/
void IrInterpreter::executeStep() {
    instptr_t inst = func->instructions.at(instructionIndex);
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
                    assm.mov(reg, x64asm::Imm64{(uint64_t)c});
                    // get the offset of the temp on the stack
                    moveTemp(t, reg);
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
                    moveTemp(t, reg);
                    // give back the scratch reg
                    returnScratchReg(reg);
                }
                break;
            }
       case IrOp::LoadGlobal:
            {
                LOG(to_string(instructionIndex) + ": LoadGlobal");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {vmPointer, name};
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
                vector<x64asm::Imm64> args = {vmPointer, name};
                vector<tempptr_t> temps = {inst->tempIndices->at(0)};
                callHelper((void *) &(helper_store_global), args, temps, nullopt);
                break;
            }
        case IrOp::StoreLocalRef:
            {
                LOG(to_string(instructionIndex) + ": StoreLocalRef");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1), // value to store
                    inst->tempIndices->at(0) // local valwrapper
                };
                tempptr_t returnTemp = inst->tempIndices->at(1);
                callHelper((void *) &(helper_store_local_ref), {}, temps, returnTemp);
                break;
            }
       case IrOp::PushFreeRef:
            {
                LOG(to_string(instructionIndex) + ": PushFreeRef");
                // the ValWrapper is already sitting in the refs array;
                // just move it into the temp

                // get address of ref array and put it in a scratch reg
                uint32_t refArrayOffset = getRefArrayOffset();
                x64asm::R64 reg = getScratchReg();
                assm.mov(
                    reg,
                    x64asm::M64{x64asm::rbp, x64asm::Imm32{-refArrayOffset}}
                );
                // increment reg to index into ref array
                uint32_t offset = 8*inst->op0.value();
                assm.add(reg, x64asm::Imm32{offset});
                // one deref gets us to the val wrapper
                assm.mov(reg, x64asm::M64{reg});
                // store this in a temp
                moveTemp(inst->tempIndices->at(0), reg);
                returnScratchReg(reg);
                break;
            }
        case IrOp::LoadReference:
            {
                LOG(to_string(instructionIndex) + ": LoadReference");
                // dereferences to get to the object itself
                vector<tempptr_t> temps = {inst->tempIndices->at(1)};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_unbox_valwrapper), {}, temps, returnTemp);
                break;
            }
        case IrOp::AllocRecord:
            {
                LOG(to_string(instructionIndex) + ": AllocRecord");
                vector<x64asm::Imm64> args = {vmPointer};
                vector<tempptr_t> temps;
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_new_record), args, temps, returnTemp);
                break;
            };
        case IrOp::FieldLoad:
            {
                LOG(to_string(instructionIndex) + ": FieldLoad");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {vmPointer, name};
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1),
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_get_record_field), args, temps, returnTemp);
                break;
            };
        case IrOp::FieldStore:
            {
                LOG(to_string(instructionIndex) + ": FieldStore");
                string* name = &inst->name0.value();
                vector<x64asm::Imm64> args = {vmPointer, name};
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
                LOG(to_string(instructionIndex) + ": IndexLoad");
                vector<x64asm::Imm64> args = {vmPointer};
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
                LOG(to_string(instructionIndex) + ": IndexStore");
                vector<x64asm::Imm64> args = {vmPointer};
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
                uint32_t numRefs = inst->op0.value();
                // the rest of the args are basically the temps minus temp0
                // create an "array" by pushing these to the stack
                tempptr_t refTemp;
                // DO NOT USE THIS BEFORE SWITCHING THE REG VALUE
                x64asm::R64 reg = getScratchReg();
                for (int i = 0; i < numRefs; ++i) {
                    // push in reverse order, so first ref is lowest
                    refTemp = inst->tempIndices->at(2 + numRefs - i - 1);
                    if (refTemp->reg) {
                        Push(refTemp->reg.value());
                    } else {
                        // get a scratch reg to use as a trampoline 
                        // to the stack
                        moveTemp(reg, refTemp);
                        Push(reg);
                    }
                }

                vector<x64asm::Imm64> args = {
                    vmPointer, // vm
                    numRefs,   // num refs
                }; 
                // this a little hacky, but put the refs in the return temp 
                tempptr_t returnTemp = inst->tempIndices->at(0);
                moveTemp(returnTemp, x64asm::rsp);
                // the func is in a temp
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1), // function
                    returnTemp // refs
                };
                callHelper((void *) &(helper_alloc_closure), args, temps, returnTemp);
                // clear the stack by incrementing rsp 
                //assm.add(x64asm::rsp, x64asm::Imm32{8*numRefs});
                for (int i = 0; i < numRefs; i++) {
                    Pop();
                }
                // give back the scratch reg
                returnScratchReg(reg);
                break;
            };
        case IrOp::Call:
            {
                // we push all the MITScript function arguments to the stack,
                // then pass %rsp (which points to the first element of that
                // array) as an argument to helper_call
                LOG(to_string(instructionIndex) + ": Call");
                uint32_t numArgs = inst->op0.value();
                // push all the MITScript function arguments to the stack
                // to make a contiguous array in memory
                tempptr_t argTemp;
                // DO NOT USE THIS W/O REASSIGNING REG USING GETSCRATCHREG
                x64asm::R64 reg = getScratchReg();
                for (int i = 0; i < numArgs; ++i) {
                    // push in reverse order, so first arg is lowest
                    argTemp = inst->tempIndices->at(2 + numArgs - i - 1);
                    if (argTemp->reg) {
                        Push(argTemp->reg.value());
                    } else {
                        uint32_t offset = getTempOffset(argTemp);
                        assm.mov(
                            reg,
                            x64asm::M64{x64asm::rbp, x64asm::Imm32{-offset}}
                        );
                        Push(reg);
                    }
                }

                // helper_call takes args: vm pointer, numArgs,
                // closure, array of args
                vector<x64asm::Imm64> args = {vmPointer, numArgs};
                // jank, but put the args in the return temp
                tempptr_t returnTemp = inst->tempIndices->at(0);
                moveTemp(returnTemp, x64asm::rsp);
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1), // closure
                    returnTemp // args
                };
                callHelper((void *) &(helper_call), args, temps, returnTemp);

                // clear the stack by incrementing
                // assm.add(x64asm::rsp, x64asm::Imm32{8*numArgs});
                for (int i = 0; i < numArgs; i++) {
                    Pop();
                }
                // now restore the scratch reg
                returnScratchReg(reg);

                break;
            };
        case IrOp::Return:
            {
                // TODO: hmm, are we sure we don't want to set finished to true
                // and break out of executing steps?
                LOG(to_string(instructionIndex) + ": Return");
                // put temp0 in rax
                moveTemp(x64asm::rax, inst->tempIndices->at(0));
                epilog();
                break;
            };
        case IrOp::Add:
            {
                LOG(to_string(instructionIndex) + ": Add");
                // call a helper to do add
                vector<x64asm::Imm64> args = {vmPointer};
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
                auto left = inst->tempIndices->at(2);
                auto right = inst->tempIndices->at(1);
                auto res = inst->tempIndices->at(0);
                // mov left into temp0
                moveTemp(res, left);
                moveTemp(res, right, TempOp::SUB);
                break;
            };
        case IrOp::Mul:
            {
                LOG(to_string(instructionIndex) + ": Mul");
                auto left = inst->tempIndices->at(2);
                auto right = inst->tempIndices->at(1);
                auto res = inst->tempIndices->at(0);

                // mul can only do mem -> reg
                if (res->reg) {
                    moveTemp(res, left);
                    moveTemp(res, right, TempOp::MUL);
                } else {
                    // get a scratch reg
                    x64asm::R64 reg = getScratchReg();
                    // note: these calls to move do NOT incur a 
                    // scratch reg because they already have a red
                    // on one side. Still only using one scratch reg
                    moveTemp(reg, left);
                    moveTemp(reg, right, TempOp::MUL);
                    moveTemp(res, reg);
                    returnScratchReg(reg);
                }
                break;
            };
        case IrOp::Div:
            {
                LOG(to_string(instructionIndex) + ": Div");
                // TODO: make this less inefficient
                // save rax, rdx, and get an extra scratch register
                Push(x64asm::rax);
                Push(x64asm::rdx);
                x64asm::R64 divisor = getScratchReg();
                moveTemp(divisor, inst->tempIndices->at(1));
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1),
                };
                callHelper((void *) &(helper_assert_nonzero), {}, temps, nullopt);
                x64asm::R64 numerator_secondhalf = x64asm::rax;
                moveTemp(numerator_secondhalf, inst->tempIndices->at(2));
                assm.cdq(); // weird asm thing to sign-extend eax into edx
                // perform the div; result stored in rax
                assm.idiv(getRegBottomHalf(divisor));
                // put the value back in the temp
                moveTemp(inst->tempIndices->at(0), x64asm::rax);
                returnScratchReg(divisor);
                // restore rax and rdx
				if (inst->tempIndices->at(0)->reg.value() == x64asm::rdx) {
					Pop();
				} else {
                	Pop(x64asm::rdx);
				}
				if (inst->tempIndices->at(0)->reg.value() == x64asm::rax) {
					Pop();
				} else {
                	Pop(x64asm::rax);
				}
                break;
            };
        case IrOp::Neg: {
                LOG(to_string(instructionIndex) + ": Neg");
                //auto operand = x64asm::rdi;
                tempptr_t temp0 = inst->tempIndices->at(0);
                // move into the return var
                moveTemp(temp0, inst->tempIndices->at(1));
                // negate temp0
                if (temp0->reg) {
                    assm.neg(temp0->reg.value());
                } else {
                    uint32_t offset = getTempOffset(temp0);
                    assm.neg(
                        x64asm::M64{x64asm::rbp, x64asm::Imm32{-offset}}
                    );
                }
                break;
            };
        case IrOp::Gt:
            {
                LOG(to_string(instructionIndex) + ": Gt");
                comparisonSetup(inst, TempBoolOp::CMOVNLE);
                break;
            };
        case IrOp::Geq:
            {
                LOG(to_string(instructionIndex) + ": Geq");
                // use a conditional move to put the bool in the right place
                // right(1) gets moved into left(0) if left was greater
                comparisonSetup(inst, TempBoolOp::CMOVNL);
                break;
            };
        case IrOp::Eq:
            {
                LOG(to_string(instructionIndex) + ": Eq");
                // call a helper to do equality
                vector<x64asm::Imm64> args = {vmPointer};
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
                auto left = inst->tempIndices->at(2);
                auto right = inst->tempIndices->at(1);
                auto res = inst->tempIndices->at(0);

                // move left into res
                moveTemp(res, left);
                // implement and as a form of move
                moveTemp(res, right, TempOp::AND);
               break;
            };
        case IrOp::Or:
            {
                LOG(to_string(instructionIndex) + ": Or");
                auto left = inst->tempIndices->at(2);
                auto right = inst->tempIndices->at(1);
                auto res = inst->tempIndices->at(0);

                // move left into res
                moveTemp(res, left);
                // implement and as a form of move
                moveTemp(res, right, TempOp::OR);
                break;
            };
        case IrOp::Not:
            {
                LOG(to_string(instructionIndex) + ": Not");
                tempptr_t returnTemp = inst->tempIndices->at(0);
                tempptr_t operand = inst->tempIndices->at(1);
                moveTemp(returnTemp, operand);
                if (returnTemp->reg) {
                    assm.xor_(
                        returnTemp->reg.value(),
                        x64asm::Imm32{1}
                    );
                } else {
                    uint32_t offset = getTempOffset(returnTemp);
                    assm.xor_(
                        x64asm::M64{x64asm::rbp, x64asm::Imm32{-offset}},
                        x64asm::Imm32{1}
                    );
                }
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
                x64asm::R64 left = getScratchReg();
                moveTemp(left, inst->tempIndices->at(0));
                assm.cmp(getRegBottomHalf(left), x64asm::Imm32{1});
				returnScratchReg(left);
                LOG("jumping if to label " + labelStr);
                assm.je_1(x64asm::Label{labelStr});
                break;
            };
       case IrOp::AssertInteger:
            {
                LOG(to_string(instructionIndex) + ": AssertInteger");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_int), {}, temps, nullopt);
                break;
            };
        case IrOp::AssertBoolean:
            {
                LOG(to_string(instructionIndex) + ": AssertBool");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_bool), {}, temps, nullopt);
                break;
            };
        case IrOp::AssertString:
            {
                LOG(to_string(instructionIndex) + ": AssertString");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_str), {}, temps, nullopt);
                break;
            };
        case IrOp::AssertRecord:
            {
                LOG(to_string(instructionIndex) + ": AssertRecord");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_record), {}, temps, nullopt);
                break;
            };
        case IrOp::AssertFunction:
            {
                LOG(to_string(instructionIndex) + ": AssertFunction");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_func), {}, temps, nullopt);
                break;
            };
        case IrOp::AssertClosure:
            {
                LOG(to_string(instructionIndex) + ": AssertClosure");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_closure), {}, temps, nullopt);
                break;
            };
        case IrOp::AssertValWrapper:
            {
                LOG(to_string(instructionIndex) + ": AssertValWrapper");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_valwrapper), {}, temps, nullopt);
                break;
            };
        case IrOp::UnboxInteger:
            {
                LOG(to_string(instructionIndex) + ": UnboxInteger");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_unbox_int), {}, temps, returnTemp);
                break;
            };
        case IrOp::UnboxBoolean:
            {
                LOG(to_string(instructionIndex) + ": UnboxBoolean");
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_unbox_bool), {}, temps, returnTemp);
                break;
            };
        case IrOp::NewInteger:
            {
                LOG(to_string(instructionIndex) + ": NewInteger");
                vector<x64asm::Imm64> args = {vmPointer};
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
                vector<x64asm::Imm64> args = {vmPointer};
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
                vector<x64asm::Imm64> args = {vmPointer};
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
                // TODO: this doesn't work
                LOG(to_string(instructionIndex) + ": GarbageCollect");
                x64asm::R64 reg = getScratchReg();
                for (auto it = activeTemps.begin(); it != activeTemps.end(); it++) {
                    LOG("pushing temp " << *it);
                    tempptr_t t = func->temps.at(*it);
                    // push
                    moveTemp(reg, t);
                    Push(reg);
                }

                tempptr_t argTemp = inst->tempIndices->at(0);
                moveTemp(argTemp, x64asm::rsp);

                vector<x64asm::Imm64> args = {vmPointer, activeTemps.size()};
                vector<tempptr_t> temps = {
                    argTemp
                };
                callHelper((void *) &(helper_gc), args, temps, nullopt);

                for (int i = 0; i < activeTemps.size(); i++) {
                    Pop();
                }

                returnScratchReg(reg);
                break;
            };
        default:
            throw RuntimeException("Should not get here: invalid ir inst");
    }
    LOG(inst->getInfo());
    assert (regPopCount == 0); // make sure we're pushing equally to popping
    assert (popCount == 0);
    updateActiveTemps(inst, instructionIndex);

    instructionIndex += 1;
    if (instructionIndex >= func->instructions.size()) {
        finished = true;
    }
}
