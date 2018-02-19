
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

{int_const} {
    yylval->intconst = atoi(yytext);
    return T_INT;
}

{str_const} {
    yylval->strconst = yytext;
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
    yylval->noneconst = 0;
    return T_NONE;
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

