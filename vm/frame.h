#pragma once

#include "types.h"
#include <string>
#include <map>
#include <stack> 

using namespace std;

class Frame {
public:
    map<string, Value> localVars;
    map<string, Value*> localRefs;
    stack<Value> operandStack;
    Instruction* instructionPtr;
    Function* func;
};
