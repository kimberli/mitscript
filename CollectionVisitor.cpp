# pragma once

# include "../AST/AbstractVisitor.h" 
# include "../AST/MITScriptNode.h"

# include <unordered_set>
# include <iostream>

typedef std::unordered_set<std::string> varset_t;

class CollectionVisitor : public AbstractVisitor {
private: 
    varset_t set;

protected:
    void addElt(std::string &var) {
        set.insert(var);    
    };

public:
    varset_t getSet() {
        return set;
    };
    void visit(NoneConstantNode &node) override {};
    void visit(StringConstantNode &node) override {};
    void visit(IntegerConstantNode &node) override {};
    void visit(VariableNode &node) override {};
    void visit(BoolConstantNode &node) override {};
    void visit(RecordNode &node) override {};
    void visit(CallNode &node) override {};
    void visit(IndexExpressionNode &node) override {};
    void visit(FieldDereferenceNode &node) override {};
    void visit(UnaryExpressionNode &node) override {};
    void visit(BinaryExpressionNode &node) override {};
    void visit(FunctionDeclarationNode &node) override {};
    void visit(ReturnNode &node) override {};
    void visit(WhileLoopNode &node) override {
        // recurse on statement
        node.getBody()->accept(*this);
    };
    void visit(IfStatementNode &node) override {
        // recurse on if and else block
        node.getThenPart()->accept(*this);
        nodeptr_t elsepart = node.getElsePart();
        if (elsepart) {
            node.getElsePart()->accept(*this);
        }
    };
    void visit(ExpressionStatementNode &node) override {};
    void visit(AssignmentStatementNode &node) override {};
    void visit(GlobalStatementNode &node) override {};
    void visit(BlockNode &node) override {
        for (auto s : node.getStatements()) {
            s->accept(*this);
        }
    };
};

class AssignsVisitor : public CollectionVisitor {
public:
    static varset_t eval(MITScriptNode &node) {
        AssignsVisitor a = AssignsVisitor();
        node.accept(a);
        return a.getSet();
    }
    void visit(AssignmentStatementNode &node) override {
        node.getLHS()->accept(*this);
    }
    void visit(VariableNode &node) override {
        // add the var name
        std::string name = node.getName();
        addElt(name);
    }
};

class GlobalsVisitor : public CollectionVisitor {
public:
    static varset_t eval(MITScriptNode &node) {
        GlobalsVisitor g = GlobalsVisitor();
        node.accept(g);
        return g.getSet();
    }
    void visit(GlobalStatementNode &node) override {
        // add global name
        std::string name = node.getName();
        addElt(name);
    }
};
