/*
 * bc_to_ir.cpp
 *
 * Convert bytecode to IR representation
 */
#include "../types.h"
#include "../ir.h"
#include "bc_to_ir.h"
#include "../opt/opt_reg_alloc.h"
#include <algorithm>

using namespace std;

class Interpreter;

// Helpers
tempptr_t IrCompiler::getNewTemp() {
	currentTemp = temps.size();
    tempptr_t newTemp = make_shared<Temp>(currentTemp);
	temps.push_back(newTemp);
	newTemp->startInterval = irInsts.size();
	newTemp->endInterval = irInsts.size();	
    return newTemp;
}
void IrCompiler::pushTemp(tempptr_t temp) {
    tempStack.push(temp);
	temp->timesInUse++;
}
tempptr_t IrCompiler::popTemp() {
    tempptr_t temp = tempStack.top();
    tempStack.pop();
	temp->timesInUse--;
    return temp;
}
void IrCompiler::checkIfUsed(tempptr_t temp) {
	if (temp->timesInUse == 0 || temp->index < func->local_vars_.size()) {
		temp->endInterval = irInsts.size();	
	}
	if (whileLevel > 0) {
		tempsInWhile.insert(temp);
	}
}
void IrCompiler::pushInstruction(instptr_t inst) {
    irInsts.push_back(inst);
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
    pushInstruction(make_shared<IrInstruction>(assertOp, val)); 
    // unbox 
    tempptr_t unboxed = getNewTemp();
    pushInstruction(make_shared<IrInstruction>(unboxOp, unboxed, val));
    // perform the operation
    tempptr_t result = getNewTemp();
    pushInstruction(make_shared<IrInstruction>(operation, result, unboxed));
    //
    tempptr_t ret = getNewTemp(); 
    pushInstruction(make_shared<IrInstruction>(castOp, ret, result));
    pushTemp(ret);
	checkIfUsed(val);
}
void IrCompiler::doBinaryArithmetic(IrOp operation, bool fromBoolean, bool toBoolean) {
    // takes in two unverified operands, asserts and casts the correct type, 
    // pushes a temp for the result of an operation, 
    // and returns a list of temps to use in the instruction 
    IrOp assertOp;
    IrOp unboxOp;
    IrOp castOp;

    if (fromBoolean) {
        assertOp = IrOp::AssertBoolean;
        unboxOp = IrOp::UnboxBoolean;
    } else {
        assertOp = IrOp::AssertInteger;
        unboxOp = IrOp::UnboxInteger;
    }

    if (toBoolean) {
        castOp = IrOp::NewBoolean;
    } else {
        castOp = IrOp::NewInteger;
    }
    tempptr_t right = popTemp();
    tempptr_t left = popTemp(); 
   
    // assert the correct type
    pushInstruction(make_shared<IrInstruction>(assertOp, right)); 
    pushInstruction(make_shared<IrInstruction>(assertOp, left)); 

    // generate instructions to unbox right
    tempptr_t rightUnboxed = getNewTemp();
    TempListPtr rightOperands = make_shared<TempList>(TempList{rightUnboxed, right});
    pushInstruction(make_shared<IrInstruction>(unboxOp, rightOperands));
    // generate to unbox left
    tempptr_t leftUnboxed = getNewTemp();
    TempListPtr leftOperands = make_shared<TempList>(TempList{leftUnboxed, left});
    pushInstruction(make_shared<IrInstruction>(unboxOp, leftOperands));

    // generate a list of operands for an arith operation
    tempptr_t result = getNewTemp(); 
    TempListPtr operands = make_shared<TempList>(TempList{result, rightUnboxed, leftUnboxed});
    // perform the op 
    pushInstruction(make_shared<IrInstruction>(operation, operands));
    // cast back to the right type
    tempptr_t ret = getNewTemp();
    pushInstruction(make_shared<IrInstruction>(castOp, ret, result));
    pushTemp(ret);
	checkIfUsed(right);
	checkIfUsed(left);
    return;
}

