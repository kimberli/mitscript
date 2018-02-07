
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

whitespace   ([ \t\n]*)

comment  "//".*"\n"

%{
// Initial declarations
// In this section of the file, you can define named regular expressions.
// like int_const and whitespace above
%}

%%


{whitespace}   { /* skip */ }

{comment}      { /* skip */ }


{int_const}    {
		// Rule to identify an integer constant.
		// The return value indicates the type of token;
		// in this case T_int as defined in parser.yy.
		// The actual value of the constant is returned
		// in the intconst field of yylval (defined in the union
		// type in parser.yy).
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
%}

%%

