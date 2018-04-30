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

//IrInterpreter::loadTemp(int tmpIdx, R64 reg) {
//    
//}

void IrInterpreter::executeStep() {
    const static R64 argRegs[] = {rdi, rsi, rdx, rcx, r8, r9};

    IrInstruction inst = func->instructions.at(instructionIndex);
    switch(inst.operation) {
        case IrOp::LoadConst: 
            {
                // TODO 
                break;
            }
        case IrOp::LoadGlobal: 
            {
                // load the interpreter pointer into the first arg 
                assm.mov(argRegs[0], Imm64{vmPointer});
                // load the string pointer into the second arg
                std::string name = inst.name0.value();
                assm.mov(argRegs[1], Imm64{&name});
                // call a helper 
                void* fn = (void*) &(helper_load_global);
                assm.mov(r12, Imm64{(uint64_t)fn});
                assm.call(r12);
                break;
            }
        case IrOp::StoreGlobal:
            {
                // load the interpreter pointer into the first arg 
                assm.mov(argRegs[0], Imm64{vmPointer});
                // load the string pointer into the second arg
                std::string name = inst.name0.value();
                assm.mov(argRegs[1], Imm64{&name});
                // load the value into the third arg 
                // FIGURE OUT A DISCIPLINED WAY OF ASSIGNING TEMPS
                //assm.mov(argRegs[2], );
                // call a helper 
                void* fn = (void*) &(helper_store_global);
                assm.mov(r12, Imm64{(uint64_t)fn});
                assm.call(r12);
                break;
            }
    }
    instructionIndex += 1;
    if (instructionIndex > func->instructions.size()) {
        finished = true;
    }
}


