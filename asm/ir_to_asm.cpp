#include "ir_to_asm.h"

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

    // translate to asm
    while (!finished) {
        executeStep();
    }

    // finish compiling
    assm.ret();
    assm.finish();
    LOG("done compiling asm");
    // return the asmFunc
    return asmFunc;
    //asmFunc.call<Value*>();
}

void IrInterpreter::getRbpOffset(uint64_t offset) {
    // use r10 and r11 for these calcs
    offset = offset + 1;
    assm.mov(x64asm::r10, x64asm::rbp); // move rbp into r10
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
    const static x64asm::R64 argRegs[] = {x64asm::rdi, x64asm::rsi, x64asm::rdx, x64asm::rcx, x64asm::r8, x64asm::r9};

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
                // load the interpreter pointer into the first arg
                assm.mov(argRegs[0], x64asm::Imm64{vmPointer});
                // load the string pointer into the second arg
                string* name = new string(inst->name0.value());  // TODO: fix memory leak?
                assm.mov(argRegs[1], x64asm::Imm64{name});
                // call a helper
                void* fn = (void*) &(helper_load_global);
                // assume that all locals are saved to the stack
                assm.mov(x64asm::r10, x64asm::Imm64{(uint64_t)fn});
                assm.call(x64asm::r10);
                // the result is stored in rax
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
                // load the interpreter pointer into the first arg
                assm.mov(argRegs[0], x64asm::Imm64{vmPointer});
                // load the string pointer into the second arg
                string* name = new string(inst->name0.value());
                assm.mov(argRegs[1], x64asm::Imm64{name});  // TODO: fix memory leak?
                // load the value into the third arg
                loadTemp(argRegs[2], inst->tempIndices->at(0));
                // call a helper
                void* fn = (void*) &(helper_store_global);
                assm.mov(x64asm::r10, x64asm::Imm64{(uint64_t)fn});
                assm.call(x64asm::r10);
                // no return val given.
                break;
            }
        case IrOp::AllocRecord:
            {
                LOG(to_string(instructionIndex) + ": AllocRecord");
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
                // load the interpreter pointer into the first arg
                assm.mov(argRegs[0], x64asm::Imm64{vmPointer});
                // load the left operand into the second arg
                loadTemp(argRegs[1], inst->tempIndices->at(2));
                // load the value into the third arg
                loadTemp(argRegs[2], inst->tempIndices->at(1));

                // now we need to call our helper
                void* fn = (void*) &(helper_add);
                assm.mov(x64asm::r10, x64asm::Imm64{(uint64_t)fn});
                assm.call(x64asm::r10);
                // the result is stored in rax
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
                break;
            };
        case IrOp::AssertBool:
            {
                LOG(to_string(instructionIndex) + ": AssertBool");
                break;
            };
        case IrOp::AssertString:
            {
                LOG(to_string(instructionIndex) + ": AssertString");
                break;
            };
        case IrOp::AssertRecord:
            {
                LOG(to_string(instructionIndex) + ": AssertRecord");
                break;
            };
        case IrOp::AssertFunction:
            {
                LOG(to_string(instructionIndex) + ": AssertFunction");
                break;
            };
        case IrOp::AssertClosure:
            {
                LOG(to_string(instructionIndex) + ": AssertClosure");
                break;
            };
        case IrOp::CastInteger:
            {
                LOG(to_string(instructionIndex) + ": CastInteger");
                break;
            };
        case IrOp::CastBool:
            {
                LOG(to_string(instructionIndex) + ": CastBool");
                break;
            };
        case IrOp::CastString:
            {
                LOG(to_string(instructionIndex) + ": CastString");
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
