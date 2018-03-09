#pragma once

#include "Exception.h"
#include <map>
#include <string>
#include <vector>

#define LOG(lvl, msg) { if (lvl > 0) std::cerr << msg << endl; }

using namespace std;

class Frame;

class Value {
public:
	virtual string toString() = 0;
    virtual bool equals(Value* other) = 0;
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

class NoneValue : public Value {
public:
    string toString();
    bool equals(Value* other);

    static const string typeS;
    string type() {
        return "NoneValue";
    };
};

class BoolValue : public Value {
public:
    bool val;
    BoolValue(bool val);

    string toString();
    bool equals(Value* other);

    static const string typeS;
    string type() {
        return "BoolValue";
    };
};

class IntValue : public Value {
public:
    int val;
    IntValue(int val);

    string toString();
    bool equals(Value* other);

    static const string typeS;
    string type() {
        return "IntValue";
    };
};

class StrValue : public Value {
public:
    string val;
    StrValue(string val);

    string toString();
    bool equals(Value* other);

    static const string typeS;
    string type() {
        return "StrValue";
    };
};

class RecordValue : public Value {
public:
    map<string, Value*> record;

    Value* getItem(string key);
    Value* setItem(string key, Value* val);

    string toString();
    bool equals(Value* other);

    static const string typeS;
    string type() {
        return "RecordValue";
    };
};

class FuncValue : public Value {
public:
    Frame& frame;
    vector<Identifier*> args;
    Block& body;
    FuncValue(Frame& frame, vector<Identifier*> args, Block& body);

    string toString();
    bool equals(Value* other);

    static const string typeS;
    string type() {
        return "FuncValue";
    };
};

class NativeFunc : public FuncValue {
public:
    NativeFunc(Frame& frame, vector<Identifier*> args, Block& body):
        FuncValue(frame, args, body) {};
    virtual Value* evalNativeFunc(Frame& currentFrame) = 0;
};

class PrintNativeFunc : public NativeFunc {
public:
    PrintNativeFunc(Frame& frame, vector<Identifier*> args, Block& body);

    Value* evalNativeFunc(Frame& currentFrame);
};

class InputNativeFunc : public NativeFunc {
public:
    InputNativeFunc(Frame& frame, vector<Identifier*> args, Block& body);

    Value* evalNativeFunc(Frame& currentFrame);
};

class IntcastNativeFunc : public NativeFunc {
public:
    IntcastNativeFunc(Frame& frame, vector<Identifier*> args, Block& body);

    Value* evalNativeFunc(Frame& currentFrame);
};

extern NoneValue& NONE;
