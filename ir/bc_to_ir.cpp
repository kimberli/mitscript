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
tempptr_t IrCompiler::getNewTemp() {
    tempptr_t newTemp = make_shared<Temp>(currentTemp);
    currentTemp ++; 
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
void IrCompiler::pushInstruction(instptr_t inst) {
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
void IrCompiler::doUnaryArithmetic(IrOp operation, bool toBoolean) {
    IrOp assertOp;
    IrOp unboxOp;
    IrOp castOp;
    if (toBoolean) {
        assertOp = IrOp::AssertBoolean;
        unboxOp = IrOp::UnboxBoolean;
        castOp = IrOp::NewBoolean;
    } else {
        assertOp = IrOp::AssertInteger;
        unboxOp = IrOp::UnboxInteger;
        castOp = IrOp::NewInteger;
    }
    tempptr_t val = popTemp();
    // assert 
    pushInstruction(make_shared<IrInstruction>(IrInstruction(assertOp, val))); 
    // unbox 
    tempptr_t unboxed = getNewTemp();
    pushInstruction(make_shared<IrInstruction>(IrInstruction(unboxOp, unboxed, val)));
    // perform the operation
    tempptr_t result = getNewTemp();
    pushInstruction(make_shared<IrInstruction>(IrInstruction(operation, result, unboxed)));
    // recast 
    tempptr_t ret = getNewTemp(); 
    pushInstruction(make_shared<IrInstruction>(IrInstruction(castOp, ret, result)));
    pushTemp(ret);
}
void IrCompiler::doBinaryArithmetic(IrOp operation, bool toBoolean) {
    // takes in two unverified operands, asserts and casts the correct type, 
    // pushes a temp for the result of an operation, 
    // and returns a list of temps to use in the instruction 
    IrOp assertOp;
    IrOp unboxOp;
    IrOp castOp;
    if (toBoolean) {
        assertOp = IrOp::AssertBoolean;
        unboxOp = IrOp::UnboxBoolean;
        castOp = IrOp::NewBoolean;
    } else {
        assertOp = IrOp::AssertInteger;
        unboxOp = IrOp::UnboxInteger;
        castOp = IrOp::NewInteger;
    }
    tempptr_t right = popTemp();
    tempptr_t left = popTemp(); 
   
    // assert the correct type
    pushInstruction(make_shared<IrInstruction>(IrInstruction(assertOp, right))); 
    pushInstruction(make_shared<IrInstruction>(IrInstruction(assertOp, left))); 

    // generate instructions to unbox right
    tempptr_t rightUnboxed = getNewTemp();
    TempListPtr rightOperands = make_shared<TempList>(TempList{rightUnboxed, right});
    pushInstruction(make_shared<IrInstruction>(IrInstruction(unboxOp, rightOperands)));
    // generate to unbox left
    tempptr_t leftUnboxed = getNewTemp();
    TempListPtr leftOperands = make_shared<TempList>(TempList{leftUnboxed, left});
    pushInstruction(make_shared<IrInstruction>(IrInstruction(unboxOp, leftOperands)));

    // generate a list of operands for an arith operation
    tempptr_t result = getNewTemp(); 
    TempListPtr operands = make_shared<TempList>(TempList{result, rightUnboxed, leftUnboxed});
    // perform the op 
    pushInstruction(make_shared<IrInstruction>(IrInstruction(operation, operands)));
    // cast back to the right type
    tempptr_t ret = getNewTemp();
    pushInstruction(make_shared<IrInstruction>(IrInstruction(castOp, ret, result)));
    pushTemp(ret);
    return;
}

// Main functionality
IrFunc IrCompiler::toIrFunc(Function* func) {
    func = func;
    tempStack = stack<tempptr_t>();
    irInsts = IrInstList();
	currentTemp = 0;

    for (int i = 0; i < func->instructions.size(); i++) {
		BcInstruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case BcOp::LoadConst:
	            {
                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);

					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::LoadConst, inst.operand0, curr)));
	                break;
	            }
	        case BcOp::LoadFunc:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::LoadFunc, inst.operand0, curr)));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::LoadLocal:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::LoadLocal, inst.operand0, curr)));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::LoadGlobal:
	            {
                    tempptr_t curr = getNewTemp();
                    optstr_t global = func->names_[inst.operand0.value()];
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::LoadGlobal, global, curr)));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::StoreLocal:
	            {
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::StoreLocal, inst.operand0, popTemp())));
	                break;
	            }
	        case BcOp::StoreGlobal:
	            {
                    optstr_t global = func->names_[inst.operand0.value()];
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::StoreGlobal, global, popTemp())));
	                break;
	            }
	        case BcOp::PushReference:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::PushReference, inst.operand0, curr)));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::LoadReference:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::LoadReference, inst.operand0, curr)));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::AllocRecord:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AllocRecord, curr)));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::FieldLoad:
	            {
					tempptr_t record = popTemp();
                    optstr_t field = func->names_[inst.operand0.value()];
                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, record});
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertRecord, record)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::FieldLoad, field, instTemps)));
	                break;
	            }
	        case BcOp::FieldStore:
	            {
					tempptr_t record = popTemp();
                    optstr_t field = func->names_[inst.operand0.value()];
					tempptr_t value = popTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{record, value});
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertRecord, record)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::FieldStore, field, instTemps)));
	                break;
	            }
	        case BcOp::IndexLoad:
	            {
					tempptr_t record = popTemp();
					tempptr_t index = popTemp();
                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, record, index});
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertRecord, record)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::CastString, index)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::IndexLoad, instTemps)));
	                break;
	            }
	        case BcOp::IndexStore:
	            {
					tempptr_t record = popTemp();
					tempptr_t index = popTemp();
					tempptr_t value = popTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{record, value, index});
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertRecord, record)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::CastString, index)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::IndexStore, instTemps)));
	                break;
	            }
	        case BcOp::AllocClosure:
	            {
					TempListPtr instTemps = make_shared<TempList>();
					for (int i = 0; i < inst.operand0; i++) {
                        // pop args 
                        tempptr_t t = popTemp();
                        // add an instruction confirming that this is a ref
                        pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertValWrapper, t)));
						instTemps->push_back(t);
					}
					tempptr_t func = popTemp();
					instTemps->push_back(func);
					tempptr_t curr = getNewTemp();
                    pushTemp(curr);
					instTemps->push_back(curr);
					reverse(instTemps->begin(), instTemps->end());
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertFunction, func)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AllocClosure, optint_t(inst.operand0), instTemps)));
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
					tempptr_t curr = getNewTemp();
                    pushTemp(curr);
					instTemps->push_back(curr);
					reverse(instTemps->begin(), instTemps->end());
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AssertClosure, clos)));
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::Call, instTemps)));
	                break;
	            }
	        case BcOp::Return:
	            {
                    pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::Return, popTemp())));
	                break;
	            }
	        case BcOp::Add:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::Add, instTemps)));
	                break;
	            }
	        case BcOp::Sub:
	            {
                    doBinaryArithmetic(IrOp::Sub, false);
	                break;
	            }
	        case BcOp::Mul:
	            {
                    doBinaryArithmetic(IrOp::Mul, false);
	                break;
	            }
	        case BcOp::Div:
	            {
                    doBinaryArithmetic(IrOp::Div, false);
                    break;
	            }
	        case BcOp::Neg:
	            {
                    doUnaryArithmetic(IrOp::Neg, false);
                    break;
	            }
	        case BcOp::Gt:
	            {
                    doBinaryArithmetic(IrOp::Gt, true);
	                break;
	            }
	        case BcOp::Geq:
	            {
                    doBinaryArithmetic(IrOp::Geq, true);
	                break;
	            }
	        case BcOp::Eq:
	            {
                    doBinaryArithmetic(IrOp::Eq, true);
	                break;
	            }
	        case BcOp::And:
	            {
                    doBinaryArithmetic(IrOp::And, true);
	                break;
	            }
	        case BcOp::Or:
	            {
                    doBinaryArithmetic(IrOp::Or, true);
	                break;
	            }
	        case BcOp::Not:
	            {
                    doUnaryArithmetic(IrOp::Not, true);
                    break;
	            }
	        case BcOp::Goto:
	            {
                    int32_t label = addLabelOffset(inst.operand0.value());
                    pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::Goto, label)));
	                break;
	            }
	        case BcOp::If:
	            {
                    tempptr_t expr = popTemp();
                    int32_t label = addLabelOffset(inst.operand0.value());
                    pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::If, label, expr)));
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
        decLabelOffsets();  // decrease remaining BcInstruction count for all labels
        for (auto item : labelOffsets) {  // check if any labels need to be inserted now
            if (item.second == 0) {
                pushInstruction(make_shared<IrInstruction>(IrInstruction(IrOp::AddLabel, item.first)));
                labelOffsets.erase(item.first);
            }
        }
	}
    int32_t temp_count = currentTemp;
    // TODO: figure out how to make refs work
    int32_t ref_count = 0; 
	IrFunc irFunc = IrFunc(irInsts, func->constants_, func->functions_, func->parameter_count_, temp_count, ref_count, func->local_vars_.size(), labelCounter);
    return irFunc;
};

IrFunc IrCompiler::toIr() {
	return toIrFunc(func);
};
