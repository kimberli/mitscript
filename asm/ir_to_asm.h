#pragma once

#include "../vm/interpreter.h"
#include "../opt/opt_tag_ptr.h"
#include "../ir.h"
#include "helpers.h"
#include <set>
#include <cassert>

#define TYPE_ERROR_LABEL "typeErrorLabel"

class Interpreter;
class IrInterpreter;
enum class TempOp;
enum class TempBoolOp;

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
    uint32_t spaceToAllocate;

    void callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, opttemp_t returnTemp);
    void callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, optreg_t lastArg, opttemp_t returnTemp);

    // prolog and helpers
    void prolog();
    void installLocalVar(tempptr_t temp, uint32_t localIdx);
    void installLocalRefVar(tempptr_t temp, uint32_t localIdx);
    void installLocalNone(tempptr_t temp);
    void installLocalRefNone(tempptr_t temp);

    void epilog();

    void executeStep();
    uint32_t getTempOffset(tempptr_t temp);
    uint32_t getLocalOffset(uint32_t localIndex);
    uint32_t getRefArrayOffset();
    void getRbpOffset(uint32_t offset);
    void loadTemp(x64asm::R32 reg, tempptr_t temp);
    void loadTemp(x64asm::R64 reg, tempptr_t temp);
    void storeTemp(x64asm::R32 reg, tempptr_t temp);
    void storeTemp(x64asm::R64 reg, tempptr_t temp);
    void comparisonSetup(instptr_t inst, TempBoolOp tempBoolOp);
    x64asm::R32 getRegBottomHalf(x64asm::R64 reg);

    int regPopCount = 0;
    int popCount = 0;
    void Pop(x64asm::R64 reg);
    void Pop();
    void Push(x64asm::R64 reg);
    // generates a new free reg by pushing/popping and returns it
    x64asm::R64 getScratchReg();
    // returns a scratch reg
    void returnScratchReg(x64asm::R64 reg);
    // moves a val between two temps efficiently given where they are currently
    void moveTemp(tempptr_t dest, tempptr_t src);
    void moveTemp(tempptr_t dest, tempptr_t src, TempOp tempOp);
    void moveTemp(x64asm::R64 dest, tempptr_t src);
    void moveTemp(x64asm::R64 dest, tempptr_t src, TempOp tempOp);
    void moveTemp(tempptr_t dest, x64asm::R64 src);
    void moveTemp32(tempptr_t dest, tempptr_t src);
    void moveTemp32(tempptr_t dest, tempptr_t src, TempOp tempOp);
    void moveTemp(x64asm::R32 dest, tempptr_t src);
    void moveTemp(x64asm::R32 dest, tempptr_t src, TempOp tempOp);
    void moveTemp(tempptr_t dest, x64asm::R32 src);
    void moveTemp(tempptr_t dest, x64asm::Imm32 src, TempOp tempOp);

public:
    //static const x64asm::R64 argRegs[];
    static const x64asm::R64 callerSavedRegs[];
    static const x64asm::R64 calleeSavedRegs[];
    static const int numCallerSaved = 9;
    static const int numCalleeSaved = 5;
    static const int numArgRegs = 6;
    IrInterpreter(IrFunc* irFunction, Interpreter* vmInterpreterPointer, vector<bool> isLocalRefVec);
    x64asm::Function run(); // runs the program
};

enum class TempOp {
    MOVE,
    SUB,
    MUL,
    CMP, 
    AND,
    OR
};

enum class TempBoolOp {
    CMOVNLE,
    CMOVNL
};
