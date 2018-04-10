#include "frame.h"

// instruction helpers
int Frame::numInstructions() {
    return func->instructions.size();
}

Instruction& Frame::getCurrInstruction() {
    return func->instructions[instructionIndex];
}

void Frame::moveToInstruction(int offset) {
    int newOffset = instructionIndex + offset;
    if (newOffset < 0 || newOffset >= func->instructions.size()) {
        throw RuntimeException("instruction " + std::to_string(instructionIndex + newOffset) + " out of bounds");
    }
    instructionIndex += offset;
}

// function value helpers
shared_ptr<Constant> Frame::getConstantByIndex(int index) {
    if (index < 0 || index >= func->constants_.size()) {
        throw RuntimeException("constant " + std::to_string(index) + " out of bounds");
    }
    return func->constants_[index];
}

shared_ptr<Function> Frame::getFunctionByIndex(int index) {
    if (index < 0 || index >= func->functions_.size()) {
        throw RuntimeException("function " + std::to_string(index) + " out of bounds");
    }
    return func->functions_[index];
}

string Frame::getLocalVarByIndex(int index) {
    if (index < 0 || index >= func->local_vars_.size()) {
        throw RuntimeException("local var " + std::to_string(index) + " out of bounds");
    }
    return func->local_vars_[index];
}

string Frame::getNameByIndex(int index) {
    if (index < 0 || index >= func->names_.size()) {
        throw RuntimeException("name " + std::to_string(index) + " out of bounds");
    }
    return func->names_[index];
}

string Frame::getRefVarByIndex(int index) {
    if (index < 0 || index >= (func->local_reference_vars_.size() + func->free_vars_.size())) {
        throw RuntimeException("name " + std::to_string(index) + " out of bounds");
    }
    if (index < func->local_reference_vars_.size()) {
        return func->local_reference_vars_[index];
    }
    return func->free_vars_[index - func->local_reference_vars_.size()];
}

// var map helpers
shared_ptr<Constant> Frame::getLocalVar(string name) {
    if (localVars.count(name) == 0) {
        throw UninitializedVariableException(name + " is not initialized");
    }
    return localVars[name];
}

shared_ptr<ValuePtr> Frame::getRefVar(string name) {
    if (localRefs.count(name) == 0) {
        throw UninitializedVariableException(name + " is not initialized");
    }
    return localRefs[name];
}

void Frame::setLocalVar(string name, shared_ptr<Constant> val) {
    if (localRefs.count(name) != 0) {
        localVars[name] = val;
        *(localRefs[name].get()->ptr) = *(val.get());
    } else {
        localVars[name] = val;
    }
    localRefs[name] = make_shared<ValuePtr>(localVars[name]);
}

void Frame::setRefVar(string name, shared_ptr<ValuePtr> val) {
    localRefs[name] = val;
}

// operand stack helpers
void Frame::opStackPush(std::shared_ptr<Value> val) {
    opStack.push(val);
}

std::shared_ptr<Value> Frame::opStackPeek() {
    if (opStack.empty()) {
        throw InsufficientStackException("peek at empty stack");
    }
    return opStack.top();
}

std::shared_ptr<Value> Frame::opStackPop() {
    if (opStack.empty()) {
        throw InsufficientStackException("pop from empty stack");
    }
    std::shared_ptr<Value> top = opStack.top();
    opStack.pop();
    return top;
}
