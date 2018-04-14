/*
 * interpreter.cpp
 *
 * Implements an Interpreter object which can be used to step through
 * bytecode instructions and store interpreter state
 */
#include "exception.h"
#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include "types.h"
#include <algorithm>
#include <stack>

using namespace std;

Interpreter::Interpreter(vptr<Function> mainFunc, int maxmem) {
    // initialize the garbage collector
    // note that mainFunc is not included in the gc's allocated list because
    // we never have to deallocate it
    collector = new CollectedHeap(maxmem, mainFunc->getSize(), &frames);

    // initialize the root frame
    int numLocals = mainFunc->names_.size();
    int numRefs = 0;
    if (mainFunc->local_reference_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame w/ nonzero ref vars");
    }
    if (mainFunc->free_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame w/ nonzero free vars");
    }
    if (mainFunc->local_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame w/ nonzero local vars");
    }
    mainFunc->local_vars_ = mainFunc->names_;
    fptr frame = collector->allocate<Frame, vptr<Function>>(mainFunc);
    frame->collector = collector;
    globalFrame = frame;
    frames.push_back(frame);
    finished = false;

    // set up native functions at the beginning of functions array
	vector<vptr<Function>> functions_;
    vector<vptr<Constant>> constants_;
	vector<string> args0 ;
	vector<string> args1 = { string("s") };
    vector<string> local_reference_vars_;
    vector<string> free_vars_;
	vector<string> names_;
    InstructionList instructions;
	vector<shared_ptr<Function>> frameFuncs;
	frame->func->functions_[0] = collector->allocate<PrintNativeFunction>(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions);
	frame->func->functions_[1] = collector->allocate<InputNativeFunction>(functions_, constants_, 0, args0, local_reference_vars_, free_vars_, names_, instructions);
	frame->func->functions_[2] = collector->allocate<IntcastNativeFunction>(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions);
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    fptr frame = frames.back();
    Instruction& inst = frame->getCurrInstruction();
    int newOffset = 1;
    // LOG("\nexecuting instruction " + to_string(frame->instructionIndex));
    switch (inst.operation) {
        case Operation::LoadConst:
            {
                auto constant = frame->getConstantByIndex(inst.operand0.value());
                frame->opStackPush(constant);
                break;
            }
        case Operation::LoadFunc:
            {
                auto func = frame->getFunctionByIndex(inst.operand0.value());
                frame->opStackPush(func);
                break;
            }
        case Operation::LoadLocal:
            {
                string name = frame->getLocalByIndex(inst.operand0.value());
                frame->opStackPush(frame->getLocalVar(name));
                break;
            }
        case Operation::StoreLocal:
            {
                string name = frame->getLocalByIndex(inst.operand0.value());
                vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreLocal");
                }
                frame->setLocalVar(name, value);
                break;
            }
        case Operation::LoadGlobal:
            {
                int index = inst.operand0.value();
                string name = frame->getNameByIndex(index);
                frame->opStackPush(globalFrame->getLocalVar(name));
                break;
            }
        case Operation::StoreGlobal:
            {
                int index = inst.operand0.value();
                vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreGlobal");
                }
                string name = frame->getNameByIndex(index);
                globalFrame->setLocalVar(name, value);
                break;
            }
        case Operation::PushReference:
            {
                string name = frame->getRefByIndex(inst.operand0.value());
                auto valuePtr = frame->getRefVar(name);
                frame->opStackPush(valuePtr);
                break;
            }
        case Operation::LoadReference:
            {
                vptr<ValuePtr> valuePtr = dynamic_cast<ValuePtr*>(frame->opStackPop());
                if (valuePtr == NULL) {
                    throw RuntimeException("expected ValuePtr on the stack for LoadReference");
                }
                frame->opStackPush(valuePtr->ptr);
                break;
            }
        case Operation::StoreReference:
            {
                vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreReference");
                }
                vptr<ValuePtr> valuePtr = dynamic_cast<ValuePtr*>(frame->opStackPop());
                if (valuePtr == NULL) {
                    throw RuntimeException("expected ValuePtr on the stack for StoreReference");
                }
				*valuePtr->ptr = *value;
                break;
            }
        case Operation::AllocRecord:
            {
				frame->opStackPush(collector->allocate<Record>());
                break;
            }
        case Operation::FieldLoad:
            {
				vptr<Record> record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    vptr<Value> val = collector->allocate<None>();
                    record->set(field, val, *collector);
                }
				frame->opStackPush(record->get(field));
                break;
            }
        case Operation::FieldStore:
            {
				vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for FieldStore");
                }
				vptr<Record> record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                record->set(field, value, *collector);
                break;
            }
        case Operation::IndexLoad:
            {
				string index = frame->opStackPop()->toString();
				vptr<Record> record = frame->opStackPop()->cast<Record>();
                if (record->value.count(index) == 0) {
                    vptr<Value> val = collector->allocate<None>();
                    record->set(index, val, *collector);
                }
				frame->opStackPush(record->get(index));
                break;
            }
        case Operation::IndexStore:
            // items are popped off in this order: value to store,
            // index
            // record
            {
				auto value = frame->opStackPop();
				string index = frame->opStackPop()->toString();
				vptr<Record> record = frame->opStackPop()->cast<Record>();
                record->set(index, value, *collector);
                break;
            }
        case Operation::AllocClosure:
            {
                // read num free vars, ref vars, and function off the stack
                int numFreeVars = inst.operand0.value();
                vector<vptr<ValuePtr>> refList;
                for (int i = 0; i < numFreeVars; i++) {
                    auto top = frame->opStackPop();
                    vptr<ValuePtr> value = dynamic_cast<ValuePtr*>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected ValuePtr on the stack for AllocClosure");
                    }
                    refList.push_back(value);
                }
                vptr<Function> func = dynamic_cast<Function*>(frame->opStackPop());
                if (func == NULL) {
                    throw RuntimeException("expected Function on the stack for AllocClosure");
                }
                if (numFreeVars != func->free_vars_.size()) {
                    throw RuntimeException("expected " + to_string(func->free_vars_.size()) + " reference variables but got " + to_string(numFreeVars));
                }
                // push new closure onto the stack
                frame->opStackPush(collector->allocate<Closure>(refList, func));
                break;
            }
        case Operation::Call:
            {
                // read num arguments, argument values, and closure off the stack
                int numArgs = inst.operand0.value();
                vector<vptr<Constant>> argsList;
                for (int i = 0; i < numArgs; i++) {
                    auto top = frame->opStackPop();
                    vptr<Constant> value = dynamic_cast<Constant*>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected Constant on the stack for Call");
                    }
                    argsList.push_back(value);
                }
				reverse(argsList.begin(), argsList.end());
                vptr<Closure> clos = dynamic_cast<Closure*>(frame->opStackPop());
                if (clos == NULL) {
                    throw RuntimeException("expected Closure on operand stack for function call");
                }
                if (numArgs != clos->func->parameter_count_) {
                    throw RuntimeException("expected " + to_string(clos->func->parameter_count_) + " arguments, got " + to_string(numArgs));
                }
                // process local refs and local vars
                int numLocals = clos->func->local_vars_.size();
                int numRefs = clos->func->free_vars_.size();
                fptr newFrame = collector->allocate<Frame>(clos->func);
                newFrame->collector = collector;
				for (int i = 0; i < numLocals; i++) {
                    if (i < numArgs) {
                        string name = clos->func->local_vars_[i];
                        newFrame->setLocalVar(name, argsList[i]);
                    } else {
                        string name = clos->func->local_vars_[i];
                        newFrame->setLocalVar(name, collector->allocate<None>());
                    }
				}
                for (int i = 0; i < numRefs; i++) {
                    string name = clos->func->free_vars_[i];
                    newFrame->setRefVar(name, clos->refs[i]);
                }
				vptr<NativeFunction> nativeFunc = dynamic_cast<NativeFunction*>(clos->func);
				if (nativeFunc != NULL) {
					vptr<Constant> val = nativeFunc->evalNativeFunction(*newFrame, *collector);
                    frame->opStackPush(val);
				} else {
					newOffset = 0;
                    frames.push_back(newFrame);
				}
                break;
            }
        case Operation::Return:
            {
                // take return val from top of stack & discard current frame
                auto returnVal = frame->opStackPeek();
                frames.pop_back();
                if (frames.empty()) {
                    finished = true;
                    return;
                }
                frame = frames.back();
                // push return val to top of new parent frame
                frame->opStackPush(returnVal);
                break;
            }
        case Operation::Add:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                // try adding strings if left or right is a string
                vptr<String> leftStr = dynamic_cast<String*>(left);
                if (leftStr != NULL) {
                    frame->opStackPush(
                        collector->allocate<String>(leftStr->value + right->toString()));
                    break;
                }
                vptr<String> rightStr = dynamic_cast<String*>(right);
                if (rightStr != NULL) {
                    frame->opStackPush(
                        collector->allocate<String>(left->toString() + rightStr->value));
                    break;
                }
                // try adding integers if left is an int
                vptr<Integer> leftInt = dynamic_cast<Integer*>(left);
                if (leftInt != NULL) {
                    int leftI = leftInt->value;
                    int rightI = right->cast<Integer>()->value;
                    frame->opStackPush(
                        collector->allocate<Integer>(leftI + rightI));
                    break;
                }
                break;
            }
        case Operation::Sub:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Integer>(left - right));
                break;
            }
        case Operation::Mul:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                       collector->allocate<Integer>(left * right));
                break;
            }
        case Operation::Div:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                if (right == 0) {
                    throw IllegalArithmeticException("cannot divide by 0");
                }
                frame->opStackPush(
                        collector->allocate<Integer>(left / right));
                break;
            }
        case Operation::Neg:
            {
                int top = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Integer>(-top));
                break;
            }
        case Operation::Gt:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left > right));
                break;
            }
        case Operation::Geq:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left >= right));
                break;
            }
        case Operation::Eq:
            {
                vptr<Value> right = frame->opStackPop();
                auto left = frame->opStackPop();
                frame->opStackPush(
                        collector->allocate<Boolean>(left->equals(right)));
                break;
            }
        case Operation::And:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left && right));
                break;
            }
        case Operation::Or:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left || right));
                break;
            }
        case Operation::Not:
            {
                bool top = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(!top));
                break;
            }
        case Operation::Goto:
            {
                newOffset = inst.operand0.value();
                break;
            }
        case Operation::If:
            {
                auto e = frame->opStackPop()->cast<Boolean>();
                if (e->value) {
                    newOffset = inst.operand0.value();
                }
                break;
            }
        case Operation::Dup:
            {
                auto top = frame->opStackPeek();
                frame->opStackPush(top);
                break;
            }
        case Operation::Swap:
            {
                auto top = frame->opStackPop();
                auto next = frame->opStackPop();
                frame->opStackPush(top);
                frame->opStackPush(next);
                break;
            }
        case Operation::Pop:
            {
                frame->opStackPop();
                break;
            }
        default:
            throw RuntimeException("should never get here - invalid instruction");
    }
	collector->gc();
    if (frames.size() == 1 && frame->instructionIndex + newOffset == frame->numInstructions()) {
        // last instruction of the whole program
        finished = true;
        return;
    }
    if (frame->instructionIndex + newOffset == frame->numInstructions()) {
        // last instruction of current function
        frames.pop_back();
        frame = frames.back();
    }
    frame->moveToInstruction(newOffset);
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    while (!finished) {
        executeStep();
    }
    // LOG("program finished");
};
