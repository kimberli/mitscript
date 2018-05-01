/*
 * bc_to_ir.cpp
 *
 * Convert bytecode to IR representation
 */
#include "../types.h"
#include "../ir.h"
#include "bc_to_ir.h"
#include <algorithm>

using namespace std;

class Interpreter;

// Helpers
tempptr_t IrCompiler::pushNewTemp() {
    tempptr_t newTemp = make_shared<Temp>(currentTemp);
    tempStack.push(newTemp);
    currentTemp++;
    return newTemp;
}
void IrCompiler::pushTemp(tempptr_t temp) {
    tempStack.push(temp);
}
tempptr_t IrCompiler::popTemp() {
    tempptr_t temp = tempStack.top();
    tempStack.pop();
    return temp;
}
void IrCompiler::pushInstruction(IrInstruction inst) {
    irInsts.push_back(inst);
}
void IrCompiler::decLabelOffsets() {
    for (auto item : labelOffsets) {
        labelOffsets[item.first] = item.second - 1;
    }
}
int32_t IrCompiler::addLabelOffset(int32_t offset) {
    labelOffsets[labelCounter] = offset;
    labelCounter++;
    return labelCounter;
}

// Main functionality
IrFunc IrCompiler::toIrFunc(Function* func) {
    func = func;
    tempStack = stack<tempptr_t>();
    irInsts = IrInstList();
	currentTemp = 0;

    for (int i = 0; i < func->instructions.size(); i++) {
        decLabelOffsets();  // decrease remaining BcInstruction count for all labels
        for (auto item : labelOffsets) {  // check if any labels need to be inserted now
            if (item.second == 0) {
                pushInstruction(IrInstruction(IrOp::AddLabel, item.first));
            }
        }
		BcInstruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case BcOp::LoadConst:
	            {
                    tempptr_t curr = pushNewTemp();
					pushInstruction(IrInstruction(IrOp::LoadConst, inst.operand0, curr));
	                break;
	            }
	        case BcOp::LoadFunc:
	            {
                    tempptr_t curr = pushNewTemp();
					pushInstruction(IrInstruction(IrOp::LoadFunc, inst.operand0, curr));
	                break;
	            }
	        case BcOp::LoadLocal:
	            {
                    tempptr_t curr = pushNewTemp();
					pushInstruction(IrInstruction(IrOp::LoadLocal, inst.operand0, curr));
	                break;
	            }
	        case BcOp::LoadGlobal:
	            {
                    tempptr_t curr = pushNewTemp();
                    optstr_t global = func->names_[inst.operand0.value()];
					pushInstruction(IrInstruction(IrOp::LoadGlobal, global, curr));
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
                    tempptr_t curr = pushNewTemp();
					pushInstruction(IrInstruction(IrOp::PushReference, inst.operand0, curr));
	                break;
	            }
	        case BcOp::LoadReference:
	            {
                    tempptr_t curr = pushNewTemp();
					pushInstruction(IrInstruction(IrOp::LoadReference, inst.operand0, curr));
	                break;
	            }
	        case BcOp::AllocRecord:
	            {
                    tempptr_t curr = pushNewTemp();
					pushInstruction(IrInstruction(IrOp::AllocRecord, curr));
	                break;
	            }
	        case BcOp::FieldLoad:
	            {
					tempptr_t record = popTemp();
                    optstr_t field = func->names_[inst.operand0.value()];
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, record});
					pushInstruction(IrInstruction(IrOp::AssertRecord, record));
					pushInstruction(IrInstruction(IrOp::FieldLoad, field, instTemps));
	                break;
	            }
	        case BcOp::FieldStore:
	            {
					tempptr_t record = popTemp();
                    optstr_t field = func->names_[inst.operand0.value()];
					tempptr_t value = popTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{record, value});
					pushInstruction(IrInstruction(IrOp::AssertRecord, record));
					pushInstruction(IrInstruction(IrOp::FieldStore, field, instTemps));
	                break;
	            }
	        case BcOp::IndexLoad:
	            {
					tempptr_t record = popTemp();
					tempptr_t index = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, record, index});
					pushInstruction(IrInstruction(IrOp::AssertRecord, record));
					pushInstruction(IrInstruction(IrOp::CastString, index));
					pushInstruction(IrInstruction(IrOp::IndexLoad, instTemps));
	                break;
	            }
	        case BcOp::IndexStore:
	            {
					tempptr_t record = popTemp();
					tempptr_t index = popTemp();
					tempptr_t value = popTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{record, value, index});
					pushInstruction(IrInstruction(IrOp::AssertRecord, record));
					pushInstruction(IrInstruction(IrOp::CastString, index));
					pushInstruction(IrInstruction(IrOp::IndexStore, instTemps));
	                break;
	            }
	        case BcOp::AllocClosure:
	            {
					TempListPtr instTemps = make_shared<TempList>();
					for (int i = 0; i < inst.operand0; i++) {
						instTemps->push_back(popTemp());
					}
					tempptr_t func = popTemp();
					instTemps->push_back(func);
					tempptr_t curr = pushNewTemp();
					instTemps->push_back(curr);
					reverse(instTemps->begin(), instTemps->end());
					pushInstruction(IrInstruction(IrOp::AssertFunction, func));
					pushInstruction(IrInstruction(IrOp::AllocClosure, instTemps));
	                break;
	            }
	        case BcOp::Call:
	            {
					TempListPtr instTemps = make_shared<TempList>();
					for (int i = 0; i < inst.operand0; i++) {
                    	instTemps->push_back(popTemp());
					}
					tempptr_t clos = popTemp();
					instTemps->push_back(clos);
					tempptr_t curr = pushNewTemp();
					instTemps->push_back(curr);
					reverse(instTemps->begin(), instTemps->end());
					pushInstruction(IrInstruction(IrOp::AssertClosure, clos));
					pushInstruction(IrInstruction(IrOp::Call, instTemps));
	                break;
	            }
	        case BcOp::Return:
	            {
                    pushInstruction(IrInstruction(IrOp::Return, popTemp()));
	                break;
	            }
	        case BcOp::Add:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::Add, instTemps));
	                break;
	            }
	        case BcOp::Sub:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempLeft));
					pushInstruction(IrInstruction(IrOp::Sub, instTemps));
	                break;
	            }
	        case BcOp::Mul:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempLeft));
					pushInstruction(IrInstruction(IrOp::Mul, instTemps));
	                break;
	            }
	        case BcOp::Div:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempLeft));
					pushInstruction(IrInstruction(IrOp::Div, instTemps));
	                break;
	            }
	        case BcOp::Neg:
	            {
                    tempptr_t val = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, val});
					pushInstruction(IrInstruction(IrOp::AssertInteger, val));
					pushInstruction(IrInstruction(IrOp::Neg, instTemps));
	                break;
	            }
	        case BcOp::Gt:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempLeft));
					pushInstruction(IrInstruction(IrOp::Gt, instTemps));
	                break;
	            }
	        case BcOp::Geq:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertInteger, tempLeft));
					pushInstruction(IrInstruction(IrOp::Geq, instTemps));
	                break;
	            }
	        case BcOp::Eq:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::Eq, instTemps));
	                break;
	            }
	        case BcOp::And:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertBool, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertBool, tempLeft));
					pushInstruction(IrInstruction(IrOp::And, instTemps));
	                break;
	            }
	        case BcOp::Or:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(IrInstruction(IrOp::AssertBool, tempRight));
					pushInstruction(IrInstruction(IrOp::AssertBool, tempLeft));
					pushInstruction(IrInstruction(IrOp::Or, instTemps));
	                break;
	            }
	        case BcOp::Not:
	            {
                    tempptr_t val = popTemp();
                    tempptr_t curr = pushNewTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, val});
					pushInstruction(IrInstruction(IrOp::AssertBool, val));
					pushInstruction(IrInstruction(IrOp::Not, instTemps));
	                break;
	            }
	        case BcOp::Goto:
	            {
                    int32_t label = addLabelOffset(inst.operand0.value());
                    pushInstruction(IrInstruction(IrOp::Goto, label));
	                break;
	            }
	        case BcOp::If:
	            {
                    tempptr_t expr = popTemp();
                    int32_t label = addLabelOffset(inst.operand0.value());
                    pushInstruction(IrInstruction(IrOp::If, label, expr));
	                break;
	            }
	        case BcOp::Dup:
	            {
                    pushTemp(tempStack.top());
	                break;
	            }
	        case BcOp::Swap:
	            {
                    tempptr_t temp1 = popTemp();
                    tempptr_t temp2 = popTemp();
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
	IrFunc irFunc = IrFunc(irInsts, func->constants_, func->functions_, func->parameter_count_, func->local_vars_.size(), labelCounter);
    return irFunc;
};

IrFunc IrCompiler::toIr() {
	return toIrFunc(func);
};
