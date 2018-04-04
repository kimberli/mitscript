#pragma once

#include "exception.h"
#include "instructions.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Value
{
    virtual ~Value() {};
    virtual string type() = 0;

    virtual string toString() = 0;
    virtual bool equals(shared_ptr<Value> other) = 0;

    template <typename T>
    T* cast() {
        auto val = dynamic_cast<T*>(this);
        if (val == NULL) {
            throw IllegalCastException("cannot cast type " + this->type() + " to " + T::typeS);
        }
        return val;
    }
};

struct Constant: public Value
{
    virtual ~Constant() {};
};

struct ValuePtr: public Value {
    shared_ptr<Value> ptr;

    ValuePtr(shared_ptr<Value> ptr): ptr(ptr) {};
    virtual ~ValuePtr() {};

    static const string typeS;
    string type() {
        return "ValuePtr";
    }

    string toString();
    bool equals(shared_ptr<Value> other);
};

struct None : public Constant
{
    virtual ~None() {}

    static const string typeS;
    string type() {
        return "None";
    }

    string toString();
    bool equals(shared_ptr<Value> other);
};

struct Integer : public Constant
{
    int32_t value;

    Integer(int32_t value) : value(value) {}
    virtual ~Integer() {}

    static const string typeS;
    string type() {
        return "Integer";
    }

    string toString();
    bool equals(shared_ptr<Value> other);
};

struct String : public Constant
{
    string value;

    String(string value): value(value) {};
    virtual ~String() {};
    
    static const string typeS;
    string type() {
        return "String";
    }

    string toString();
    bool equals(shared_ptr<Value> other);
};

struct Boolean : public Constant
{
    bool value;

    Boolean(bool value): value(value) {};
    virtual ~Boolean() {};

    static const string typeS;
    string type() {
        return "Boolean";
    }

    string toString();
    bool equals(shared_ptr<Value> other);

};

struct Record : public Constant
{
    Record() {
		value = *(new map<string, shared_ptr<Value>>());
	}
    Record(map<string, shared_ptr<Value>> value): value(value) {}
	map<string, shared_ptr<Value>> value;

    virtual ~Record() {}
    string toString();
    bool equals(shared_ptr<Value> other);
    static const string typeS;
    string type() {
        return "Record";
    }
};

struct Function : public Constant
{
    // List of functions defined within this function (but not functions defined inside of nested functions)
    vector<shared_ptr<Function>> functions_;

    // List of constants used by the instructions within this function (but not nested functions)
    vector<shared_ptr<Constant>> constants_;

    // The number of parameters to the function
    uint32_t parameter_count_;

    // List of local variables
    // The first parameter_count_ variables are the function's parameters
    // in their order as given in the paraemter list
    vector<string> local_vars_;

    // List of local variables accessed by reference (LocalReference)
    vector<string> local_reference_vars_;

    // List of the names of non-global and non-local variables accessed by the function
    vector<string> free_vars_;

    // List of global variable and field names used inside the function
    vector<string> names_;

    InstructionList instructions;

    Function() {};
    virtual ~Function() {};

    Function(vector<shared_ptr<Function>> functions_,
            vector<shared_ptr<Constant>> constants_,
            uint32_t parameter_count_,
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
        instructions(instructions) {}

    string toString();
    bool equals(shared_ptr<Value> other);
    static const string typeS;
    string type() {
        return "Function";
    }
};

struct Closure: public Constant {
    vector<shared_ptr<ValuePtr>> refs;
    shared_ptr<Function> func;

    Closure(vector<shared_ptr<ValuePtr>> refs, shared_ptr<Function> func):
        refs(refs), func(func) {};
    virtual ~Closure() {};

    static const string typeS;
    string type() {
        return "Closure";
    }

    string toString();
    bool equals(shared_ptr<Value> other);
};
