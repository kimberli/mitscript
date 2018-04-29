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

    vptr<IrFunc> func;
    int instructionIndex;
    bool finished;

    void executeStep();
public: 
    IrInterpreter(vptr<IrFunc> irFunction, vptr<Interpreter> vmInterpreterPointer); 
    void run(); // runs the program 
};
