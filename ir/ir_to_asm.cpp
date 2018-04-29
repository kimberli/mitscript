#include "ir_to_asm.h"

IrInterpreter::IrInterpreter(vptr<IrProgram> irProgram) {
    program = irProgram; 
    // by convention, the first ir function is the main function
    curFunc = program->functions.at(0); 
    instructionIndex = 0;
    finished = false;
}

void IrInterpreter::run() {
    while (!finished) {
        executeStep();
    }
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
