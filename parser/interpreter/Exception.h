#include <string>

class InterpreterException : public exception {
    public:
	    string message;
	    InterpreterException(const string& message): message(message) {};
};

class UninitializedVariableException: public InterpreterException {
    public:
	    UninitializedVariableException(const string& message):
            InterpreterException(message) {};
};

class IllegalCastException : public InterpreterException {
    public:
	    IllegalCastException(const string& message):
            InterpreterException(message) {};
};

class IllegalArithmeticException : public InterpreterException {
    public:
	    IllegalArithmeticException(const string& message):
            InterpreterException(message) {};
};

class RuntimeException : public InterpreterException {
    public:
	    RuntimeException(const string& message):
            InterpreterException(message) {};
};
