# pragma once 

#include "ir.h"
#include "include/x64asm.h"
#include "machine_code_func.cpp"

using namespace std; 
using namespace x64asm; 

class IrInterpreter {
private: 
    x64asm::Assembler assm; 
    x64asm::Function asmFunc;

    vptr<IrProgram> program;
    vptr<IrFunc> curFunc;
    int instructionIndex;
    bool finished;

    void executeStep();
public: 
    IrInterpreter(vptr<IrProgram> irProgram); 
    void run(); // runs the program 
};
