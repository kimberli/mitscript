
%{

#include <string>
#include "parser.h"
using std::string;
// You can put additional header files here.

%}

%option reentrant
%option noyywrap
%option never-interactive

int_const [0-9][0-9]*
str_const \"(\\[nt\\\"]|[^\\\"])*\"
true_const true
false_const false
none_const None

global_kywd global
if_kywd if
else_kywd else
while_kywd while
return_kywd return
fun_kywd fun

brace [\(\)\[\]\{\}]
stmt_end ;
arg_sep ,
record_sep :

identifier [a-zA-Z_]\w*

whitespace   ([ \t\n]*)

comment  "//".*"\n"

%{
// Initial declarations
// In this section of the file, you can define named regular expressions.
// like int_const and whitespace above
%}

%%


{whitespace} {} /* skip */

{comment} {} /* skip */

{identifier} {
    yylval->strconst = new string(yytext);
    return T_ID;
}

{int_const} {
    yylval->intconst = atoi(yytext);
    return T_INT;
}

{str_const} {
    yylval->strconst = new string(yytext);
    return T_STR;
}

{true_const} {
    yylval->boolconst = true;
    return T_BOOL;
}

{false_const} {
    yylval->boolconst = false;
    return T_BOOL;
}

{none_const} {
    return T_NONE;
}

{global_kywd} {
    return T_GLOBAL;
}

{if_kywd} {
    return T_IF;
}

{else_kywd} {
    return T_ELSE;
}

{while_kywd} {
    return T_WHILE;
}

{return_kywd} {
    return T_RETURN;
}

{fun_kywd} {
    return T_FUN;
}

= {
    return T_EQ;
}

\< {
    return T_LT;
}

\> {
    return T_GT;
}

\<= {
    return T_LTEQ;
}

\>= {
    return T_GTEQ;
}

== {
    return T_EQEQ;
}

\+ {
    return T_PLUS;
}

\- {
    return T_MINUS;
}

\* {
    return T_TIMES;
}

\/ {
    return T_DIVIDE;
}

\& {
    return T_AND;
}

\| {
    return T_OR;
}

! {
    return T_NOT;
}

. {
    cout << "ERROR unrecognized symbol!" << endl;
}

%{
// The rest of your lexical rules go here.
// rules have the form
// pattern action
// we have defined a few rules for you above, but you need
// to provide additional lexical rules for string constants,
// operators, keywords and identifiers.
%}

%%

