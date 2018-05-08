# pragma once 

#include "../ir.h"
#include "helpers.h"
#include "include/x64asm.h"
#include <experimental/optional>

class Interpreter; 
class IrInterpreter;

using namespace std; 

class IrInterpreter {
private: 
    Interpreter* vmPointer;

    x64asm::Assembler assm; 
    x64asm::Function asmFunc;

    IrFunc* func;
    int instructionIndex;
    bool finished;

    void callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temp, opttemp_t returnTemp);
    void prolog();
    void epilog();
    uint32_t spaceToAllocate;

    void executeStep();
    uint32_t getTempOffset(tempptr_t temp);
    uint32_t getLocalOffset(uint32_t localIndex);
    uint32_t getRefArrayOffset();
    void getRbpOffset(uint32_t offset);
    void loadTemp(x64asm::R64 reg, tempptr_t temp);
    void storeTemp(x64asm::R64 reg, tempptr_t temp);
    void comparisonSetup(x64asm::R64 left, x64asm::R64 right, instptr_t inst);
public: 
    static const x64asm::R64 argRegs[];
    static const x64asm::R64 callerSavedRegs[];
    static const x64asm::R64 calleeSavedRegs[];
    static const int numCallerSaved = 9;
    static const int numCalleeSaved = 5;
    static const int numArgRegs = 6;
    IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer); 
    x64asm::Function run(); // runs the program 
};
