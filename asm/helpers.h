#pragma once

#include <string>
#include <cstdarg>

#include "../types.h"
#include "../opt/opt_tag_ptr.h"

class Interpreter;

void helper_store_global(Interpreter* interpreter, string* name, tagptr_t ptr);

tagptr_t helper_load_global(Interpreter* interpreter, string* name);

void helper_store_local_ref(tagptr_t ptr, tagptr_t ref);

tagptr_t helper_add(Interpreter* interpreter, tagptr_t left, tagptr_t right);

tagptr_t helper_eq(Interpreter* interpreter, tagptr_t left, tagptr_t right);

tagptr_t helper_alloc_closure(Interpreter* interpreter, int numRefs, tagptr_t func_ptr, tagptr_t* refs);

tagptr_t helper_call(Interpreter* interpreter, int numArgs, tagptr_t clos_ptr, tagptr_t* args);

void helper_gc(Interpreter* interpreter);

void helper_assert_int(tagptr_t ptr);
void helper_assert_str(tagptr_t ptr);
void helper_assert_bool(tagptr_t ptr);
void helper_assert_record(tagptr_t ptr);
void helper_assert_func(tagptr_t ptr);
void helper_assert_closure(tagptr_t ptr);
void helper_assert_valwrapper(tagptr_t ptr);
void helper_assert_nonzero(int32_t v);

int32_t helper_unbox_int(Integer* v);
bool helper_unbox_bool(Boolean* b);
tagptr_t helper_unbox_valwrapper(tagptr_t v);

tagptr_t helper_new_record(Interpreter* interpreter);
tagptr_t helper_new_valwrapper(Interpreter* interpreter, tagptr_t ptr);

tagptr_t helper_cast_string(Interpreter* interpreter, tagptr_t ptr);

tagptr_t helper_get_record_field(Interpreter* interpreter, string* field, tagptr_t record_ptr);
tagptr_t helper_set_record_field(Interpreter* interpreter, string* field, tagptr_t record_ptr, tagptr_t ptr);
tagptr_t helper_get_record_index(Interpreter* interpreter, tagptr_t index, tagptr_t record_ptr);
tagptr_t helper_set_record_index(Interpreter* interpreter, tagptr_t index, tagptr_t record_ptr, tagptr_t ptr);
