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
#include <stack>

#define DEBUG(msg) { if (1) cout << msg << endl; }

using namespace std;

Interpreter::Interpreter(shared_ptr<Function> mainFunc) {
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
	frameFuncs.push_back(make_shared<PrintNativeFunction>(*(new PrintNativeFunction(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions))));
	frameFuncs.push_back(make_shared<InputNativeFunction>(*(new InputNativeFunction(functions_, constants_, 0, args0, local_reference_vars_, free_vars_, names_, instructions))));
	frameFuncs.push_back(make_shared<IntcastNativeFunction>(*(new IntcastNativeFunction(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions))));
	frameFuncs.insert(frameFuncs.end(), frame.get()->func.get()->functions_.begin(), frame.get()->func.get()->functions_.end());
	frame.get()->func.get()->functions_ = frameFuncs;
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    shared_ptr<Frame> frame = frames.top();
    Instruction& inst = frame->getCurrInstruction();
    int newOffset = 1;
    DEBUG("executing instruction " + to_string(frame->instructionIndex));
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
                string localName = frame->getLocalVarByIndex(inst.operand0.value());
                frame->opStackPush(frame->getLocalVar(localName));
                break;
            }
        case Operation::StoreLocal:
            {
                string localName = frame->getLocalVarByIndex(inst.operand0.value());
                auto value = dynamic_pointer_cast<Constant>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("cannot store non-constant value as local var");
                }
                frame->setLocalVar(localName, value);
                break;
            }
        case Operation::LoadGlobal:
            {
                string globalName = frame->getNameByIndex(inst.operand0.value());
                frame->opStackPush(globalFrame->getLocalVar(globalName));
                break;
            }
        case Operation::StoreGlobal:
            {
                string globalName = frame->getNameByIndex(inst.operand0.value());
                auto value = dynamic_pointer_cast<Constant>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("cannot store non-constant value as global var");
                }
                globalFrame->setLocalVar(globalName, value);
                break;
            }
        case Operation::PushReference:
            {
                string refName = frame->getRefVarByIndex(inst.operand0.value());
                auto valuePtr = make_shared<ValuePtr>(frame->getRefVar(refName));
                frame->opStackPush(valuePtr);
                break;
            }
        case Operation::LoadReference:
            {
                auto valuePtr = dynamic_pointer_cast<ValuePtr>(frame->opStackPop());
                if (valuePtr == NULL) {
                    throw RuntimeException("cannot store non-value ptr as ref var");
                }
                frame->opStackPush(valuePtr);
                break;
            }
        case Operation::StoreReference:
            {
                auto value = frame->opStackPop();
                auto valuePtr = dynamic_pointer_cast<ValuePtr>(frame->opStackPop());
                if (valuePtr == NULL) {
                    throw RuntimeException("cannot store non-value ptr as ref var");
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
                // TODO: check for local var refs (not allowed)
				auto record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    record->value[field] = make_shared<None>();
                }
				frame->opStackPush(record->value[field]);
                break;
            }
        case Operation::FieldStore:
            {
                // TODO: check for local var refs (not allowed)
				auto value = frame->opStackPop();
				auto record = frame->opStackPop()->cast<Record>()->value;
				string field = frame->getNameByIndex(inst.operand0.value());
				record[field] = value;
                break;
            }
        case Operation::IndexLoad:
            {
                // TODO: check for local var refs (not allowed)
				auto record = frame->opStackPop()->cast<Record>();
				string index = frame->getConstantByIndex(inst.operand0.value()).get()->toString();
                if (record->value.count(index) == 0) {
                    record->value[index] = make_shared<None>();
                }
				frame->opStackPush(record->value[index]);
                break;
            }
        case Operation::IndexStore:
            {
                // TODO: check for local var refs (not allowed)
				auto value = frame->opStackPop();
				auto record = frame->opStackPop()->cast<Record>()->value;
				string index = frame->getConstantByIndex(inst.operand0.value()).get()->toString();
				record[index] = value;
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
                        throw RuntimeException("free variable in closure alloc is not a reference");
                    }
                    refList.push_back(value);
                }
                auto func = dynamic_pointer_cast<Function>(frame->opStackPop());
                if (func == NULL) {
                    throw RuntimeException("expected Function on operand stack for closure alloc");
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
                        throw RuntimeException("call argument is not a constant");
                    }
                    argsList.push_back(value);
                }
                auto clos = dynamic_pointer_cast<Closure>(frame->opStackPop());
                if (clos == NULL) {
                    throw RuntimeException("expected Closure on operand stack for function call");
                }

                if (numArgs != clos->func->parameter_count_) {
                    throw RuntimeException("mismatched arguments in function call");
                }

                // process local refs and local vars
                LocalVarMap localVars;
                LocalRefMap localRefs;
                for (int i = 0; i < clos->refs.size(); i++) {
                    string free_var = clos->func->free_vars_[i];
                    localRefs[free_var] = clos->refs[i];
                }
                for (int i = 0; i < numArgs; i++) {
                    string arg = clos->func->local_vars_[i];
                    localVars[arg] = argsList[i];
                }
                shared_ptr<Frame> newFrame = make_shared<Frame>(Frame(clos->func, localVars, localRefs));
				auto nativeFunc = dynamic_cast<NativeFunction*>(clos->func.get());
				if (nativeFunc != NULL) {
					shared_ptr<Constant> val = nativeFunc->evalNativeFunction(*newFrame.get());
					if (dynamic_cast<None*>(val.get()) == NULL) {
						frame->opStackPush(val);
					}
				} else {
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
                // try adding integers if left is an int
                auto leftInt = dynamic_cast<Integer*>(left.get());
                if (leftInt != NULL) {
                    int leftI = leftInt->value;
                    int rightI = right->cast<Integer>()->value;
                    frame->opStackPush(
                        make_shared<Integer>(leftI + rightI));
                    break;
                }
                // try adding strings if left or right is a string
                auto leftStr = dynamic_cast<String*>(left.get());
                if (leftStr != NULL) {
                    frame->opStackPush(
                        make_shared<String>(leftStr->value + right->toString()));
                    break;
                }
                auto rightStr = dynamic_cast<String*>(right.get());
                if (rightStr != NULL) {
                    frame->opStackPush(
                        make_shared<String>(left->toString() + rightStr->value));
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
                // TODO: throw an exception when trying to go to nonexistent last index of instructions?
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

    if (frame->instructionIndex + newOffset == globalFrame->numInstructions()) {
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
    DEBUG("program finished");
};
