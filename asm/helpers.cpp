#include "helpers.h"
// this needs to be here to break a circular dependency loop
#include "../vm/interpreter.h"

void helper_store_global(Interpreter* interpreter, string* name, Value* val) {
    interpreter->storeGlobal(*name, val);
}

Value* helper_load_global(Interpreter* interpreter, string* name) {
    return interpreter->loadGlobal(*name);
}

Value* helper_add(Interpreter* interpreter, Value* left, Value* right) {
    return interpreter->add(left, right);
}

Value* helper_alloc_closure(Interpreter* interpreter, int numRefs, Function* func, ValWrapper* refs...) {
    va_list args;
    va_start(args, refs);
    vector<ValWrapper*> refVec;
    for (int i = 0; i < numRefs; i++) {
        refVec.push_back(va_arg(args, ValWrapper*));
    }
    va_end(args);
    return interpreter->collector->allocate<Closure>(refVec, func);
}

Value* helper_call(Interpreter* interpreter, Constant* args[], Closure* closure) {
    // this function needs to take in args and somehow put them in the right place. Maybe put them into a vector that could be used by the vm immediately or could be passed in the MachineCodeFunction from the example
    // this function takes the args from the assembly stack and puts them in a vector
    // and then
    //
    //
    //
    // So the helper in the vm should take A) a closure and B) a list of args.
    // TODO
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

void helper_assert_valwrapper(Value* v) {
    v->cast<ValWrapper>();
}

int32_t helper_unbox_int(Integer* v) {
    return v->value;
}

bool helper_unbox_bool(Boolean* b) {
    return b->value;
}

Integer* helper_new_integer(Interpreter* interpreter, int32_t val) {
    return interpreter->collector->allocate<Integer>(val);
}

Boolean* helper_new_boolean(Interpreter* interpreter, bool val) {
    return interpreter->collector->allocate<Boolean>(val);
}

Record* helper_new_record(Interpreter* interpreter) {
    return interpreter->collector->allocate<Record>();
}

String* helper_cast_string(Interpreter* interpreter, Value* val) {
    return interpreter->collector->allocate<String>(val->toString());
}

Value* helper_get_record_field(Interpreter* interpreter, string* field, Record* record) {
	return record->get(*field);
}

Record* helper_set_record_field(Interpreter* interpreter, string* field, Record* record, Value* val) {
	record->set(*field, val, *interpreter->collector);
	return record;
}

Value* helper_get_record_index(Interpreter* interpreter, Value* index, Record* record) {
	return record->get(index->toString());
}

Record* helper_set_record_index(Interpreter* interpreter, Value* index, Record* record, Value* val) {
	record->set(index->toString(), val, *interpreter->collector);
	return record;
}
