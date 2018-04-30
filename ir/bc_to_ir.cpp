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
    for (int i = 0; i < func->instructions.size(); i++) {
		TempList temp;
		Instruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case Operation::LoadConst:
	            {
					temp.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadConst, inst.operand0, optstr_t(), temp));
					currentTemp++;
	                break;
	            }
	        case Operation::LoadFunc:
	            {
					temp.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadFunc, inst.operand0, optstr_t(), temp));
					currentTemp++;
	                break;
	            }
	        case Operation::LoadLocal:
	            {
					temp.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadLocal, inst.operand0, optstr_t(), temp));
					currentTemp++;
	                break;
	            }
	        case Operation::StoreLocal:
	            {
					temp.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::StoreLocal, inst.operand0, optstr_t(), temp));
					currentTemp--;
	                break;
	            }
	        case Operation::LoadGlobal:
	            {
					temp.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadGlobal, inst.operand0, optstr_t(), temp));
					currentTemp++;
	                break;
	            }
	        case Operation::StoreGlobal:
	            {
					temp.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::StoreGlobal, inst.operand0, optstr_t(), temp));
					currentTemp--;
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
