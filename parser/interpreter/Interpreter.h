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

    void processAssign(Expression* lhs, Value* rhs) {
        auto id = dynamic_cast<Identifier*>(lhs);
        if (id != NULL) {
            currentFrame->assign(id->name, rhs);
            return;
        }
        auto fieldD = dynamic_cast<FieldDeref*>(lhs);
        if (fieldD != NULL) {
            Value* exp = eval(&fieldD->base);
            auto recVal = exp->cast<RecordValue>();
            recVal->setItem(fieldD->field.name, rhs);
        }
        auto indexE = dynamic_cast<IndexExpr*>(lhs);
        if (indexE != NULL) {
            Value* exp = eval(&indexE->base);
            string field = eval(&indexE->index)->toString();
            auto recVal = exp->cast<RecordValue>();
            recVal->setItem(field, rhs);
        }
    }

    void processFuncVars(Frame& frame, Block* block) {
        for (auto &stmt : block->stmts) {
            auto globStmt = dynamic_cast<Global*>(stmt);
            // add global
            if (globStmt != NULL) {
                frame.setGlobal(globStmt->name.name);
            }
            // add locals to None
            auto asmtStmt = dynamic_cast<Assignment*>(stmt);
            if (asmtStmt != NULL) {
                processAssign(&asmtStmt->lhs, &NONE);
            }
            // recurse into other blocks
            auto ifStmt = dynamic_cast<IfStatement*>(stmt);
            if (ifStmt != NULL) {
                processFuncVars(frame, &ifStmt->thenBlock);
                if (ifStmt->elseBlock != NULL) {
                    processFuncVars(frame, ifStmt->elseBlock);
                }
                break;
            }
            auto whileStmt = dynamic_cast<WhileLoop*>(stmt);
            if (whileStmt != NULL) {
                processFuncVars(frame, &whileStmt->body);
                break;
            }
            auto blockStmt = dynamic_cast<Block*>(stmt);
            if (blockStmt != NULL) {
                processFuncVars(frame, blockStmt);
                break;
            }

        }
    }


    void visit(Block& exp) override {
        // TODO
        for (auto &stmt : exp.stmts) {
            stmt->accept(*this);
        }
    };

    void visit(Global& exp) override {
        // globals are handled in function calls
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
        exp.thenBlock.accept(*this);
        if (exp.elseBlock) {
            exp.elseBlock->accept(*this);
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
        rval = new FuncValue(currentFrame, exp.args, exp.body);
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
                // first, check if either operand is a StrValue
                auto sLeft = dynamic_cast<StrValue*>(left);
                auto sRight = dynamic_cast<StrValue*>(right);
                // if so, automatically cast and concatenate
                if (sLeft != NULL) {
                    rval = new StrValue(sLeft->val + right->toString());
                    break;
                } else if (sRight != NULL) {
                    rval = new StrValue(left->toString() + sRight->val);
                    break;
                }
                // otherwise, make sure both arguments are IntValues and add
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
        // first, check to make sure base exp is a FuncValue
        auto func = eval(&exp.target)->cast<FuncValue>();
        // next, eval args left to right and make sure args length is correct
        vector<Value*>* args = new vector<Value*>();
        for (auto a = func->args.begin(), end = func->args.end(); a != end; a++) {
            args->push_back(eval(*a));
        }
        if (args->size() != func->args.size()) {
            throw RuntimeException();
        }
        // next, allocate a new stack frame and add globals and locals to it
        currentFrame = new Frame(func->frame, rootFrame);
        processFuncVars(*currentFrame, &func->body);
        // set all params to the right values
        for (int i = 0; i < args->size(); i++) {
            currentFrame->setLocal(func->args.at(i)->name, args->at(i));
        }
        // eval function body
        rval = eval(&func->body);
    };

    void visit(Record& exp) override {
        RecordValue* val = new RecordValue();
        for (auto &r : exp.record) {
            val->setItem(r.first->name, eval(r.second));
        }
        rval = val;
    };

    void visit(Identifier& exp) override {
        rval = currentFrame->lookup_read(exp.name);
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
