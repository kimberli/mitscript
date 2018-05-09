#include "ir_to_asm.h"

const x64asm::R64 IrInterpreter::argRegs[] = {
    x64asm::rdi,
    x64asm::rsi,
    x64asm::rdx,
    x64asm::rcx,
    x64asm::r8,
    x64asm::r9
};

const x64asm::R64 IrInterpreter::callerSavedRegs[] = {
    x64asm::rax,
    x64asm::rcx,
    x64asm::rdx,
    x64asm::rsi,
    x64asm::rdi,
    x64asm::r8,
    x64asm::r9,
    x64asm::r10,
    x64asm::r11
};

const x64asm::R64 IrInterpreter::calleeSavedRegs[] = {
    // rbp and rsp are handled a little differently, so will not consider
    // callee-saved
    //x64asm::rbp,
    //x64asm::rsp,
    x64asm::rbx,
    x64asm::r12,
    x64asm::r13,
    x64asm::r14,
    x64asm::r15
};

IrInterpreter::IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer) {
    vmPointer = vmInterpreterPointer;
    func = irFunction;
    // by convention, the first ir function is the main function
    instructionIndex = 0;
    finished = false;
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
// allocate space for locals, refs, and temps on the stack // by decrementing rsp 
// and note that we are only storing on ref pointer by pushing the pointer to
// the array
    spaceToAllocate = 8*(func->local_count_ + 1 + func->temp_count_); 
    assm.assemble({x64asm::SUB_R64_IMM32, {x64asm::rsp, x64asm::Imm32{spaceToAllocate}}});

    // implement putting args in the correct locals
    for (uint64_t i = 0; i < func->parameter_count_; i++) {
        // rd1 stores arg0 which is the list of arguments to the MS func
        // put the pointer to the start of the args in a reg
        
        // Calculate mem address for the local
        getRbpOffset(getLocalOffset(i));
        // syntax for scaling: base, offset, scale 
        // move offset into r11
        assm.mov(x64asm::r11, x64asm::Imm64{i});
        // move val of arg into r11
        assm.assemble({x64asm::MOV_R64_M64, {
                x64asm::r11, 
                x64asm::M64{x64asm::rdi, x64asm::r11, x64asm::Scale::TIMES_8}
        }});
        // move r11 into address stored in r10
        assm.mov(x64asm::M64{x64asm::r10}, x64asm::r11);
    }

    // TODO: set all other args to None

    // put a pointer to the references onto the stack
    // ptr to ref array is the second arg
    getRbpOffset(getRefArrayOffset());
    assm.mov(x64asm::M64{x64asm::r10}, x64asm::rsi);
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
    prolog();
	asmFunc.reserve(func->instructions.size() * 100); // TODO: figure out how to allocate the right amount of memory

    // translate to asm
    while (!finished) {
        executeStep();
    }

    // TODO: move None into rax. 
    epilog();

    // finish compiling
    assm.finish();
    LOG("done compiling asm");
    return asmFunc;
}

void IrInterpreter::callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, opttemp_t returnTemp) {
    // STEP 1: save caller-saved registers to stack
    for (int i = 0; i < numCallerSaved; ++i) {
        assm.push(callerSavedRegs[i]);
    }

    // STEP 2: push first 6 arguments to registers, rest to stack
    // args + temps are put in that order as arguments for the function to call
    // on the stack
    // args should contain pointers to values or immediate numbers
    // temps are pointers to Temps which store stack offsets
    int numArgs = args.size() + temps.size();
    int argIndex = 0;
    LOG("  calling helper w/ " + to_string(numArgs) + " total args");
    while (argIndex < numArgs) {
        while (argIndex < args.size()) {
            if (argIndex < numArgRegs) {
                // put args 1 - 6 into regs
                assm.mov(argRegs[argIndex], args[argIndex]);
            } else {
                // push args 7 - n on the stack; n gets pushed first
                assm.mov(x64asm::r10, args[args.size() - argIndex - 1]);
                assm.push(x64asm::r10);
            }
            argIndex++;
        }
        while (argIndex < numArgs) {
            if (argIndex < numArgRegs) {
                // put args 1 - 6 into regs
                tempptr_t tempToPush = temps[argIndex - args.size()];
                loadTemp(x64asm::rax, tempToPush);
                assm.mov(argRegs[argIndex], x64asm::rax);
            } else {
                int tempIndex = argIndex - args.size();
                // push args 7 - n on the stack; n gets pushed first
                tempptr_t tempToPush = temps[temps.size() - tempIndex - 1];
                loadTemp(x64asm::rax, tempToPush);
                assm.push(x64asm::rax);
            }
            argIndex++;
        }
    }
    assm.mov(x64asm::r10, x64asm::Imm64{fn}); 
    
    assm.call(x64asm::r10);

    // STEP 3: pop arguments from stack
    // TODO: do this by just moving rsp
    if (numArgs > numArgRegs) {
        for (int i = 0; i < numArgs - numArgRegs; ++i) {
            assm.pop(x64asm::r10);
        }
    }

    if (returnTemp) {
        storeTemp(x64asm::rax, returnTemp.value());
    }

    // STEP 4: restore caller-saved registers from stack
    for (int i = 1; i <= numCallerSaved; ++i) {
        assm.pop(callerSavedRegs[numCallerSaved - i]);
    }
}

