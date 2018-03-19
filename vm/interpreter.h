#pragma once

#include "types.h"
#include "frame.h"
#include <stack>
#include <iostream>

using namespace std;

#define LOG(msg) { if (true) std::cerr << msg << endl; }

class Interpreter {
private:
    Function* currFunc;
    stack<Frame*>* frames; 
    Instruction* instructionPtr;
    void executeStep();
    bool finished;
public:
    Interpreter(Function* mainFunc);
    void run();
};
