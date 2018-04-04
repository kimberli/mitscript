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
    int instructionIndex;
    Function& func;

    Frame(int ix, Function& func): instructionIndex(ix), func(func) {};

    void moveToInstruction(int offset) {
        int newOffset = instructionIndex + offset;
        if (newOffset < 0 || newOffset >= func.instructions.size()) {
            throw RuntimeException("instruction " + std::to_string(instructionIndex + newOffset) + " out of bounds");
        }
        instructionIndex += offset;
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
