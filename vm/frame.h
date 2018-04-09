/*
 * frame.h
 *
 * Defines the Frame class used in the interpreter which stores information
 * about the current function, instruction location, local variables and
 * reference variables, as well as the operand stack.
 */
#pragma once

#include "types.h"
#include <string>
#include <map>
#include <stack>

using namespace std;

// TODO: change these to vectors
typedef map<string, std::shared_ptr<Constant>> LocalVarMap;
typedef map<string, std::shared_ptr<ValuePtr>> LocalRefMap;

class Frame {
    // Class representing a stack frame in interpreter execution

    // map of local variable names to values
    LocalVarMap localVars;
    // map of local reference variable names to values
    LocalRefMap localRefs;

public:
    // function that the frame is for
    std::shared_ptr<Function> func;
    // operand stack
    stack<std::shared_ptr<Value>> opStack;
    // index of current instruction in func's instructions list
    int instructionIndex = 0;

    Frame(std::shared_ptr<Function> func): func(func) {};
    Frame(std::shared_ptr<Function> func, LocalVarMap& localVars, LocalRefMap& localRefs):
        func(func), localVars(localVars), localRefs(localRefs) {};

    // instruction helpers
    int numInstructions() {
        return func->instructions.size();
    }

    Instruction& getCurrInstruction() {
        return func->instructions[instructionIndex];
    }

    void moveToInstruction(int offset) {
        int newOffset = instructionIndex + offset;
        if (newOffset < 0 || newOffset >= func->instructions.size()) {
            throw RuntimeException("instruction " + std::to_string(instructionIndex + newOffset) + " out of bounds");
        }
        instructionIndex += offset;
    }

    // function value helpers
    shared_ptr<Constant> getConstantByIndex(int index) {
        if (index < 0 || index >= func->constants_.size()) {
            throw RuntimeException("constant " + std::to_string(index) + " out of bounds");
        }
        return func->constants_[index];
    }

    shared_ptr<Function> getFunctionByIndex(int index) {
        if (index < 0 || index >= func->functions_.size()) {
            throw RuntimeException("function " + std::to_string(index) + " out of bounds");
        }
        return func->functions_[index];
    }

    string getLocalVarByIndex(int index) {
        if (index < 0 || index >= func->local_vars_.size()) {
            throw RuntimeException("local var " + std::to_string(index) + " out of bounds");
        }
        return func->local_vars_[index];
    }

    string getNameByIndex(int index) {
        if (index < 0 || index >= func->names_.size()) {
            throw RuntimeException("name " + std::to_string(index) + " out of bounds");
        }
        return func->names_[index];
    }

    string getRefVarByIndex(int index) {
        if (index < 0 || index >= (func->local_reference_vars_.size() + func->free_vars_.size())) {
            throw RuntimeException("name " + std::to_string(index) + " out of bounds");
        }
		if (index < func->local_reference_vars_.size()) {
        	return func->local_reference_vars_[index];
		}
		return func->free_vars_[index - func->local_reference_vars_.size()];
    }

    // var map helpers
    shared_ptr<Constant> getLocalVar(string name) {
        if (localVars.count(name) == 0) {
            throw UninitializedVariableException(name + " is not initialized");
        }
        return localVars[name];
    }

    shared_ptr<ValuePtr> getRefVar(string name) {
        if (localRefs.count(name) == 0) {
            throw UninitializedVariableException(name + " is not initialized");
        }
        return localRefs[name];
    }

    void setLocalVar(string name, shared_ptr<Constant> val) {
        localVars[name] = val;
		localRefs[name] = make_shared<ValuePtr>(localVars[name]);
    }

    void setRefVar(string name, shared_ptr<ValuePtr> val) {
        localRefs[name] = val;
    }

    // operand stack helpers
    void opStackPush(std::shared_ptr<Value> val) {
        opStack.push(val);
    }

    std::shared_ptr<Value> opStackPeek() {
        if (opStack.empty()) {
            throw InsufficientStackException("peek at empty stack");
        }
        return opStack.top();
    }

    std::shared_ptr<Value> opStackPop() {
        if (opStack.empty()) {
            throw InsufficientStackException("pop from empty stack");
        }
        std::shared_ptr<Value> top = opStack.top();
        opStack.pop();
        return top;
    }
};
