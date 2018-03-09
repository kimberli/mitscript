#pragma once

#include <string>

using namespace std;

class InterpreterException : public exception {
    public:
	    string& message;
        InterpreterException(string& message): message(message) {};
        virtual string toString() = 0;
};

class UninitializedVariableException: public InterpreterException {
    public:
	    UninitializedVariableException(string message):
            InterpreterException(message) {};
        string toString() {
            return "UninitializedVariableException: " + message;
        };
};

class IllegalCastException : public InterpreterException {
    public:
	    IllegalCastException(string message):
            InterpreterException(message) {};
        string toString() {
            return "IllegalCastException: " + message;
        };
};

class IllegalArithmeticException : public InterpreterException {
    public:
	    IllegalArithmeticException(string message):
            InterpreterException(message) {};
        string toString() {
            return "IllegalArithmeticException: " + message;
        };
};

class RuntimeException : public InterpreterException {
    public:
	    RuntimeException(string message): InterpreterException(message) {};
        string toString() {
            return "RuntimeException: " + message;
        };
};
