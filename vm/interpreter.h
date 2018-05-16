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
#include "../opt/opt.h"
#include "../opt/opt_reg_alloc.h"
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

 public:
    // static None
    tagptr_t NONE;

    CollectedHeap* collector;
    list<Collectable*>* rootset;
    Interpreter(Function* mainFunc, int maxmem, bool callAsm);
    void run();  // executes all instructions until termination

    // handle different call methods for vm vs asm exeuction
    tagptr_t call(vector<tagptr_t> argsList, tagptr_t clos_ptr);
    // handles calling from the vm
    tagptr_t callVM(vector<tagptr_t> argsList, tagptr_t clos_ptr);
    // handles calling from asm
    tagptr_t callAsm(vector<tagptr_t> argsList, tagptr_t clos_ptr);

    void addToRootset(Collectable* obj);
    void removeFromRootset(Collectable* obj);
    void setFrameCollectables(int numTemps, Collectable** collectables);

    // asm helpers
    void storeGlobal(string name, tagptr_t val);
    tagptr_t loadGlobal(string name);
};
