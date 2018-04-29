# pragma once 

#include "ir.h"
#include "include/x64asm.h"

using namespace std; 
using namespace x64asm; 

class IrInterpreter {
private: 
    vptr<IrProgram> program;
    vptr<IrFunc> curFunc;
    int instructionIndex;
    bool finished;

    void executeStep();
public: 
    IrInterpreter(vptr<IrProgram> irProgram); 
    void run(); // runs the program 
};
