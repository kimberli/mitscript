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

void IrInterpreter::loadTemp(x64asm::R64 reg, tempptr_t temp) {
    // figure out where the temp is stored 
    getRbpOffset(temp->stackOffset); // location in r10 
    // put the thing from that mem addres into the reg 
    assm.mov(reg, x64asm::M64{x64asm::r10});
}

void IrInterpreter::executeStep() {
    const static x64asm::R64 argRegs[] = {x64asm::rdi, x64asm::rsi, x64asm::rdx, x64asm::rcx, x64asm::r8, x64asm::r9};

    IrInstruction inst = func->instructions.at(instructionIndex);
    switch(inst.op) {
        case IrOp::LoadConst: 
            {
                LOG("LoadConst");
                int constIndex = inst.op0.value(); 
                Constant* c = func->constants_.at(constIndex);
                // load a constant into a register 
                assm.mov(x64asm::rdi, x64asm::Imm64{(uint64_t)c}); 
                // move from the register into a temp on the stack
                storeTemp(x64asm::rdi, inst.tempIndices->at(0));
                break;
            }
        case IrOp::LoadFunc: 
            {
                LOG("LoadFunc");
                int funcIndex = inst.op0.value();
                // load the func pointer into a register
                assm.mov(x64asm::rdi, x64asm::Imm64{func->functions_.at(funcIndex)});
                // move from the register into a temp on the stack 
                storeTemp(x64asm::rdi, inst.tempIndices->at(0));
                break;
            }
        case IrOp::LoadLocal: 
            {
                LOG("LoadLocal");
                int64_t offset = inst.op0.value();
                getRbpOffset(offset); // puts the address of the local in r10
                assm.mov(x64asm::rdi, x64asm::r10); // r10 will be used later in storeTemp
                assm.mov(x64asm::rdi, x64asm::M64{x64asm::r10}); // hopefully this loads the actual val
                storeTemp(x64asm::rdi, inst.tempIndices->at(0));
                break;
            }
        case IrOp::LoadGlobal: 
            {
                LOG("LoadGlobal");
                // mov DEST, SRC
                // load the interpreter pointer into the first arg 
                assm.mov(argRegs[0], x64asm::Imm64{vmPointer});
                // load the string pointer into the second arg
                string name = inst.name0.value();
                assm.mov(argRegs[1], x64asm::Imm64{&name});
                // call a helper 
                void* fn = (void*) &(helper_load_global);
                // assume that all locals are saved to the stack
                assm.mov(x64asm::r10, x64asm::Imm64{(uint64_t)fn});
                assm.call(x64asm::r10);
                // the result is stored in rax
                // put the return val in the temp
                storeTemp(x64asm::rax, inst.tempIndices->at(0));
                break;
            }
        case IrOp::StoreLocal: 
            {
                LOG("StoreLocal");
                // first put the temp val in a reg
                loadTemp(x64asm::rdi, inst.tempIndices->at(0));
                // put find out where the constant is located
                int64_t offset = inst.op0.value();
                getRbpOffset(offset); // address of local in r10 
                // move the val from the reg to memory
                assm.mov(x64asm::M64{x64asm::r10}, x64asm::rdi);
                break;
            }
       case IrOp::StoreGlobal:
            {
                LOG("StoreGlobal");
                // load the interpreter pointer into the first arg 
                assm.mov(argRegs[0], x64asm::Imm64{vmPointer});
                // load the string pointer into the second arg
                string name = inst.name0.value();
                assm.mov(argRegs[1], x64asm::Imm64{&name});
                // load the value into the third arg 
                loadTemp(argRegs[2], inst.tempIndices->at(0));
                // call a helper 
                void* fn = (void*) &(helper_store_global);
                assm.mov(x64asm::r10, x64asm::Imm64{(uint64_t)fn});
                assm.call(x64asm::r10);
                // no return val given. 
                break;
            }
        case IrOp::AllocRecord:
            {
                LOG("AllocRecord");
                break;
            };
        case IrOp::FieldLoad:
            {
                LOG("FieldLoad");
                break;
            };
        case IrOp::FieldStore:
            {
                LOG("FieldStore");
                break;
            };
        case IrOp::IndexLoad:
            {
                LOG("IndexLoad");
                break;
            };
        case IrOp::IndexStore:
            {
                LOG("IndexStore");
                break;
            };
        case IrOp::AllocClosure: 
            {
                LOG("AllocClosure");
                break;
            };
        case IrOp::Call: 
            {
                LOG("Call");
                break;
            };
        case IrOp::Return: 
            {
                LOG("Return");
                break;
            };
        case IrOp::Add: 
            {
                LOG("Add");
                break;
            };
        case IrOp::Sub: 
            {
                LOG("Sub");
                break;
            };
        case IrOp::Mul: 
            {
                LOG("Mul");
                break;
            };
        case IrOp::Div: 
            {
                LOG("Div");
                break;
            };
        case IrOp::Neg: 
            {
                LOG("Neg");
                break;
            };
        case IrOp::Gt: 
            {
                LOG("Gt");
                break;
            };
        case IrOp::Geq : 
            {
                LOG("Geq");
                break;
            };
        case IrOp::Eq: 
            {
                LOG("Eq");
                break;
            };
        case IrOp::And: 
            {
                LOG("And");
                break;
            };
        case IrOp::Or: 
            {
                LOG("Or");
                break;
            };
        case IrOp::Not: 
            {
                LOG("Not");
                break;
            };
        case IrOp::Goto: 
            {
                LOG("Goto");
                break;
            };
        case IrOp::If: 
            {
                LOG("If");
                break;
            };
       case IrOp::AssertInteger: 
            {
                LOG("AssertInteger");
                break;
            };
        case IrOp::AssertBool: 
            {
                LOG("AssertBool");
                break;
            };
        case IrOp::AssertString: 
            {
                LOG("AssertString");
                break;
            };
        case IrOp::AssertRecord: 
            {
                LOG("AssertRecord");
                break;
            };
        case IrOp::AssertFunction: 
            {
                LOG("AssertFunction");
                break;
            };
        case IrOp::AssertClosure: 
            {
                LOG("AssertClosure");
                break;
            };
        case IrOp::CastInteger: 
            {
                LOG("CastInteger");
                break;
            };
        case IrOp::CastBool: 
            {
                LOG("CastBool");
                break;
            };
        case IrOp::CastString: 
            {
                LOG("CastString");
                break;
            };
        case IrOp::AddLabel:
            {
                LOG("AddLabel");
                break;
            };
        case IrOp::GarbageCollect: 
            {
                LOG("GarbageCollect");
                break;
            };
        default: 
            throw RuntimeException("Should not get here: invalid ir inst");
    }
    LOG(inst.getInfo());
    instructionIndex += 1;
    if (instructionIndex >= func->instructions.size()) {
        finished = true;
    }
}


