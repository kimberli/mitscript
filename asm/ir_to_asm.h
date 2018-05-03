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

    void callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temp);

    void executeStep();
    void getRbpOffset(uint64_t offset);
    void loadTemp(x64asm::R64 reg, tempptr_t temp);
    void storeTemp(x64asm::R64 reg, tempptr_t temp);
    void comparisonSetup(x64asm::R64 left, x64asm::R64 right, instptr_t inst);
public: 
    static const x64asm::R64 argRegs[];
    static const x64asm::R64 callerSavedRegs[];
    static const int numCallerSaved = 9;
    static const int numArgRegs = 6;
    IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer); 
    x64asm::Function run(); // runs the program 
};
