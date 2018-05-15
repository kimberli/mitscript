/*
 * types.cpp
 *
 * Implements the Value, Constant, and Function types used in the interpreter
 */
#include "types.h"
#include "frame.h"
#include "gc/gc.h"


/* Constant */
const string Constant::typeS = "Constant";

/* ValWrapper */
const string ValWrapper::typeS = "ValWrapper";
string ValWrapper::toString() {
    throw RuntimeException("can't cast ValWrapper to String");
}
size_t ValWrapper::getSize() {
    return sizeof(ValWrapper);
}
bool ValWrapper::equals(Value* other) {
    throw RuntimeException("can't call equals on ValWrapper");
}
void ValWrapper::follow(CollectedHeap& heap){
    // mark the value this points to
    if (ptr && !is_tagged(ptr)) {
        heap.markSuccessors(get_collectable(ptr));
    }
}

/* Function */
const string Function::typeS = "Function";
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
    for (tagptr_t c : constants_) {
        if (!is_tagged(c)) {
            heap.markSuccessors(get_collectable(c));
        }
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
    size_t instrSize = sizeof(instructions) + instructions.capacity()*sizeof(BcInstruction);
    return overhead + funcsSize + consSize + localsSize + refsSize + freeSize + namesSize + instrSize;
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
size_t None::getSize() {
    return sizeof(None);
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
size_t Integer::getSize() {
    return sizeof(Integer);
}

/* String */
const string String::typeS = "String";
string String::toString() {
    string replaced(value);
    auto pos = replaced.find("\\");
    while (pos != string::npos) {
        if (replaced.at(pos + 1) == 'n') {
            replaced.replace(pos, 2, "\n");
        } else if (replaced.at(pos + 1) == 't') {
            replaced.replace(pos, 2, "\t");
        } else if (replaced.at(pos + 1) == '\\') {
            replaced.replace(pos, 2, "\\");
        } else if (replaced.at(pos + 1) == '"') {
            replaced.replace(pos, 2, "\"");
        }
        pos = replaced.find("\\", pos + 1);
    }
    return replaced;
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
size_t String::getSize() {
    size_t overhead = sizeof(String);
    size_t stringSize = getStringSize(value);
    return overhead + stringSize;
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
size_t Boolean::getSize() {
    return sizeof(Boolean);
}

/* Record */
const string Record::typeS = "Record";
string Record::toString() {
    string res = "{";
    for (auto x: value) {
        res += x.first + ":" + ptr_to_str(x.second) + " ";
    }
    res += "}";
    return res;
}
tagptr_t Record::get(string key) {
    return value[key];
}
void Record::set(string key, tagptr_t val, CollectedHeap& collector) {
    if (value.count(key) == 0) {
        collector.increment(sizeof(key) + key.size() + sizeof(val));
    }
    value[key] = val;
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
    for (auto it = value.begin(); it != value.end(); it++) {
        if (!is_tagged(it->second)) {
            heap.markSuccessors(get_collectable(it->second));
        }
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
size_t Closure::getSize() {
    size_t overhead = sizeof(Closure);
    size_t refsSize = getVecSize(refs);
    return overhead + refsSize;
}
void Closure::follow(CollectedHeap& heap) {
    // follow the refs and the function
    for (ValWrapper* v : refs) {
        heap.markSuccessors(v);
    }
    heap.markSuccessors(func);
}

/* Native functions */
tagptr_t PrintNativeFunction::evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
    cout << ptr_to_str(val) << endl;
    return ch.allocate<None>();
};
tagptr_t InputNativeFunction::evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) {
    string* input = new string();
    getline(cin, *input);
    return make_ptr(&input);
};
tagptr_t IntcastNativeFunction::evalNativeFunction(Frame& currentFrame, CollectedHeap& ch) {
    string name = currentFrame.getLocalByIndex(0);
    auto val = currentFrame.getLocalVar(name);
    if (check_tag(val, INT_TAG)) {
        return val;
    }
    if (check_tag(val, STR_TAG)) {
        string* s = get_str(val);
        if (*s == "0") {
            return make_ptr(0);
        }
        int result = atoi(s->c_str());
        if (result == 0) {
            throw IllegalCastException("cannot convert value " + *s + " to IntValue");
        }
        return make_ptr(result);
    }
    throw IllegalCastException("expected String for intcast");
};
