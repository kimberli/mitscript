/*
 * bc_to_ir.cpp
 *
 * Convert bytecode to IR representation
 */
#include "../types.h"
#include "../ir.h"
#include "bc_to_ir.h"

using namespace std;

class Interpreter;

IrFunc IrCompiler::toIrFunc(Function* func) {
    func = func;
    tempStack = stack<Temp>();  // are we gonna garbage collect this?
    irInsts = IrInstList();
	temp_t currentTemp = 0;

    for (int i = 0; i < func->instructions.size(); i++) {
		BcInstruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case BcOp::LoadConst:
	            {
                    pushTemp(currentTemp);
					pushInstruction(IrInstruction(IrOp::LoadConst, inst.operand0, currentTemp));
					currentTemp++;
	                break;
	            }
	        case BcOp::LoadFunc:
	            {
                    pushTemp(currentTemp);
					pushInstruction(IrInstruction(IrOp::LoadFunc, inst.operand0, currentTemp));
					currentTemp++;
	                break;
	            }
	        case BcOp::LoadLocal:
	            {
                    pushTemp(currentTemp);
					pushInstruction(IrInstruction(IrOp::LoadLocal, inst.operand0, currentTemp));
					currentTemp++;
	                break;
	            }
	        case BcOp::LoadGlobal:
	            {
                    pushTemp(currentTemp);
                    
                    optstr_t global = func->names_[inst.operand0.value()];
					pushInstruction(IrInstruction(IrOp::LoadGlobal, global, currentTemp));
					currentTemp++;
	                break;
	            }
	        case BcOp::StoreLocal:
	            {
					pushInstruction(IrInstruction(IrOp::StoreLocal, inst.operand0, popTemp()));
	                break;
	            }
	        case BcOp::StoreGlobal:
	            {
                    optstr_t global = func->names_[inst.operand0.value()];
					pushInstruction(IrInstruction(IrOp::StoreGlobal, global, popTemp()));
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
                    pushInstruction(IrInstruction(IrOp::Return, popTemp()));
	                break;
	            }
	        case BcOp::Add:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::Add, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Sub:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::Sub, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Mul:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::Mul, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Div:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::Div, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Neg:
	            {
                    Temp val = popTemp();
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, val));
					pushInstruction(IrInstruction(IrOp::Neg, inst.operand0, val));
					currentTemp++;
	                break;
	            }
	        case BcOp::Gt:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::Gt, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Geq:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::Geq, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Eq:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::Eq, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::And:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertBool, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertBool, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::And, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Or:
	            {
                    Temp tempRight = popTemp();
                    Temp tempLeft = popTemp();
                    TempList instTemps{currentTemp, tempRight, tempLeft};
					pushInstruction(IrInstruction(IrOp::AssertBool, inst.operand0, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertBool, inst.operand0, tempLeft));
					pushInstruction(IrInstruction(IrOp::Or, inst.operand0, instTemps));
					currentTemp++;
	                break;
	            }
	        case BcOp::Not:
	            {
                    Temp val = popTemp();
					pushInstruction(IrInstruction(IrOp::AssertBool, inst.operand0, val));
					pushInstruction(IrInstruction(IrOp::Not, inst.operand0, val));
					currentTemp++;
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
                    pushTemp(tempStack.top());
	                break;
	            }
	        case BcOp::Swap:
	            {
                    Temp temp1 = popTemp();
                    Temp temp2 = popTemp();
                    pushTemp(temp1);
                    pushTemp(temp2);
	                break;
	            }
	        case BcOp::Pop:
	            {
                    popTemp();
	                break;
	            }
	        default:
	            throw RuntimeException("should never get here - invalid instruction");
	    }
	}
	IrFunc irFunc = IrFunc(irInsts, func->constants_, func->parameter_count_, func->local_vars_.size());
    return irFunc;
};

IrFunc IrCompiler::toIr() {
	return toIrFunc(func);
};
