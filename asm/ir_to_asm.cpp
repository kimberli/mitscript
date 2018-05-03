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

IrInterpreter::IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer) {
    vmPointer = vmInterpreterPointer;
    func = irFunction;
    // by convention, the first ir function is the main function
    instructionIndex = 0;
    finished = false;
}

x64asm::Function IrInterpreter::run() {
    // start the assembler on the function
    assm.start(asmFunc);

    // TODO: do prologue

    // translate to asm
    while (!finished) {
        executeStep();
    }

    // TODO: do epilogue

    // finish compiling
    assm.ret();
    assm.finish();
    LOG("done compiling asm");
    // return the asmFunc
    return asmFunc;
    //asmFunc.call<Value*>();
}

void IrInterpreter::callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps) {
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
                getRbpOffset(temps[argIndex - args.size()]->stackOffset);
                assm.mov(argRegs[argIndex], x64asm::M64{x64asm::r10});
            } else {
                int tempIndex = argIndex - args.size();
                // push args 7 - n on the stack; n gets pushed first
                getRbpOffset(temps[temps.size() - tempIndex - 1]->stackOffset);
                assm.push(x64asm::r10);
            }
            argIndex++;
        }
    }
    assm.mov(x64asm::r10, x64asm::Imm64{fn}); assm.call(x64asm::r10);

    // STEP 3: pop arguments from stack
    if (numArgs > numArgRegs) {
        for (int i = 0; i < numArgs - numArgRegs; ++i) {
            assm.pop(x64asm::r10);
        }
    }

    // STEP 4: restore caller-saved registers from stack
    for (int i = 1; i <= numCallerSaved; ++i) {
        assm.pop(callerSavedRegs[numCallerSaved - i]);
    }
}

void IrInterpreter::getRbpOffset(uint64_t offset) {
    // leaves result in r10
    offset = offset + 1;
    assm.mov(x64asm::r10, x64asm::rbp);
    assm.mov(x64asm::r11, x64asm::Imm64{8*offset}); // move 8*offset into another reg
    assm.sub(x64asm::r10, x64asm::r11); // sub r11 from r10. now the correct mem is in r10
}

// storeTemp puts a reg value into a temp
// TODO fix how temps are assigned; these should be pushed onto the stack.
void IrInterpreter::storeTemp(x64asm::R64 reg, tempptr_t temp) {
    // right now, put everything on the stack
    // assign the temp an offset (from rbp)
    // Assume we are not saving any callee save.
    temp->stackOffset = func->parameter_count_ +  stackSize;
    // in assembly, calculate where this var is located
    getRbpOffset(temp->stackOffset); // leaves correct address into r10
    // Move the val in reg into the mem address stored in r10
    assm.mov(x64asm::M64{x64asm::r10}, reg);
    stackSize++; //inc stack size for next temp
}

