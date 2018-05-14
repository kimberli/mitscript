/*
 * bc-parser.yy
 *
 * Defines the bytecode parser; compiled with bison
 * Generates AST nodes as defined in types.h
 */
%code requires{

#include <iostream>
#include <string>
#undef YY_DECL
#define YY_DECL int bclex (BCSTYPE* yylval, BCLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER
#include "lexer.h"
#endif

using namespace std;

//The macro below is used by bison for error reporting
//it comes from stacck overflow
//http://stackoverflow.com/questions/656703/how-does-flex-support-bison-location-exactly
#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }


#include "../../instructions.h"
#include "../../types.h"

#include <cassert>

int32_t safe_cast(int64_t value);
uint32_t safe_unsigned_cast(int64_t value);

}


%define api.pure full
%parse-param {yyscan_t yyscanner} {Function*& out}
%lex-param {yyscan_t yyscanner}
%locations
%define parse.error verbose

%code provides{
YY_DECL;
int yyerror(BCLTYPE * yylloc, yyscan_t yyscanner, Function*& out, const char* message);
}


%union {
	int32_t intconst;
	std::string* strconst;

	Function*   func;
	vector<Function*>*   funclist;

    vector<string>* identlist;
    map<int, int>* maplist;

	BcInstruction* inst;
	vector<BcInstruction>* instlist;


    tagptr_t constant;
    vector<tagptr_t>* constantlist;
}

//Below is where you define your tokens and their types.
//for example, we have defined for you a T_int token, with type intconst
//the type is the name of a field from the union above


%token T_none
%token T_false
%token T_true
%token<intconst> T_int
%token<strconst> T_string
%token<strconst> T_ident


%token T_function;
%token T_functions;
%token T_constants;

%token T_parameter_count;
%token T_local_vars;
%token T_local_ref_vars;
%token T_free_vars;
%token T_names;
%token T_labels;
%token T_instructions;

%token T_load_const
%token T_load_func
%token T_load_local
%token T_store_local
%token T_load_global
%token T_store_global

%token T_push_ref
%token T_load_ref

%token T_alloc_record
%token T_field_load
%token T_field_store
%token T_index_load
%token T_index_store
%token T_alloc_closure
%token T_call
%token T_return
%token T_add
%token T_sub
%token T_mul
%token T_div
%token T_neg
%token T_gt
%token T_geq
%token T_eq
%token T_and
%token T_or
%token T_not
%token T_goto
%token T_if
%token T_label
%token T_dup
%token T_swap
%token T_pop

%type<func> Function

%type<funclist> FunctionListStar
%type<funclist> FunctionListPlus

%type<identlist> IdentListStar
%type<identlist> IdentListPlus

%type<maplist> MapListStar
%type<maplist> MapListPlus

%type<constant> Constant
%type<constantlist> ConstantListStar
%type<constantlist> ConstantListPlus

%type<inst> Instruction
%type<instlist> InstructionList

%start Function

// Grammar
%%

Function:
  T_function '{'
  T_functions '=' '[' FunctionListStar ']'       ','
  T_constants '=' '[' ConstantListStar ']'   ','
  T_parameter_count '='  T_int               ','
  T_local_vars '=' '[' IdentListStar ']'         ','
  T_local_ref_vars '='  '[' IdentListStar ']'    ','
  T_free_vars '=' '[' IdentListStar ']'          ','
  T_names     '=' '[' IdentListStar ']'          ','
  T_labels    '=' '{' MapListStar '}'          ','
  T_instructions '=' '[' InstructionList ']'
  '}'
{
	$$ = new Function{*$6, *$12, safe_cast($17), *$22, *$28, *$34, *$40, *$46, *$52};

    out = $$;
}

FunctionListStar:
  %empty { $$ = new vector<Function*>(); }
| FunctionListPlus
{
	$$ = $1;
}

FunctionListPlus:
Function
{
	auto list = new vector<Function*>();

  	list->insert(list->begin(), $1);

	$$ = list;
}
| Function ',' FunctionListPlus
{
     auto list = $3;

     list->insert(list->begin(), $1);

	 $$ = list;
}

IdentListStar:
  %empty { $$ = new vector<string>(); }
| IdentListPlus
{
	$$ = $1;
}

IdentListPlus:
T_ident
{
	auto list = new vector<string>();

	list->insert(list->begin(), *$1);
	delete $1;

	$$ = list;
}
| T_ident ',' IdentListPlus
{
	auto list = $3;

	list->insert(list->begin(), *$1);
	delete $1;

	$$ = list;
}

MapListStar:
  %empty { $$ = new map<int, int>(); }
| MapListPlus
{
    $$ = $1;
}

MapListPlus:
T_int ':' T_int
{
    auto m = new map<int, int>();

    m->insert(map<int, int>::value_type($1, $3));

    $$ = m;
}
| T_int ':' T_int ';' MapListPlus
{
    auto m = $5;

    m->insert(map<int, int>::value_type($1, $3));

    $$ = m;
}

