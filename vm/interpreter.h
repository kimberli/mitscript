/*
 * interpreter.h
 *
 * Defines an Interpreter object which can be used to step through
 * bytecode instructions and store interpreter state
 */
#pragma once

#include "../types.h"
#include "../frame.h"
#include "../instructions.h"
#include "../exception.h"
#include "../machine_code_func.cpp"
#include "../ir/bc_to_ir.h"
#include "../asm/ir_to_asm.h"
#include <list>
#include <iostream>

using namespace std;

class Interpreter {
    // class used to handle interpreter state
private:
    Frame* globalFrame;  // root function frame
    list<Frame*> frames;  // stack of frames
    void executeStep();  // execute a single next instruction
    bool finished;  // true when the program has terminated
    bool shouldCallAsm;
    int32_t labelCounter = 0;  // global label indexing for all generated asm

 public:
    // static None
    None* NONE;

    CollectedHeap* collector;
    Interpreter(Function* mainFunc, int maxmem, bool callAsm);
    void run();  // executes all instructions until termination

    // handle different call methods for vm vs asm exeuction
    Value* call(vector<Constant*> argsList, Value* closure);
    // handles calling from the vm
    Value* callVM(vector<Constant*> argsList, Closure* clos);
    // handles calling from asm
    Value* callAsm(vector<Constant*> argsList, Closure* clos);

    // asm helpers
    Value* add(Value* left, Value* right);
    void storeGlobal(string name, Value* val);
    Value* loadGlobal(string name);
};
