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

typedef vector<shared_ptr<ValuePtr>> LocalVars;
typedef vector<shared_ptr<ValuePtr>> LocalRefs;

class Frame {
    // Class representing a stack frame in interpreter execution

    // vector of local variable names to values
    // local_vars[i] = func.local_vars_[i]
    LocalVars localVars;
    // vector of local reference variable names to values
    // local_vars[i] = func.free_vars_[i] if i < s
    LocalRefs localRefs;

public:
    // function that the frame is for
    std::shared_ptr<Function> func;
    // operand stack
    stack<std::shared_ptr<Value>> opStack;
    // index of current instruction in func's instructions list
    int instructionIndex = 0;

    Frame(shared_ptr<Function> func, int numLocals, int numRefs):
        func(func), instructionIndex(0) {
            localVars.reserve(numLocals);
            for (int i = 0; i < numLocals; i++) {
                localVars.push_back(make_shared<ValuePtr>());
            }
            localRefs.reserve(numRefs);
            for (int i = 0; i < numRefs; i++) {
                localRefs.push_back(make_shared<ValuePtr>());
            }
	};

    // instruction helpers
    int numInstructions();
    Instruction& getCurrInstruction();
    void moveToInstruction(int offset);

    // function value helpers
    int getLocalIndex(string name);
    shared_ptr<Constant> getConstantByIndex(int index);
    shared_ptr<Function> getFunctionByIndex(int index);
    string getNameByIndex(int index);
    string getRefVarByIndex(int index);
    shared_ptr<ValuePtr> getRefToLocal(string name);

    // var map helpers
    shared_ptr<Constant> getLocalVar(int index, string name);
    shared_ptr<ValuePtr> getRefVar(int index);

    void setLocalVar(int index, shared_ptr<Constant> val);
    void setRefVar(int index, shared_ptr<ValuePtr> val);

    // operand stack helpers
    void opStackPush(std::shared_ptr<Value> val);
    std::shared_ptr<Value> opStackPeek();
    std::shared_ptr<Value> opStackPop();
};
