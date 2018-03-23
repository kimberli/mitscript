#pragma once

#include "types.h"
#include <string>
#include <map>
#include <stack> 

using namespace std;

class Frame {
public:
    map<string, std::shared_ptr<Value>> localVars;
    map<string, std::shared_ptr<ValuePtr>> localRefs;
    stack<std::shared_ptr<Value>> operandStack;
    Instruction* instructionPtr;
    Function& func;

    Frame(Instruction* ip, Function& func): instructionPtr(ip), func(func) {};
};
