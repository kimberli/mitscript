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
#include <algorithm>
#include <iterator>
#include <cassert>

#include "../parser/Visitor.h" 
#include "../parser/AST.h"

// have the eval return a list of symbol tables that correspond to all functions
struct SymbolTable;

typedef std::set<std::string> nameset_t;
typedef std::shared_ptr<SymbolTable> stptr_t;
typedef std::vector<std::shared_ptr<SymbolTable>> stvec_t;

struct VarDesc {
    bool isGlobal;
    bool isLocalRef;
    VarDesc(): isGlobal(false), isLocalRef(false) {};
    VarDesc(bool isGlobal, bool isLocalRef): isGlobal(isGlobal), isLocalRef(isLocalRef) {};
};

struct SymbolTable {
public:
    std::map<std::string, VarDesc> vars;
    stptr_t parent;
    static VarDesc resolve(std::string varName, stptr_t table);
};

class SymbolTableBuilder : public Visitor {
private: 
    nameset_t global;
    nameset_t local;
    nameset_t referenced;
    stvec_t tables;
    stptr_t curTable;
public:
    stvec_t eval(Expression& exp);
    void markLocalRef(std::string varName, stptr_t child, bool isLocalScope);
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
