%code requires{

#include <iostream>
#include <string>
#define YY_DECL int yylex (YYSTYPE* yylval, YYLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER
#include "lexer.h"
#endif

using namespace std;

// The macro below is used by bison for error reporting
// it comes from stackoverflow
// http://stackoverflow.com/questions/656703/how-does-flex-support-bison-location-exactly
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


#include "AST.h"
// If you need additional header files, put them here.


}


%define api.pure full
%parse-param {yyscan_t yyscanner} {Statement*& out}
%lex-param {yyscan_t yyscanner}
%locations
%define parse.error verbose

%code provides{
YY_DECL;
int yyerror(YYLTYPE * yylloc, yyscan_t yyscanner, Statement*& out, const char* message);
}



// The union directive defines a union type that will be used to store
// the return values of all the parse rules. We have initialized for you
// with an intconst field that you can use to store an integer, and a
// stmt field with a pointer to a statement. Note that one limitation
// is that you can only use primitive types and pointers in the union.
%union {
	int intconst;
	Statement*   stmt;
}

// Below is where you define your tokens and their types.
// for example, we have defined for you a T_int token, with type intconst
// the type is the name of a field from the union above
%token<intconst> T_int

// Use the %type directive to specify the types of AST nodes produced by each production.
// For example, you will have a program non-terimnal in your grammar, and it will
// return a Statement*. As with tokens, the name of the type comes
// from the union defined earlier.

%type<stmt> Program

%start Program

// You must also define any associativity directives that are necessary
// to resolve ambiguities and properly parse the code.

%%

// Your grammar rules should be written here.

Program:
StatementList {  
    $$ = // assign a new block to $$.
    // and make sure the out variable is set, because that is what main is going to read.
out = $$;

%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Statement*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg;
  return 0;
}

