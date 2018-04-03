#pragma once 

#include <unordered_map>
#include <iostream>

#include "../parser/Visitor.h"
#include "../parser/AST.h"
#include "instructions.h"
#include "types.h"

class AST_node;
class BB;
class CFG;

//typedef std::shared_ptr<Function> funcptr_t;
typedef std::shared_ptr<Constant> constptr_t;
typedef std::shared_ptr<CFG> cfgptr_t;
typedef std::shared_ptr<BB> bbptr_t;
typedef std::experimental::optional<int32_t> optint_t;

class BB {
public: 
    BB(bool epsOutput, InstructionList instr);
    // represents a single node in the CFG 
    // two kinds of blocks: one outgoing epsilon edge,
    // two edges (one true, one false)
    bool hasEpsOutput;
    bbptr_t epsOut;
    bbptr_t trueOut;
    bbptr_t falseOut;
    // content of this block; a list of statements
    InstructionList instructions;
};

class CFG {
public:
    // very similar data structure to the eventual bytecode function,
    // but we store a CFG representation instead of bytecode.
    CFG();

    // CFG rep of internal functions
    std::vector<cfgptr_t> functions_;

    // copied from the Function spec
    std::vector<constptr_t> constants_;
    uint32_t parameter_count;
    std::vector<std::string> local_vars_;
    std::vector<std::string> local_reference_vars_;
    std::vector<std::string> free_vars_;
    std::vector<std::string> names_;

    // This is the entry point of the corresponding graph
    bbptr_t codeEntry;
};

class CFGBuilder : public Visitor {
private: 
    // Invariant: visiting a statement or a block should return a CFG with
    // a single endpoint and a single exitpoint, to be deposited 
    // below 
    bbptr_t retEnter;
    bbptr_t retExit;

    // Visiting anything else should return a list of instructions. 
    InstructionList retInstr;
    InstructionList getInstructions(AST_node& expr);

    // same as defined in lecture
    int allocConstant(constptr_t c);

    // helper that takes in a constant pointer, takes care of allocation
    // and creating the appropriate bb. 
    void loadConstant(constptr_t c);

    // takes care of writing assignments
    InstructionList getWriteInstr(Expression* lhs);

public:
    CFGBuilder();
    // stores the CFG data structure corresponding to the 
    // function we are currently building
    cfgptr_t curFunc;

    void visit(Block& exp) override;
    
    void visit(Global& exp) override;

    void visit(Assignment& exp) override;

    // I guess just call a function?
    void visit(CallStatement& exp) override {}

    void visit(IfStatement& exp) override;

    void visit(WhileLoop& exp) override;

    void visit(Return& exp) override;

    void visit(FunctionExpr& exp) override {}

    void visit(BinaryExpr& exp) override;
    void visit(UnaryExpr& exp) override;
    void visit(FieldDeref& exp) override {}
    void visit(IndexExpr& exp) override {}
    void visit(Call& exp) override {}
    void visit(RecordExpr& exp) override {}
    void visit(Identifier& exp) override;
    void visit(IntConst& exp) override;
    void visit(StrConst& exp) override;
    void visit(BoolConst& exp) override;
    void visit(NoneConst& exp) override;
};
