/*
 * bc_to_ir.cpp
 *
 * Convert bytecode to IR representation
 */
#include "../vm/interpreter.h"
#include "../types.h"
#include "../ir.h"
#include "bc_to_ir.h"
#include <algorithm>
#include <stack>

using namespace std;

IrCompiler::IrCompiler(vptr<Function> mainFunc, vptr<Interpreter> vmInterpreterPointer) {
    vmPointer = vmInterpreterPointer;
	func = mainFunc;
};

IrFunc IrCompiler::toIrFunc(vptr<Function> func) {
	IrInstList irInsts;
	int32_t currentTemp = 0;
	stack<Temp> tempStack;
    for (int i = 0; i < func->instructions.size(); i++) {
		TempList temps;
		Instruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case Operation::LoadConst:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					currentTemp++;
					irInsts.push_back(IrInstruction(IrOp::LoadConst, inst.operand0, temps));
	                break;
	            }
	        case Operation::LoadFunc:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					currentTemp++;
					irInsts.push_back(IrInstruction(IrOp::LoadFunc, inst.operand0, temps));
	                break;
	            }
	        case Operation::LoadLocal:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadLocal, inst.operand0, temps));
					currentTemp++;
	                break;
	            }
	        case Operation::StoreLocal:
	            {
					Temp temp = tempStack.top();
					tempStack.pop();
					temps.push_back(temp.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::StoreLocal, inst.operand0, temps));
	                break;
	            }
	        case Operation::LoadGlobal:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadGlobal, inst.operand0, temps));
					currentTemp++;
	                break;
	            }
	        case Operation::StoreGlobal:
	            {
					Temp temp = tempStack.top();
					tempStack.pop();
					temps.push_back(temp.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::StoreGlobal, inst.operand0, temps));
	                break;
	            }
	        case Operation::PushReference:
	            {
	                break;
	            }
	        case Operation::LoadReference:
	            {
	                break;
	            }
	        case Operation::AllocRecord:
	            {
	                break;
	            }
	        case Operation::FieldLoad:
	            {
	                break;
	            }
	        case Operation::FieldStore:
	            {
	                break;
	            }
	        case Operation::IndexLoad:
	            {
	                break;
	            }
	        case Operation::IndexStore:
	            {
	                break;
	            }
	        case Operation::AllocClosure:
	            {
	                break;
	            }
	        case Operation::Call:
	            {
	                break;
	            }
	        case Operation::Return:
	            {
	                break;
	            }
	        case Operation::Add:
	            {
	                break;
	            }
	        case Operation::Sub:
	            {
	                break;
	            }
	        case Operation::Mul:
	            {
	                break;
	            }
	        case Operation::Div:
	            {
	                break;
	            }
	        case Operation::Neg:
	            {
	                break;
	            }
	        case Operation::Gt:
	            {
	                break;
	            }
	        case Operation::Geq:
	            {
	                break;
	            }
	        case Operation::Eq:
	            {
	                break;
	            }
	        case Operation::And:
	            {
	                break;
	            }
	        case Operation::Or:
	            {
	                break;
	            }
	        case Operation::Not:
	            {
	                break;
	            }
	        case Operation::Goto:
	            {
	                break;
	            }
	        case Operation::If:
	            {
	                break;
	            }
	        case Operation::Dup:
	            {
	                break;
	            }
	        case Operation::Swap:
	            {
	                break;
	            }
	        case Operation::Pop:
	            {
	                break;
	            }
	        default:
	            throw RuntimeException("should never get here - invalid instruction");
	    }
	}
	IrFunc irFunc = IrFunc(irInsts, func->constants_, func->parameter_count_, func->local_vars_.size());
	//irFuncs.push_back(irFunc);
	//for (vptr<Function> f: func->functions_) {
	//	toIrFunc(f);
	//}
    return irFunc;
};

IrFunc IrCompiler::toIr() {
	//irFuncs = vector<vptr<IrFunc>>();
	return toIrFunc(func);
	//return IrProgram(f);
};
