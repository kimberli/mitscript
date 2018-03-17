#include "interpreter.h"
#include "types.h"
#include "frame.h"

using namespace std;

Interpreter::Interpreter(Function* currentFunc): currentFunc(currentFunc) {};
