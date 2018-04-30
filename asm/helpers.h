#pragma once 

#include "../vm/interpreter.h"

void helper_store_global(vptr<Interpreter> interpreter, std::string &name, vptr<Value> val); 

vptr<Value> helper_load_global(vptr<Interpreter> interpreter, std::string &name); 

vptr<Value> helper_add(vptr<Interpreter> interpreter, Value* left, Value* right);

vptr<Value> helper_call(vptr<Interpreter> interpreter, vptr<Closure> closure); 

void helper_gc(vptr<Interpreter> interpreter);

void helper_assert_int(Value* v); 
void helper_assert_str(Value* v); 
void helper_assert_bool(Value* v); 
void helper_assert_record(Value* v); 
void helper_assert_func(Value* v); 
void helper_assert_closure(Value* v); 


