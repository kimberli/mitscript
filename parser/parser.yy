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
	int         intconst;
    string      *strconst;
    bool        boolconst;

	Statement   *stmt_type;
    vector<Statement*> *stmt_list_type;
}

// Below is where you define your tokens and their types.
// for example, we have defined for you a T_int token, with type intconst
// the type is the name of a field from the union above
%token<intconst> T_INT
%token<strconst> T_STR
%token<boolconst> T_BOOL
%token<intconst> T_NONE
%token<intconst> T_GLOBAL T_IF T_ELSE T_WHILE T_RETURN T_FUN
%token<intconst> T_EQ
%token<intconst> T_LT T_GT T_LTEQ T_GTEQ T_EQEQ
%token<intconst> T_PLUS T_MINUS T_TIMES T_DIVIDE
%token<intconst> T_AND T_OR T_NOT
%token<strconst> T_ID

// Use the %type directive to specify the types of AST nodes produced by each production.
// For example, you will have a program non-terimnal in your grammar, and it will
// return a Statement*. As with tokens, the name of the type comes
// from the union defined earlier.

%type<stmt_list_type> Program

%start Program

// You must also define any associativity directives that are necessary
// to resolve ambiguities and properly parse the code.
// TODO

%%

// Your grammar rules should be written here.

Program:
StatementList {  
    // $$ = $1; // TODO
    // out = $$;
}
;

StatementList:
%empty
| StatementList Statement {
}
;

Statement:
Assignment {
}
| CallStatement {
}
| Global {
}
| IfStatement {
}
| WhileLoop {
}
| Return {
}
;

Global:
T_GLOBAL Name ';' {
}
;

Assignment:
Lhs T_EQ Expression ';' {
}
;

CallStatement:
Call ';' {
}

Block:
'{' StatementList '}' {
}

IfStatement:
T_IF '(' Expression ')' Block T_ELSE Block {
}
| T_IF '(' Expression ')' Block {
}

WhileLoop:
T_WHILE '(' Expression ')' Block {
}

Return:
T_RETURN Expression ';' {
}

Expression:
Function {
}
Boolean {
}
Record {
}

Function:
T_FUN '(' ArgsList ')' Block {
}

ArgsList:
%empty
| ArgsListHead Name {
}

ArgsListHead:
%empty
| ArgsListHead Name ',' {
}

Boolean:
Conjunction {
}
| Conjunction T_OR Boolean {
}

Conjunction:
BoolUnit {
}
| BoolUnit T_AND Conjunction {
}

BoolUnit:
Predicate {
}
| T_NOT Predicate {
}

Predicate:
Arithmetic {
}
| Arithmetic T_LT Arithmetic {
}
| Arithmetic T_GT Arithmetic {
}
| Arithmetic T_LTEQ Arithmetic {
}
| Arithmetic T_GTEQ Arithmetic {
}
| Arithmetic T_EQEQ Arithmetic {
}

Arithmetic:
Product {
}
| Product T_PLUS Arithmetic {
}
| Product T_MINUS Arithmetic {
}

Product:
Unit {
}
| Unit T_TIMES Product {
}
| Unit T_DIVIDE Product {
}

Unit:
PosUnit {
}
| T_MINUS PosUnit {
}

PosUnit:
Lhs {
}
| Constant {
}
| Call {
}
| '(' Boolean ')' {
}

Lhs:
Name Rhs {
}

Rhs:
%empty
| Rhs '.' Name {
}
| Rhs '[' Expression ']' {
}

Call:
Lhs '(' ExpressionList ')' {
}

ExpressionList:
%empty
| ExpressionListHead Expression {
}

ExpressionListHead:
%empty
| ExpressionListHead Expression ',' {
}

Record:
'{' RecordList '}' {
}

RecordList:
%empty
| RecordList Name ':' Expression ';' {
}

Name:
T_ID {
}

Constant:
T_INT {
}
| T_STR {
}
| T_BOOL {
}
| T_NONE {
}

%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Statement*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg;
  return 0;
}

