/*
 * symboltable.h
 *
 * Defines a visitor that takes charge of figuring out scoping descriptors
 */
#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <iterator>

#include "../parser/Visitor.h" 
#include "../parser/AST.h"

// have the eval return a list of symbol tables that correspond to all functions
struct SymbolTable;
struct VarDesc;

typedef std::set<std::string> nameset_t;
typedef std::shared_ptr<SymbolTable> stptr_t;
typedef std::vector<std::shared_ptr<SymbolTable>> stvec_t;
typedef std::shared_ptr<VarDesc> desc_t;

enum VarType {
    GLOBAL,
    LOCAL, 
    FREE
};

struct VarDesc {
    VarType type;
    bool isReferenced;
    int32_t index;
    int32_t refIndex;
    VarDesc(): type(LOCAL), isReferenced(false) {};
    VarDesc(VarType type): type(type), isReferenced(false) {};
};

struct SymbolTable {
public:
    std::map<std::string, desc_t> vars;
    stptr_t parent;
    nameset_t referenced;
};

class SymbolTableBuilder : public Visitor {
private: 
    // per-function vars
    nameset_t global;
    nameset_t local;
    nameset_t referenced;

    // persistent vars
    nameset_t sneakyGlobals; // globals not declared in global frame
    stvec_t tables;
    stptr_t curTable;
public:
    stvec_t eval(Expression& exp);
    desc_t markLocalRef(std::string varName, stptr_t child);
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
