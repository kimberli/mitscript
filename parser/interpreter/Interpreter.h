#pragma once

#include "../AST.h"
#include "State.h"
#include <string>
#include <stack>
#include <iostream>

using namespace std;

class Interpreter : public Visitor {
    Frame* rootFrame;
    Frame* currentFrame;
    Value* rval;
    Value& NONE = *new NoneValue();

    Value* eval(Expression* exp){
        exp->accept(*this);
        return rval;
    }

    void visit(Block& exp) override {
        for (auto &stmt : exp.stmts) {
            stmt->accept(*this);
        }
    };

    void visit(Global& exp) override {
        exp.name.accept(*this);
    };

    void visit(Assignment& exp) override {
        exp.lhs.accept(*this);
        exp.expr.accept(*this);
    };

    void visit(CallStatement& exp) override {
        exp.call.accept(*this);
    };

    void visit(IfStatement& exp) override {
        exp.condition.accept(*this);
        exp.then_block.accept(*this);
        if (exp.else_block) {
            exp.else_block->accept(*this);
        }
    };

    void visit(WhileLoop& exp) override {
        exp.condition.accept(*this);
        exp.body.accept(*this);
    };
    void visit(Return& exp) override {
        exp.expr.accept(*this);
    };

    void visit(Function& exp) override {
        for (auto it = exp.args.begin(), end = exp.args.end(); it != end; it++) {
            (*it)->accept(*this);
        }
        exp.body.accept(*this);
    };

    void visit(BinaryExpr& exp) override {
        exp.left.accept(*this);
        exp.right.accept(*this);
    };

    void visit(UnaryExpr& exp) override {
        exp.expr.accept(*this);
    };

    void visit(FieldDeref& exp) override {
        exp.base.accept(*this);
        exp.field.accept(*this);
    };

    void visit(IndexExpr& exp) override {
        exp.base.accept(*this);
        exp.index.accept(*this);
    };

    void visit(Call& exp) override {
        exp.target.accept(*this);
        for (auto it = exp.args.begin(), end = exp.args.end(); it != end; it++) {
            (*it)->accept(*this);
        }
    };

    void visit(Record& exp) override {
        RecordValue* val = new RecordValue();
        for (auto &r : exp.record) {
            val->setItem(r.first->name, eval(r.second));
        }
        rval = val;
    };

    void visit(Identifier& exp) override {
        rval = currentFrame->lookup(exp.name);
    };

    void visit(IntConst& exp) override {
        rval = new IntValue(exp.val);
    };

    void visit(StrConst& exp) override {
        rval = new StrValue(exp.val);
    };

    void visit(BoolConst& exp) override {
        rval = new BoolValue(exp.val);
    };

    void visit(NoneConst& exp) override {
        rval = &NONE;
    };

    public:
        Interpreter() : currentFrame(rootFrame), rval(&NONE) {};
};
