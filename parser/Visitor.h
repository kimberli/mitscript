#pragma once

#include <memory>

using namespace std;

class Block;
class Global;
class Assignment;
class CallStatement;
class IfStatement;
class WhileLoop;
class Return;
class FunctionExpr;
class BinaryExpr;
class UnaryExpr;
class FieldDeref;
class IndexExpr;
class Call;
class Record;
class Identifier;
class IntConst;
class StrConst;
class BoolConst;
class NoneConst;

class Visitor {
public:
    virtual void visit(Block& exp) = 0;
    virtual void visit(Global& exp) = 0;
    virtual void visit(Assignment& exp) = 0;
    virtual void visit(CallStatement& exp) = 0;
    virtual void visit(IfStatement& exp) = 0;
    virtual void visit(WhileLoop& exp) = 0;
    virtual void visit(Return& exp) = 0;
    virtual void visit(FunctionExpr& exp) = 0;
    virtual void visit(BinaryExpr& exp) = 0;
    virtual void visit(UnaryExpr& exp) = 0;
    virtual void visit(FieldDeref& exp) = 0;
    virtual void visit(IndexExpr& exp) = 0;
    virtual void visit(Call& exp) = 0;
    virtual void visit(Record& exp) = 0;
    virtual void visit(Identifier& exp) = 0;
    virtual void visit(IntConst& exp) = 0;
    virtual void visit(StrConst& exp) = 0;
    virtual void visit(BoolConst& exp) = 0;
    virtual void visit(NoneConst& exp) = 0;
};


