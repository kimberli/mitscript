# pragma once 

#include "../vm/interpreter.h"
#include "../ir.h"
#include "helpers.h"
#include <set>

class Interpreter; 
class IrInterpreter;

typedef set<x64asm::R64> regset_t;

using namespace std; 

class IrInterpreter {
private: 
    Interpreter* vmPointer;

    x64asm::Assembler assm; 
    x64asm::Function asmFunc;

    IrFunc* func;
    int instructionIndex;
    bool finished;
    vector<bool> isLocalRef;

    void callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temp, opttemp_t returnTemp);
    void callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temp, optreg_t lastArg, opttemp_t returnTemp);
    void prolog();
    void epilog();
    uint32_t spaceToAllocate;

    void executeStep();
    uint32_t getTempOffset(tempptr_t temp);
    uint32_t getLocalOffset(uint32_t localIndex);
    uint32_t getRefArrayOffset();
    void getRbpOffset(uint32_t offset);
    void loadTemp(x64asm::R32 reg, tempptr_t temp);
    void loadTemp(x64asm::R64 reg, tempptr_t temp);
    void storeTemp(x64asm::R32 reg, tempptr_t temp);
    void storeTemp(x64asm::R64 reg, tempptr_t temp);
    void comparisonSetup(x64asm::R32 left, x64asm::R32 right, instptr_t inst);

    // helpers and state to interface with register allocation
    // stores a set of currently avlb regs 
    regset_t freeRegs;
    // helpers to update free regs 
    void updateFreeRegs(instptr_t inst);
    // returns a register containing the value of that temp 
    x64asm::R64 getReg(tempptr_t temp); 
    // makes sure the value in register reg is put in the right place for 
    // the temp 
    void setReg(tempptr_t temp, x64asm::R64 reg);
    // gets a free reg from the pool (or generates a new one 
    // by pushing/popping) and returns it 
    x64asm::R64 getScratchReg();
    // returns a scratch reg 
    void returnScratchReg(x64asm::R64 reg);

public: 
    static const x64asm::R64 argRegs[];
    static const x64asm::R64 callerSavedRegs[];
    static const x64asm::R64 calleeSavedRegs[];
    static const int numCallerSaved = 9;
    static const int numCalleeSaved = 5;
    static const int numArgRegs = 6;
    IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer, vector<bool> isLocalRefVec); 
    x64asm::Function run(); // runs the program 
};