Constant :
  T_none
{
	$$ = make_ptr(new None());
}
| T_true
{
	$$ = make_ptr(true);
}
| T_false
{
	$$ = make_ptr(false);
}
|  T_string
{
	$$ = make_ptr(new string(*$1));

	delete $1;
}
| T_int
{
	$$ = make_ptr(safe_cast($1));
}

ConstantListStar:
%empty { $$ = new vector<tagptr_t>(); }
| ConstantListPlus
{
	$$ = $1;
}

ConstantListPlus:
Constant
{
	auto list = new vector<tagptr_t>();

	list->insert(list->begin(), $1);

	$$ = list;
}
| Constant ',' ConstantListPlus
{
	auto list = $3;

	list->insert(list->begin(), $1);

	$$ = list;
}

Instruction:
  T_load_const T_int
{
	$$ = new BcInstruction(BcOp::LoadConst, safe_cast($2));
}
| T_load_func T_int
{
	$$ = new BcInstruction(BcOp::LoadFunc, safe_cast($2));
}

| T_load_local T_int
{
	$$ = new BcInstruction(BcOp::LoadLocal, safe_cast($2));
}
| T_store_local T_int
{
	$$ = new BcInstruction(BcOp::StoreLocal, safe_cast($2));
}
| T_load_global T_int
{
	$$ = new BcInstruction(BcOp::LoadGlobal, safe_cast($2));
}
| T_store_global T_int
{
	$$ = new BcInstruction(BcOp::StoreGlobal, safe_cast($2));
}
| T_push_ref T_int
{
	$$ = new BcInstruction(BcOp::PushReference, safe_cast($2));
}
| T_load_ref
{
	$$ = new BcInstruction(BcOp::LoadReference, { });
}
| T_alloc_record
{
	$$ = new BcInstruction(BcOp::AllocRecord, { });
}
| T_field_load T_int
{
	$$ = new BcInstruction(BcOp::FieldLoad, safe_cast($2));
}
| T_field_store T_int
{
	$$ = new BcInstruction(BcOp::FieldStore, safe_cast($2));
}
| T_index_load
{
	$$ = new BcInstruction(BcOp::IndexLoad, { });
}
| T_index_store
{
	$$ = new BcInstruction(BcOp::IndexStore, { });
}
| T_alloc_closure T_int
{
	$$ = new BcInstruction(BcOp::AllocClosure, safe_cast($2));
}
| T_call T_int
{
	$$ = new BcInstruction(BcOp::Call, safe_cast($2));
}
| T_return
{
	$$ = new BcInstruction(BcOp::Return, { });
}
| T_add
{
	$$ = new BcInstruction(BcOp::Add, { });
}
| T_sub
{
	$$ = new BcInstruction(BcOp::Sub, { });
}
| T_mul
{
	$$ = new BcInstruction(BcOp::Mul, { });
}
| T_div
{
	$$ = new BcInstruction(BcOp::Div, { });
}
| T_neg
{
	$$ = new BcInstruction(BcOp::Neg, { });
}
| T_gt
{
	$$ = new BcInstruction(BcOp::Gt, { });
}
| T_geq
{
	$$ = new BcInstruction(BcOp::Geq, { });
}
| T_eq
{
	$$ = new BcInstruction(BcOp::Eq, { });
}
| T_and
{
	$$ = new BcInstruction(BcOp::And, { });
}
| T_or
{
	$$ = new BcInstruction(BcOp::Or, { });
}
| T_not
{
	$$ = new BcInstruction(BcOp::Not, { });
}
| T_goto T_int
{
	$$ = new BcInstruction(BcOp::Goto, safe_cast($2));
}
| T_if T_int
{
	$$ = new BcInstruction(BcOp::If, safe_cast($2));
}
| T_label T_int
{
	$$ = new BcInstruction(BcOp::Label, safe_cast($2));
}
| T_dup
{
	$$ = new BcInstruction(BcOp::Dup, { });
}
| T_swap
{
	$$ = new BcInstruction(BcOp::Swap, { });
}
| T_pop
{
	$$ = new BcInstruction(BcOp::Pop, { });
}

InstructionList:
  %empty { $$ = new vector<BcInstruction>(); }
| Instruction InstructionList
{
	auto list = $2;
	list->insert(list->begin(), BcInstruction(*$1));

	delete $1;

	$$ = list;
}
%%

// Error reporting function. You should not have to modify this.
int yyerror(BCLTYPE * yylloc, void* p, Function*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg <<endl;
  return 0;
}

int32_t safe_cast(int64_t value)
{
	int32_t new_value = (int32_t) value;

	assert(new_value == value);

	return new_value;
}


uint32_t safe_unsigned_cast(int64_t value)
{
	int32_t new_value = (uint32_t) value;

	assert(new_value == value);

	assert (0 <= new_value);

	return new_value;
}
