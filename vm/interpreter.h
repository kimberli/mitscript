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
    shared_ptr<Frame> globalFrame;  // root function frame
    stack<shared_ptr<Frame>> frames;  // stack of frames
    void executeStep();  // execute a single next instruction
    bool finished;  // true when the program has terminated
public:
    Interpreter(shared_ptr<Function> mainFunc);
    void run();  // executes all instructions until termination
};
