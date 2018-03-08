#include "State.h"
#include "Value.h"
#include <string>
#include <map>

using namespace std;

/* NoneValue */
string NoneValue::toString() {
    return "None";
};

bool NoneValue::equals(Value* other) {
    auto o = dynamic_cast<NoneValue*>(other);
    if (o == NULL) {
        return false;
    }
    return true;
};

/* BoolValue */
BoolValue::BoolValue(bool val) : val(val) {};

string BoolValue::toString() {
    return val? "true" : "false";
};

bool BoolValue::equals(Value* other) {
    auto o = dynamic_cast<BoolValue*>(other);
    if (o == NULL) {
        return false;
    }
    return o->val == this->val;
};

/* IntValue */
IntValue::IntValue(int val) : val(val) {};

string IntValue::toString() {
    return std::to_string(val);
};

bool IntValue::equals(Value* other) {
    auto o = dynamic_cast<IntValue*>(other);
    if (o == NULL) {
        return false;
    }
    return o->val == this->val;
};

/* StrValue */
StrValue::StrValue(string val) : val(val) {};

string StrValue::toString() {
    return val;
};

bool StrValue::equals(Value* other) {
    auto o = dynamic_cast<StrValue*>(other);
    if (o == NULL) {
        return false;
    }
    return o->val.compare(this->val) == 0;
};

/* RecordValue */
Value* RecordValue::getItem(string key) {
    auto search = record.find(key);
    if (search != record.end()) {
        return search->second;
    }
    return NULL;
};

Value* RecordValue::setItem(string key, Value* val) {
    record[key] = val;
    }

string RecordValue::toString() {
    string result = "{";
    for (auto &r : record) {
        result += r.first + " : ";
        result += r.second->toString() + ";";
    }
    result += "}";
    return result;
}

bool RecordValue::equals(Value* other) {
    auto o = dynamic_cast<RecordValue*>(other);
    if (o == NULL) {
        return false;
    }
    return o->record == this->record;
};

/* FuncValue */
FuncValue::FuncValue(Frame* parent, Frame* global) {
    frame = new Frame(parent, global);
}
