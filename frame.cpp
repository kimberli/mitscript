#include "frame.h"
#include "gc/gc.h"

#include <algorithm>

using namespace std;

// instruction helpers
int Frame::numInstructions() {
    return func->instructions.size();
}

BcInstruction& Frame::getCurrInstruction() {
    return func->instructions[instructionIndex];
}

void Frame::checkLegalInstruction() {
    if (instructionIndex < 0 || instructionIndex >= numInstructions()) {
        throw RuntimeException("instruction " + to_string(instructionIndex) + " out of bounds");
    }
}

// function value helpers
tagptr_t Frame::getConstantByIndex(int index) {
    if (index < 0 || index >= func->constants_.size()) {
        throw RuntimeException("constant " + to_string(index) + " out of bounds");
    }
    return func->constants_[index];
}

Function* Frame::getFunctionByIndex(int index) {
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
tagptr_t Frame::getLocalVar(string name) {
    if (vars.count(name) != 0) {
        tagptr_t result = vars[name]->ptr;
        if (result == NULL_PTR) {
            throw UninitializedVariableException(name + " is not defined");
        }
        return result;
    }
    throw UninitializedVariableException(name + " is not defined");
}

ValWrapper* Frame::getRefVar(string name) {
    if (vars.count(name) != 0) {
        return vars[name];
    }
    throw RuntimeException(name + " has not been created in its frame's vars");
}

void Frame::setLocalVar(string name, tagptr_t val) {
    if (vars.count(name) == 0) {
        vars[name] = collector->allocate<ValWrapper>(val);
        collector->increment(sizeof(name) + name.size() + sizeof(val));
    } else {
        vars[name]->ptr = val;
    }
}

void Frame::setRefVar(string name, tagptr_t val) {
    if (vars.count(name) == 0) {
        collector->increment(sizeof(name) + name.size() + sizeof(val));
    }
    ValWrapper* v = cast_val<ValWrapper>(val);
    vars[name] = v;
}

// operand stack helpers
void Frame::opStackPush(tagptr_t val) {
    collector->increment(sizeof(val));
    opStack.push_back(val);
}

tagptr_t Frame::opStackPeek() {
    if (opStack.empty()) {
        throw InsufficientStackException("peek at empty stack");
    }
    return opStack.back();
}

tagptr_t Frame::opStackPop() {
    if (opStack.empty()) {
        throw InsufficientStackException("pop from empty stack");
    }
    tagptr_t top = opStack.back();
    int size = sizeof(top);
    collector->increment(-size);
    opStack.pop_back();
    return top;
}

void Frame::follow(CollectedHeap& heap) {
    // follow the function it contains
    // As well as all the stuff on the op stack?
    heap.markSuccessors(func);
    for (tagptr_t v : opStack) {
        if (!is_tagged(v)) {
            heap.markSuccessors(get_collectable(v));
        }
    }
    for (Collectable* v : collectables) {
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
