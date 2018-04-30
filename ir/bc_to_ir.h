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
#include <vector>

using namespace std;

class Interpreter;

class IrCompiler {
    // class used to handle interpreter state
private:
    Interpreter* vmPointer;
    Function* func;  // root function
	vector<IrFunc*> irFuncs;
public:
    IrCompiler(Function* mainFunc, Interpreter* vmInterpreterPointer);
	IrFunc toIr();
	IrFunc toIrFunc(Function*);
};
