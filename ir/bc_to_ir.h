/*
 * bc-to-ir.h
 *
 * Defines an IRCompiler object which can convert bytecode instructions to
 * IR representation
 */
#pragma once

#include "ir.h"
#include <list>
#include <iostream>

using namespace std;

class IrCompiler {
    // class used to handle interpreter state
private:
    vptr<Function> globalFunc;  // root function
public:
    IrCompiler(vptr<Function> mainFunc);
	IrProgram toIr();
	IrFunc toIrFunc();
};
