#include "Exception.h"
#include <string>
#include <map>

using namespace std;

class Value {
public:
	virtual string toString() = 0;
};

class NoneValue : public Value {
public:
    string toString() {
        return "None";
    };
};

class BoolValue : public Value {
public:
    bool val;
    BoolValue(bool val) : val(val) {};

    string toString() {
        return val? "true" : "false";
    };
};

class IntValue : public Value {
public:
    int val;
    IntValue(bool val) : val(val) {};

    string toString() {
        return std::to_string(val);
    };
};

class StrValue : public Value {
public:
    string val;
    StrValue(string val) : val(val) {};

    string toString() {
        return val;
    };
};

class RecordValue : public Value {
public:
    map<string, Value*> record;

    Value* getItem(string key) {
        auto search = record.find(key);
        if (search != record.end()) {
            return search->second;
        }
        throw IllegalCastException("record item not found");
    };

    Value* setItem(string key, Value* val) {
        record[key] = val;
        }

    string toString() {
        string result = "{";
        for (auto &r : record) {
            result += r.first + " : ";
            result += r.second->toString() + ";";
        }
        result += "}";
        return result;
    }
};

class FuncValue : public Value {
};
