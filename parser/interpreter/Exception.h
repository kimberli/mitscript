#include <string>

class InterpreterException : public exception {
    public:
	    string message;
        InterpreterException(string message): message(message) {};
};

class UninitializedVariableException: public InterpreterException {
    public:
	    UninitializedVariableException():
            InterpreterException("UninitializedVariableException") {};
};

class IllegalCastException : public InterpreterException {
    public:
	    IllegalCastException():
            InterpreterException("IllegalCastException") {};
};

class IllegalArithmeticException : public InterpreterException {
    public:
	    IllegalArithmeticException():
            InterpreterException("IllegalArithmeticException") {};
};

class RuntimeException : public InterpreterException {
    public:
	    RuntimeException():
            InterpreterException("RuntimeException") {};
};
