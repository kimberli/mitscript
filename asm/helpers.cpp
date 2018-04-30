#include "helpers.h"
// this needs to be here to break a circular dependency loop
#include "../vm/interpreter.h"

void helper_store_global(Interpreter* interpreter, string &name, Value* val) {
    interpreter->storeGlobal(name, val);
}

Value* helper_load_global(Interpreter* interpreter, string &name) {
    return interpreter->loadGlobal(name);
}

Value* helper_add(Interpreter* interpreter, Value* left, Value* right) {
    return interpreter->add(left, right);
}

Value* helper_call(Interpreter* interpreter, Closure* closure) {
    // this function needs to take in args and somehow put them in the right place. Maybe put them into a vector that could be used by the vm immediately or could be passed in the MachineCodeFunction from the example 
    // this function takes the args from the assembly stack and puts them in a vector 
    // and then 
    //
    //
    //
    // So the helper in the vm should take A) a closure and B) a list of args.
}

void helper_gc(Interpreter* interpreter) {
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
