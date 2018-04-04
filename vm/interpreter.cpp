#include "exception.h"
#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include "types.h"
#include <stack>

using namespace std;

Interpreter::Interpreter(Function* mainFunc) {
    frames = new stack<Frame*>();
    Frame* frame = new Frame(0, *mainFunc);
    globalFrame = frame;
    frames->push(frame);
    finished = false;
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    Frame* frame = frames->top();
    Instruction& inst = frame->getCurrInstruction();
    int newOffset = 1;
    // TODO: debugging only; remove this later
    cout << "executing instruction " << frame->instructionIndex << endl;
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
                string localVar = frame->getLocalVarByIndex(inst.operand0.value());
                frame->opStackPush(frame->localVars[localVar]);
                break;
            }
        case Operation::StoreLocal:
            {
                string localVar = frame->getLocalVarByIndex(inst.operand0.value());
                auto value = frame->opStackPop();
                frame->localVars[localVar] = value;
                break;
            }
        case Operation::LoadGlobal:
            {
                string globalVar = frame->getNameByIndex(inst.operand0.value());
                frame->opStackPush(globalFrame->localVars[globalVar]);
                break;
            }
        case Operation::StoreGlobal:
            {
                string globalVar = frame->getNameByIndex(inst.operand0.value());
                auto value = frame->opStackPop();
                globalFrame->localVars[globalVar] = value;
                break;
            }
        case Operation::PushReference:
            {
                string localRef = frame->getRefVarByIndex(inst.operand0.value());
                auto valuePtr = std::make_shared<ValuePtr>(frame->localRefs[localRef]);
                frame->opStackPush(valuePtr);
                break;
            }
        case Operation::LoadReference:
            {
                ValuePtr* valuePtr = frame->opStackPop().get()->cast<ValuePtr>();
                frame->opStackPush(valuePtr->ptr);
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
				frame->opStackPush(std::make_shared<Record>());
                break;
            }
        case Operation::FieldLoad:
            {
				auto record = frame->opStackPop().get()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    record->value[field] = std::make_shared<None>();
                }
				frame->opStackPush(record->value[field]);
                break;
            }
        case Operation::FieldStore:
            {
				auto value = frame->opStackPop();
				Record* record = frame->opStackPop().get()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
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
                Value* right = frame->opStackPop().get();
                Value* left = frame->opStackPop().get();
                // try adding integers if left is an int
                auto leftInt = dynamic_cast<Integer*>(left);
                if (leftInt != NULL) {
                    int leftI = leftInt->value;
                    int rightI = right->cast<Integer>()->value;
                    frame->opStackPush(
                        std::make_shared<Integer>(leftI + rightI));
                    break;
                }
                // try adding strings if left or right is a string
                auto leftStr = dynamic_cast<String*>(left);
                if (leftStr != NULL) {
                    frame->opStackPush(
                        std::make_shared<String>(leftStr->value + right->toString()));
                    break;
                }
                auto rightStr = dynamic_cast<String*>(right);
                if (rightStr != NULL) {
                    frame->opStackPush(
                        std::make_shared<String>(left->toString() + rightStr->value));
                    break;
                }
                break;
            }
        case Operation::Sub:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->opStackPush(
                        std::make_shared<Integer>(left - right));
                break;
            }
        case Operation::Mul:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->opStackPush(
                        std::make_shared<Integer>(left * right));
                break;
            }
        case Operation::Div:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->opStackPush(
                        std::make_shared<Integer>(left / right));
                break;
            }
        case Operation::Neg:
            {
                int top = frame->opStackPop().get()->cast<Integer>()->value;
                frame->opStackPush(
                        std::make_shared<Integer>(-top));
                break;
            }
        case Operation::Gt:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->opStackPush(
                        std::make_shared<Boolean>(left > right));
                break;
            }
        case Operation::Geq:
            {
                int right = frame->opStackPop().get()->cast<Integer>()->value;
                int left = frame->opStackPop().get()->cast<Integer>()->value;
                frame->opStackPush(
                        std::make_shared<Boolean>(left >= right));
                break;
            }
        case Operation::Eq:
            {
                shared_ptr<Value> right = frame->opStackPop();
                Value* left = frame->opStackPop().get();
                frame->opStackPush(
                        std::make_shared<Boolean>(left->equals(right)));
                break;
            }
        case Operation::And:
            {
                bool right = frame->opStackPop().get()->cast<Boolean>()->value;
                bool left = frame->opStackPop().get()->cast<Boolean>()->value;
                frame->opStackPush(
                        std::make_shared<Boolean>(left && right));
                break;
            }
        case Operation::Or:
            {
                bool right = frame->opStackPop().get()->cast<Boolean>()->value;
                bool left = frame->opStackPop().get()->cast<Boolean>()->value;
                frame->opStackPush(
                        std::make_shared<Boolean>(left || right));
                break;
            }
        case Operation::Not:
            {
                bool top = frame->opStackPop().get()->cast<Boolean>()->value;
                frame->opStackPush(
                        std::make_shared<Boolean>(!top));
                break;
            }
        case Operation::Goto:
            {
                newOffset = inst.operand0.value();
                break;
            }
        case Operation::If:
            {
                auto e = frame->opStackPop().get()->cast<Boolean>();
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
    }

    if (frame->instructionIndex + newOffset == globalFrame->numInstructions()) {
        // last instruction of the whole program
        finished = true;
        return;
    }
    if (frame->instructionIndex + newOffset == frame->numInstructions()) {
        // last instruction of current function
        // TODO destruct current frame
        frames->pop();
        frame = frames->top();
    }
    frame->moveToInstruction(newOffset);
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    while (!finished) {
        executeStep();
    }
};
