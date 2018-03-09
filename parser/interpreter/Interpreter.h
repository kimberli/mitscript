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
    NativeFunc* nativePrint;
    NativeFunc* nativeInput;
    NativeFunc* nativeIntcast;
    bool returned;

    Value* eval(Expression* exp){
        exp->accept(*this);
        return rval;
    }

    void processAssign(Expression* lhs, Value* rhs) {
        auto id = dynamic_cast<Identifier*>(lhs);
        if (id != NULL) {
            LOG(1, "\tprocessAssign: cast to ID " + id->name);
            currentFrame->assign(id->name, rhs);
            LOG(1, "\tprocessAssign: done");
            return;
        }
        auto fieldD = dynamic_cast<FieldDeref*>(lhs);
        if (fieldD != NULL) {
            LOG(1, "\tprocessAssign: cast to FIELD_DEREF");
            Value* exp = eval(&fieldD->base);
            auto recVal = exp->cast<RecordValue>();
            recVal->setItem(fieldD->field.name, rhs);
            LOG(1, "\tprocessAssign: done");
        }
        auto indexE = dynamic_cast<IndexExpr*>(lhs);
        if (indexE != NULL) {
            LOG(1, "\tprocessAssign: cast to INDEX_EXPR");
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
                LOG(1, "\tprocessFuncVars: adding global " + globStmt->name.name);
                frame.setGlobal(globStmt->name.name);
            }
            // add locals to None
            auto asmtStmt = dynamic_cast<Assignment*>(stmt);
            if (asmtStmt != NULL) {
                LOG(1, "\tprocessFuncVars: init local");
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
        LOG(1, "\tprocessFuncVars: done");
    }

    void visit(Block& exp) override {
        LOG(2, "Visiting Block");
        for (auto &stmt : exp.stmts) {
            auto retExp = dynamic_cast<Return*>(stmt);
            if (retExp != NULL) {
                retExp->accept(*this);
                return;
            }
            stmt->accept(*this);
        }
    };

    void visit(Global& exp) override {
        // globals are handled in function calls
        LOG(2, "Visiting Global");
    };

    void visit(Assignment& exp) override {
        LOG(2, "Visiting Assignment");
        Value* rhs = eval(&exp.expr);
        processAssign(&exp.lhs, rhs);
    };

    void visit(CallStatement& exp) override {
        LOG(2, "Visiting CallStatement");
        eval(&exp.call);
    };

    void visit(IfStatement& exp) override {
        LOG(2, "Visiting IfStatement");
        Value* condition = eval(&exp.condition);
        auto condVal = condition->cast<BoolValue>();
        if (condVal->val) {
            LOG(1, "\tIfStatement: took if block");
            eval(&exp.thenBlock);
        } else if (exp.elseBlock != NULL) {
            LOG(1, "\tIfStatement: took else block");
            eval(exp.elseBlock);
        }
    };

    void visit(WhileLoop& exp) override {
        LOG(2, "Visiting WhileLoop");
        while (true) {
            Value* condition = eval(&exp.condition);
            auto condVal = condition->cast<BoolValue>();
            if (!condVal->val) {
                break;
            }
            eval(&exp.body);
        }
    };

    void visit(Return& exp) override {
        LOG(2, "Visiting Return");
        rval = eval(&exp.expr);
        returned = true;
    };

    void visit(Function& exp) override {
        LOG(2, "Visiting Function");
        rval = new FuncValue(*currentFrame, exp.args, exp.body);
    };

    void visit(BinaryExpr& exp) override {
        LOG(2, "Visiting BinaryExpr");
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
                    throw IllegalArithmeticException("cannot divide by zero");
                }
                rval = new IntValue(iLeft->val / iRight->val);
                break;
            }
            default:
                throw RuntimeException("shouldn't get here");
        }
    };

    void visit(UnaryExpr& exp) override {
        LOG(2, "Visiting UnaryExpr");
        Value* innerVal = eval(&exp.expr);
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
                throw RuntimeException("shouldn't get here");
        }
    };

    void visit(FieldDeref& exp) override {
        LOG(2, "Visiting FieldDeref");
        string key = exp.field.name;
        Value* base = eval(&exp.base);
        auto rBase = base->cast<RecordValue>();
        rval = rBase->getItem(key);
        if (rval == NULL) {
            rval = &NONE;
        }
    };

    void visit(IndexExpr& exp) override {
        LOG(2, "Visiting IndexExpr");
        string key = eval(&exp.index)->toString();
        Value* base = eval(&exp.base);
        auto rBase = base->cast<RecordValue>();
        rval = rBase->getItem(key);
        if (rval == NULL) {
            rval = &NONE;
        }
    };

    void visit(Call& exp) override {
        LOG(2, "Visiting Call");
        returned = false;
        Frame* oldFrame = currentFrame;
        // first, check to make sure base exp is a FuncValue
        LOG(1, "\tCall: check for func value");
        auto func = eval(&exp.target)->cast<FuncValue>();
        // next, eval args left to right and make sure args length is correct
        LOG(1, "\tCall: eval args and check length");
        vector<Value*>* args = new vector<Value*>();
        for (auto a = exp.args.begin(), end = exp.args.end(); a != end; a++) {
            args->push_back(eval(*a));
        }
        if (args->size() != func->args.size()) {
            throw RuntimeException("mismatched number of arguments");
        }
        // next, allocate a new stack frame and add globals and locals to it
        LOG(1, "\tCall: alloc new frame, load globals and locals");
        currentFrame = new Frame(&func->frame, rootFrame);
        processFuncVars(*currentFrame, &func->body);
        // set all params to the right values
        LOG(1, "\tCall: set params");
        for (int i = 0; i < args->size(); i++) {
            currentFrame->setLocal(func->args.at(i)->name, args->at(i));
        }
        // eval function body
        LOG(1, "\tCall: eval function body");
        auto nFunc = dynamic_cast<NativeFunc*>(func);
        if (nFunc != NULL) {
            rval = nFunc->evalNativeFunc(*currentFrame);
            LOG(1, "\tReturning " + rval->toString());
        } else {
            eval(&func->body);
            if (!returned) {
                rval = &NONE;
            }
            LOG(1, "\tReturning " + rval->toString());
        }
        returned = false;
        currentFrame = oldFrame;
    };

    void visit(Record& exp) override {
        LOG(2, "Visiting Record");
        RecordValue* val = new RecordValue();
        for (auto &r : exp.record) {
            val->setItem(r.first->name, eval(r.second));
        }
        rval = val;
    };

    void visit(Identifier& exp) override {
        LOG(2, "Visiting Identifier: " + exp.name);
        rval = currentFrame->lookup_read(exp.name);
        LOG(1, "\tIdentifier: got val " + rval->toString());
    };

    void visit(IntConst& exp) override {
        LOG(2, "Visiting IntConst");
        rval = new IntValue(exp.val);
    };

    void visit(StrConst& exp) override {
        LOG(2, "Visiting StrConst " + exp.val);
        rval = new StrValue(exp.val);
    };

    void visit(BoolConst& exp) override {
        LOG(2, "Visiting BoolConst");
        rval = new BoolValue(exp.val);
    };

    void visit(NoneConst& exp) override {
        LOG(2, "Visiting NoneConst");
        rval = &NONE;
    };

    public:
        Interpreter() {
            rootFrame = new Frame();
            currentFrame = rootFrame;
            rval = &NONE;
            returned = false;
            // create native functions
            Block* emptyBlock = new Block({});
            Identifier* s = new Identifier("s");
            vector<Identifier*> args0;
            vector<Identifier*> args1 = { s };
            nativePrint = new PrintNativeFunc(*rootFrame, args1, *emptyBlock);
            nativeInput = new InputNativeFunc(*rootFrame, args0, *emptyBlock);
            nativeIntcast = new IntcastNativeFunc(*rootFrame, args1, *emptyBlock);

            Identifier* print = new Identifier("print");
            Identifier* input = new Identifier("input");
            Identifier* intcast = new Identifier("intcast");

            processAssign(print, nativePrint);
            processAssign(input, nativeInput);
            processAssign(intcast, nativeIntcast);
        };
};
