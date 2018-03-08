#include "Exception.h"
#include <string>
#include <map>

using namespace std;

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
    string toString() {
        return "None";
    };

    bool equals(Value* other) {
        auto o = dynamic_cast<NoneValue*>(other);
        if (o == NULL) {
            return false;
        }
        return true;
    };
};

class BoolValue : public Value {
public:
    bool val;
    BoolValue(bool val) : val(val) {};

    string toString() {
        return val? "true" : "false";
    };

    bool equals(Value* other) {
        auto o = dynamic_cast<BoolValue*>(other);
        if (o == NULL) {
            return false;
        }
        return o->val == this->val;
    };
};

class IntValue : public Value {
public:
    int val;
    IntValue(int val) : val(val) {};

    string toString() {
        return std::to_string(val);
    };

    bool equals(Value* other) {
        auto o = dynamic_cast<IntValue*>(other);
        if (o == NULL) {
            return false;
        }
        return o->val == this->val;
    };
};

class StrValue : public Value {
public:
    string val;
    StrValue(string val) : val(val) {};

    string toString() {
        return val;
    };

    bool equals(Value* other) {
        auto o = dynamic_cast<StrValue*>(other);
        if (o == NULL) {
            return false;
        }
        return o->val.compare(this->val) == 0;
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
        throw IllegalCastException();
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

    bool equals(Value* other) {
        auto o = dynamic_cast<RecordValue*>(other);
        if (o == NULL) {
            return false;
        }
        return o->record == this->record;
    };
};

class FuncValue : public Value {
};
