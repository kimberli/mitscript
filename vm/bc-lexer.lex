
%{

#include <string>
#include "bc-parser.h"
using std::string;
// You can put additional header files here.

%}

%option reentrant
%option noyywrap
%option never-interactive

int_const -?[0-9][0-9]*

whitespace   ([ \t\n]*)
%{
// Initial declarations
// In this section of the file, you can define named regular expressions.
// like int_const and whitespace above
//begin_student_code
%}
name	[a-zA-Z_][a-zA-Z0-9_]*


string_const ("\""[^\n\"]*"\"")

Operator     ([\%\/\<\>\;\!\?\*\-\+\,\.\:\[\]\(\)\{\}\=\|\&\^\$])


comment      ("//"[^\n]*)
%{
//end_student_code
%}

%%


{whitespace}   { /* skip */ }

{comment}      { /* skip */ }


{int_const}    { 
		//Rule to identify an integer constant. 
		//The return value indicates the type of token;
		//in this case T_int as defined in parser.yy.
		//The actual value of the constant is returned
		//in the intconst field of yylval (defined in the union
		//type in parser.yy).
			yylval->intconst = atoi(yytext);
			return T_int;
		}

%{
// The rest of your lexical rules go here. 
// rules have the form 
// pattern action
// we have defined a few rules for you above, but you need
// to provide additional lexical rules for string constants, 
// operators, keywords and identifiers.
//begin_student_code
%}


{string_const}  {

			string*  tmp = new string(yytext);
			*tmp = tmp->substr(1, tmp->size() -2);
			yylval->strconst = tmp;
			return T_string;
		}



"None" 		{ return T_none; }
"true" 		{ return T_true; }
"false"		{ return T_false; }
"function"  { return T_function; }
"functions" { return T_functions; }
"constants" { return T_constants; }
"parameter_count" { return T_parameter_count; }
"local_vars" { return T_local_vars; }
"local_ref_vars" { return T_local_ref_vars; }
"names" { return T_names; }
"free_vars" { return T_free_vars; }
"instructions" { return T_instructions; }

"load_const" { return T_load_const; }
"load_func" { return T_load_func; }
"load_local" { return T_load_local; }
"store_local" { return T_store_local; }
"load_global" { return T_load_global; }
"store_global" { return T_store_global; }
"push_ref"     { return T_push_ref; }
"load_ref"     { return T_load_ref; }
"store_ref"    { return T_store_ref; }
"alloc_record" { return T_alloc_record; }
"field_load"   { return T_field_load; }
"field_store"  { return T_field_store; }
"index_load"   { return T_index_load; }
"index_store"  { return T_index_store; }
"alloc_closure" { return T_alloc_closure; }
"call"  { return T_call; }
"return" { return T_return; }
"add" { return T_add; }
"sub" { return T_sub; }
"mul" { return T_mul; }
"div" { return T_div; }
"neg" { return T_neg; }
"gt" { return T_gt; }
"geq" { return T_geq; }
"eq" { return T_eq; }
"and" { return T_and; }
"or" { return T_or; }
"not" { return T_not; }
"goto" { return T_goto; }
"if"  { return T_if; }
"dup" { return T_dup; }
"swap" { return T_swap; }
"pop" {return T_pop; }

{Operator} {  return yytext[0]; }

{name} 		{ 
			yylval->strconst = new std::string(yytext);
			return T_ident;
		}

%{
//end_student_code
%}

%%

