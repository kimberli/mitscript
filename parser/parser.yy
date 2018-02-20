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
    string* strconst;
    bool boolconst;

	Statement* stmt_type;
    Expression* expr_type;
    Block* block_type;
    vector<Expression*>* expr_list_type;
    vector<string>* str_list_type;
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

%type<block_type> Program
%type<block_type> StatementList Block
%type<stmt_type> Statement Assignment CallStatement Global IfStatement WhileLoop Return
%type<expr_type> Expression Function Boolean Record Constant
%type<expr_list_type> ExpressionList ExpressionListHead RecordList
%type<str_list_type> ArgsList ArgsListHead
%type<expr_type> Conjunction BoolUnit Predicate Arithmetic Product Unit PosUnit Lhs LhsTail Call
%type<strconst> Name

%start Program

// You must also define any associativity directives that are necessary
// to resolve ambiguities and properly parse the code.
// TODO: don't think I need this? since left associative

%%

// Your grammar rules should be written here.

Program:
StatementList {  
    cout << "Program: " << $$ <<  endl;
    $$ = new Block(*$1);
    out = $$->stmts.front(); // TODO: what type to return?
}

StatementList:
%empty
| StatementList Statement {
    cout << "Statement done" << endl;
    $1->stmts.push_back($2);
}

Block:
'{' StatementList '}' {
    $$ = $2;
}

Statement:
Assignment
| CallStatement
| Global
| IfStatement
| WhileLoop
| Return

Assignment:
Lhs T_EQ Expression ';' {
    $$ = new Assignment(*$1, *$3);
    cout << "Assignment" << endl;
}

CallStatement:
Call ';' {
    $$ = new CallStatement(*$1);
    cout << "Call" << endl;
}

Global:
T_GLOBAL Name ';' {
    $$ = new Global(*$2);
    cout << "Global" << endl;
}

IfStatement:
T_IF '(' Expression ')' Block T_ELSE Block {
    $$ = new IfStatement(*$3, *$5, $7);
    cout << "IfStatement" << endl;
}
| T_IF '(' Expression ')' Block {
    $$ = new IfStatement(*$3, *$5, nullptr);
    cout << "IfStatement" << endl;
}

WhileLoop:
T_WHILE '(' Expression ')' Block {
    $$ = new WhileLoop(*$3, *$5);
    cout << "WhileLoop" << endl;
}

Return:
T_RETURN Expression ';' {
    $$ = new Return(*$2);
    cout << "Return" << endl;
}

Expression:
Function
| Boolean
| Record

Function:
T_FUN '(' ArgsList ')' Block {
    $$ = new Function(*$3, *$5);
}

ArgsList:
%empty {
    $$ = new vector<string>();
}
| ArgsListHead Name {
    $$ = new vector<string>();
    $$->push_back(*$2);
}

ArgsListHead:
%empty
| ArgsListHead Name ',' {
    $$->push_back(*$2);
}

Boolean:
Conjunction {
    $$ = new UnaryExpr(*$1);
}
| Conjunction T_OR Boolean {
    $$ = new BinaryExpr(Op::Or, *$1, *$3);
}

Conjunction:
BoolUnit {
    $$ = new UnaryExpr(*$1);
}
| BoolUnit T_AND Conjunction {
    $$ = new BinaryExpr(Op::And, *$1, *$3);
}

BoolUnit:
Predicate {
    $$ = new UnaryExpr(*$1);
}
| T_NOT Predicate {
    $$ = new UnaryExpr(Op::Not, *$2);
}

Predicate:
Arithmetic {
    $$ = new UnaryExpr(*$1);
}
| Arithmetic T_LT Arithmetic {
    $$ = new BinaryExpr(Op::Lt, *$1, *$3);
}
| Arithmetic T_GT Arithmetic {
    $$ = new BinaryExpr(Op::Gt, *$1, *$3);
}
| Arithmetic T_LTEQ Arithmetic {
    $$ = new BinaryExpr(Op::Lt_eq, *$1, *$3);
}
| Arithmetic T_GTEQ Arithmetic {
    $$ = new BinaryExpr(Op::Gt_eq, *$1, *$3);
}
| Arithmetic T_EQEQ Arithmetic {
    $$ = new BinaryExpr(Op::Eq_eq, *$1, *$3);
}

Arithmetic:
Product {
    $$ = new UnaryExpr(*$1);
}
| Product T_PLUS Arithmetic {
    $$ = new BinaryExpr(Op::Plus, *$1, *$3);
}
| Product T_MINUS Arithmetic {
    $$ = new BinaryExpr(Op::Minus, *$1, *$3);
}

Product:
Unit {
    $$ = new UnaryExpr(*$1);
}
| Unit T_TIMES Product {
    $$ = new BinaryExpr(Op::Times, *$1, *$3);
}
| Unit T_DIVIDE Product {
    $$ = new BinaryExpr(Op::Divide, *$1, *$3);
}

Unit:
PosUnit {
    $$ = new UnaryExpr(*$1);
}
| T_MINUS PosUnit {
    $$ = new UnaryExpr(Op::Minus, *$2);
}

PosUnit:
Lhs
| Constant
| Call
| '(' Boolean ')' {
    $$ = $2;
}

Lhs:
Name LhsTail {
}

LhsTail:
%empty
| LhsTail '.' Name {
}
| LhsTail '[' Expression ']' {
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

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg<<endl;
  return 0;
}

