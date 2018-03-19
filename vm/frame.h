#pragma once

#include "types.h"
#include <string>
#include <map>
#include <stack> 

using namespace std;

class Frame {
public:
    map<string, Constant> localVars;
    map<string, Constant*> localRefs;
    stack<Constant> operandStack;
    Instruction* instructionPtr;
    Function* func;
};
