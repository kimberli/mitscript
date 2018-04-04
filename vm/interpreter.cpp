#include "exception.h"
#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include "types.h"
#include <stack>

#define DEBUG(msg) { if (1) cout << msg << endl; }

using namespace std;

Interpreter::Interpreter(Function& mainFunc) {
    shared_ptr<Frame> frame = std::make_shared<Frame>(Frame(0, mainFunc));
    globalFrame = frame;
    frames.push(frame);
    finished = false;
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    shared_ptr<Frame> frame = frames.top();
    Instruction& inst = frame->getCurrInstruction();
    int newOffset = 1;
    DEBUG("executing instruction " + std::to_string(frame->instructionIndex));
    // TODO: remove .get() in shared_ptr calls
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
                auto valuePtr = std::make_shared<ValuePtr>(frame->getRefVar(refName));
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
				*valuePtr.get()->ptr = *value.get();
                break;
            }
        case Operation::AllocRecord:
            {
				frame->opStackPush(std::make_shared<Record>());
                break;
            }
        case Operation::FieldLoad:
            {
                // TODO: check for local var refs (not allowed)
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
                // TODO: check for local var refs (not allowed)
				auto value = frame->opStackPop();
				auto record = frame->opStackPop().get()->cast<Record>()->value;
				string field = frame->getNameByIndex(inst.operand0.value());
				record[field] = value;
                break;
            }
        case Operation::IndexLoad:
            {
                // TODO: check for local var refs (not allowed)
                break;
            }
        case Operation::IndexStore:
            {
                // TODO: check for local var refs (not allowed)
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
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                // try adding integers if left is an int
                auto leftInt = dynamic_cast<Integer*>(left.get());
                if (leftInt != NULL) {
                    int leftI = leftInt->value;
                    int rightI = right->cast<Integer>()->value;
                    frame->opStackPush(
                        std::make_shared<Integer>(leftI + rightI));
                    break;
                }
                // try adding strings if left or right is a string
                auto leftStr = dynamic_cast<String*>(left.get());
                if (leftStr != NULL) {
                    frame->opStackPush(
                        std::make_shared<String>(leftStr->value + right->toString()));
                    break;
                }
                auto rightStr = dynamic_cast<String*>(right.get());
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
                if (right == 0) {
                    throw IllegalArithmeticException("cannot divide by 0");
                }
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
                // TODO: throw an exception when trying to go to nonexistent last index of instructions?
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
