#pragma once

#include "../exception.h"
#include "../types.h"

typedef int64_t tagptr_t;
#define PTR_TAG 0
#define INT_TAG 1
#define BOOL_TAG 2
#define STR_TAG 3
#define ALL_TAG 3
#define CLEAR_TAG ~3
#define NULL_PTR 0

using namespace std;

struct Value;

bool check_tag(tagptr_t ptr, int tag);
bool is_tagged(tagptr_t ptr);
string get_type(tagptr_t ptr);

tagptr_t make_ptr(int val);
tagptr_t make_ptr(bool val);
tagptr_t make_ptr(string* val);
tagptr_t make_ptr(Constant* val);
tagptr_t make_ptr(Function* val);
tagptr_t make_ptr(ValWrapper* val);

int get_int(tagptr_t ptr);
bool get_bool(tagptr_t ptr);
string* get_str(tagptr_t ptr);
Collectable* get_collectable(tagptr_t ptr);
Value* get_val(tagptr_t ptr);

template<typename T>
T* cast_val(tagptr_t ptr);

string ptr_to_str(tagptr_t ptr);
tagptr_t ptr_equals(tagptr_t left, tagptr_t right);
tagptr_t ptr_add(tagptr_t left, tagptr_t right);
