
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
    yylval->intconst = atoi(yytext);
    return T_INT;
}

{str_const} {
    string* replaced = new string(yytext + 1);
    auto pos = replaced->find("\\");
    while (pos != string::npos) {
        if (replaced->at(pos + 1) == 'n') {
            replaced->replace(pos, 2, "\n");
        } else if (replaced->at(pos + 1) == 't') {
            replaced->replace(pos, 2, "\t");
        } else if (replaced->at(pos + 1) == '\\') {
            replaced->replace(pos, 2, "\\");
        } else if (replaced->at(pos + 1) == '"') {
            replaced->replace(pos, 2, "\"");
        }
        pos = replaced->find("\\", pos + 1);
    }
    replaced->pop_back();

    yylval->strconst = replaced;
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

{brace} {
    return yytext[0];
}

{sep} {
    return yytext[0];
}

{identifier} {
    yylval->strconst = new string(yytext);
    return T_ID;
}

<<EOF>> {
    yyterminate();
}

. {
    return *yytext;
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

