#pragma once

#include "types.h"
#include "frame.h"
#include <stack>

using namespace std;

class Interpreter {
private:
    Function* currentFunc;
    Frame* currentFrame; 
    int instructionPtr;
public:
    Interpreter(Function* mainFunc);
};
