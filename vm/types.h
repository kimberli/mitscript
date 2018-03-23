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
    virtual ~Value() { }
    virtual string type() = 0;

    template <typename T>
    T* cast() {
        auto val = dynamic_cast<T*>(this);
        if (val == NULL) {
            throw IllegalCastException("cannot cast type " + this->type() + " to " + T::typeS);
        }
        return val;
    }
};

struct ValuePtr: public Value {
    std::shared_ptr<Value> ptr;
    ValuePtr(std::shared_ptr<Value> ptr): ptr(ptr) {}

    static const string typeS;
    string type() {
        return "ValuePtr";
    }
};

struct Constant: public Value
{
    virtual ~Constant() { }

    static const string typeS;
};

struct None : public Constant
{
    virtual ~None() { }

    static const string typeS;
    string type() {
        return "None";
    }
};

struct Integer : public Constant
{
    Integer(int32_t value)
    : value(value)
    {

    }

    int32_t value;

    virtual ~Integer() { }

    static const string typeS;
    string type() {
        return "Integer";
    }
};

struct String : public Constant
{
    String(std::string value)
    : value(value)
    {

    }

    std::string value;

    virtual ~String() { }

    static const string typeS;
    string type() {
        return "String";
    }
};

struct Boolean : public Constant
{
    Boolean(bool value)
    : value(value)
    {

    }

    bool value;

    virtual ~Boolean() { }

    static const string typeS;
    string type() {
        return "Boolean";
    }
};

struct Record : public Constant
{
    Record() {
		value = *(new map<string, std::shared_ptr<Value>>());
	}
    Record(map<string, std::shared_ptr<Value>> value): value(value) {}
	map<string, std::shared_ptr<Value>> value;

    virtual ~Record() { }

    static const string typeS;
    string type() {
        return "Record";
    }
};

struct Function : public Value
{
    // List of functions defined within this function (but not functions defined inside of nested functions)
    std::vector<std::shared_ptr<Function>> functions_;

    // List of constants used by the instructions within this function (but not nested functions)
    std::vector<std::shared_ptr<Constant>> constants_;

    // The number of parameters to the function
    uint32_t parameter_count_;

    // List of local variables
    // The first parameter_count_ variables are the function's parameters
    // in their order as given in the paraemter list
    std::vector<std::string> local_vars_;

    // List of local variables accessed by reference (LocalReference)
    std::vector<std::string> local_reference_vars_;

    // List of the names of non-global and non-local variables accessed by the function
    std::vector<std::string> free_vars_;

    // List of global variable and field names used inside the function
    std::vector<std::string> names_;

    InstructionList instructions;

    Function() {};

    Function(std::vector<std::shared_ptr<Function>> functions_,
            std::vector<std::shared_ptr<Constant>> constants_,
            uint32_t parameter_count_,
	        std::vector<std::string> local_vars_,
            std::vector<std::string> local_reference_vars_,
            std::vector<std::string> free_vars_,
	        std::vector<std::string> names_,
            InstructionList instructions):
        functions_(functions_),
        constants_(constants_),
	    parameter_count_(parameter_count_),
        local_vars_(local_vars_),
        local_reference_vars_(local_reference_vars_),
	    free_vars_(free_vars_),
        names_(names_),
        instructions(instructions) {}

    static const string typeS;
    string type() {
        return "Function";
    }
};
