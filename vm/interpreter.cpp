#include "frame.h"
#include "instructions.h"
#include "interpreter.h"
#include <stack>
#include "types.h"

using namespace std;

Interpreter::Interpreter(Function* mainFunc): currFunc(mainFunc) {
    frames = new stack<Frame*>();
    frames->push(new Frame());
    instructionPtr = &mainFunc->instructions.front();
    finished = false;
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    if (instructionPtr == &currFunc->instructions.back()) {
        finished = true;
    }
    switch (instructionPtr->operation) {
        case Operation::LoadConst: 
            {
                break;
            }
    }
    instructionPtr++;
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    while (!finished) {
        executeStep();
    }
};
