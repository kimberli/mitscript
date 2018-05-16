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
#include <map>
#include <set>

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
    vector<vector<tempptr_t>> localTemps;
    vector<tempptr_t> otherTemps;
	int whileLevel = 0;
	set<tempptr_t> tempsInWhile;

    // helpers
    tempptr_t getNewTemp();
    tempptr_t getNewLocalTemp(int i);
    tempptr_t loadLocalTemp(int i);
    void pushTemp(tempptr_t temp);
    tempptr_t popTemp();
    void pushInstruction(instptr_t inst);
    void doUnaryArithmetic(IrOp operation, bool toBoolean);
    void doBinaryArithmetic(IrOp operation, bool fromBoolean, bool toBoolean);
	void checkIfUsed(tempptr_t temp);
public:
    // vector of booleans corresponding to whether the local in the 
    // corresponding index is a local ref var or not
    vector<bool> isLocalRef;
    IrCompiler(Function* mainFunc, Interpreter* vmInterpreterPointer):
        func(mainFunc),
        vmPointer(vmInterpreterPointer) {
        for (int i = 0; i < mainFunc->local_vars_.size(); i++) {
			vector<tempptr_t> local;	
            localTemps.push_back(local);
        }
		currentTemp = localTemps.size();
    };
	IrFunc toIr();
	IrFunc toIrFunc(Function*);
};
