#pragma once

#include "Exception.h"
#include <string>
#include <map>

using namespace std;

class Frame;

class Value {
public:
	virtual string toString() = 0;
    virtual bool equals(Value* other) = 0;

    template <typename T>
    T* cast() {
        auto val = dynamic_cast<T*>(this);
        if (val == NULL) {
            throw IllegalCastException();
        }
        return val;
    }

};

class NoneValue : public Value {
public:
    string toString();
    bool equals(Value* other);
};

class BoolValue : public Value {
public:
    bool val;
    BoolValue(bool val);

    string toString();
    bool equals(Value* other);
};

class IntValue : public Value {
public:
    int val;
    IntValue(int val);

    string toString();
    bool equals(Value* other);
};

class StrValue : public Value {
public:
    string val;
    StrValue(string val);

    string toString();
    bool equals(Value* other);
};

class RecordValue : public Value {
public:
    map<string, Value*> record;

    Value* getItem(string key);
    Value* setItem(string key, Value* val);

    string toString();
    bool equals(Value* other);
};

class FuncValue : public Value {
public:
    Frame* frame;
    FuncValue(Frame* parent, Frame* global);
};
