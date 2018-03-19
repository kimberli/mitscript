#pragma once

#include "AST.h"
#include <string>
#include <iostream>

using namespace std;

class PrettyPrinter : public Visitor {
    int tab_count = 0;
    map<BinOp, string> bin_op_map = {
        { BinOp::Or, "|"},
        { BinOp::And, "&"},
        { BinOp::Lt, "<"},
        { BinOp::Gt, ">"},
        { BinOp::Lt_eq, "<="},
        { BinOp::Gt_eq, ">-"},
        { BinOp::Eq_eq, "=="},
        { BinOp::Plus, "+"},
        { BinOp::Minus, "-"},
        { BinOp::Times, "*"},
        { BinOp::Divide, "/"},
    };
    map<UnOp, string> un_op_map = {
        { UnOp::Not, "!"},
        { UnOp::Neg, "-"},
    };
    string tabs() {
        return string(tab_count, '\t');
    };
    void visit(Block& exp) override {
        cout << "{\n";
        tab_count++;
        for (auto &stmt : exp.stmts) {
            stmt->accept(*this);
        }
        tab_count--;
        cout << tabs() << "}";
    };
    void visit(Global& exp) override {
        cout << tabs() << "global ";
        exp.name.accept(*this);
        cout << ";\n";
    };
    void visit(Assignment& exp) override {
        cout << tabs();
        exp.lhs.accept(*this);
        cout << " = ";
        exp.expr.accept(*this);
        cout << ";\n";
    };
    void visit(CallStatement& exp) override {
        cout << tabs();
        exp.call.accept(*this);
        cout << ";\n";
    };
    void visit(IfStatement& exp) override {
        cout << tabs() << "if (";
        exp.condition.accept(*this);
        cout << ") ";
        exp.thenBlock.accept(*this);
        if (exp.elseBlock) {
            cout << " else ";
            exp.elseBlock->accept(*this);
        }
        cout << "\n";
    };
    void visit(WhileLoop& exp) override {
        cout << tabs() << "while (";
        exp.condition.accept(*this);
        cout << ") ";
        exp.body.accept(*this);
        cout << "\n";
    };
    void visit(Return& exp) override {
        cout << tabs() << "return ";
        exp.expr.accept(*this);
        cout << ";\n";
    };
    void visit(FunctionExpr& exp) override {
        cout << "fun(";
        for (auto it = exp.args.begin(), end = exp.args.end(); it != end; it++) {
            if (it != exp.args.begin()) cout << ", ";
            (*it)->accept(*this);
        }
        cout << ") ";
        exp.body.accept(*this);
    };
    void visit(BinaryExpr& exp) override {
        cout << "(";
        exp.left.accept(*this);
        cout << bin_op_map[exp.op];
        exp.right.accept(*this);
        cout << ")";
    };
    void visit(UnaryExpr& exp) override {
        cout << un_op_map[exp.op];
        cout << "(";
        exp.expr.accept(*this);
        cout << ")";
    };
    void visit(FieldDeref& exp) override {
        exp.base.accept(*this);
        cout << ".";
        exp.field.accept(*this);
    };
    void visit(IndexExpr& exp) override {
        exp.base.accept(*this);
        cout << "[";
        exp.index.accept(*this);
        cout << "]";
    };
    void visit(Call& exp) override {
        exp.target.accept(*this);
        cout << "(";
        for (auto it = exp.args.begin(), end = exp.args.end(); it != end; it++) {
            if (it != exp.args.begin()) cout << ", ";
            (*it)->accept(*this);
        }
        cout << ")";
    };
    void visit(Record& exp) override {
        cout << "{\n";
        tab_count++;
        for (auto &r : exp.record) {
            cout << tabs();
            r.first->accept(*this);
            cout << ":";
            r.second->accept(*this);
            cout << ";\n";
        }
        tab_count--;
        cout << tabs() << "}";
    };
    void visit(Identifier& exp) override {
        cout << exp.name;
    };
    void visit(IntConst& exp) override {
        cout << exp.val;
    };
    void visit(StrConst& exp) override {
        cout << "\"" << exp.val << "\"";
    };
    void visit(BoolConst& exp) override {
        if (exp.val) {
            cout << "true";
        } else {
            cout << "false";
        }
    };
    void visit(NoneConst& exp) override {
        cout << "None";
    };
};
