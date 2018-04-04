#include "exception.h"
#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include "types.h"
#include <stack>

using namespace std;

Interpreter::Interpreter(Function* mainFunc) {
    frames = new stack<Frame*>();
    Instruction* ip = &mainFunc->instructions.front();
    Frame* frame = new Frame(ip, *mainFunc);
    globalFrame = frame;
    frames->push(frame);
    finished = false;
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    Frame* frame = frames->top();
    Instruction* inst = frame->instructionPtr;
    switch (inst->operation) {
        case Operation::LoadConst:
            {
                auto constant = frame->func.constants_[inst->operand0.value()];
                frame->operandStack.push(constant);
                break;
            }
        case Operation::LoadFunc:
            {
                auto func = frame->func.functions_[inst->operand0.value()];
                frame->operandStack.push(func);
                break;
            }
        case Operation::LoadLocal:
            {
                string localVar = frame->func.local_vars_[inst->operand0.value()];
                frame->operandStack.push(frame->localVars[localVar]);
                break;
            }
        case Operation::StoreLocal:
            {
                string localVar = frame->func.local_vars_[inst->operand0.value()];
                auto value = frame->opStackPop();
                frame->localVars[localVar] = value;
                break;
            }
        case Operation::LoadGlobal:
            {
                string globalVar = frame->func.names_[inst->operand0.value()];
                frame->operandStack.push(globalFrame->localVars[globalVar]);
                break;
            }
        case Operation::StoreGlobal:
            {
                string globalVar = frame->func.names_[inst->operand0.value()];
                auto value = frame->opStackPop();
                globalFrame->localVars[globalVar] = value;
                break;
            }
        case Operation::PushReference:
            {
                string localRef = frame->func.
                    local_reference_vars_[inst->operand0.value()];
                auto valuePtr = std::make_shared<ValuePtr>(frame->localRefs[localRef]);
                frame->operandStack.push(valuePtr);
                break;
            }
        case Operation::LoadReference:
            {
                ValuePtr* valuePtr = frame->opStackPop().get()->cast<ValuePtr>();
                frame->operandStack.push(valuePtr->ptr);
                break;
            }
        case Operation::StoreReference:
            {
                auto value = frame->opStackPop();
                ValuePtr* valuePtr = frame->opStackPop().get()->cast<ValuePtr>();
				*valuePtr->ptr.get() = *value.get();
                break;
            }
        case Operation::AllocRecord:
            {
				frame->operandStack.push(
					std::make_shared<Record>());
                break;
            }
        case Operation::FieldLoad:
            {
				auto record = frame->opStackPop().get()->cast<Record>();
				string field = frame->func.names_[inst->operand0.value()];
                if (record->value.count(field) == 0) {
                    record->value[field] = std::make_shared<None>();
                }
				frame->operandStack.push(record->value[field]);
                break;
            }
        case Operation::FieldStore:
            {
				auto value = frame->opStackPop();
				Record* record = frame->opStackPop().get()->cast<Record>();
				string field = frame->func.names_[inst->operand0.value()];
				record->value[field] = value;
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
                // TODO double-check operand order
                Value* right = frame->opStackPop().get();
                Value* left = frame->opStackPop().get();
                // try adding integers if left is an int
                auto leftInt = dynamic_cast<Integer*>(left);
                if (leftInt != NULL) {
                    int leftI = leftInt->value;
                    int rightI = right->cast<Integer>()->value;
                    frame->operandStack.push(
                        std::make_shared<Integer>(leftI + rightI));
                    break;
                }
                // try adding strings if left or right is a string
                auto leftStr = dynamic_cast<String*>(left);
                if (leftStr != NULL) {
                    frame->operandStack.push(
                        std::make_shared<String>(leftStr->value + right->toString()));
                    break;
                }
                auto rightStr = dynamic_cast<String*>(right);
                if (rightStr != NULL) {
                    frame->operandStack.push(
                        std::make_shared<String>(left->toString() + rightStr->value));
                    break;
                }
                break;
            }
        case Operation::Sub:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->operandStack.push(
                        std::make_shared<Integer>(left - right));
                break;
            }
        case Operation::Mul:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->operandStack.push(
                        std::make_shared<Integer>(left * right));
                break;
            }
        case Operation::Div:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->operandStack.push(
                        std::make_shared<Integer>(left / right));
                break;
            }
        case Operation::Neg:
            {
                int top = frame->opStackPop().get()->cast<Integer>()->value;
                frame->operandStack.push(
                        std::make_shared<Integer>(-top));
                break;
            }
        case Operation::Gt:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(left > right));
                break;
            }
        case Operation::Geq:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(left >= right));
                break;
            }
        case Operation::Eq:
            {
                shared_ptr<Value> right = frame->opStackPop();
                Value* left = frame->opStackPop().get();
                frame->operandStack.push(
                        std::make_shared<Boolean>(left->equals(right)));
                break;
            }
        case Operation::And:
            {
                bool right = frame->opStackPop().get()->cast<Boolean>()->value;
                bool left = frame->opStackPop().get()->cast<Boolean>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(left && right));
                break;
            }
        case Operation::Or:
            {
                bool right = frame->opStackPop().get()->cast<Boolean>()->value;
                bool left = frame->opStackPop().get()->cast<Boolean>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(left || right));
                break;
            }
        case Operation::Not:
            {
                bool top = frame->opStackPop().get()->cast<Boolean>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(!top));
                break;
            }
        case Operation::Goto:
            {
                // move to offset - 1 since we increment at the end
                // TODO: error handling for out of bounds instructions
                int offset = inst->operand0.value();
                frame->moveToInstruction(offset - 1);
                break;
            }
        case Operation::If:
            {
                auto e = frame->opStackPop().get()->cast<Boolean>();
                if (e->value) {
                    // move to offset - 1 since we increment at the end
                    int offset = inst->operand0.value();
                    frame->moveToInstruction(offset - 1);
                }
                break;
            }
        case Operation::Dup:
            {
                // TODO make this less inefficient
                auto top = frame->opStackPop();
                frame->operandStack.push(top);
                frame->operandStack.push(top);
                break;
            }
        case Operation::Swap:
            {
                auto top = frame->opStackPop();
                auto next = frame->opStackPop();
                frame->operandStack.push(top);
                frame->operandStack.push(next);
                break;
            }
        case Operation::Pop:
            {
                frame->opStackPop();
                break;
            }
    }

    if (frame->instructionPtr == &globalFrame->func.instructions.back()) {
        // last instruction of the whole program
        finished = true;
        return;
    }
    if (frame->instructionPtr == &frame->func.instructions.back()) {
        // last instruction of current function
        // TODO destruct current frame
        frames->pop();
        frame = frames->top();
    }
    frame->instructionPtr++;
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    while (!finished) {
        executeStep();
    }
};
