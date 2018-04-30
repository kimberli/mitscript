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
		BcInstruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case BcOp::LoadConst:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					currentTemp++;
					irInsts.push_back(IrInstruction(IrOp::LoadConst, inst.operand0, temps));
	                break;
	            }
	        case BcOp::LoadFunc:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					currentTemp++;
					irInsts.push_back(IrInstruction(IrOp::LoadFunc, inst.operand0, temps));
	                break;
	            }
	        case BcOp::LoadLocal:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadLocal, inst.operand0, temps));
					currentTemp++;
	                break;
	            }
	        case BcOp::StoreLocal:
	            {
					Temp temp = tempStack.top();
					tempStack.pop();
					temps.push_back(temp);
					irInsts.push_back(IrInstruction(IrOp::StoreLocal, inst.operand0, temps));
	                break;
	            }
	        case BcOp::LoadGlobal:
	            {
					tempStack.push(Temp(currentTemp));
					temps.push_back(currentTemp);
					irInsts.push_back(IrInstruction(IrOp::LoadGlobal, inst.operand0, temps));
					currentTemp++;
	                break;
	            }
	        case BcOp::StoreGlobal:
	            {
					Temp temp = tempStack.top();
					tempStack.pop();
					temps.push_back(temp);
					irInsts.push_back(IrInstruction(IrOp::StoreGlobal, inst.operand0, temps));
	                break;
	            }
	        case BcOp::PushReference:
	            {
	                break;
	            }
	        case BcOp::LoadReference:
	            {
	                break;
	            }
	        case BcOp::AllocRecord:
	            {
	                break;
	            }
	        case BcOp::FieldLoad:
	            {
	                break;
	            }
	        case BcOp::FieldStore:
	            {
	                break;
	            }
	        case BcOp::IndexLoad:
	            {
	                break;
	            }
	        case BcOp::IndexStore:
	            {
	                break;
	            }
	        case BcOp::AllocClosure:
	            {
	                break;
	            }
	        case BcOp::Call:
	            {
	                break;
	            }
	        case BcOp::Return:
	            {
	                break;
	            }
	        case BcOp::Add:
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
	        case BcOp::Sub:
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
	        case BcOp::Mul:
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
	        case BcOp::Div:
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
	        case BcOp::Neg:
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
	        case BcOp::Gt:
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
	        case BcOp::Geq:
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
	        case BcOp::Eq:
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
	        case BcOp::And:
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
	        case BcOp::Or:
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
	        case BcOp::Not:
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
	        case BcOp::Goto:
	            {
	                break;
	            }
	        case BcOp::If:
	            {
	                break;
	            }
	        case BcOp::Dup:
	            {
					tempStack.push(Temp(tempStack.top()));
	                break;
	            }
	        case BcOp::Swap:
	            {
					Temp temp1 = tempStack.top();
					tempStack.pop();
					Temp temp2 = tempStack.top();
					tempStack.pop();
					tempStack.push(temp1);
					tempStack.push(temp2);
	                break;
	            }
	        case BcOp::Pop:
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
