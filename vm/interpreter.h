#pragma once

#include "types.h"

using namespace std;

class Interpreter {
private:
    Function* mainFunc;
public:
    Interpreter(Function* mainFunc);
};
