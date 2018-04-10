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

Interpreter::Interpreter(shared_ptr<Function> mainFunc) {
    int numLocals = mainFunc->names_.size();
    int numRefs = 0;
    if (mainFunc->local_reference_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame with nonzero ref vars");
    }
    if (mainFunc->free_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame with nonzero free vars");
    }
    if (mainFunc->local_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame with nonzero local vars");
    }
    mainFunc->local_vars_ = mainFunc->names_;
    shared_ptr<Frame> frame = make_shared<Frame>(Frame(mainFunc));
    globalFrame = frame;
    frames.push(frame);
    finished = false;

	vector<shared_ptr<Function>> functions_;
    vector<shared_ptr<Constant>> constants_;
	vector<string> args0 ;
	vector<string> args1 = { string("s") };
    vector<string> local_reference_vars_;
    vector<string> free_vars_;
	vector<string> names_;
    InstructionList instructions;
	vector<shared_ptr<Function>> frameFuncs;
	// add native functions at the beginning of functions array
	frame->func->functions_[0] = make_shared<PrintNativeFunction>(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions);
	frame->func->functions_[1] = make_shared<InputNativeFunction>(functions_, constants_, 0, args0, local_reference_vars_, free_vars_, names_, instructions);
	frame->func->functions_[2] = make_shared<IntcastNativeFunction>(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions);
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    shared_ptr<Frame> frame = frames.top();
    Instruction& inst = frame->getCurrInstruction();
    int newOffset = 1;
    LOG("executing instruction " + to_string(frame->instructionIndex));
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
                auto value = dynamic_pointer_cast<Constant>(frame->opStackPop());
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
                auto value = dynamic_pointer_cast<Constant>(frame->opStackPop());
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
                auto valuePtr = dynamic_pointer_cast<ValuePtr>(frame->opStackPop());
                if (valuePtr == NULL) {
                    throw RuntimeException("expected ValuePtr on the stack for LoadReference");
                }
                frame->opStackPush(valuePtr->ptr);
                break;
            }
        case Operation::StoreReference:
            {
                auto value = dynamic_pointer_cast<Constant>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreReference");
                }
                auto valuePtr = dynamic_pointer_cast<ValuePtr>(frame->opStackPop());
                if (valuePtr == NULL) {
                    throw RuntimeException("expected ValuePtr on the stack for StoreReference");
                }
				*valuePtr->ptr = *value;
                break;
            }
        case Operation::AllocRecord:
            {
				frame->opStackPush(make_shared<Record>());
                break;
            }
        case Operation::FieldLoad:
            {
				Record* record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    record->value[field] = make_shared<None>();
                }
				frame->opStackPush(record->value[field]);
                break;
            }
        case Operation::FieldStore:
            {
				auto value = dynamic_pointer_cast<Constant>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for FieldStore");
                }
				Record* record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
				record->value[field] = value;
                break;
            }
        case Operation::IndexLoad:
            {
				string index = frame->opStackPop()->toString();
				Record* record = frame->opStackPop()->cast<Record>();
                if (record->value.count(index) == 0) {
                    record->value[index] = make_shared<None>();
                }
				frame->opStackPush(record->value[index]);
                break;
            }
        case Operation::IndexStore:
            {
				string index = frame->opStackPop()->toString();
				auto value = frame->opStackPop();
				Record* record = frame->opStackPop()->cast<Record>();
				record->value[index] = value;
                break;
            }
        case Operation::AllocClosure:
            {
                // read num free vars, ref vars, and function off the stack
                int numFreeVars = inst.operand0.value();
                vector<shared_ptr<ValuePtr>> refList;
                for (int i = 0; i < numFreeVars; i++) {
                    auto top = frame->opStackPop();
                    auto value = dynamic_pointer_cast<ValuePtr>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected ValuePtr on the stack for AllocClosure");
                    }
                    refList.push_back(value);
                }
                auto func = dynamic_pointer_cast<Function>(frame->opStackPop());
                if (func == NULL) {
                    throw RuntimeException("expected Function on the stack for AllocClosure");
                }

                if (numFreeVars != func->free_vars_.size()) {
                    throw RuntimeException("expected " + to_string(func->free_vars_.size()) + " reference variables but got " + to_string(numFreeVars));
                }

                // push new closure onto the stack
                frame->opStackPush(make_shared<Closure>(refList, func));
                break;
            }
        case Operation::Call:
            {
                // read num arguments, argument values, and closure off the stack
                int numArgs = inst.operand0.value();
                vector<shared_ptr<Constant>> argsList;
                for (int i = 0; i < numArgs; i++) {
                    auto top = frame->opStackPop();
                    auto value = dynamic_pointer_cast<Constant>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected Constant on the stack for Call");
                    }
                    argsList.push_back(value);
                }
				reverse(argsList.begin(), argsList.end());
                auto clos = dynamic_pointer_cast<Closure>(frame->opStackPop());
                if (clos == NULL) {
                    throw RuntimeException("expected Closure on operand stack for function call");
                }

                if (numArgs != clos->func->parameter_count_) {
                    throw RuntimeException("expected " + to_string(clos->func->parameter_count_) + " arguments, got " + to_string(numArgs));
                }

                // process local refs and local vars
                int numLocals = clos->func->local_vars_.size();
                int numRefs = clos->func->free_vars_.size();
                shared_ptr<Frame> newFrame = make_shared<Frame>(Frame(clos->func));
				for (int i = 0; i < numLocals; i++) {
                    if (i < numArgs) {
                        string name = clos->func->local_vars_[i];
                        newFrame->setLocalVar(name, argsList[i]);
                    } else {
                        string name = clos->func->local_vars_[i];
                        newFrame->setLocalVar(name, make_shared<None>());
                    }
				}
                for (int i = 0; i < numRefs; i++) {
                    string name = clos->func->free_vars_[i];
                    newFrame->setRefVar(name, clos->refs[i]);
                }
				auto nativeFunc = dynamic_pointer_cast<NativeFunction>(clos->func);
				if (nativeFunc != NULL) {
					shared_ptr<Constant> val = nativeFunc->evalNativeFunction(*newFrame);
					if (dynamic_pointer_cast<None>(val) == NULL) {
						frame->opStackPush(val);
					}
				} else {
					newOffset = 0;
                    frames.push(newFrame);
				}
                break;
            }
        case Operation::Return:
            {
                // take return val from top of stack & discard current frame
                auto returnVal = frame->opStackPeek();
                frames.pop();
                frame = frames.top();
                // push return val to top of new parent frame
                frame->opStackPush(returnVal);
                break;
            }
        case Operation::Add:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                // try adding strings if left or right is a string
                auto leftStr = dynamic_pointer_cast<String>(left);
                if (leftStr != NULL) {
                    frame->opStackPush(
                        make_shared<String>(leftStr->value + right->toString()));
                    break;
                }
                auto rightStr = dynamic_pointer_cast<String>(right);
                if (rightStr != NULL) {
                    frame->opStackPush(
                        make_shared<String>(left->toString() + rightStr->value));
                    break;
                }
                // try adding integers if left is an int
                auto leftInt = dynamic_pointer_cast<Integer>(left);
                if (leftInt != NULL) {
                    int leftI = leftInt->value;
                    int rightI = right->cast<Integer>()->value;
                    frame->opStackPush(
                        make_shared<Integer>(leftI + rightI));
                    break;
                }
                break;
            }
        case Operation::Sub:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        make_shared<Integer>(left - right));
                break;
            }
        case Operation::Mul:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        make_shared<Integer>(left * right));
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
                        make_shared<Integer>(left / right));
                break;
            }
        case Operation::Neg:
            {
                int top = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        make_shared<Integer>(-top));
                break;
            }
        case Operation::Gt:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        make_shared<Boolean>(left > right));
                break;
            }
        case Operation::Geq:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        make_shared<Boolean>(left >= right));
                break;
            }
        case Operation::Eq:
            {
                shared_ptr<Value> right = frame->opStackPop();
                auto left = frame->opStackPop();
                frame->opStackPush(
                        make_shared<Boolean>(left->equals(right)));
                break;
            }
        case Operation::And:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        make_shared<Boolean>(left && right));
                break;
            }
        case Operation::Or:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        make_shared<Boolean>(left || right));
                break;
            }
        case Operation::Not:
            {
                bool top = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        make_shared<Boolean>(!top));
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

    if (frames.size() == 1 && frame->instructionIndex + newOffset == frame->numInstructions()) {
        // last instruction of the whole program
        finished = true;
        return;
    }
    if (frame->instructionIndex + newOffset == frame->numInstructions()) {
        // last instruction of current function
        frames.pop();
        frame = frames.top();
    }
    frame->moveToInstruction(newOffset);
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    while (!finished) {
        executeStep();
    }
    LOG("program finished");
};
