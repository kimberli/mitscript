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

    // helpers
    tempptr_t pushNewTemp();
    void pushTemp(tempptr_t temp);
    tempptr_t popTemp();
    void pushInstruction(IrInstruction inst);
public:
    IrCompiler(Function* mainFunc, Interpreter* vmInterpreterPointer):
        func(mainFunc),
        vmPointer(vmInterpreterPointer) {};
	IrFunc toIr();
	IrFunc toIrFunc(Function*);
};
