/*
 * types.cpp
 *
 * Implements the Value, Constant, and Function types used in the interpreter
 */
#include "types.h"
#include "frame.h"
#include "../gc/gc.h"

/* ValuePtr */
const string ValuePtr::typeS = "ValuePtr";
string ValuePtr::toString() {
    throw RuntimeException("can't cast ValuePtr to String");
}

size_t ValuePtr::getSize() {
    return sizeof(ValuePtr);
}
bool ValuePtr::equals(vptr<Value> other) {
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
bool Function::equals(vptr<Value> other) {
    throw RuntimeException("can't call equals on a Function (try Closure instead)");
}
void Function::follow(CollectedHeap& heap) {
    // follow functions_ and constants_,
    for (vptr<Function> f : functions_) {
        heap.markSuccessors(f);
    }
    for (vptr<Constant> c : constants_) {
        heap.markSuccessors(c);
    }
}
size_t Function::getSize() {
    size_t overhead = sizeof(Function);
    size_t funcsSize = getVecSize(functions_);
    size_t consSize = getVecSize(constants_);
    size_t localsSize = getVecSize(local_vars_);
    size_t refsSize = getVecSize(local_reference_vars_);
    size_t freeSize = getVecSize(free_vars_);
    size_t namesSize = getVecSize(names_);
    // templates are not workin for this, just copy code
    //size_t instrSize = getVecSize(instructions); 
    size_t instrSize = sizeof(instructions) + instructions.capacity()*sizeof(Instruction);
    return overhead + funcsSize + consSize + localsSize + refsSize + freeSize + namesSize + instrSize;
}

/* None */
const string None::typeS = "None";
string None::toString() {
    return "None";
}
bool None::equals(vptr<Value> other) {
    vptr<None> otherV = dynamic_cast<vptr<None>>(other);
    if (otherV == NULL) {
        return false;
    }
    return true;
}
void None::follow(CollectedHeap& heap) {
    // no-op; no pointers
}
size_t None::getSize() {
    return sizeof(None);
}
/* Integer */
const string Integer::typeS = "Integer";
string Integer::toString() {
    return to_string(value);
}
bool Integer::equals(vptr<Value> other) {
    auto otherV = dynamic_cast<Integer*>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}
void Integer::follow(CollectedHeap& heap) {
    // no-op: no pointers
}
size_t Integer::getSize() {
    return sizeof(Integer);
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
bool String::equals(vptr<Value> other) {
    auto otherV = dynamic_cast<vptr<String>>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value.compare(otherV->value) == 0;
}
void String::follow(CollectedHeap& heap) {
    // no-op: no pointers
}
size_t String::getSize() {
    return sizeof(String);
}
/* Boolean */
const string Boolean::typeS = "Boolean";
string Boolean::toString() {
    return value? "true" : "false";
};
bool Boolean::equals(vptr<Value> other) {
    auto otherV = dynamic_cast<Boolean*>(other);
    if (otherV == NULL) {
        return false;
    }
    return this->value == otherV->value;
}
void Boolean::follow(CollectedHeap& heap) {
    // no-op; no pointers
}
size_t Boolean::getSize() {
    return sizeof(Boolean);
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
bool Record::equals(vptr<Value> other) {
    auto otherV = dynamic_cast<Record*>(other);
    if (otherV == NULL) {
        return false;
    }
    return value == otherV->value;
}
void Record::follow(CollectedHeap& heap) {
    // point to all the values contained in the record
    for (std::map<string, vptr<Value>>::iterator it = value.begin(); it != value.end(); it++) {
        heap.markSuccessors(it->second);
    }
}
size_t Record::getSize() {
    size_t overhead = sizeof(Record);
    size_t mapSize = getMapSize(value);
    return overhead + mapSize;
}
/* Closure */
const string Closure::typeS = "Closure";
string Closure::toString() {
    return "FUNCTION";
}
bool Closure::equals(vptr<Value> other) {
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
size_t Closure::getSize() {
    size_t overhead = sizeof(Closure);
    size_t refsSize = getVecSize(refs);
    return overhead + refsSize;
}
void Closure::follow(CollectedHeap& heap) {
    // follow the refs and the function
    for (vptr<ValuePtr> v : refs) {
        heap.markSuccessors(v);
    }
    heap.markSuccessors(func);
}
/* Native functions */
vptr<Constant> PrintNativeFunction::evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
    cout << val->toString() << endl;
    return ch.allocate<None>();
};

vptr<Constant> InputNativeFunction::evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) {
    string input;
    getline(cin, input);
    return ch.allocate<String, string>(input);
};

vptr<Constant> IntcastNativeFunction::evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
	auto intVal = dynamic_cast<Integer*>(val);
    if (intVal != NULL) {
        return ch.allocate<Integer, int>(intVal->value);
    }
    auto strVal = val->cast<String>();
    if (strVal->value == "0") {
        return ch.allocate<Integer, int>(0);
    }
    int result = atoi(strVal->value.c_str());
    if (result == 0) {
        throw IllegalCastException("cannot convert value " + strVal->value + " to IntValue");
    }
    return ch.allocate<Integer, int>(result);
};
