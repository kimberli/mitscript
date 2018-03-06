#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include "Visitor.h"

#define Assert(cond, msg) if(!(cond)){ std::cerr<<msg<<endl; throw SystemException("Bad stuff"); }

enum Op {
    None,
    Or,
    And,
    Not,
    Lt,
    Gt,
    Lt_eq,
    Gt_eq,
    Eq_eq,
    Plus,
    Minus,
    Times,
    Divide
};

using namespace std;

class SystemException {
	string msg_;
public:
	SystemException(const string& msg) :msg_(msg) {}
};


class AST_node {
public:
	virtual void accept(Visitor& v) = 0;
};

class Expression : public AST_node {
};

class Statement: public AST_node {
};

class Constant: public Expression {
};

class Block: public AST_node {
public:
    vector<Statement*> stmts;
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Global: public Statement {
public:
    Identifier& name;
    Global(Identifier& name): name(name) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Assignment: public Statement {
public:
    Expression& lhs;
    Expression& expr;
    Assignment(Expression& lhs, Expression& expr): lhs(lhs), expr(expr) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class CallStatement: public Statement {
public:
    Expression& call;
    CallStatement(Expression& call): call(call) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class IfStatement: public Statement {
public:
    Expression& condition;
    Block& then_block;
    Block* else_block;
    IfStatement(Expression& condition, Block& then_block, Block* else_block):
        condition(condition), then_block(then_block), else_block(else_block) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class WhileLoop: public Statement {
public:
    Expression& condition;
    Block& body;
    WhileLoop(Expression& condition, Block& body):
        condition(condition), body(body) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Return: public Statement {
public:
    Expression& expr;
    Return(Expression& expr): expr(expr) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Function: public Expression {
public:
    vector<Identifier*> args;
    Block& body;
    Function(vector<Identifier*> args, Block& body): args(args), body(body) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class BinaryExpr: public Expression {
public:
    Expression& left;
    Op op;
    Expression& right;
    BinaryExpr(Op op, Expression& left, Expression& right):
        left(left), op(op), right(right) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class UnaryExpr: public Expression {
public:
    Op op = Op::None;
    Expression& expr;
    UnaryExpr(Op op, Expression& expr): op(op), expr(expr) {};
    UnaryExpr(Expression& expr): expr(expr) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class FieldDeref: public Expression {
public:
    Expression& base;
    Identifier& field;
    FieldDeref(Expression& base, Identifier& field): base(base), field(field) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class IndexExpr: public Expression {
public:
    Expression& base;
    Expression& index;
    IndexExpr(Expression& base, Expression& index): base(base), index(index) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Call: public Expression {
public:
    Expression& target;
    vector<Expression*> args;
    Call(Expression& target, vector<Expression*> args):
        target(target), args(args) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Record: public Expression {
public:
    map<Identifier*, Expression*> record;
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class Identifier: public Expression {
public:
    string val;
    Identifier(string val): val(val) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class IntConst: public Constant{
public:
    int val;
    IntConst(int val): val(val) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class StrConst: public Constant {
public:
    string val;
    StrConst(string val): val(val) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class BoolConst: public Constant {
public:
    bool val;
    BoolConst(bool val): val(val) {};
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

class NoneConst: public Constant {
public:
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};
