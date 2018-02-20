#pragma once

#include "AST.h"
#include <string>
#include <iostream>

using namespace std;

class PrettyPrinter : public Visitor {
    void visit(Block& exp) override {
    };
    void visit(Global& exp) override {
    };
    void visit(Assignment& exp) override {
    };
    void visit(CallStatement& exp) override {
    };
    void visit(IfStatement& exp) override {
    };
    void visit(WhileLoop& exp) override {
    };
    void visit(Return& exp) override {
    };
    void visit(Function& exp) override {
    };
    void visit(BinaryExpr& exp) override {
    };
    void visit(UnaryExpr& exp) override {
    };
    void visit(FieldDeref& exp) override {
    };
    void visit(IndexExpr& exp) override {
    };
    void visit(Call& exp) override {
    };
    void visit(Record& exp) override {
    };
    void visit(Identifier& exp) override {
    };
    void visit(IntConst& exp) override {
    };
    void visit(StrConst& exp) override {
    };
    void visit(BoolConst& exp) override {
    };
    void visit(NoneConst& exp) override {
    };
};
