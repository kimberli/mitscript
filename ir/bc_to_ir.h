/*
 * bc_to_ir.h
 *
 * Defines an IRCompiler object which can convert bytecode instructions to
 * IR representation
 */
#pragma once

#include "../ir.h"
#include <list>
#include <iostream>
#include <map>
#include <stack>
#include <vector>

using namespace std;

class Interpreter;

class IrCompiler {
    // class used to handle ir state per function
private:
    Interpreter* vmPointer;
    Function* func;  // current function
	vector<IrFunc*> irFuncs;
	stack<tempptr_t> tempStack;
	IrInstList irInsts;
	offset_t currentTemp = 0;
    map<int32_t, int32_t> labelOffsets;  // map of string label to remaining BcInstruction offset
    int32_t labelCounter;  // initial label index for this instance of IrCompiler

    // helpers
    tempptr_t pushNewTemp();
    void pushTemp(tempptr_t temp);
    tempptr_t popTemp();
    void pushInstruction(IrInstruction inst);
    void decLabelOffsets();
    int32_t addLabelOffset(int32_t offset);
public:
    IrCompiler(Function* mainFunc, int32_t labelCounter, Interpreter* vmInterpreterPointer):
        func(mainFunc),
        labelCounter(labelCounter),
        vmPointer(vmInterpreterPointer) {};
	IrFunc toIr();
	IrFunc toIrFunc(Function*);
};
