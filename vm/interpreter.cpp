#include "interpreter.h"
#include "types.h"
#include "frame.h"

using namespace std;

Interpreter::Interpreter(Function* mainFunc): currentFunc(mainFunc) {};
