/*
 * bc_to_ir.cpp
 *
 * Convert bytecode to IR representation
 */
#include "../types.h"
#include "../ir.h"
#include "bc_to_ir.h"
#include <algorithm>
#include <stack>

using namespace std;

class Interpreter;

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
					temps.push_back(temp);
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
					temps.push_back(temp);
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
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::Add, inst.operand0, temps));
	                break;
	            }
	        case Operation::Sub:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::Sub, inst.operand0, temps));
	                break;
	            }
	        case Operation::Mul:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::Mul, inst.operand0, temps));
	                break;
	            }
	        case Operation::Div:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::Div, inst.operand0, temps));
	                break;
	            }
	        case Operation::Neg:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, temps));
					irInsts.push_back(IrInstruction(IrOp::Neg, inst.operand0, temps));
	                break;
	            }
	        case Operation::Gt:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::Gt, inst.operand0, temps));
	                break;
	            }
	        case Operation::Geq:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertInteger, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::Geq, inst.operand0, temps));
	                break;
	            }
	        case Operation::Eq:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::Eq, inst.operand0, temps));
	                break;
	            }
	        case Operation::And:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertBool, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertBool, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::And, inst.operand0, temps));
	                break;
	            }
	        case Operation::Or:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp1.stackOffset);
					temps.push_back(temp2.stackOffset);
					TempList first;
					first.push_back(temp1.stackOffset);
					TempList second;
					second.push_back(temp2.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertBool, inst.operand0, first));
					irInsts.push_back(IrInstruction(IrOp::AssertBool, inst.operand0, second));
					irInsts.push_back(IrInstruction(IrOp::Or, inst.operand0, temps));
	                break;
	            }
	        case Operation::Not:
	            {
					Temp temp = tempStack.top();
					tempStack.pop();
					temps.push_back(currentTemp);
					currentTemp++;
					temps.push_back(temp.stackOffset);
					irInsts.push_back(IrInstruction(IrOp::AssertBool, inst.operand0, temps));
					irInsts.push_back(IrInstruction(IrOp::Not, inst.operand0, temps));
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
					tempStack.push(Temp(tempStack.top()));
	                break;
	            }
	        case Operation::Swap:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					tempStack.push(temp1);
					tempStack.push(temp2);
	                break;
	            }
	        case Operation::Pop:
	            {
					tempStack.pop();
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
