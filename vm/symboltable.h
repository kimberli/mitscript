#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>

#include "../parser/Visitor.h" 
#include "../parser/AST.h"

typedef std::set<std::string> nameset_t;

struct SymbolTable {
public: 
    nameset_t global;
    nameset_t local;
    nameset_t free;
    SymbolTable(nameset_t global, nameset_t local, nameset_t free): global(global), local(local), free(free) {};
};

class SymbolTableBuilder : public Visitor {
private: 
    nameset_t global;
    nameset_t local;
    nameset_t referenced;
public:
    SymbolTable eval(Expression& exp);
    virtual void visit(Block& exp) override;
    virtual void visit(Global& exp) override;
    virtual void visit(Assignment& exp) override;
    virtual void visit(CallStatement& exp) override;
    virtual void visit(IfStatement& exp) override;
    virtual void visit(WhileLoop& exp) override;
    virtual void visit(Return& exp) override;
    virtual void visit(FunctionExpr& exp) override;
    virtual void visit(BinaryExpr& exp) override;
    virtual void visit(UnaryExpr& exp) override;
    virtual void visit(FieldDeref& exp) override;
    virtual void visit(IndexExpr& exp) override;
    virtual void visit(Call& exp) override;
    virtual void visit(RecordExpr& exp) override;
    virtual void visit(Identifier& exp) override;
    virtual void visit(IntConst& exp) override;
    virtual void visit(StrConst& exp) override;
    virtual void visit(BoolConst& exp) override;
    virtual void visit(NoneConst& exp) override;
};
