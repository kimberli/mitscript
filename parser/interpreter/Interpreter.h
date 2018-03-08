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
        // TODO
        for (auto &stmt : exp.stmts) {
            stmt->accept(*this);
        }
    };

    void visit(Global& exp) override {
        // TODO
        exp.name.accept(*this);
    };

    void visit(Assignment& exp) override {
        // TODO
        exp.lhs.accept(*this);
        exp.expr.accept(*this);
    };

    void visit(CallStatement& exp) override {
        // TODO
        exp.call.accept(*this);
    };

    void visit(IfStatement& exp) override {
        // TODO
        exp.condition.accept(*this);
        exp.then_block.accept(*this);
        if (exp.else_block) {
            exp.else_block->accept(*this);
        }
    };

    void visit(WhileLoop& exp) override {
        // TODO
        exp.condition.accept(*this);
        exp.body.accept(*this);
    };

    void visit(Return& exp) override {
        // TODO
        exp.expr.accept(*this);
    };

    void visit(Function& exp) override {
        // TODO
        for (auto it = exp.args.begin(), end = exp.args.end(); it != end; it++) {
            (*it)->accept(*this);
        }
        exp.body.accept(*this);
    };

    void visit(BinaryExpr& exp) override {
        Value* left = eval(&exp.left);
        Value* right = eval(&exp.right);
        switch(exp.op) {
            case BinOp::Or:
            {
                auto bLeft = left->cast<BoolValue>();
                auto bRight = right->cast<BoolValue>();
                rval = new BoolValue(bLeft->val || bRight->val);
                break;
            }
            case BinOp::And:
            {
                auto bLeft = left->cast<BoolValue>();
                auto bRight = right->cast<BoolValue>();
                rval = new BoolValue(bLeft->val & bRight->val);
                break;
            }
            case BinOp::Lt:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new BoolValue(iLeft->val < iRight->val);
                break;
            }
            case BinOp::Gt:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new BoolValue(iLeft->val > iRight->val);
                break;
            }
            case BinOp::Lt_eq:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new BoolValue(iLeft->val <= iRight->val);
                break;
            }
            case BinOp::Gt_eq:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new BoolValue(iLeft->val >= iRight->val);
                break;
            }
            case BinOp::Eq_eq:
            {
                rval = new BoolValue(left->equals(right));
                break;
            }
            case BinOp::Plus:
            {
                auto sLeft = dynamic_cast<StrValue*>(left);
                auto sRight = dynamic_cast<StrValue*>(right);
                if (sLeft != NULL) {
                    rval = new StrValue(sLeft->val + right->toString());
                    break;
                } else if (sRight != NULL) {
                    rval = new StrValue(left->toString() + sRight->val);
                    break;
                }
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new IntValue(iLeft->val + iRight->val);
                break;
            }
            case BinOp::Minus:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new IntValue(iLeft->val - iRight->val);
                break;
            }
            case BinOp::Times:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                rval = new IntValue(iLeft->val * iRight->val);
                break;
            }
            case BinOp::Divide:
            {
                auto iLeft = left->cast<IntValue>();
                auto iRight = right->cast<IntValue>();
                if (iRight->val == 0) {
                    throw IllegalArithmeticException();
                }
                rval = new IntValue(iLeft->val / iRight->val);
                break;
            }
        }
    };

    void visit(UnaryExpr& exp) override {
        Value* innerVal = eval(&exp);
        switch(exp.op) {
            case UnOp::Not:
            {
                auto bval = innerVal->cast<BoolValue>();
                rval = new BoolValue(!bval->val);
                break;
            }
            case UnOp::Neg:
            {
                auto ival = innerVal->cast<IntValue>();
                rval = new IntValue(-ival->val);
                break;
            }
            default:
                throw RuntimeException(); // should never get here
        }
        exp.expr.accept(*this);
    };

    void visit(FieldDeref& exp) override {
        string key = exp.field.name;
        Value* base = eval(&exp.base);
        auto rBase = base->cast<RecordValue>();
        rval = rBase->getItem(key);
        if (rval == NULL) {
            rval = &NONE;
        }
    };

    void visit(IndexExpr& exp) override {
        string key = eval(&exp.index)->toString();
        Value* base = eval(&exp.base);
        auto rBase = base->cast<RecordValue>();
        rval = rBase->getItem(key);
        if (rval == NULL) {
            rval = &NONE;
        }
    };

    void visit(Call& exp) override {
        // TODO
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
