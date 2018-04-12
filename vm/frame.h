/*
 * frame.h
 *
 * Defines the Frame class used in the interpreter which stores information
 * about the current function, instruction location, local variables and
 * reference variables, as well as the operand stack.
 */
#pragma once

#include "types.h"
#include "../gc/gc.h"
#include <string>
#include <map>
#include <stack>

using namespace std;

typedef map<string, ValuePtr*> VarMap;

class Frame : public Collectable {
    // Class representing a stack frame in interpreter execution

    // vector of local variable names to values (stored in ValuePtr)
    // and local reference names to shared ValuePtrs
    VarMap vars;
    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
public:
    // function that the frame is for
    Function* func;
    // operand stack
    stack<Value*> opStack;
    // index of current instruction in func's instructions list
    int instructionIndex = 0;

    Frame(Function* func): func(func) {
	};

    // instruction helpers
    int numInstructions();
    Instruction& getCurrInstruction();
    void moveToInstruction(int offset);

    // function value helpers
    Constant* getConstantByIndex(int index);
    Function* getFunctionByIndex(int index);
    string getLocalByIndex(int index);
    string getNameByIndex(int index);
    string getRefByIndex(int index);

    // var map helpers
    Constant* getLocalVar(string name);
    ValuePtr* getRefVar(string name);

    void setLocalVar(string name, Constant* val);
    void setRefVar(string name, ValuePtr* val);

    // operand stack helpers
    void opStackPush(Value* val);
    Value* opStackPeek();
    Value* opStackPop();
};
