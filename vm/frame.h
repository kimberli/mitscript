#pragma once

#include "types.h"
#include <string>
#include <map>
#include <stack> 

using namespace std;

class Frame {
    Function& func;

public:
    map<string, std::shared_ptr<Value>> localVars;
    map<string, std::shared_ptr<ValuePtr>> localRefs;
    stack<std::shared_ptr<Value>> operandStack;
    int instructionIndex;

    Frame(int ix, Function& func): instructionIndex(ix), func(func) {};

    // instruction helpers
    int numInstructions() {
        return func.instructions.size();
    }

    Instruction& getCurrInstruction() {
        return func.instructions[instructionIndex];
    }

    void moveToInstruction(int offset) {
        int newOffset = instructionIndex + offset;
        if (newOffset < 0 || newOffset >= func.instructions.size()) {
            throw RuntimeException("instruction " + std::to_string(instructionIndex + newOffset) + " out of bounds");
        }
        instructionIndex += offset;
    }

    // function value helpers
    shared_ptr<Constant> getConstantByIndex(int index) {
        if (index < 0 || index >= func.constants_.size()) {
            throw RuntimeException("constant " + std::to_string(index) + " out of bounds");
        }
        return func.constants_[index];
    }

    shared_ptr<Function> getFunctionByIndex(int index) {
        if (index < 0 || index >= func.functions_.size()) {
            throw RuntimeException("function " + std::to_string(index) + " out of bounds");
        }
        return func.functions_[index];
    }

    string getLocalVarByIndex(int index) {
        if (index < 0 || index >= func.local_vars_.size()) {
            throw RuntimeException("local var " + std::to_string(index) + " out of bounds");
        }
        return func.local_vars_[index];
    }

    string getNameByIndex(int index) {
        if (index < 0 || index >= func.names_.size()) {
            throw RuntimeException("name " + std::to_string(index) + " out of bounds");
        }
        return func.names_[index];
    }

    string getRefVarByIndex(int index) {
        if (index < 0 || index >= func.local_reference_vars_.size()) {
            throw RuntimeException("name " + std::to_string(index) + " out of bounds");
        }
        return func.local_reference_vars_[index];
    }

    // operand stack helpers
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
