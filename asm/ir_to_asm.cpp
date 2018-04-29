#include "ir_to_asm.h"

IrInterpreter::IrInterpreter(vptr<IrProgram> irProgram) {
    program = irProgram; 
    // by convention, the first ir function is the main function
    curFunc = program->functions.at(0); 
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

void IrInterpreter::executeStep() {

    IrInstruction inst = curFunc->instructions.at(instructionIndex);
    switch(inst.operation) {
        case IrOp::LoadConst: 
            {
                // implementation 
            }
    }
    instructionIndex += 1;
    if (instructionIndex > curFunc->instructions.size()) {
        finished = true;
    }
}
