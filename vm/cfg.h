# pragma once

#include <unordered_map>

#include "../parser/Visitor.h"

class AST_node;

class CFG {
    // TODO: how to represent a function
public: 
    // two kinds of blocks: one outgoing epsilon edge,
    // two edges (one true, one false) 
    bool hasEpsOutput;
    CFG& epsOut;
    CFG& trueOut;
    CFG& falseOut;
    // content of this block
    AST_node& bb;
    // constants 
    std::vector<Constant*> c;
};

class CFGBuilder : public Visitor {
private: 
    // stores a mapping of function names defined in the program
    // to their cfg representation 
    std::unordered_map<std::string, CFG> functions; 
    // cfg for the main program
    CFG main;
    // temp storage for return vals
    CFG ret;
public:
    void visit(Block& exp) override {}
    void visit(Global& exp) override {}
    void visit(Assignment& exp) override {}
    void visit(CallStatement& exp) override {}
    void visit(IfStatement& exp) override {}
    void visit(WhileLoop& exp) override {}
    void visit(Return& exp) override {}
    void visit(FunctionExpr& exp) override {}
    void visit(BinaryExpr& exp) override {}
    void visit(UnaryExpr& exp) override {}
    void visit(FieldDeref& exp) override {}
    void visit(IndexExpr& exp) override {}
    void visit(Call& exp) override {}
    void visit(Record& exp) override {}
    void visit(Identifier& exp) override {}
    void visit(IntConst& exp) override {}
    void visit(StrConst& exp) override {}
    void visit(BoolConst& exp) override {}
    void visit(NoneConst& exp) override {}
};
