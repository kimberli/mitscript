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

typedef map<string, vptr<ValuePtr>> VarMap;

class Frame : public Collectable {
    // Class representing a stack frame in interpreter execution

    // vector of local variable names to values (stored in ValuePtr)
    // and local reference names to shared ValuePtrs
    VarMap vars;
    void follow(CollectedHeap& heap) override;
public:
    // function that the frame is for
    vptr<Function> func;
    // operand stack
    stack<vptr<Value>> opStack;
    // index of current instruction in func's instructions list
    int instructionIndex = 0;

    Frame(vptr<Function> func): func(func) {
	};

    // instruction helpers
    int numInstructions();
    Instruction& getCurrInstruction();
    void moveToInstruction(int offset);

    // function value helpers
    vptr<Constant> getConstantByIndex(int index);
    vptr<Function> getFunctionByIndex(int index);
    string getLocalByIndex(int index);
    string getNameByIndex(int index);
    string getRefByIndex(int index);

    // var map helpers
    vptr<Constant> getLocalVar(string name);
    vptr<ValuePtr> getRefVar(string name);

    void setLocalVar(string name, vptr<Constant> val);
    void setRefVar(string name, vptr<ValuePtr> val);

    // operand stack helpers
    void opStackPush(vptr<Value> val);
    vptr<Value> opStackPeek();
    vptr<Value> opStackPop();
};
