/*
 * interpreter.h
 *
 * Defines an Interpreter object which can be used to step through
 * bytecode instructions and store interpreter state
 */
#pragma once

#include "../types.h"
#include "../frame.h"
#include <list>
#include <iostream>

using namespace std;

class Interpreter {
    // class used to handle interpreter state
private:
    fptr globalFrame;  // root function frame
    list<fptr> frames;  // stack of frames
    void executeStep();  // execute a single next instruction
    bool finished;  // true when the program has terminated
    CollectedHeap* collector;
    int newOffset;
public:
    Interpreter(vptr<Function> mainFunc, int maxmem);
    void run();  // executes all instructions until termination

    // two different call methods for vm vs asm exeuction 
    vptr<Value> callVM(fptr frame, int numArgs);
    // TODO assembly compilation

    // asm helpers
    vptr<Value> add(vptr<Value> left, vptr<Value> right);
    void storeGlobal(string name, vptr<Value> val);
    vptr<Value> loadGlobal(string name);
};
