#include "types.h"

const string Constant::typeS = "Constant";

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
    // TODO
}

/* Integer */
const string Integer::typeS = "Integer";
string Integer::toString() {
    return to_string(value);
}
bool Integer::equals(shared_ptr<Value> other) {
    // TODO
}

/* String */
const string String::typeS = "String";
string String::toString() {
    return value;
}
bool String::equals(shared_ptr<Value> other) {
    // TODO
}

/* Boolean */
const string Boolean::typeS = "Boolean";
string Boolean::toString() {
    return value? "true" : "false";
};
bool Boolean::equals(shared_ptr<Value> other) {
    // TODO
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
}

/* Function */
const string Function::typeS = "Function";
string Function::toString() {
    return "FUNCTION";
}
bool Function::equals(shared_ptr<Value> other) {
    // TODO
}
