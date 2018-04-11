#include "../AST.h"
#include "State.h"
#include "Value.h"
#include <map>
#include <string>
#include <vector>

using namespace std;

NoneValue& NONE = *(new NoneValue());

const string NoneValue::typeS = "NoneValue";
const string BoolValue::typeS = "BoolValue";
const string IntValue::typeS = "IntValue";
const string StrValue::typeS = "StrValue";
const string RecordValue::typeS = "RecordValue";
const string FuncValue::typeS = "FuncValue";

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
    string* replaced = new string(val);
    auto pos = replaced->find("\\");
    while (pos != string::npos) {
        if (replaced->at(pos + 1) == 'n') {
            replaced->replace(pos, 2, "\n");
        } else if (replaced->at(pos + 1) == 't') {
            replaced->replace(pos, 2, "\t");
        } else if (replaced->at(pos + 1) == '\\') {
            replaced->replace(pos, 2, "\\");
        } else if (replaced->at(pos + 1) == '"') {
            replaced->replace(pos, 2, "\"");
        }
        pos = replaced->find("\\", pos + 1);
    }

    return *replaced;
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
    if (record.count(key)) {
        return record[key];
    }
    return NULL;
};

Value* RecordValue::setItem(string key, Value* val) {
    record[key] = val;
    }

string RecordValue::toString() {
    string result = "{";
    for (auto &r : record) {
        result += r.first + ":";
        result += r.second->toString() + " ";
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
FuncValue::FuncValue(Frame& frame, vector<Identifier*> args, Block& body):
    frame(frame), args(args), body(body) {};

string FuncValue::toString() {
    return "FUNCTION";
}

bool FuncValue::equals(Value* other) {
    auto o = dynamic_cast<FuncValue*>(other);
    if (o == NULL) {
        return false;
    }
    return &o->frame == &this->frame &&
        o->args == this->args &&
        &o->body == &this->body;
}

/* Native functions */
PrintNativeFunc::PrintNativeFunc(Frame& frame, vector<Identifier*> args, Block& body):
    NativeFunc(frame, args, body) {};

Value* PrintNativeFunc::evalNativeFunc(Frame& currentFrame) {
    LOG(1, "\tevalNativeFunc: print");
    Identifier* s = args[0];
    Value* val = currentFrame.getLocal(s->name);
    cout << val->toString() << endl;
    return &NONE;
};

InputNativeFunc::InputNativeFunc(Frame& frame, vector<Identifier*> args, Block& body):
    NativeFunc(frame, args, body) {};

Value* InputNativeFunc::evalNativeFunc(Frame& currentFrame) {
    LOG(1, "\tevalNativeFunc: input");
    string input;
    getline(cin, input);
    return new StrValue(input);
};

IntcastNativeFunc::IntcastNativeFunc(Frame& frame, vector<Identifier*> args, Block& body):
    NativeFunc(frame, args, body) {};

Value* IntcastNativeFunc::evalNativeFunc(Frame& currentFrame) {
    LOG(1, "\tevalNativeFunc: intcast");
    Identifier* s = args[0];
    Value* val = currentFrame.getLocal(s->name);
    if (dynamic_cast<IntValue*>(val) != NULL) {
        return val;
    }
    auto strVal = val->cast<StrValue>();
    if (strVal->val == "0") {
        return new IntValue(0);
    }
    int result = atoi(strVal->val.c_str());
    if (result == 0) {
        throw IllegalCastException("cannot convert value " + strVal->val + " to IntValue");
    }
    return new IntValue(result);
};
