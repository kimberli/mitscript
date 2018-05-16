#include "helpers.h"
// this needs to be here to break a circular dependency loop
#include "../vm/interpreter.h"

void helper_store_global(Interpreter* interpreter, string* name, Value* val) {
    interpreter->storeGlobal(*name, val);
}

Value* helper_load_global(Interpreter* interpreter, string* name) {
    return interpreter->loadGlobal(*name);
}

void helper_store_local_ref(Constant* val, ValWrapper* ref) {
	ref->ptr = val;
}

Value* helper_add(Interpreter* interpreter, Value* left, Value* right) {
    return interpreter->add(left, right);
}

Boolean* helper_eq(Interpreter* interpreter, Value* left, Value* right) {
    return interpreter->collector->allocate<Boolean>(left->equals(right));
}

Value* helper_alloc_closure(Interpreter* interpreter, int numRefs, Function* func, ValWrapper** refs) {
    vector<ValWrapper*> refVec;
    for (int i = 0; i < numRefs; i++) {
        refVec.push_back(refs[i]); // these should be in order just like that
    }
    return interpreter->collector->allocate<Closure>(refVec, func);
}

Value* helper_call(Interpreter* interpreter, int numArgs, Closure* closure, Constant** args) {
    // package this stuff into the right format and call back the vm 
    vector<Constant*> argVec;
    for (int i = 0; i < numArgs; i++) {
        argVec.push_back(args[i]);
    }
    return interpreter->call(argVec, closure);
}

void helper_gc(Interpreter* interpreter, int numTemps, Collectable** temps) {
    for (int i = 0; i < numTemps; i++) {
        Collectable* temp = temps[i];
        interpreter->collector->markSuccessors(temp);
    }
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

void helper_assert_nonzero(int32_t v) {
    if (v == 0) {
		throw IllegalArithmeticException("cannot divide by 0");
	};
}

int32_t helper_unbox_int(Integer* v) {
    return v->value;
}

bool helper_unbox_bool(Boolean* b) {
    return b->value;
}

Value* helper_unbox_valwrapper(ValWrapper* v) {
    return v->ptr;
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

ValWrapper* helper_new_valwrapper(Interpreter* interpreter, Constant* ptr) {
    return interpreter->collector->allocate<ValWrapper>(ptr);
}

String* helper_cast_string(Interpreter* interpreter, Value* val) {
    return interpreter->collector->allocate<String>(val->toString());
}

Value* helper_get_record_field(Interpreter* interpreter, string* field, Record* record) {
    if (record->value.count(*field) == 0) {
        return interpreter->NONE;
    }
	return record->get(*field);
}

Record* helper_set_record_field(Interpreter* interpreter, string* field, Record* record, Value* val) {
	record->set(*field, val, *interpreter->collector);
	return record;
}

Value* helper_get_record_index(Interpreter* interpreter, Value* index, Record* record) {
    string key = index->toString();
    if (record->value.count(key) == 0) {
        return interpreter->NONE;
    }
	return record->get(key);
}

Record* helper_set_record_index(Interpreter* interpreter, Value* index, Record* record, Value* val) {
	record->set(index->toString(), val, *interpreter->collector);
	return record;
}
