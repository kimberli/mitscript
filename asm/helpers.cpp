#include "helpers.h"
// this needs to be here to break a circular dependency loop
#include "../vm/interpreter.h"

void helper_store_global(Interpreter* interpreter, string* name, tagptr_t ptr) {
    interpreter->storeGlobal(*name, ptr);
}

tagptr_t helper_load_global(Interpreter* interpreter, string* name) {
    return interpreter->loadGlobal(*name);
}

void helper_store_local_ref(tagptr_t ptr, tagptr_t ref) {
    ValWrapper* v = cast_val<ValWrapper>(ref);
	v->ptr = ptr;
}

tagptr_t helper_add(Interpreter* interpreter, tagptr_t left, tagptr_t right) {
    // TODO: remove interpreter
    return ptr_add(left, right);
}

tagptr_t helper_eq(Interpreter* interpreter, tagptr_t left, tagptr_t right) {
    // TODO: remove interpreter
    return ptr_equals(left, right);
}

tagptr_t helper_alloc_closure(Interpreter* interpreter, int numRefs, tagptr_t func_ptr, tagptr_t* refs) {
    Function* func = cast_val<Function>(func_ptr);
    vector<ValWrapper*> refVec;
    for (int i = 0; i < numRefs; i++) {
        ValWrapper* ref = cast_val<ValWrapper>(refs[i]);
        refVec.push_back(ref); // these should be in order just like that
    }
    return make_ptr(interpreter->collector->allocate(refVec, func));
}

tagptr_t helper_call(Interpreter* interpreter, int numArgs, tagptr_t clos_ptr, tagptr_t* args) {
    // package this stuff into the right format and call back the vm 
    vector<tagptr_t> argVec;
    for (int i = 0; i < numArgs; i++) {
        argVec.push_back(args[i]);
    }
    return interpreter->call(argVec, clos_ptr);
}

void helper_gc(Interpreter* interpreter) {
    interpreter->collector->gc();
}

void helper_assert_int(tagptr_t ptr) {
    get_int(ptr);  // will throw exception if not INT_TAG
}

void helper_assert_str(tagptr_t ptr) {
    get_str(ptr);  // will throw exception if not STR_TAG
}

void helper_assert_bool(tagptr_t ptr) {
    get_bool(ptr);  // will throw exception if not BOOL_TAG
}

void helper_assert_record(tagptr_t ptr) {
    cast_val<Record>(ptr); // will throw exception if not Record
}

void helper_assert_func(tagptr_t ptr) {
    cast_val<Function>(ptr); // will throw exception if not Function
}

void helper_assert_closure(tagptr_t ptr) {
    cast_val<Closure>(ptr); // will throw exception if not Closure
}

void helper_assert_valwrapper(tagptr_t ptr) {
    cast_val<ValWrapper>(ptr); // will throw exception if not ValWrapper
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

tagptr_t helper_unbox_valwrapper(tagptr_t val_ptr) {
    ValWrapper* v = cast_val<ValWrapper>(val_ptr);
    return v->ptr;
}

tagptr_t helper_new_record(Interpreter* interpreter) {
    return interpreter->collector->allocate<Record>();
}

tagptr_t helper_new_valwrapper(Interpreter* interpreter, tagptr_t ptr) {
    return make_ptr(interpreter->collector->allocate<ValWrapper>(ptr));
}

tagptr_t helper_cast_string(Interpreter* interpreter, tagptr_t ptr) {
    return make_ptr(ptr_to_str(ptr));
}

tagptr_t helper_get_record_field(Interpreter* interpreter, string* field, tagptr_t record_ptr) {
    Record* record = cast_val<Record>(record_ptr);
    if (record->value.count(*field) == 0) {
        return interpreter->NONE;
    }
	return record->get(*field);
}

tagptr_t helper_set_record_field(Interpreter* interpreter, string* field, tagptr_t record_ptr, tagptr_t ptr) {
    Record* record = cast_val<Record>(record_ptr);
	record->set(*field, ptr, *interpreter->collector);
	return record_ptr;
}

tagptr_t helper_get_record_index(Interpreter* interpreter, tagptr_t index, tagptr_t record_ptr) {
    Record* record = cast_val<Record>(record_ptr);
    string key = *ptr_to_str(index);
    if (record->value.count(key) == 0) {
        return interpreter->NONE;
    }
	return record->get(key);
}

tagptr_t helper_set_record_index(Interpreter* interpreter, tagptr_t index, tagptr_t record_ptr, tagptr_t ptr) {
    Record* record = cast_val<Record>(record_ptr);
	record->set(*ptr_to_str(index), ptr, *interpreter->collector);
	return record_ptr;
}
