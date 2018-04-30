#include "ir_to_asm.h"

IrInterpreter::IrInterpreter(vptr<IrFunc> irFunction, vptr<Interpreter> vmInterpreterPointer) {
    vmPointer = vmInterpreterPointer;
    func = irFunction; 
    // by convention, the first ir function is the main function
    instructionIndex = 0;
    finished = false;
}

void IrInterpreter::run() {
    // start the assembler on the function 
    assm.start(asmFunc);

    // translate to asm 
    while (!finished) {
        executeStep();
    }

    // finish compiling 
    assm.finish();
    // call the asmFunc. Since this is main, presumably there are no args
    asmFunc.call<Value*>();
}

void IrInterpreter::getRbpOffset(uint64_t offset) {
    // use r10 and r11 for these calcs
    assm.mov(r10, rbp); // move rbp into r10
    assm.mov(r11, Imm64{8*offset}); // move 8*offset into another reg
    assm.sub(r10, r11); // sub r11 from r10. now the correct mem is in r10
}

void IrInterpreter::storeTemp(R64 reg, Temp &temp) {
    // right now, put everything on the stack 
    // assign the temp an offset (from rbp)
    // Assume we are not saving any callee save. 
    temp.stackOffset = func->parameter_count_ +  stackSize;
    // in assembly, calculate where this var is located 
    getRbpOffset(temp.stackOffset); // leaves correct address into r10
    // Move the val in reg into the mem address stored in r10 
    assm.mov(M64{r10}, reg);
    stackSize++; //inc stack size for next temp
}

void IrInterpreter::loadTemp(R64 reg, Temp &temp) {
    // figure out where the temp is stored 
    getRbpOffset(temp.stackOffset); // location in r10 
    // put the thing from that mem addres into the reg 
    assm.mov(reg, M64{r10});
}

void IrInterpreter::executeStep() {
    const static R64 argRegs[] = {rdi, rsi, rdx, rcx, r8, r9};

    IrInstruction inst = func->instructions.at(instructionIndex);
    switch(inst.op) {
        case IrOp::LoadConst: 
            {
                int constIndex = inst.op0.value(); 
                // load a constant into a register 
                assm.mov(rdi, Imm64{func->constants_.at(constIndex)}); 
                // move from the register into a temp on the stack
                storeTemp(rdi, inst.tempIndices.at(0));
                break;
            }
        case IrOp::LoadLocal: 
            {
                int64_t offset = inst.op0.value();
                getRbpOffset(offset); // puts the address of the local in r10
                assm.mov(rdi, r10); // r10 will be used later in storeTemp
                assm.mov(rdi, M64{r10}); // hopefully this loads the actual val
                storeTemp(rdi, inst.tempIndices.at(0));
                break;
            }
        case IrOp::StoreLocal: 
            {
                // first put the temp val in a reg
                loadTemp(rdi, inst.tempIndices.at(0));
                // put find out where the constant is located
                int64_t offset = inst.op0.value();
                getRbpOffset(offset); // address of local in r10 
                // move the val from the reg to memory
                assm.mov(M64{r10}, rdi);
                break;
            }
        case IrOp::LoadGlobal: 
            {
                // mov DEST, SRC
                // load the interpreter pointer into the first arg 
                assm.mov(argRegs[0], Imm64{vmPointer});
                // load the string pointer into the second arg
                std::string name = inst.global.value();
                assm.mov(argRegs[1], Imm64{&name});
                // call a helper 
                void* fn = (void*) &(helper_load_global);
                // assume that all locals are saved to the stack
                assm.mov(r10, Imm64{(uint64_t)fn});
                assm.call(r10);
                // the result is stored in rax
                // put the return val in the temp
                storeTemp(rax, inst.tempIndices.at(0));
                break;
            }
        case IrOp::StoreGlobal:
            {
                // load the interpreter pointer into the first arg 
                assm.mov(argRegs[0], Imm64{vmPointer});
                // load the string pointer into the second arg
                std::string name = inst.global.value();
                assm.mov(argRegs[1], Imm64{&name});
                // load the value into the third arg 
                loadTemp(argRegs[2], inst.tempIndices.at(0));
                // call a helper 
                void* fn = (void*) &(helper_store_global);
                assm.mov(r10, Imm64{(uint64_t)fn});
                assm.call(r10);
                // no return val given. 
                break;
            }
    }
    instructionIndex += 1;
    if (instructionIndex > func->instructions.size()) {
        finished = true;
    }
}


