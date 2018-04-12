/*
 * types.h
 *
 * Defines the Value, Constant, and Function types used in the interpreter
 */
#pragma once

#include "exception.h"
#include "instructions.h"
#include "../gc/gc.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define LOG(msg) { if (false) std::cerr << msg << endl; }

template<typename T>
using vptr = T*;

class Frame;
typedef Frame* fptr;


struct Value : public Collectable {
    // Abstract class for program values that can be stored on a frame's
    // operand stack
    virtual ~Value() {};

    // instance function that returns type of value as a string
    virtual string type() = 0;

    // instance function that returns printable representation of this value's data
    virtual string toString() = 0;
    // instance function to determine whether this value is equal to another one
    virtual bool equals(vptr<Value> other) = 0;

    // helper function to dynamically cast value to a specific subclass type
    // and raise an IllegalCastException if the cast fails
    template <typename T>
    T* cast() {
        auto val = dynamic_cast<T*>(this);
        if (val == NULL) {
            throw IllegalCastException("cannot cast type " + this->type() + " to " + T::typeS);
        }
        return val;
    }
};

struct Constant: public Value {
    // Abstract class for constant program values
    virtual ~Constant() {};
};

struct Function : public Value {
    // Class for function type; produced by bytecode compiler

    // functions defined within this function (but not inside nested functions)
    vector<vptr<Function>> functions_;

    // constants used by the instructions within this function (but not inside nested functions)
    vector<vptr<Constant>> constants_;

    // number of parameters to the function
    int32_t parameter_count_;

    // list of local variables
    // note: the first parameter_count_ variables are the function's parameters
    // in their order as given in the paraemter list
    vector<string> local_vars_;

    // list of local variables accessed by reference (LocalReference)
    vector<string> local_reference_vars_;

    // list of the names of non-global and non-local variables accessed by the function
    vector<string> free_vars_;

    // list of global variable and field names used inside the function
    vector<string> names_;

    InstructionList instructions;

    Function() {};
    virtual ~Function() {};

    Function(vector<vptr<Function>> functions_,
            vector<vptr<Constant>> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            InstructionList instructions):
        functions_(functions_),
        constants_(constants_),
	    parameter_count_(parameter_count_),
        local_vars_(local_vars_),
        local_reference_vars_(local_reference_vars_),
	    free_vars_(free_vars_),
        names_(names_),
        instructions(instructions) {};

    string toString();
    bool equals(vptr<Value> other);
    static const string typeS;
    string type() {
        return "Function";
    }

    void follow(CollectedHeap& heap) override;
};

struct ValuePtr: public Value {
    // Class for reference variables
    vptr<Constant> ptr;

    ValuePtr() {};
    ValuePtr(vptr<Constant> ptr): ptr(ptr) {};
    virtual ~ValuePtr() {};

    static const string typeS;
    string type() {
        return "ValuePtr";
    }

    string toString();
    bool equals(vptr<Value> other);

    void follow(CollectedHeap& heap) override;
};

struct None : public Constant {
    // Class for None type
    virtual ~None() {}

    static const string typeS;
    string type() {
        return "None";
    }

    string toString();
    bool equals(vptr<Value> other);

    void follow(CollectedHeap& heap) override;
};

struct Integer : public Constant {
    // Class for integer type
    int32_t value;

    Integer(int32_t value) : value(value) {}
    virtual ~Integer() {}

    static const string typeS;
    string type() {
        return "Integer";
    }

    string toString();
    bool equals(vptr<Value> other);

    void follow(CollectedHeap& heap) override;
};

struct String : public Constant {
    // Class for string type
    string value;

    String(string value): value(value) {};
    virtual ~String() {};

    static const string typeS;
    string type() {
        return "String";
    }

    string toString();
    bool equals(vptr<Value> other);

    void follow(CollectedHeap& heap) override;
};

struct Boolean : public Constant{
    // Class for boolean type
    bool value;

    Boolean(bool value): value(value) {};
    virtual ~Boolean() {};

    static const string typeS;
    string type() {
        return "Boolean";
    }

    string toString();
    bool equals(vptr<Value> other);

    void follow(CollectedHeap& heap) override;
};

struct Record : public Constant {
    // Class for record type (note that this is mutable)
	map<string, vptr<Value>> value;

    Record() {
		value = *(new map<string, vptr<Value>>());
	}
    Record(map<string, vptr<Value>> value): value(value) {}

    virtual ~Record() {}
    string toString();
    bool equals(vptr<Value> other);
    static const string typeS;
    string type() {
        return "Record";
    }

    void follow(CollectedHeap& heap) override;
};

struct Closure: public Constant {
    // Class for closure type

    // list of reference variables, in the same order as listed in the function's
    // free_vars_ list
    vector<vptr<ValuePtr>> refs;

    // function that the closure is for
    vptr<Function> func;

    Closure(vector<vptr<ValuePtr>> refs, vptr<Function> func):
        refs(refs), func(func) {};
    virtual ~Closure() {};

    static const string typeS;
    string type() {
        return "Closure";
    }

    string toString();
    bool equals(vptr<Value> other);

    void follow(CollectedHeap& heap) override;
};

class NativeFunction : public Function {
    // Abstract class for native functions
public:
    NativeFunction(vector<vptr<Function>> functions_,
            vector<vptr<Constant>> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            InstructionList instructions):
			Function(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
    virtual vptr<Constant> evalNativeFunction(Frame& currentFrame) = 0;
};

class PrintNativeFunction : public NativeFunction {
    // Class for print native function
public:
    PrintNativeFunction(vector<vptr<Function>> functions_,
            vector<vptr<Constant>> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            InstructionList instructions):
			NativeFunction(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
   vptr<Constant> evalNativeFunction(Frame& currentFrame);
};

class InputNativeFunction : public NativeFunction {
    // Class for input native function
public:
    InputNativeFunction(vector<vptr<Function>> functions_,
            vector<vptr<Constant>> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            InstructionList instructions):
			NativeFunction(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
    vptr<Constant> evalNativeFunction(Frame& currentFrame);
};

class IntcastNativeFunction : public NativeFunction {
    // Class for intcast native function
public:
    IntcastNativeFunction(vector<vptr<Function>> functions_,
            vector<vptr<Constant>> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            InstructionList instructions):
			NativeFunction(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
    vptr<Constant> evalNativeFunction(Frame& currentFrame);
};
