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
    // class used to handle interpreter state
private:
    Interpreter* vmPointer;
    Function* func;  // root function
	vector<IrFunc*> irFuncs;
	stack<Temp> tempStack;
	IrInstList irInsts;

    void pushTemp(temp_t index) {
        tempStack.push(Temp(index));
    }

    void pushTemp(Temp temp) {
        tempStack.push(temp);
    }

    Temp popTemp() {
		Temp temp = tempStack.top();
        tempStack.pop();
    }

    void pushInstruction(IrInstruction inst) {
        irInsts.push_back(inst);
    }
public:
    IrCompiler(Function* mainFunc, Interpreter* vmInterpreterPointer):
        func(mainFunc),
        vmPointer(vmInterpreterPointer) {};
	IrFunc toIr();
	IrFunc toIrFunc(Function*);
};