// loadTemp takes a temp value and puts it in a register
void IrInterpreter::loadTemp(x64asm::R64 reg, tempptr_t temp) {
    // figure out where the temp is stored
    getRbpOffset(temp->stackOffset); // location in r10
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
                int64_t offset = inst->op0.value();
                getRbpOffset(offset); // puts the address of the local in r10
                assm.mov(x64asm::rdi, x64asm::r10); // r10 will be used later in storeTemp
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10}); // hopefully this loads the actual val
                storeTemp(x64asm::rdi, inst->tempIndices->at(0));
                break;
            }
        case IrOp::LoadGlobal:
            {
                LOG(to_string(instructionIndex) + ": LoadGlobal");
                // mov DEST, SRC
                string* name = new string(inst->name0.value());  // TODO: fix memory leak?
                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name},
                };
                vector<tempptr_t> temps;
                callHelper((void *) &(helper_load_global), args, temps);

                // put the return val in the temp
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            }
        case IrOp::StoreLocal:
            {
                LOG(to_string(instructionIndex) + ": StoreLocal");
                // first put the temp val in a reg
                loadTemp(x64asm::rdi, inst->tempIndices->at(0));
                // put find out where the constant is located
                int64_t offset = inst->op0.value();
                getRbpOffset(offset); // address of local in r10
                // move the val from the reg to memory
                assm.mov(x64asm::M64{x64asm::r10}, x64asm::rdi);
                break;
            }
       case IrOp::StoreGlobal:
            {
                LOG(to_string(instructionIndex) + ": StoreGlobal");
                string* name = new string(inst->name0.value());

                vector<x64asm::Imm64> args = {
                    x64asm::Imm64{vmPointer},
                    x64asm::Imm64{name},
                };
                vector<tempptr_t> temps = {inst->tempIndices->at(0)};
                callHelper((void *) &(helper_store_global), args, temps);
                break;
            }
        case IrOp::AllocRecord:
            {
                LOG(to_string(instructionIndex) + ": AllocRecord");
                vector<x64asm::Imm64> args = {};
                vector<tempptr_t> temps = {};
                callHelper((void *) &(helper_new_record), args, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            };
        case IrOp::FieldLoad:
            {
                LOG(to_string(instructionIndex) + ": FieldLoad");
                break;
            };
        case IrOp::FieldStore:
            {
                LOG(to_string(instructionIndex) + ": FieldStore");
                break;
            };
        case IrOp::IndexLoad:
            {
                LOG(to_string(instructionIndex) + ": IndexLoad");
                break;
            };
        case IrOp::IndexStore:
            {
                LOG(to_string(instructionIndex) + ": IndexStore");
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
                vector<tempptr_t> temps(inst->tempIndices->begin() + 1, inst->tempIndices->end());
                callHelper((void *) &(helper_alloc_closure), immArgs, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Call:
            {
                LOG(to_string(instructionIndex) + ": Call");
                break;
            };
        case IrOp::Return:
            {
                LOG(to_string(instructionIndex) + ": Return");
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
                callHelper((void *) &(helper_add), args, temps);
                // put the return val in the temp
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
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
//                x64asm::R32 numerator_firsthalf = x64asm::rdx;
                x64asm::R64 numerator_secondhalf = x64asm::rax;
                loadTemp(numerator_secondhalf, inst->tempIndices->at(1));
    			assm.cdq(); // weird asm thing to sign-extend rax into rdx
                x64asm::R64 divisor = x64asm::rbx;
				loadTemp(divisor, inst->tempIndices->at(2));
                // perform the div; result stored in rax
                assm.idiv(x64asm::ebx);
//                assm.assemble({IDIV_R64, {}});
                // put the value back in the temp
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            }; case IrOp::Neg: {
                LOG(to_string(instructionIndex) + ": Neg");
                x64asm::R64 operand = x64asm::rdi;
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
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                comparisonSetup(left, right, inst);
                assm.cmovg(left, right);
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Geq :
            {
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
                LOG(to_string(instructionIndex) + ": Eq");
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                comparisonSetup(left, right, inst);
                assm.cmove(left, right);
                storeTemp(left, inst->tempIndices->at(0));
                break;
            };
        case IrOp::And:
            {
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
                LOG(to_string(instructionIndex) + ": Not");
                x64asm::R64 operand = x64asm::rdi;
                loadTemp(operand, inst->tempIndices->at(1));
                //assm.not(operand);
                assm.assemble({x64asm::NOT_R64, {operand}});
                storeTemp(operand, inst->tempIndices->at(0));
                break;
            };
        case IrOp::Goto:
            {
                LOG(to_string(instructionIndex) + ": Goto");
                int32_t labelIdx = inst->op0.value();
                assm.jmp(x64asm::Label{to_string(labelIdx)});
                break;
            };
        case IrOp::If:
            {
                LOG(to_string(instructionIndex) + ": If");
                int32_t labelIdx = inst->op0.value();
                // load the temp into a reg
                x64asm::R64 left = x64asm::rdi;
                x64asm::R64 right = x64asm::rsi;
                loadTemp(left, inst->tempIndices->at(0));
                assm.mov(right, x64asm::Imm64{1});
                assm.cmp(left, right);
                assm.je(x64asm::Label{to_string(labelIdx)});
                break;
            };
       case IrOp::AssertInteger:
            {
                LOG(to_string(instructionIndex) + ": AssertInteger");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_int), args, temps);
                break;
            };
        case IrOp::AssertBoolean:
            {
                LOG(to_string(instructionIndex) + ": AssertBool");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_bool), args, temps);
                break;
            };
        case IrOp::AssertString:
            {
                LOG(to_string(instructionIndex) + ": AssertString");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_str), args, temps);
                break;
            };
        case IrOp::AssertRecord:
            {
                LOG(to_string(instructionIndex) + ": AssertRecord");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_record), args, temps);
                break;
            };
        case IrOp::AssertFunction:
            {
                LOG(to_string(instructionIndex) + ": AssertFunction");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_func), args, temps);
                break;
            };
        case IrOp::AssertClosure:
            {
                LOG(to_string(instructionIndex) + ": AssertClosure");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(0)
                };
                callHelper((void *) &(helper_assert_closure), args, temps);
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
                callHelper((void *) &(helper_assert_valwrapper), args, temps);
                break;
            };
        case IrOp::UnboxInteger:
            {
                LOG(to_string(instructionIndex) + ": UnboxInteger");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                callHelper((void *) &(helper_unbox_int), args, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            };
        case IrOp::UnboxBoolean:
            {
                LOG(to_string(instructionIndex) + ": UnboxBoolean");
                vector<x64asm::Imm64> args;
                vector<tempptr_t> temps = {
                    inst->tempIndices->at(1)
                };
                callHelper((void *) &(helper_unbox_bool), args, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
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
                callHelper((void *) &(helper_new_integer), args, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
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
                callHelper((void *) &(helper_new_boolean), args, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
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
                callHelper((void *) &(helper_cast_string), args, temps);
                storeTemp(x64asm::rax, inst->tempIndices->at(0));
                break;
            };
        case IrOp::AddLabel:
            {
                LOG(to_string(instructionIndex) + ": AddLabel");
                break;
            };
        case IrOp::GarbageCollect:
            {
                LOG(to_string(instructionIndex) + ": GarbageCollect");
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
