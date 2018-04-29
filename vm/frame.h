/*
 * frame.h
 *
 * Defines the Frame class used in the interpreter which stores information
 * about the current function, instruction location, local variables and
 * reference variables, as well as the operand stack.
 */
#pragma once

#include "../types.h"
#include "../gc/gc.h"
#include <string>
#include <map>
#include <list>

using namespace std;

typedef map<string, vptr<ValWrapper>> VarMap;
typedef Frame* fptr;

class Frame : public Collectable {
    // Class representing a stack frame in interpreter execution

    // and local reference names to shared ValWrappers
    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
public:
    // vector of local variable names to values (stored in ValWrapper)
    VarMap vars;
    // function that the frame is for
    vptr<Function> func;
    // operand stack
    list<vptr<Value>> opStack;
    // index of current instruction in func's instructions list
    int instructionIndex = 0;
    // garbage collector
    CollectedHeap* collector;

    Frame(vptr<Function> func): func(func) {};

    virtual ~Frame() {}

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
    vptr<ValWrapper> getRefVar(string name);

    void setLocalVar(string name, vptr<Constant> val);
    void setRefVar(string name, vptr<ValWrapper> val);

    // operand stack helpers
    void opStackPush(vptr<Value> val);
    vptr<Value> opStackPeek();
    vptr<Value> opStackPop();

    void opStackPrint() {
        for (Value* v : opStack) {
            LOG(v->toString());
        }
    }
};
