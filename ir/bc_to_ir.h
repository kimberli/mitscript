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

class IrCompiler {
    // class used to handle interpreter state
private:
    vptr<Function> globalFunc;  // root function
	vector<vptr<IrFunc>> irFuncs;
public:
    IrCompiler(vptr<Function> mainFunc);
	IrProgram toIr();
	void toIrFunc(vptr<Function>);
};
