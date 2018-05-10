/*
 * types.h
 *
 * Defines the Value, Constant, and Function types used in the interpreter
 */
#pragma once

#include "exception.h"
#include "instructions.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gc/gc.h"

using namespace std;

#define LOG(msg) { if (true) cerr << msg << endl; }

class Frame;
class Collectable;

struct Value : public Collectable {
    // Abstract class for program values that can be stored on a frame's
    // operand stack
    virtual ~Value() {}

    // instance function that returns type of value as a string
    virtual string type() = 0;

    // instance function that returns printable representation of this value's data
    virtual string toString() = 0;
    // instance function to determine whether this value is equal to another one
    virtual bool equals(Value* other) = 0;

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
    vector<Function*> functions_;

    // constants used by the instructions within this function (but not inside nested functions)
    vector<Constant*> constants_;

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

    // map of label indices to instruction indices
    map<int, int> labels_;

    BcInstructionList instructions;

    Function() {};
    virtual ~Function() {};

    Function(vector<Function*> functions_,
            vector<Constant*> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            BcInstructionList instructions):
        functions_(functions_),
        constants_(constants_),
	    parameter_count_(parameter_count_),
        local_vars_(local_vars_),
        local_reference_vars_(local_reference_vars_),
	    free_vars_(free_vars_),
        names_(names_),
        labels_(),
        instructions(instructions) {};

    string toString();
    bool equals(Value* other);
    static const string typeS;
    string type() {
        return "Function";
    }

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
};

struct ValWrapper: public Value {
    // Class for reference variables
    Constant* ptr;

    ValWrapper() {}
    ValWrapper(Constant* ptr): ptr(ptr) {};
    virtual ~ValWrapper() {}

    static const string typeS;
    string type() {
        return "ValWrapper";
    }

    string toString();
    bool equals(Value* other);

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
};

struct None : public Constant {
    // Class for None type
    virtual ~None() {}

    static const string typeS;
    string type() {
        return "None";
    }

    string toString();
    bool equals(Value* other);

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
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
    bool equals(Value* other);

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
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
    bool equals(Value* other);

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
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
    bool equals(Value* other);

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
};

struct Record : public Constant {
    // Class for record type (note that this is mutable)
	map<string, Value*> value;

    Value* get(string key);
    void set(string key, Value* value, CollectedHeap& collector);

    virtual ~Record() {}
    string toString();
    bool equals(Value* other);
    static const string typeS;
    string type() {
        return "Record";
    }

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
};

struct Closure: public Constant {
    // Class for closure type

    // list of reference variables, in the same order as listed in the function's
    // free_vars_ list
    vector<ValWrapper*> refs;

    // function that the closure is for
    Function* func;

    Closure(vector<ValWrapper*> refs, Function* func):
        refs(refs), func(func) {};
    virtual ~Closure() {}

    static const string typeS;
    string type() {
        return "Closure";
    }

    string toString();
    bool equals(Value* other);

    void follow(CollectedHeap& heap) override;
    size_t getSize() override;
};

class NativeFunction : public Function {
    // Abstract class for native functions
public:
    NativeFunction(vector<Function*> functions_,
            vector<Constant*> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            BcInstructionList instructions):
			Function(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
    virtual Constant* evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) = 0;
};

class PrintNativeFunction : public NativeFunction {
    // Class for print native function
public:
    PrintNativeFunction(vector<Function*> functions_,
            vector<Constant*> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            BcInstructionList instructions):
			NativeFunction(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
   Constant* evalNativeFunction(Frame& currentFrame, CollectedHeap& ch);
};

class InputNativeFunction : public NativeFunction {
    // Class for input native function
public:
    InputNativeFunction(vector<Function*> functions_,
            vector<Constant*> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            BcInstructionList instructions):
			NativeFunction(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
    Constant* evalNativeFunction(Frame& currentFrame, CollectedHeap& ch);
};

class IntcastNativeFunction : public NativeFunction {
    // Class for intcast native function
public:
    IntcastNativeFunction(vector<Function*> functions_,
            vector<Constant*> constants_,
            int32_t parameter_count_,
	        vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
	        vector<string> names_,
            BcInstructionList instructions):
			NativeFunction(functions_, constants_, parameter_count_,
					 local_vars_, local_reference_vars_, free_vars_,
					 names_, instructions) {};
    Constant* evalNativeFunction(Frame& currentFrame, CollectedHeap& ch);
};
