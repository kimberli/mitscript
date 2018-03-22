#include "exception.h"
#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include <stack>
#include "types.h"

using namespace std;

Interpreter::Interpreter(Function* mainFunc) {
    frames = new stack<Frame*>();
    Frame* frame = new Frame();
    globalFrame = frame;
    frame->instructionPtr = &mainFunc->instructions.front();
    frame->func = mainFunc;
    frames->push(frame);
    finished = false;
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    Frame* frame = frames->top();
    if (frame->instructionPtr == &frame->func->instructions.back()) {
        finished = true;
    }
    Instruction* inst = frame->instructionPtr;
    switch (inst->operation) {
        case Operation::LoadConst: 
            {
		frame->operandStack.push(*frame->func->constants_[inst->operand0.value()]);
                break;
            }
        case Operation::LoadFunc: 
            {
		frame->operandStack.push(*frame->func->functions_[inst->operand0.value()]);
                break;
            }
        case Operation::LoadLocal: 
            {
		string localVar = frame->func->local_vars_[inst->operand0.value()];
		frame->operandStack.push(frame->localVars[localVar]);
                break;
            }
        case Operation::StoreLocal: 
            {
		string localVar = frame->func->local_vars_[inst->operand0.value()];
		Value value = frame->operandStack.top();
		frame->operandStack.pop();
		frame->localVars[localVar] = value;
                break;
            }
        case Operation::LoadGlobal: 
            {
		string globalVar = frame->func->names_[inst->operand0.value()];
		frame->operandStack.push(globalFrame->localVars[globalVar]);
                break;
            }
        case Operation::StoreGlobal: 
            {
		string globalVar = frame->func->names_[inst->operand0.value()];
		Value value = frame->operandStack.top();
		frame->operandStack.pop();
		globalFrame->localVars[globalVar] = value;
                break;
            }
        case Operation::PushReference: 
            {
		string localRef = frame->func->local_reference_vars_[inst->operand0.value()];
		frame->operandStack.push(*(new ValuePtr(frame->localRefs[localRef])));
                break;
            }
        case Operation::LoadReference: 
            {
		Value* ref = &frame->operandStack.top();
		auto valuePtr = dynamic_cast<ValuePtr*>(ref);
		if (valuePtr == NULL) {
		    throw RuntimeException("Not a reference");
		}
		Value* ptr = valuePtr->ptr;
		frame->operandStack.pop();
		frame->operandStack.push(*ptr);
                break;
            }
        case Operation::StoreReference: 
            {
		string globalVar = frame->func->names_[inst->operand0.value()];
		Value value = frame->operandStack.top();
		frame->operandStack.pop();
		globalFrame->localVars[globalVar] = value;
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
