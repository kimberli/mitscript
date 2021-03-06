/*
 * frame.h
 *
 * Defines the Frame class used in the interpreter which stores information
 * about the current function, instruction location, local variables and
 * reference variables, as well as the operand stack.
 */
#pragma once

#include "types.h"
#include "gc/gc.h"
#include <string>
#include <map>
#include <list>

using namespace std;

typedef map<string, ValWrapper*> VarMap;

class Frame : public Collectable {
    // Class representing a stack frame in interpreter execution

    // and local reference names to shared ValWrappers
    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
public:
    // vector of local variable names to values (stored in ValWrapper)
    VarMap vars;
    // function that the frame is for
    Function* func;
    // operand stack
    list<tagptr_t> opStack;
    // index of current instruction in func's instructions list
    int instructionIndex = 0;
    // garbage collector
    CollectedHeap* collector;
    // offset to keep track of stuff
    int offset = 0;

    Frame(Function* func): func(func) {};

    virtual ~Frame() {}

    // instruction helpers
    int numInstructions();
    BcInstruction& getCurrInstruction();
    void checkLegalInstruction();

    // function value helpers
    tagptr_t getConstantByIndex(int index);
    Function* getFunctionByIndex(int index);
    string getLocalByIndex(int index);
    string getNameByIndex(int index);
    string getRefByIndex(int index);

    // var map helpers
    tagptr_t getLocalVar(string name);
    ValWrapper* getRefVar(string name);

    void setLocalVar(string name, tagptr_t val);
    void setRefVar(string name, tagptr_t val);

    // operand stack helpers
    void opStackPush(tagptr_t val);
    tagptr_t opStackPeek();
    tagptr_t opStackPop();
};
