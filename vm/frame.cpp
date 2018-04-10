#include "frame.h"
#include <algorithm>

using namespace std;

// instruction helpers
int Frame::numInstructions() {
    return func->instructions.size();
}

Instruction& Frame::getCurrInstruction() {
    return func->instructions[instructionIndex];
}

void Frame::moveToInstruction(int offset) {
    int newOffset = instructionIndex + offset;
    if (newOffset < 0 || newOffset >= numInstructions()) {
        throw RuntimeException("instruction " + to_string(instructionIndex + newOffset) + " out of bounds");
    }
    instructionIndex += offset;
}

// function value helpers
shared_ptr<Constant> Frame::getConstantByIndex(int index) {
    if (index < 0 || index >= func->constants_.size()) {
        throw RuntimeException("constant " + to_string(index) + " out of bounds");
    }
    return func->constants_[index];
}

shared_ptr<Function> Frame::getFunctionByIndex(int index) {
    if (index < 0 || index >= func->functions_.size()) {
        throw RuntimeException("function " + to_string(index) + " out of bounds");
    }
    return func->functions_[index];
}

string Frame::getLocalByIndex(int index) {
    if (index < 0 || index >= func->local_vars_.size()) {
        throw RuntimeException("var " + to_string(index) + " out of bounds");
    }
    return func->local_vars_[index];
}

string Frame::getNameByIndex(int index) {
    if (index < 0 || index >= func->names_.size()) {
        throw RuntimeException("name " + to_string(index) + " out of bounds");
    }
    return func->names_[index];
}

string Frame::getRefByIndex(int index) {
    if (index < 0 || index >= (func->local_reference_vars_.size() + func->free_vars_.size())) {
        throw RuntimeException("ref var " + to_string(index) + " out of bounds");
    }
    if (index < func->local_reference_vars_.size()) {
        return func->local_reference_vars_[index];
    }
    return func->free_vars_[index - func->local_reference_vars_.size()];
}

// var map helpers
shared_ptr<Constant> Frame::getLocalVar(string name) {
    if (vars.count(name) != 0) {
        shared_ptr<Constant> result = vars[name]->ptr;
        if (result == NULL) {
            throw UninitializedVariableException(name + " is not defined");
        }
        return result;
    }
    throw UninitializedVariableException(name + " is not defined");
}

shared_ptr<ValuePtr> Frame::getRefVar(string name) {
    if (vars.count(name) != 0) {
        return vars[name];
    }
    throw RuntimeException(name + " has not been created in its frame's vars");
}

void Frame::setLocalVar(string name, shared_ptr<Constant> val) {
    if (vars.count(name) == 0) {
        vars[name] = make_shared<ValuePtr>(val);
    } else {
        vars[name]->ptr = val;
    }
}

void Frame::setRefVar(string name, shared_ptr<ValuePtr> val) {
    vars[name] = val;
}

// operand stack helpers
void Frame::opStackPush(shared_ptr<Value> val) {
    opStack.push(val);
}

shared_ptr<Value> Frame::opStackPeek() {
    if (opStack.empty()) {
        throw InsufficientStackException("peek at empty stack");
    }
    return opStack.top();
}

shared_ptr<Value> Frame::opStackPop() {
    if (opStack.empty()) {
        throw InsufficientStackException("pop from empty stack");
    }
    shared_ptr<Value> top = opStack.top();
    opStack.pop();
    return top;
}
