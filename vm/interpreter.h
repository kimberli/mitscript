/*
 * interpreter.h
 *
 * Defines an Interpreter object which can be used to step through
 * bytecode instructions and store interpreter state
 */
#pragma once

#include "types.h"
#include "frame.h"
#include <stack>
#include <iostream>

using namespace std;

class Interpreter {
    // class used to handle interpreter state
private:
    Frame* globalFrame;  // root function frame
    stack<Frame*> frames;  // stack of frames
    void executeStep();  // execute a single next instruction
    bool finished;  // true when the program has terminated
    CollectedHeap* collector;
public:
    Interpreter(Function* mainFunc, CollectedHeap* gCollector);
    void run();  // executes all instructions until termination
};
