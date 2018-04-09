/*
 * types.cpp
 *
 * Implements the Value, Constant, and Function types used in the interpreter
 */
#include "types.h"
#include "frame.h"

/* ValuePtr */
const string ValuePtr::typeS = "ValuePtr";
string ValuePtr::toString() {
    throw RuntimeException("can't cast ValuePtr to String");
}
bool ValuePtr::equals(shared_ptr<Value> other) {
    throw RuntimeException("can't call equals on ValuePtr");
}

/* None */
const string None::typeS = "None";
string None::toString() {
    return "None";
}
bool None::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_cast<None*>(other.get());
    if (otherV == NULL) {
        return false;
    }
    return true;
}

/* Integer */
const string Integer::typeS = "Integer";
string Integer::toString() {
    return to_string(value);
}
bool Integer::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_cast<Integer*>(other.get());
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}

/* String */
const string String::typeS = "String";
string String::toString() {
    return value;
}
bool String::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_cast<String*>(other.get());
    if (otherV == NULL) {
        return false;
    }
    return this->value.compare(otherV->value) == 0;
}

/* Boolean */
const string Boolean::typeS = "Boolean";
string Boolean::toString() {
    return value? "true" : "false";
};
bool Boolean::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_cast<Boolean*>(other.get());
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}

/* Record */
const string Record::typeS = "Record";
string Record::toString() {
    string res = "{";
    for (auto x: value) {
        res += x.first + ":" + x.second.get()->toString() + " ";
    }
    res += "}";
    return res;
}
bool Record::equals(shared_ptr<Value> other) {
    // TODO
    throw RuntimeException("unimplemented");
}

/* Function */
const string Function::typeS = "Function";
string Function::toString() {
    // TODO: do we ever use this?
    return "FUNCTION";
}
bool Function::equals(shared_ptr<Value> other) {
    // TODO
    throw RuntimeException("unimplemented");
}

/* Closure */
const string Closure::typeS = "Closure";
string Closure::toString() {
    return "FUNCTION";
}
bool Closure::equals(shared_ptr<Value> other) {
    // TODO
    throw RuntimeException("unimplemented");
}

/* Native functions */
shared_ptr<Constant> PrintNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string arg = local_vars_[0];
    Constant* val = currentFrame.getLocalVar(arg).get();
    cout << val->toString() << endl;
    return make_shared<None>();
};

shared_ptr<Constant> InputNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string input;
    getline(cin, input);
    return make_shared<String>(input);
};

shared_ptr<Constant> IntcastNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string arg = local_vars_[0];
    Constant* val = currentFrame.getLocalVar(arg).get();
	auto intVal = dynamic_cast<Integer*>(val);
    if (intVal != NULL) {
        return make_shared<Integer>(intVal->value);
    }
    auto strVal = val->cast<String>();
    if (strVal->value == "0") {
        return make_shared<Integer>(0);
    }
    int result = atoi(strVal->value.c_str());
    if (result == 0) {
        throw IllegalCastException("cannot convert value " + strVal->value + " to IntValue");
    }
    return make_shared<Integer>(result);
};
