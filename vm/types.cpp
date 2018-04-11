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
void ValuePtr::follow(CollectedHeap& heap){
}

/* Function */
string Function::toString() {
    throw RuntimeException("can't cast Function to a String (try Closure instead)");
}
bool Function::equals(shared_ptr<Value> other) {
    throw RuntimeException("can't call equals on a Function (try Closure instead)");
}
void Function::follow(CollectedHeap& heap) {
}

/* None */
const string None::typeS = "None";
string None::toString() {
    return "None";
}
bool None::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_pointer_cast<None>(other);
    if (otherV == NULL) {
        return false;
    }
    return true;
}
void None::follow(CollectedHeap& heap) {
}
/* Integer */
const string Integer::typeS = "Integer";
string Integer::toString() {
    return to_string(value);
}
bool Integer::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_pointer_cast<Integer>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}
void Integer::follow(CollectedHeap& heap) {
}
/* String */
const string String::typeS = "String";
string String::toString() {
    return value;
}
bool String::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_pointer_cast<String>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value.compare(otherV->value) == 0;
}
void String::follow(CollectedHeap& heap) {
}
/* Boolean */
const string Boolean::typeS = "Boolean";
string Boolean::toString() {
    return value? "true" : "false";
};
bool Boolean::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_pointer_cast<Boolean>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}
void Boolean::follow(CollectedHeap& heap) {
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
    auto otherV = dynamic_pointer_cast<Record>(other);
    if (otherV == NULL) {
        return false;
    }
    return value == otherV->value;
}
void Record::follow(CollectedHeap& heap) {
}
/* Closure */
const string Closure::typeS = "Closure";
string Closure::toString() {
    return "FUNCTION";
}
bool Closure::equals(shared_ptr<Value> other) {
    auto otherV = dynamic_pointer_cast<Closure>(other);
    if (otherV == NULL) {
        return false;
    }
    if (func != otherV->func) {
        return false;
    }
    if (refs.size() != otherV->refs.size()) {
        return false;
    }
    for (int i = 0; i < refs.size(); i++) {
        if (refs[i] != otherV->refs[i]) {
            return false;
        }
    }
    return true;
}
void Closure::follow(CollectedHeap& heap) {
}
/* Native functions */
shared_ptr<Constant> PrintNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
    cout << val->toString() << endl;
    return make_shared<None>();
};

shared_ptr<Constant> InputNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string input;
    getline(cin, input);
    return make_shared<String>(input);
};

shared_ptr<Constant> IntcastNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
	auto intVal = dynamic_pointer_cast<Integer>(val);
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
