#include "frame.h"
#include "../gc/gc.h"

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
vptr<Constant> Frame::getConstantByIndex(int index) {
    if (index < 0 || index >= func->constants_.size()) {
        throw RuntimeException("constant " + to_string(index) + " out of bounds");
    }
    return func->constants_[index];
}

vptr<Function> Frame::getFunctionByIndex(int index) {
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
vptr<Constant> Frame::getLocalVar(string name) {
    if (vars.count(name) != 0) {
        vptr<Constant> result = vars[name]->ptr;
        if (result == NULL) {
            throw UninitializedVariableException(name + " is not defined");
        }
        return result;
    }
    throw UninitializedVariableException(name + " is not defined");
}

vptr<ValWrapper> Frame::getRefVar(string name) {
    if (vars.count(name) != 0) {
        return vars[name];
    }
    throw RuntimeException(name + " has not been created in its frame's vars");
}

void Frame::setLocalVar(string name, vptr<Constant> val) {
    if (vars.count(name) == 0) {
        vars[name] = collector->allocate<ValWrapper>(val);
        collector->increment(sizeof(name) + name.size() + sizeof(val));
    } else {
        vars[name]->ptr = val;
    }
}

void Frame::setRefVar(string name, vptr<ValWrapper> val) {
    if (vars.count(name) == 0) {
        collector->increment(sizeof(name) + name.size() + sizeof(val));
    }
    vars[name] = val;
}

// operand stack helpers
void Frame::opStackPush(vptr<Value> val) {
    collector->increment(sizeof(val));
    opStack.push_back(val);
}

vptr<Value> Frame::opStackPeek() {
    if (opStack.empty()) {
        throw InsufficientStackException("peek at empty stack");
    }
    return opStack.back();
}

vptr<Value> Frame::opStackPop() {
    if (opStack.empty()) {
        throw InsufficientStackException("pop from empty stack");
    }
    vptr<Value> top = opStack.back();
    int size = sizeof(top);
    collector->increment(-size);
    opStack.pop_back();
    return top;
}

void Frame::follow(CollectedHeap& heap) {
    // follow the function it contains
    // As well as all the stuff on the op stack?
    heap.markSuccessors(func);
    for (Collectable* v : opStack) {
        heap.markSuccessors(v);
    }
   	for (string arg : func->local_vars_) {
		if (vars.count(arg) != 0) {
			Collectable* localVar = vars[arg];
   	    	heap.markSuccessors(localVar); }
   	}
   	for (string arg : func->local_reference_vars_) {
		if (vars.count(arg) != 0) {
			Collectable* localRef = vars[arg];
   	    	heap.markSuccessors(localRef);
		}
   	}
}

size_t Frame::getSize() {
    size_t overhead = sizeof(Frame);
    size_t stackSize = getStackSize(opStack);
    size_t varsSize = getMapSize(vars);
    return overhead + stackSize + varsSize;
}
