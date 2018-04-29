# pragma once 

#include "../ir.h"
#include "helpers.h"
#include "include/x64asm.h"

class Interpreter; 

using namespace std; 
using namespace x64asm; 

class IrInterpreter {
private: 
    vptr<Interpreter> vmPointer;

    x64asm::Assembler assm; 
    x64asm::Function asmFunc;

    vptr<IrProgram> program;
    vptr<IrFunc> curFunc;
    int instructionIndex;
    bool finished;

    void executeStep();
public: 
    IrInterpreter(vptr<IrProgram> irProgram, vptr<Interpreter> vmInterpreterPointer); 
    void run(); // runs the program 
};
