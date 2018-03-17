#pragma once

#include "types.h"
#include "frame.h"
#include <stack>

using namespace std;

class Interpreter {
private:
    Function* currentFunc;
    stack<Constant> operandStack;
    Frame frame; 
public:
    Interpreter(Function* mainFunc);
};