uint32_t IrInterpreter::getLocalOffset(uint32_t localIndex) {
    return 8*(1 + numCalleeSaved + localIndex);
}

uint32_t IrInterpreter::getRefArrayOffset() {
    return 8*(1 + numCalleeSaved + func->local_count_);
}

uint32_t IrInterpreter::getTempOffset(tempptr_t temp) {
    return 8*(1 + numCalleeSaved + func->local_count_ + 1 + temp->stackOffset);
}

void IrInterpreter::getRbpOffset(uint32_t offset) {
    // TODO: the following would be much more efficient, but how to use negative scales? 
    //assm.mov(x64asm::r10, x64asm::M64{x64asm::Scale::TIMES_8, x64asm::rbp});
    
    // put rbp in a reg
    assm.mov(x64asm::r10, x64asm::rbp);
    // subtract the offset from rbp 
    assm.assemble({x64asm::SUB_R64_IMM32, {x64asm::r10, x64asm::Imm32{offset}}});
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
void IrInterpreter::loadTemp(x64asm::R64 reg, tempptr_t temp) {
    getRbpOffset(getTempOffset(temp)); // location in r10
    // put the thing from that mem addres into the reg
    assm.mov(reg, x64asm::M64{x64asm::r10});
}

void IrInterpreter::executeStep() {

    instptr_t inst = func->instructions.at(instructionIndex);
    switch(inst->op) {
        case IrOp::LoadConst:
            {
                LOG(to_string(instructionIndex) + ": LoadConst");
                int constIndex = inst->op0.value();
                Constant* c = func->constants_.at(constIndex);
                // load a constant into a register
                assm.mov(x64asm::rdi, x64asm::Imm64{(uint64_t)c});
                // move from the register into a temp on the stack
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::LoadFunc:
            {
                LOG(to_string(instructionIndex) + ": LoadFunc");
                int funcIndex = inst->op0.value();
                // load the func pointer into a register
                assm.mov(x64asm::rdi, x64asm::Imm64{func->functions_.at(funcIndex)});
                // move from the register into a temp on the stack
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::LoadLocal:
            {
                LOG(to_string(instructionIndex) + ": LoadLocal");
                int64_t localIndex = inst->op0.value();
                getRbpOffset(getLocalOffset(localIndex)); // puts the address of the local in r10
                assm.mov(x64asm::rdi, x64asm::r10); // r10 will be used later in storeTemp
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10}); // loads the actual value
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::LoadGlobal:
            {
                LOG(to_string(instructionIndex) + ": LoadGlobal");
                string* name = new string(inst->name0.value());  // TODO: fix memory leak?
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
                // first put the temp val in a reg
                loadTemp(x64asm::rdi, inst->tempIndices->at(0));
                // put find out where the local is located
                int64_t localIndex = inst->op0.value();
                getRbpOffset(getLocalOffset(localIndex)); // address of local in r10
                // move the val from the reg to memory
                assm.mov(x64asm::M64{x64asm::r10}, x64asm::rdi);
                break;
            }
       case IrOp::StoreGlobal:
            {
                LOG(to_string(instructionIndex) + ": StoreGlobal");
                string* name = new string(inst->name0.value());  // TODO: fix memory leak?
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name},
                };
                vector<tempptr_t> temps = {inst->tempIndices->at(0)};
                callHelper((void *) &(helper_store_global), args, temps, opttemp_t());
                break;
            }
        case IrOp::PushReference: 
            {
                LOG(to_string(instructionIndex) + ": PushReference");
                vector<x64asm::Imm64> args = {x64asm::Imm64{vmPointer}};

                int64_t localIndex = inst->op0.value();
                getRbpOffset(getLocalOffset(localIndex)); // puts the address of the local in r10
                assm.mov(x64asm::rdi, x64asm::r10); // r10 will be used later in storeTemp
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10}); // loads the actual value
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
     
                vector<tempptr_t> temps = {inst->tempIndices->at(0)};
                callHelper((void *) &(helper_new_valwrapper), args, temps, opttemp_t(inst->tempIndices->at(0)));
                break;
            }
        case IrOp::LoadReference:
            {
                LOG(to_string(instructionIndex) + ": LoadReference");
                // get the mem address of the ref array; put in rdi
                getRbpOffset(getRefArrayOffset());
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10});
                // increment rdi to index into ref array
                uint32_t offset = 8*inst->op0.value();
                assm.assemble({x64asm::ADD_R64_IMM32, {x64asm::rdi, x64asm::Imm32{offset}}});
                // now rdi holds the mem of the pointer; we gotta deref that
                // dereferences to get the ValWrapper
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10});
                // dereference to get the Value*
                // TODO check if this works
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10});
                // store the Value* in a temp 
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::AllocRecord:
            {
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
                LOG(to_string(instructionIndex) + ": FieldLoad");
                string* name = new string(inst->name0.value());  // TODO: fix memory leak?
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
                LOG(to_string(instructionIndex) + ": FieldStore");
                string* name = new string(inst->name0.value());  // TODO: fix memory leak?
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
                LOG(to_string(instructionIndex) + ": IndexStore");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(2),
                    inst->tempIndices->at(0),
                    inst->tempIndices->at(1),
				};
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_set_record_index), args, temps, returnTemp);
                break;
            };
        case IrOp::AllocClosure:
            {
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
                    getRbpOffset(getTempOffset(refTemp)); // leaves correct address into r10
                    assm.mov(x64asm::r10, x64asm::M64{x64asm::r10});
                    assm.push(x64asm::r10);
                }

                // store the array pointer in a temp so callHelper can use it
                storeTemp(x64asm::rsp, inst->tempIndices->at(0));
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1), // function
                    inst->tempIndices->at(0), // array of refs
                };

                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_alloc_closure), immArgs, temps, returnTemp);
                // clear the stack
                for (int i = 0; i < numRefs; i++) {
                    assm.pop(x64asm::r10);
                };
                break;
            };
        case IrOp::Call:
            {
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
                    // TODO: must use getTempOffset here! 
                    argTemp = inst->tempIndices->at(2 + numArgs - i - 1);
                    getRbpOffset(getTempOffset(argTemp)); // leaves correct address into r10
                    assm.mov(x64asm::r10, x64asm::M64{x64asm::r10});
                    assm.push(x64asm::r10);
                }

                // putting rsp in temp0 for now because I don't want to have to
                // write a new callHelper
                vector<x64asm::Imm64> immArgs = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{numArgs}
                };

                // TODO: we might need to inc rsp here cause I think rsp
                // points to the last empty space
                // put rsp into a temp to pas easily
                storeTemp(x64asm::rsp, inst->tempIndices->at(0));
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0), // args 
                    inst->tempIndices->at(1) // closure
                };
                tempptr_t returnTemp = inst->tempIndices->at(0);
                callHelper((void *) &(helper_call), immArgs, temps, returnTemp);

                // clear the stack
                for (int i = 0; i < numArgs; i++) {
                    assm.pop(x64asm::r10);
                };
                
                break;
            };
        case IrOp::Return:
            {
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
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
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
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
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
                x64asm::R64 numerator_secondhalf = x64asm::rax;
                loadTemp(numerator_secondhalf, inst->tempIndices->at(2));
    			assm.cdq(); // weird asm thing to sign-extend rax into rdx
                x64asm::R64 divisor = x64asm::rbx;
				loadTemp(divisor, inst->tempIndices->at(1));
                // perform the div; result stored in rax
                assm.idiv(x64asm::ebx);
                // put the value back in the temp
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Neg: {
                // TODO: maybe also broken? untested
                LOG(to_string(instructionIndex) + ": Neg");
                x64asm::R64 operand = x64asm::rdi;
                loadTemp(operand, inst->tempIndices->at(1));
                assm.neg(operand);
                storeTemp(operand, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Gt:
            {
                // TODO: maybe also broken? untested
                LOG(to_string(instructionIndex) + ": Gt");
                // use a conditional move to put the bool in the right place
                // right(1) gets moved into left(0) if left was greater
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                comparisonSetup(left, right, inst);
                assm.cmovg(left, right);
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Geq:
            {
                // TODO: maybe also broken? untested
                LOG(to_string(instructionIndex) + ": Geq");
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                // use a conditional move to put the bool in the right place
                // right(1) gets moved into left(0) if left was greater
                comparisonSetup(left, right, inst);
                assm.cmovge(left, right);
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Eq:
            {
                // TODO: maybe also broken? untested
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
                // TODO: maybe also broken? untested
                LOG(to_string(instructionIndex) + ": And");
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                loadTemp(left, inst->tempIndices->at(2));
                // load right temp into a reg
                loadTemp(right, inst->tempIndices->at(1));
                // perform the sub; result stored in left
                //assm.AND(left, right);
                assm.assemble({x64asm::AND_R64_R64, {left, right}});
                // put the value back in the temp
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Or:
            {
                // TODO: maybe also broken? untested
                LOG(to_string(instructionIndex) + ": Or");
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                loadTemp(left, inst->tempIndices->at(2));
                // load right temp into a reg
                loadTemp(right, inst->tempIndices->at(1));
                // perform the sub; result stored in left
                assm.assemble({x64asm::OR_R64_R64, {left, right}});
                //assm.or(left, right);
                // put the value back in the temp
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Not:
            {
                // TODO: maybe also broken? untested
                LOG(to_string(instructionIndex) + ": Not");
                x64asm::R64 operand = x64asm::rdi;
				x64asm::R64 one = x64asm::rsi;
                loadTemp(operand, inst->tempIndices->at(1));
                assm.mov(one, x64asm::Imm64{1});
				assm.xor_(operand,one);
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
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                loadTemp(left, inst->tempIndices->at(0));
                assm.mov(right, x64asm::Imm64{1});
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
                callHelper((void *) &(helper_assert_int), args, temps, opttemp_t());
                break;
            };
        case IrOp::AssertBoolean:
            {
                LOG(to_string(instructionIndex) + ": AssertBool");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_bool), args, temps, opttemp_t());
                break;
            };
        case IrOp::AssertString:
            {
                LOG(to_string(instructionIndex) + ": AssertString");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_str), args, temps, opttemp_t());
                break;
            };
        case IrOp::AssertRecord:
            {
                LOG(to_string(instructionIndex) + ": AssertRecord");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_record), args, temps, opttemp_t());
                break;
            };
        case IrOp::AssertFunction:
            {
                LOG(to_string(instructionIndex) + ": AssertFunction");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_func), args, temps, opttemp_t());
                break;
            };
        case IrOp::AssertClosure:
            {
                LOG(to_string(instructionIndex) + ": AssertClosure");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_closure), args, temps, opttemp_t());
                break;
            };
        case IrOp::AssertValWrapper: 
            {
                LOG(to_string(instructionIndex) + ": AssertValWrapper");
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                };
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_valwrapper), args, temps, opttemp_t());
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
                callHelper((void *) &(helper_gc), args, temps, opttemp_t());
                break;
            };
        default:
            throw RuntimeException("Should not get here: invalid ir inst");
    }
    LOG(inst->getInfo());
    instructionIndex += 1;
    if (instructionIndex >= func->instructions.size()) {
        finished = true;
    }
}

void IrInterpreter::comparisonSetup(x64asm::R64 left, x64asm::R64 right, instptr_t inst) {
    // load right temp into a reg
    loadTemp(right, inst->tempIndices->at(1));
    // load left temp into a reg
    loadTemp(left, inst->tempIndices->at(2));
    // perform the sub; result stored in left
    assm.cmp(left, right); // this sets flags
    // load 0 and 1 into two diff regs
    assm.mov(right, x64asm::Imm64{1});
    assm.mov(left, x64asm::Imm64{0});
};
