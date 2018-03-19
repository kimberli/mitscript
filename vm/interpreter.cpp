#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include <stack>
#include "types.h"

using namespace std;

Interpreter::Interpreter(Function* mainFunc) {
    frames = new stack<Frame*>();
    Frame* frame = new Frame();
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
    switch (frame->instructionPtr->operation) {
        case Operation::LoadConst: 
            {
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
