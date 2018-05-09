#pragma once

#include <string>
#include <cstdarg>

//#include "../vm/interpreter.h"
#include "../types.h"

class Interpreter;

void helper_store_global(Interpreter* interpreter, string* name, Value* val);

Value* helper_load_global(Interpreter* interpreter, string* name);

Value* helper_add(Interpreter* interpreter, Value* left, Value* right);

Boolean* helper_eq(Interpreter* interpreter, Value* left, Value* right);

Value* helper_alloc_closure(Interpreter* interpreter, int numRefs, Function* func, ValWrapper** refs);

Value* helper_call(Interpreter* interpreter, int numArgs, Constant** args, Closure* closure);

void helper_gc(Interpreter* interpreter);

void helper_assert_int(Value* v);
void helper_assert_str(Value* v);
void helper_assert_bool(Value* v);
void helper_assert_record(Value* v);
void helper_assert_func(Value* v);
void helper_assert_closure(Value* v);
void helper_assert_valwrapper(Value* v);

int32_t helper_unbox_int(Integer* v);
bool helper_unbox_bool(Boolean* b);

Integer* helper_new_integer(Interpreter* interpreter, int32_t val);
Boolean* helper_new_boolean(Interpreter* interpreter, bool val);
Record* helper_new_record(Interpreter* interpreter);
ValWrapper* helper_new_valwrapper(Interpreter* interpreter, Constant* ptr);

String* helper_cast_string(Interpreter* interpreter, Value* val);

Value* helper_get_record_field(Interpreter* interpreter, string* field, Record* record);
Record* helper_set_record_field(Interpreter* interpreter, string* field, Record* record, Value* val);
Value* helper_get_record_index(Interpreter* interpreter, Value* index, Record* record);
Record* helper_set_record_index(Interpreter* interpreter, Value* index, Record* record, Value* val);
