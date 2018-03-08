#include "Exception.h"
#include <string>
#include <map>

using namespace std;

class Value {
};

class NoneValue : public Value {
};

class BoolValue : public Value {
    public:
        bool val;
        BoolValue(bool val) : val(val) {};
};

class IntValue : public Value {
    public:
        int val;
        IntValue(bool val) : val(val) {};
};

class StrValue : public Value {
    public:
        string val;
        StrValue(string val) : val(val) {};
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
};

class FuncValue : public Value {
};
