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
    if (frame->instructionPtr == &frame->func.instructions.back()) {
        finished = true;
    }
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
                auto value = frame->operandStack.top();
                frame->operandStack.pop();
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
                auto value = frame->operandStack.top();
                frame->operandStack.pop();
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
                Value* ref = frame->operandStack.top().get();
                auto valuePtr = dynamic_cast<ValuePtr*>(ref);
                if (valuePtr == NULL) {
                    throw RuntimeException("Not a reference");
                }
                auto ptr = valuePtr->ptr;
                frame->operandStack.pop();
                frame->operandStack.push(ptr);
                break;
            }
        case Operation::StoreReference:
            {
                auto value = frame->operandStack.top();
                frame->operandStack.pop();
                Value* ref = frame->operandStack.top().get();
				auto valuePtr = dynamic_cast<ValuePtr*>(ref);
				if (valuePtr == NULL) {
					throw RuntimeException("Not a reference");
				}
                frame->operandStack.pop();
				*valuePtr->ptr = *value.get();
                break;
            }
        case Operation::AllocRecord:
            {
                break;
            }
        case Operation::FieldLoad:
            {
                break;
            }
        case Operation::FieldStore:
            {
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
                break;
            }
        case Operation::Or:
            {
                break;
            }
        case Operation::Not:
            {
                break;
            }
        case Operation::Goto:
            {
                break;
            }
        case Operation::If:
            {
                break;
            }
        case Operation::Dup:
            {
                auto top = frame->operandStack.top();
                frame->operandStack.push(top);
                break;
            }
        case Operation::Swap:
            {
                auto top = frame->operandStack.top();
                auto next = frame->operandStack.top();
                frame->operandStack.pop();
                frame->operandStack.pop();
                frame->operandStack.push(top);
                frame->operandStack.push(next);
                break;
            }
        case Operation::Pop:
            {
                frame->operandStack.pop();
                break;
            }
    }
    frame->instructionPtr++;
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    while (!finished) {
        executeStep();
    }
};
