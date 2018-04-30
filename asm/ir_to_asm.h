# pragma once 

#include "../ir.h"
#include "helpers.h"
#include "include/x64asm.h"
#include <experimental/optional>
#include <map>

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
    map<int, TempLocation> temps;

    void executeStep();
public: 
    IrInterpreter(vptr<IrFunc> irFunction, vptr<Interpreter> vmInterpreterPointer); 
    void run(); // runs the program 
};
