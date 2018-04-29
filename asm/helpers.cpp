#include "helpers.h"
#include "../types.h"

void helper_store_global(vptr<Interpreter> interpreter, std::string* name, vptr<Value> val) {
    interpreter->storeGlobal(name, val);
}

vptr<Value> helper_load_global(vptr<Interpreter> interpreter, std::string* name) {
    return interpreter->loadGlobal(name);
}

vptr<Value> helper_add(vptr<Interpreter> interpreter, Value* left, Value* right) {
    return interpreter->add(left, right);
}

vptr<Value> helper_call(vptr<Interpreter> interpreter, vptr<Closure> closure) {
    // this function needs to take in args and somehow put them in the right place. Maybe put them into a vector that could be used by the vm immediately or could be passed in the MachineCodeFunction from the example 
}

void helper_gc(vptr<Interpreter> interpreter) {
    interpreter->collector->gc();
}

void helper_assert_int(Value* v) {
    v->cast<Integer>();
}

void helper_assert_str(Value* v) {
    v->cast<String>();
}

void helper_assert_bool(Value* v) {
    v->cast<Boolean>();
}

void helper_assert_record(Value* v) {
    v->cast<Record>();
}

void helper_assert_func(Value* v) {
    v->cast<Function>();
}

void helper_assert_closure(Value* v) {
    v->cast<Closure>();
}