// Main functionality
IrFunc IrCompiler::toIrFunc(Function* func) {
    func = func;
    tempStack = stack<tempptr_t>();
    irInsts = IrInstList();

    for (int i = 0; i < func->instructions.size(); i++) {
		BcInstruction inst = func->instructions[i];
	    switch (inst.operation) {
	        case BcOp::LoadConst:
	            {
                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);

					pushInstruction(make_shared<IrInstruction>(IrOp::LoadConst, inst.operand0, curr));
	                break;
	            }
	        case BcOp::LoadFunc:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::LoadFunc, inst.operand0, curr));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::LoadLocal:
	            {
                    // if the local at index i is also a ref var, generate PushLocalRef, LoadRef
                    int localIndex = inst.operand0.value();
                    if (isLocalRef.at(localIndex)) {
                        // it is a local ref var; generate PushLocalRef, LoadRef
                        tempptr_t localTemp = temps.at(localIndex);
                        tempptr_t val = getNewTemp();
                        pushInstruction(make_shared<IrInstruction>(IrOp::LoadReference, val, localTemp));
                        pushTemp(val);
                    } else {
                        // it is not a local ref var; generate plain LoadLocal
                        //tempptr_t val = getNewTemp();
                        tempptr_t localTemp = temps.at(localIndex);
                        //pushInstruction(make_shared<IrInstruction>(IrOp::LoadLocal, val, localTemp));
                        //pushTemp(val);
                        pushTemp(localTemp);
                    }
	                break;
	            }
	        case BcOp::LoadGlobal:
	            {
                    tempptr_t curr = getNewTemp();
                    optstr_t global = func->names_[inst.operand0.value()];
					pushInstruction(make_shared<IrInstruction>(IrOp::LoadGlobal, global, curr));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::StoreLocal:
	            {
                    IrOp op;
                    int localIndex = inst.operand0.value();
                    if (isLocalRef.at(localIndex)) {
                        // the local is a ref var; generate storelocalref
                        op = IrOp::StoreLocalRef;
                    } else {
                        op = IrOp::StoreLocal;
                    }
					tempptr_t val = popTemp();
                    tempptr_t localTemp = temps.at(localIndex);
                    pushInstruction(make_shared<IrInstruction>(op, localTemp, val));
					checkIfUsed(val);
					localTemp->endInterval = irInsts.size();
	                break;
	            }
	        case BcOp::StoreGlobal:
	            {
                    optstr_t global = func->names_[inst.operand0.value()];
					tempptr_t temp = popTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::StoreGlobal, global, temp));
					checkIfUsed(temp);
	                break;
	            }
	        case BcOp::PushReference:
	            {
                    tempptr_t curr = getNewTemp();
                    int instrIdx = inst.operand0.value();
                    // if the index falls within the locals array, 
                    if (instrIdx < func->local_reference_vars_.size()) {
                        // then generate a PushLocalRef instr 
                        // TOD0: make this preprocessing more efficient
                        int localIndex;
                        for (int i = 0; i < func->local_vars_.size(); i++) {
                            if (func->local_vars_.at(i) == func->local_reference_vars_.at(instrIdx)) {
                                localIndex = i;
                                break;
                            }
                        } 
                        tempptr_t localTemp = temps.at(localIndex);
					    pushInstruction(make_shared<IrInstruction>(IrOp::PushLocalRef, curr, localTemp));
                    } else {
                        // else, generate a PushFreeRef instruction
                        int refIndex = instrIdx - func->local_reference_vars_.size();
					    pushInstruction(make_shared<IrInstruction>(IrOp::PushFreeRef, refIndex, curr));
                    }
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::LoadReference:
	            {
                    tempptr_t ref = popTemp();
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::LoadReference, curr, ref));
                    pushTemp(curr);
                    checkIfUsed(ref);
	                break;
	            }
	        case BcOp::AllocRecord:
	            {
                    tempptr_t curr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::AllocRecord, curr));
                    pushTemp(curr);
	                break;
	            }
	        case BcOp::FieldLoad:
	            {
                    // pop and assert record
					tempptr_t record = popTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::AssertRecord, record));

                    optstr_t field = func->names_[inst.operand0.value()];

                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, record});
					pushInstruction(make_shared<IrInstruction>(IrOp::FieldLoad, field, instTemps));
					checkIfUsed(record);
	                break;
	            }
	        case BcOp::FieldStore:
	            {
                    optstr_t field = func->names_[inst.operand0.value()];
					tempptr_t value = popTemp();
					tempptr_t record = popTemp();
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{record, value});
					pushInstruction(make_shared<IrInstruction>(IrOp::AssertRecord, record));
					pushInstruction(make_shared<IrInstruction>(IrOp::FieldStore, field, instTemps));
					checkIfUsed(value);
					checkIfUsed(record);
	                break;
	            }
	        case BcOp::IndexLoad:
	            {
                    // get index and cast to string
					tempptr_t index = popTemp();
                    tempptr_t indexStr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::CastString, indexStr, index));

                    // get and confirm record to load into
					tempptr_t record = popTemp();
                    pushInstruction(make_shared<IrInstruction>(IrOp::AssertRecord, record));
                    
                    // perform the operation 
                    tempptr_t ret = getNewTemp();
                    pushTemp(ret);
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{ret, record, indexStr});
					pushInstruction(make_shared<IrInstruction>(IrOp::IndexLoad, instTemps));
					checkIfUsed(index);
					checkIfUsed(indexStr);
					checkIfUsed(record);
	                break;
	            }
	        case BcOp::IndexStore:
	            {
					tempptr_t value = popTemp();

                    // get pop and cast index to string
					tempptr_t index = popTemp();
                    tempptr_t indexStr = getNewTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::CastString, indexStr, index));

                    // get and assert record
					tempptr_t record = popTemp();
					pushInstruction(make_shared<IrInstruction>(IrOp::AssertRecord, record));

                    // store value
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{record, value, indexStr});
					pushInstruction(make_shared<IrInstruction>(IrOp::IndexStore, instTemps));
					checkIfUsed(value);
					checkIfUsed(index);
					checkIfUsed(indexStr);
					checkIfUsed(record);
	                break;
	            }
	        case BcOp::AllocClosure:
	            {
					TempListPtr instTemps = make_shared<TempList>();
					tempptr_t curr = getNewTemp();
					for (int i = 0; i < inst.operand0; i++) {
                        // pop args 
                        tempptr_t t = popTemp();
                        // add an instruction confirming that this is a ref
                        pushInstruction(make_shared<IrInstruction>(IrOp::AssertValWrapper, t));
						instTemps->push_back(t);
					}
					reverse(instTemps->begin(), instTemps->end());
					tempptr_t func = popTemp();
					instTemps->push_back(func);
                    pushTemp(curr);
					instTemps->push_back(curr);
					reverse(instTemps->begin(), instTemps->end());
					pushInstruction(make_shared<IrInstruction>(IrOp::AssertFunction, func));
					pushInstruction(make_shared<IrInstruction>(IrOp::AllocClosure, inst.operand0, instTemps));
					checkIfUsed(func);
					for (tempptr_t t: *instTemps) {
						checkIfUsed(t);
					}
	                break;
	            }
	        case BcOp::Call:
	            {
					TempListPtr instTemps = make_shared<TempList>();
					tempptr_t curr = getNewTemp();
					for (int i = 0; i < inst.operand0; i++) {
                        tempptr_t t = popTemp();
                    	instTemps->push_back(t);
					}
					tempptr_t clos = popTemp();
					instTemps->push_back(clos);
                    pushTemp(curr);
					instTemps->push_back(curr);
					reverse(instTemps->begin(), instTemps->end());
					pushInstruction(make_shared<IrInstruction>(IrOp::AssertClosure, clos));
					pushInstruction(make_shared<IrInstruction>(IrOp::Call, inst.operand0, instTemps));
					checkIfUsed(clos);
					for (tempptr_t t: *instTemps) {
						checkIfUsed(t);
					}
	                break;
	            }
	        case BcOp::Return:
	            {
                    pushInstruction(make_shared<IrInstruction>(IrOp::Return, popTemp()));
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
					pushInstruction(make_shared<IrInstruction>(IrOp::Add, instTemps));
					checkIfUsed(tempRight);
					checkIfUsed(tempLeft);
	                break;
	            }
	        case BcOp::Sub:
	            {
                    doBinaryArithmetic(IrOp::Sub, false, false);
	                break;
	            }
	        case BcOp::Mul:
	            {
                    doBinaryArithmetic(IrOp::Mul, false, false);
	                break;
	            }
	        case BcOp::Div:
	            {
                    doBinaryArithmetic(IrOp::Div, false, false);
                    break;
	            }
	        case BcOp::Neg:
	            {
                    doUnaryArithmetic(IrOp::Neg, false);
                    break;
	            }
	        case BcOp::Gt:
	            {
                    doBinaryArithmetic(IrOp::Gt, false, true);
	                break;
	            }
	        case BcOp::Geq:
	            {
                    doBinaryArithmetic(IrOp::Geq, false, true);
	                break;
	            }
	        case BcOp::Eq:
	            {
                    tempptr_t tempRight = popTemp();
                    tempptr_t tempLeft = popTemp();
                    tempptr_t curr = getNewTemp();
                    pushTemp(curr);
                    TempListPtr instTemps = make_shared<TempList>(
                                TempList{curr, tempRight, tempLeft});
					pushInstruction(make_shared<IrInstruction>(IrOp::Eq, instTemps));
					checkIfUsed(tempRight);
					checkIfUsed(tempLeft);
	                break;
	            }
	        case BcOp::And:
	            {
                    doBinaryArithmetic(IrOp::And, true, true);
	                break;
	            }
	        case BcOp::Or:
	            {
                    doBinaryArithmetic(IrOp::Or, true, true);
	                break;
	            }
	        case BcOp::Not:
	            {
                    doUnaryArithmetic(IrOp::Not, true);
                    break;
	            }
	        case BcOp::StartWhile:
	            {
					whileLevel++;
                    break;
	            }
	        case BcOp::EndWhile:
	            {
					whileLevel--;
					if (whileLevel == 0) {
						for (tempptr_t temp: tempsInWhile) {
							temp->endInterval = irInsts.size();	
						}
						tempsInWhile.clear();
					}
                    break;
	            }
	        case BcOp::Goto:
	            {
                    pushInstruction(make_shared<IrInstruction>(IrOp::Goto, inst.operand0.value()));
	                break;
	            }
	        case BcOp::If:
	            {
                    tempptr_t expr = popTemp();
                    tempptr_t exprVal = getNewTemp();
                    pushInstruction(make_shared<IrInstruction>(IrOp::AssertBoolean, expr));
                    pushInstruction(make_shared<IrInstruction>(IrOp::UnboxBoolean, exprVal, expr));
                    pushInstruction(make_shared<IrInstruction>(IrOp::If, inst.operand0.value(), exprVal));
					checkIfUsed(expr);
	                break;
	            }
            case BcOp::Label:
                {
                    pushInstruction(make_shared<IrInstruction>(IrOp::AddLabel, inst.operand0.value()));
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
                    tempptr_t temp = popTemp();
					checkIfUsed(temp);
	                break;
	            }
	        default:
	            throw RuntimeException("should never get here - invalid instruction");
	    }
        // TODO: PUT BACK 
        //pushInstruction(make_shared<IrInstruction>(IrOp::GarbageCollect, optint_t()));
	}
    // TODO: figure out how to make refs work
    int32_t ref_count = 0; 
	IrFunc irFunc = IrFunc(irInsts, 
            func->constants_, 
            func->functions_,
			temps,
            func->parameter_count_, 
            func->local_vars_.size(), 
            ref_count);
    return irFunc;
};

IrFunc IrCompiler::toIr() {
    // do some preprocessing here 
    for (string local : func->local_vars_) {
        if (find(func->local_reference_vars_.begin(), 
                 func->local_reference_vars_.end(), 
                 local) != func->local_reference_vars_.end()) {
            // this local is a local ref var 
            isLocalRef.push_back(true);
        } else {
            isLocalRef.push_back(false);
        }
    }
	return toIrFunc(func);
};
