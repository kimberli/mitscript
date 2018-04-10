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
    if (newOffset < 0 || newOffset >= func->instructions.size()) {
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

string Frame::getNameByIndex(int index) {
    if (index < 0 || index >= func->names_.size()) {
        throw RuntimeException("name " + to_string(index) + " out of bounds");
    }
    return func->names_[index];
}

string Frame::getRefVarByIndex(int index) {
    if (index < 0 || index >= (func->local_reference_vars_.size() + func->free_vars_.size())) {
        throw RuntimeException("name " + to_string(index) + " out of bounds");
    }
    if (index < func->local_reference_vars_.size()) {
        return func->local_reference_vars_[index];
    }
    return func->free_vars_[index - func->local_reference_vars_.size()];
}

// var map helpers
int Frame::getLocalIndex(string name) {
    vector<string> localVarNames = func->local_vars_;
    vector<string>::iterator it = find(localVarNames.begin(), localVarNames.end(), name);
    if (it != localVarNames.end()) {
        int index = it - localVarNames.begin();
        return index;
    }
    throw RuntimeException("can't find local variable " + name);
}

shared_ptr<Constant> Frame::getLocalVar(int index, string name) {
    if (index < 0 || index >= localVars.capacity()) {
        throw RuntimeException("local var " + to_string(index) + " out of bounds");
    }
    shared_ptr<Constant> result = localVars[index]->ptr;
    if (result == NULL) {
        throw UninitializedVariableException(name + " is not defined");
    }
    return result;
}

shared_ptr<ValuePtr> Frame::getRefVar(int index) {
    if (index < 0 || index >= localRefs.capacity()) {
        throw RuntimeException("local ref " + to_string(index) + " out of bounds");
    }
    return localRefs[index];
}

void Frame::setLocalVar(int index, shared_ptr<Constant> val) {
    if (index < 0 || index >= localVars.capacity()) {
        throw RuntimeException("local var " + to_string(index) + " out of bounds");
    }
    localVars[index]->ptr = val;
}

void Frame::setRefVar(int index, shared_ptr<ValuePtr> val) {
    if (index < 0 || index >= localRefs.capacity()) {
        throw RuntimeException("local ref " + to_string(index) + " out of bounds");
    }
    localRefs[index] = val;
}

shared_ptr<ValuePtr> Frame::getRefToLocal(string name) {
    vector<string> localVarNames = func->local_vars_;
    auto it = find(localVarNames.begin(), localVarNames.end(), name);
    if (it != localVarNames.end()) {
        int index = it - localVarNames.begin();
        return localVars[index];
    }
    throw RuntimeException("can't find local variable for reference to " + name);
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
