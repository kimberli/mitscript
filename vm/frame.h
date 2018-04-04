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

    void moveToInstruction(int offset) {
        // TODO: check for illegal offsets
        instructionPtr += offset;
    }

    void opStackPush(std::shared_ptr<Value> val) {
        operandStack.push(val);
    }

    std::shared_ptr<Value> opStackPeek() {
        return operandStack.top();
    }

    std::shared_ptr<Value> opStackPop() {
        if (operandStack.empty()) {
            throw InsufficientStackException("pop from empty stack");
        }
        std::shared_ptr<Value> top = operandStack.top();
        operandStack.pop();
        return top;
    }
};
