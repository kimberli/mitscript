# pragma once 

#include "../ir.h"
#include "helpers.h"
#include "include/x64asm.h"
#include <experimental/optional>

class Interpreter; 

using namespace std; 
using namespace x64asm; 

struct TempLocation {
    // a temp can be stored either in memory or a register
    // for the first round, we're basically gonna store all temps on the heap.
    bool present = false;
    std::experimental::optional<R64> reg;
    std::experimental::optional<M64> memloc;
};

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
