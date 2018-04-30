# pragma once 

#include "../ir.h"
#include "helpers.h"
#include "include/x64asm.h"
#include <experimental/optional>

class Interpreter; 

using namespace std; 
using namespace x64asm; 

class IrInterpreter {
private: 
    vptr<Interpreter> vmPointer;

    x64asm::Assembler assm; 
    x64asm::Function asmFunc;

    vptr<IrFunc> func;
    int stackSize = 0;
    int instructionIndex;
    bool finished;

    void executeStep();
    void getTempLocation(uint64_t offset);
    void loadTemp(R64 reg, Temp &temp);
    void storeTemp(R64 reg, Temp &temp);
public: 
    IrInterpreter(vptr<IrFunc> irFunction, vptr<Interpreter> vmInterpreterPointer); 
    void run(); // runs the program 
};
