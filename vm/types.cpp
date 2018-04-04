#include "types.h"

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
}

/* Function */
const string Function::typeS = "Function";
string Function::toString() {
    return "FUNCTION";
}
bool Function::equals(shared_ptr<Value> other) {
    // TODO
}
