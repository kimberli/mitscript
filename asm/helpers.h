#pragma once 

#include <string>

//#include "../vm/interpreter.h"
#include "../types.h"

class Interpreter;

void helper_store_global(Interpreter* interpreter, std::string &name, Value* val); 

Value* helper_load_global(Interpreter* interpreter, std::string &name); 

Value* helper_add(Interpreter* interpreter, Value* left, Value* right);

Value* helper_call(Interpreter* interpreter, Closure* closure); 

void helper_gc(Interpreter* interpreter);

void helper_assert_int(Value* v); 
void helper_assert_str(Value* v); 
void helper_assert_bool(Value* v); 
void helper_assert_record(Value* v); 
void helper_assert_func(Value* v); 
void helper_assert_closure(Value* v); 


