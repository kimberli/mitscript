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
                Value* ref = frame->opStackPop().get();
                auto valuePtr = ref->cast<ValuePtr>();
                frame->operandStack.push(valuePtr->ptr);
                break;
            }
        case Operation::StoreReference:
            {
                auto value = frame->opStackPop();
                Value* ref = frame->opStackPop().get();
				auto valuePtr = ref->cast<ValuePtr>();
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
				auto record = frame->opStackPop()->cast<Record>();
				string field = frame->func.names_[inst->operand0.value()];
				auto it = record->value.find(field);
				if (it == record->value.end()) {
					record->value[field] = std::make_shared<None>();
				}
				frame->operandStack.push(record->value[field]);
                break;
            }
        case Operation::FieldStore:
            {
				auto value = frame->opStackPop();
				auto record = frame->opStackPop()->cast<Record>();
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
                break;
            }
        case Operation::Sub:
            {
                break;
            }
        case Operation::Mul:
            {
                break;
            }
        case Operation::Div:
            {
                break;
            }
        case Operation::Neg:
            {
                break;
            }
        case Operation::Gt:
            {
                break;
            }
        case Operation::Geq:
            {
                break;
            }
        case Operation::Eq:
            {
                break;
            }
        case Operation::And:
            {
                Value* right = frame->opStackPop().get();
                Value* left = frame->opStackPop().get();
                bool rightE = right->cast<Boolean>()->value;
                bool leftE = left->cast<Boolean>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(leftE && rightE));
                break;
            }
        case Operation::Or:
            {
                Value* right = frame->opStackPop().get();
                Value* left = frame->opStackPop().get();
                bool rightE = right->cast<Boolean>()->value;
                bool leftE = left->cast<Boolean>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(leftE || rightE));
                break;
            }
        case Operation::Not:
            {
                Value* top = frame->opStackPop().get();
                bool e = top->cast<Boolean>()->value;
                frame->operandStack.push(
                        std::make_shared<Boolean>(!e));
                break;
            }
        case Operation::Goto:
            {
                // move to offset - 1 since we increment at the end
                int offset = inst->operand0.value();
                frame->moveToInstruction(offset - 1);
                break;
            }
        case Operation::If:
            {
                Value* top = frame->opStackPop().get();
                auto e = top->cast<Boolean>();
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
