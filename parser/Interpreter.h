#pragma once

#include "AST.h"
#include <string>
#include <iostream>

using namespace std;

class InterpreterException : public exception {
    public:
	    string message;
	    InterpreterException(const string& message): message(message) {};
};

class Interpreter : public Visitor {
    int tab_count = 0;
    map<Op, string> op_map = {
        { Op::Or, "|"},
        { Op::And, "&"},
        { Op::Not, "!"},
        { Op::Lt, "<"},
        { Op::Gt, ">"},
        { Op::Lt_eq, "<="},
        { Op::Gt_eq, ">-"},
        { Op::Eq_eq, "=="},
        { Op::Plus, "+"},
        { Op::Minus, "-"},
        { Op::Times, "*"},
        { Op::Divide, "/"},
    };
    string tabs() {
        return string(tab_count, '\t');
    }
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
        exp.then_block.accept(*this);
        if (exp.else_block) {
            cout << " else ";
            exp.else_block->accept(*this);
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
    void visit(Function& exp) override {
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
        cout << op_map[exp.op];
        exp.right.accept(*this);
        cout << ")";
    };
    void visit(UnaryExpr& exp) override {
        cout << op_map[exp.op];
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
        cout << exp.val;
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
