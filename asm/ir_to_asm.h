# pragma once 

#include "../ir.h"
#include "helpers.h"
#include "include/x64asm.h"
#include <experimental/optional>

class Interpreter; 
class IrInterpreter;

using namespace std; 
//using namespace x64asm; 

class IrInterpreter {
private: 
    Interpreter* vmPointer;

    x64asm::Assembler assm; 
    x64asm::Function asmFunc;

    IrFunc* func;
    int stackSize = 0;
    int instructionIndex;
    bool finished;

    void executeStep();
    void getRbpOffset(uint64_t offset);
    void loadTemp(x64asm::R64 reg, Temp &temp);
    void storeTemp(x64asm::R64 reg, Temp &temp);
public: 
    IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer); 
    x64asm::Function run(); // runs the program 
};
