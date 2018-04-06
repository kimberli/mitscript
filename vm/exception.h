/*
 * exception.h
 *
 * Defines InterpreterExceptions raised during bytecode runtime
 */
#pragma once

#include <string>

using namespace std;

class InterpreterException : public exception {
    public:
	    string message;
        InterpreterException(string message): message(message) {};
        virtual string toString() = 0;
};

class UninitializedVariableException: public InterpreterException {
    // Thrown when code attempts to read from a variable that hasn't
    // been initialized
    public:
	    UninitializedVariableException(string message):
            InterpreterException(message) {};
        string toString() {
            return "UninitializedVariableException: " + message;
        };
};

class IllegalCastException : public InterpreterException {
    // Thrown when an operation is made on a value of the wrong type
    public:
	    IllegalCastException(string message):
            InterpreterException(message) {};
        string toString() {
            return "IllegalCastException: " + message;
        };
};

class IllegalArithmeticException : public InterpreterException {
    // Thrown when code attempts to divide by zero
    public:
	    IllegalArithmeticException(string message):
            InterpreterException(message) {};
        string toString() {
            return "IllegalArithmeticException: " + message;
        };
};

class InsufficientStackException : public InterpreterException {
    // Thrown when a bytecode instruction is called without sufficient
    // operands on the stack
    public:
	    InsufficientStackException(string message): InterpreterException(message) {};
        string toString() {
            return "InsufficientStackException: " + message;
        };
};

class RuntimeException : public InterpreterException {
    // General catch-all exception
    public:
	    RuntimeException(string message): InterpreterException(message) {};
        string toString() {
            return "RuntimeException: " + message;
        };
};
