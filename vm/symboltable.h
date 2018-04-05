#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>

#include "../parser/Visitor.h" 
#include "../parser/AST.h"

enum class Descriptor; 
class SymbolTable;

typedef std::shared_ptr<SymbolTable> stptr_t;
typedef std::vector<Descriptor> descvec_t;

enum class Descriptor {
    Global, 
    Local, 
    Reference
};

class SymbolTable {
public: 
    stptr_t parent; 
    map<std::string, descvec_t> vars;
};

class SymbolTableBuilder : public Visitor {
private: 
    stptr_t table;
public:
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
