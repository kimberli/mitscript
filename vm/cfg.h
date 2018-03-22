
#include <unordered_map>

#include "../parser/Visitor.h"
#include "instructions.h"
#include "types.h"

class AST_node;
class BB;
class CFG;

//typedef std::shared_ptr<Function> funcptr_t;
typedef std::shared_ptr<Constant> constptr_t;
typedef std::shared_ptr<CFG> cfgptr_t;

class BB {
    // represents a single node in the CFG 
public: 
    // two kinds of blocks: one outgoing epsilon edge,
    // two edges (one true, one false) 
    bool hasEpsOutput;
    BB& epsOut;
    BB& trueOut;
    BB& falseOut;
    // content of this block; a list of statements
    InstructionList instructions;
};

class CFG {
    // very similar data structure to the eventual bytecode function,
    // but we store a CFG representation instead of bytecode.

    // CFG rep of internal functions
    std::vector<cfgptr_t> functions_;

    // copied from the Function spec
    std::vector<constptr_t> constants_;
    uint32_t parameter_count;
    std::vector<std::string> local_vars_;
    std::vector<std::string> local_reference_vars;
    std::vector<std::string> free_vars_;
    std::vector<std::string> names_;

    // This is the entry point of the corresponding graph
    BB code_entry;
};

class CFGBuilder : public Visitor {
private: 
    // stores a mapping of function names defined in the program
    // to their cfg representation 
    //std::unordered_map<std::string, CFG> functions; 
    // Visiting each node adds to the CFG 
    // should return the single entrance point to the CFG 
    // and the single exit point
    BB retEnter;
    BB retExit;
public:
    // make a cfg for each statement 
    // chain together exits and entrances in seq
    // return the first enter and the last exit
    void visit(Block& exp) override {}
    
    // ???
    void visit(Global& exp) override {}

    // ???
    void visit(Assignment& exp) override {}

    // I guess just call a function?
    void visit(CallStatement& exp) override {}

    // generate a t-f bb for the test expression 
    // generate an eps bb for the if statement
    // generate an eps bb for the else statement
    // generate an epty bb for the endpoint
    // connect the test t transition to the if entrance
    // connect if entrance to the endpoint
    // connect the test f transition to the else entrance
    // connect the else exit to the endpoint
    // return entrance to the test expression, endpoint exit
    void visit(IfStatement& exp) override {}

    // generate an empty bb for a starting block 
    // connect starting block exit to expression bblock
    // generate a t-f bb for the test expression
    // generate an eps bb for the statement
    // generate an empty bb for the endpoint
    // connect the test t transition to the statement entrance
    // connect the statement exit to the test expression entrance
    // connect the test f transition to the endpoint
    // return starting block entrance and endpoint exit
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
