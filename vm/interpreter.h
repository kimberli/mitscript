/*
 * interpreter.h
 *
 * Defines an Interpreter object which can be used to step through
 * bytecode instructions
 */
#pragma once

#include "types.h"
#include "frame.h"
#include <stack>
#include <iostream>

using namespace std;

#define LOG(msg) { if (true) std::cerr << msg << endl; }

class Interpreter {
private:
    shared_ptr<Frame> globalFrame;
    stack<shared_ptr<Frame>> frames;
    void executeStep();
    bool finished;
public:
    Interpreter(shared_ptr<Function> mainFunc);
    void run();
};
