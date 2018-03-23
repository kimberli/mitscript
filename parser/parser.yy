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
%parse-param {yyscan_t yyscanner} {Block*& out}
%lex-param {yyscan_t yyscanner}
%locations
%define parse.error verbose
%debug

%code provides{
YY_DECL;
int yyerror(YYLTYPE * yylloc, yyscan_t yyscanner, Block*& out, const char* message);
}



%union {
	int intconst;
    string* strconst;
    bool boolconst;

	Statement* stmt_type;
    Expression* expr_type;
    Block* block_type;
    RecordExpr* record_type;
    Identifier* id_type;
    vector<Expression*>* expr_list_type;
    vector<Identifier*>* id_list_type;
}

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

%type<block_type> Program
%type<block_type> StatementList Block
%type<stmt_type> Statement Assignment CallStatement Global IfStatement WhileLoop Return
%type<expr_type> Expression FunctionExpr Boolean RecordExpr ConstantExpr
%type<expr_list_type> ExpressionList ExpressionListHead
%type<id_list_type> ArgsList ArgsListHead
%type<expr_type> Conjunction BoolUnit Predicate Arithmetic Product Unit PosUnit Lhs Call
%type<record_type> RecordList
%type<id_type> Name

%start Program

// You must also define any associativity directives that are necessary
// to resolve ambiguities and properly parse the code.
// TODO: don't think I need this? since left associative

%%

// Your grammar rules should be written here.

Program:
StatementList {
    $$ = $1;
    out = $$;
}

StatementList:
%empty {
    $$ = new Block();
}
| StatementList Statement {
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
}

CallStatement:
Call ';' {
    $$ = new CallStatement(*$1);
}

Global:
T_GLOBAL Name ';' {
    $$ = new Global(*$2);
}

IfStatement:
T_IF '(' Expression ')' Block T_ELSE Block {
    $$ = new IfStatement(*$3, *$5, $7);
}
| T_IF '(' Expression ')' Block {
    $$ = new IfStatement(*$3, *$5, nullptr);
}

WhileLoop:
T_WHILE '(' Expression ')' Block {
    $$ = new WhileLoop(*$3, *$5);
}

Return:
T_RETURN Expression ';' {
    $$ = new Return(*$2);
}

Expression:
FunctionExpr
| Boolean
| RecordExpr

FunctionExpr:
T_FUN '(' ArgsList ')' Block {
    $$ = new FunctionExpr(*$3, *$5);
}

ArgsList:
%empty {
    $$ = new vector<Identifier*>();
}
| ArgsListHead Name {
    $1->push_back($2);
}

ArgsListHead:
%empty {
    $$ = new vector<Identifier*>();
}
| ArgsListHead Name ',' {
    $1->push_back($2);
    $$ = $1;
}

Boolean:
Conjunction {
    $$ = $1;
}
| Boolean T_OR Conjunction {
    $$ = new BinaryExpr(BinOp::Or, *$1, *$3);
}

Conjunction:
BoolUnit {
    $$ = $1;
}
| Conjunction T_AND BoolUnit {
    $$ = new BinaryExpr(BinOp::And, *$1, *$3);
}

BoolUnit:
Predicate {
    $$ = $1;
}
| T_NOT Predicate {
    $$ = new UnaryExpr(UnOp::Not, *$2);
}

Predicate:
Arithmetic {
    $$ = $1;
}
| Arithmetic T_LT Arithmetic {
    $$ = new BinaryExpr(BinOp::Lt, *$1, *$3);
}
| Arithmetic T_GT Arithmetic {
    $$ = new BinaryExpr(BinOp::Gt, *$1, *$3);
}
| Arithmetic T_LTEQ Arithmetic {
    $$ = new BinaryExpr(BinOp::Lt_eq, *$1, *$3);
}
| Arithmetic T_GTEQ Arithmetic {
    $$ = new BinaryExpr(BinOp::Gt_eq, *$1, *$3);
}
| Arithmetic T_EQEQ Arithmetic {
    $$ = new BinaryExpr(BinOp::Eq_eq, *$1, *$3);
}

Arithmetic:
Product {
    $$ = $1;
}
| Arithmetic T_PLUS Product {
    $$ = new BinaryExpr(BinOp::Plus, *$1, *$3);
}
| Arithmetic T_MINUS Product {
    $$ = new BinaryExpr(BinOp::Minus, *$1, *$3);
}

Product:
Unit {
    $$ = $1;
}
| Product T_TIMES Unit {
    $$ = new BinaryExpr(BinOp::Times, *$1, *$3);
}
| Product T_DIVIDE Unit {
    $$ = new BinaryExpr(BinOp::Divide, *$1, *$3);
}

Unit:
PosUnit {
    $$ = $1;
}
| T_MINUS PosUnit {
    $$ = new UnaryExpr(UnOp::Neg, *$2);
}

PosUnit:
Lhs
| ConstantExpr
| Call
| '(' Boolean ')' {
    $$ = $2;
}

Lhs:
Name {
    $$ = new Identifier(*$1);
}
| Lhs '.' Name {
    $$ = new FieldDeref(*$1, *$3);
}
| Lhs '[' Expression ']' {
    $$ = new IndexExpr(*$1, *$3);
}

Call:
Lhs '(' ExpressionList ')' {
    $$ = new Call(*$1, *$3);
}

ExpressionList:
%empty {
    $$ = new vector<Expression*>();
}
| ExpressionListHead Expression {
    $1->push_back($2);
    $$ = $1;
}

ExpressionListHead:
%empty {
    $$ = new vector<Expression*>();
}
| ExpressionListHead Expression ',' {
    $1->push_back($2);
}

RecordExpr:
'{' RecordList '}' {
    $$ = $2;
}

RecordList:
%empty {
    $$ = new RecordExpr();
}
| RecordList Name ':' Expression ';' {
    $1->record.insert(make_pair($2, $4));
}

Name:
T_ID {
    $$ = new Identifier(*$1);
}

ConstantExpr:
T_INT {
    $$ = new IntConst($1);
}
| T_STR {
    $$ = new StrConst(*$1);
}
| T_BOOL {
    $$ = new BoolConst($1);
}
| T_NONE {
    $$ = new NoneConst();
}

%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Block*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg<<"\n";
  return 0;
}

