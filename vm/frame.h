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
        func(func), localVars(localVars), localRefs(localRefs) {
	};

    // instruction helpers
    int numInstructions();
    Instruction& getCurrInstruction();
    void moveToInstruction(int offset);

    // function value helpers
    shared_ptr<Constant> getConstantByIndex(int index);
    shared_ptr<Function> getFunctionByIndex(int index);
    string getLocalVarByIndex(int index);
    string getNameByIndex(int index);
    string getRefVarByIndex(int index);

    // var map helpers
    shared_ptr<Constant> getLocalVar(string name);
    shared_ptr<ValuePtr> getRefVar(string name);

    void setLocalVar(string name, shared_ptr<Constant> val);
    void setRefVar(string name, shared_ptr<ValuePtr> val);

    // operand stack helpers
    void opStackPush(std::shared_ptr<Value> val);
    std::shared_ptr<Value> opStackPeek();
    std::shared_ptr<Value> opStackPop();
};
