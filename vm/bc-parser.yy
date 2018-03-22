%code requires{

#include <iostream>
#include <string>
#define YY_DECL int bclex (BCSTYPE* yylval, BCLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER 
#include "bc-lexer.h"
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


#include "instructions.h"
#include "types.h"

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
	vector<shared_ptr<Function>>*   funclist;

    vector<string>* identlist;
	
	Instruction* inst;
	vector<Instruction>* instlist;
	

    Constant* constant;
    vector<shared_ptr<Constant>>* constantlist;
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
%token T_instructions;

%token T_load_const
%token T_load_func
%token T_load_local
%token T_store_local
%token T_load_global
%token T_store_global

%token T_push_ref
%token T_load_ref
%token T_store_ref

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
%token T_dup
%token T_swap
%token T_pop

%type<func> Function

%type<funclist> FunctionListStar
%type<funclist> FunctionListPlus

%type<identlist> IdentListStar
%type<identlist> IdentListPlus

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
  T_instructions '=' '[' InstructionList ']' 
  '}'
{
	$$ = new Function{*$6, *$12, safe_unsigned_cast($17), *$22, *$28, *$34, *$40, *$46};

    out = $$;
}

FunctionListStar:
  %empty { $$ = new vector<shared_ptr<Function>>(); } 
| FunctionListPlus
{
	$$ = $1;
}

FunctionListPlus:
Function
{
	auto list = new vector<shared_ptr<Function>>();
  
  	list->insert(list->begin(), std::shared_ptr<Function>($1));
	
	$$ = list;
}
| Function ',' FunctionListPlus 
{
     auto list = $3;

     list->insert(list->begin(), std::shared_ptr<Function>($1));
	 
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

Constant : 
  T_none
{
	$$ = new None();
}
| T_true
{
	$$ = new Boolean(true);
}
| T_false
{
	$$ = new Boolean(false);
}
|  T_string 
{
	$$ = new String(*$1);

	delete $1;
}
| T_int 
{
	$$ = new Integer{$1};
}

ConstantListStar:
%empty { $$ = new vector<shared_ptr<Constant> >(); } 
| ConstantListPlus
{	
	$$ = $1;
}

ConstantListPlus:
Constant 
{
	auto list = new vector<shared_ptr<Constant>>();

	list->insert(list->begin(), std::shared_ptr<Constant>($1));
	
	$$ = list;
}
| Constant ',' ConstantListPlus 
{
	auto list = $3;

	list->insert(list->begin(), std::shared_ptr<Constant>($1));

	$$ = list;
}

Instruction:	
  T_load_const T_int
{
	$$ = new Instruction(Operation::LoadConst, safe_unsigned_cast($2));
}
| T_load_func T_int
{
	$$ = new Instruction(Operation::LoadFunc, safe_unsigned_cast($2));
}

| T_load_local T_int
{
	$$ = new Instruction(Operation::LoadLocal, safe_unsigned_cast($2));
}
| T_store_local T_int
{
	$$ = new Instruction(Operation::StoreLocal, safe_unsigned_cast($2));
}
| T_load_global T_int
{
	$$ = new Instruction(Operation::LoadGlobal, safe_unsigned_cast($2));
}
| T_store_global T_int
{
	$$ = new Instruction(Operation::StoreGlobal, safe_unsigned_cast($2));
}
| T_push_ref T_int
{
	$$ = new Instruction(Operation::PushReference, safe_unsigned_cast($2));
}
| T_load_ref
{
	$$ = new Instruction(Operation::LoadReference, { });
}
| T_store_ref
{
	$$ = new Instruction(Operation::StoreReference, { });
}
| T_alloc_record
{
	$$ = new Instruction(Operation::AllocRecord, { });
}
| T_field_load T_int
{
	$$ = new Instruction(Operation::FieldLoad, safe_unsigned_cast($2));
}
| T_field_store T_int
{
	$$ = new Instruction(Operation::FieldStore, safe_unsigned_cast($2));
}
| T_index_load
{
	$$ = new Instruction(Operation::IndexLoad, { });
}
| T_index_store
{
	$$ = new Instruction(Operation::IndexStore, { });
}
| T_alloc_closure T_int
{
	$$ = new Instruction(Operation::AllocClosure, safe_unsigned_cast($2));	
}
| T_call
{
	$$ = new Instruction(Operation::Call, { });	
}
| T_return
{
	$$ = new Instruction(Operation::Return, { });	
}
| T_add
{
	$$ = new Instruction(Operation::Add, { });	
}
| T_sub
{
	$$ = new Instruction(Operation::Sub, { });	
}
| T_mul
{
	$$ = new Instruction(Operation::Mul, { });	
}
| T_div
{
	$$ = new Instruction(Operation::Div, { });	
}
| T_neg
{
	$$ = new Instruction(Operation::Neg, { });	
}
| T_gt
{
	$$ = new Instruction(Operation::Gt, { });	
}
| T_geq
{
	$$ = new Instruction(Operation::Geq, { });	
}
| T_eq
{
	$$ = new Instruction(Operation::Eq, { });	
}
| T_and
{
	$$ = new Instruction(Operation::And, { });	
}
| T_or
{
	$$ = new Instruction(Operation::Or, { });	
}
| T_not
{
	$$ = new Instruction(Operation::Not, { });	
}
| T_goto T_int
{
	$$ = new Instruction(Operation::Goto, safe_cast($2));	
}
| T_if T_int
{
	$$ = new Instruction(Operation::If, safe_cast($2));	
}
| T_dup
{
	$$ = new Instruction(Operation::Dup, { });	
}
| T_swap
{
	$$ = new Instruction(Operation::Swap, { });	
}
| T_pop
{
	$$ = new Instruction(Operation::Pop, { });	
}

InstructionList:
  %empty { $$ = new vector<Instruction>(); } 
| Instruction InstructionList
{
	auto list = $2;
	list->insert(list->begin(), Instruction(*$1));

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
