
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
sep [;:,\.]

identifier [a-zA-Z_][a-zA-Z0-9_]*

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

{int_const} {
    DEBUG(1, "\tlexed " << yytext << endl);
    yylval->intconst = atoi(yytext);
    return T_INT;
}

{str_const} {
    DEBUG(1, "\tlexed " << yytext << endl);
    yylval->strconst = new string(yytext);
    return T_STR;
}

{true_const} {
    DEBUG(1, "\tlexed " << yytext << endl);
    yylval->boolconst = true;
    return T_BOOL;
}

{false_const} {
    DEBUG(1, "\tlexed " << yytext << endl);
    yylval->boolconst = false;
    return T_BOOL;
}

{none_const} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_NONE;
}

{global_kywd} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_GLOBAL;
}

{if_kywd} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_IF;
}

{else_kywd} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_ELSE;
}

{while_kywd} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_WHILE;
}

{return_kywd} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_RETURN;
}

{fun_kywd} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_FUN;
}

= {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_EQ;
}

\< {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_LT;
}

\> {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_GT;
}

\<= {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_LTEQ;
}

\>= {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_GTEQ;
}

== {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_EQEQ;
}

\+ {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_PLUS;
}

\- {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_MINUS;
}

\* {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_TIMES;
}

\/ {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_DIVIDE;
}

\& {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_AND;
}

\| {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_OR;
}

! {
    DEBUG(1, "\tlexed " << yytext << endl);
    return T_NOT;
}

{brace} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return yytext[0];
}

{sep} {
    DEBUG(1, "\tlexed " << yytext << endl);
    return yytext[0];
}

{identifier} {
    DEBUG(1, "\tlexed " << yytext << endl);
    yylval->strconst = new string(yytext);
    return T_ID;
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

