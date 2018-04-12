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
bool ValuePtr::equals(Value* other) {
    throw RuntimeException("can't call equals on ValuePtr");
}
void ValuePtr::follow(CollectedHeap& heap){
    // mark the value this points to 
    heap.markSuccessors(ptr);
}

/* Function */
string Function::toString() {
    throw RuntimeException("can't cast Function to a String (try Closure instead)");
}
bool Function::equals(Value* other) {
    throw RuntimeException("can't call equals on a Function (try Closure instead)");
}
void Function::follow(CollectedHeap& heap) {
    // follow functions_ and constants_,
    for (Function* f : functions_) {
        heap.markSuccessors(f);
    }
    for (Constant* c : constants_) {
        heap.markSuccessors(c);
    }
}

/* None */
const string None::typeS = "None";
string None::toString() {
    return "None";
}
bool None::equals(Value* other) {
    None* otherV = dynamic_cast<None*>(other);
    if (otherV == NULL) {
        return false;
    }
    return true;
}
void None::follow(CollectedHeap& heap) {
    // no-op; no pointers
}
/* Integer */
const string Integer::typeS = "Integer";
string Integer::toString() {
    return to_string(value);
}
bool Integer::equals(Value* other) {
    auto otherV = dynamic_cast<Integer*>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}
void Integer::follow(CollectedHeap& heap) {
    // no-op: no pointers
}
/* String */
const string String::typeS = "String";
string String::toString() {
    string* replaced = new string(value);
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
}
bool String::equals(Value* other) {
    auto otherV = dynamic_cast<String*>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value.compare(otherV->value) == 0;
}
void String::follow(CollectedHeap& heap) {
    // no-op: no pointers
}
/* Boolean */
const string Boolean::typeS = "Boolean";
string Boolean::toString() {
    return value? "true" : "false";
};
bool Boolean::equals(Value* other) {
    auto otherV = dynamic_cast<Boolean*>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}
void Boolean::follow(CollectedHeap& heap) {
    // no-op; no pointers
}
/* Record */
const string Record::typeS = "Record";
string Record::toString() {
    string res = "{";
    for (auto x: value) {
        res += x.first + ":" + x.second->toString() + " ";
    }
    res += "}";
    return res;
}
bool Record::equals(Value* other) {
    auto otherV = dynamic_cast<Record*>(other);
    if (otherV == NULL) {
        return false;
    }
    return value == otherV->value;
}
void Record::follow(CollectedHeap& heap) {
    // point to all the values contained in the record
    for (std::map<string, Value*>::iterator it = value.begin(); it != value.end(); it++) {
        heap.markSuccessors(it->second);
    }
}
/* Closure */
const string Closure::typeS = "Closure";
string Closure::toString() {
    return "FUNCTION";
}
bool Closure::equals(Value* other) {
    auto otherV = dynamic_cast<Closure*>(other);
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
    // follow the refs and the function
    for (ValuePtr* v : refs) {
        heap.markSuccessors(v);
    }
    heap.markSuccessors(func);
}
/* Native functions */
Constant* PrintNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
    cout << val->toString() << endl;
    return new None();
};

Constant* InputNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string input;
    getline(cin, input);
    return new String(input);
};

Constant* IntcastNativeFunction::evalNativeFunction(Frame& currentFrame) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
	auto intVal = dynamic_cast<Integer*>(val);
    if (intVal != NULL) {
        return new Integer(intVal->value);
    }
    auto strVal = val->cast<String>();
    if (strVal->value == "0") {
        return new Integer(0);
    }
    int result = atoi(strVal->value.c_str());
    if (result == 0) {
        throw IllegalCastException("cannot convert value " + strVal->value + " to IntValue");
    }
    return new Integer(result);
};
