/*
 * compiler.h
 *
 * Defines a visitor that visits AST nodes generated in MITScript parsing
 * and outputs a root Function
 */

# pragma once

#include "../parser/AST.h"
#include "../parser/Visitor.h"
#include "instructions.h"
#include "types.h"
#include "symboltable.h"

#include <memory>
#include <functional>
#include <cassert>

using namespace std;

typedef std::shared_ptr<Constant> constptr_t;
typedef std::shared_ptr<Function> funcptr_t;
typedef std::experimental::optional<int32_t> optint_t;

class BytecodeCompiler : public Visitor {
    /*
     * Visitor that converts the AST to a CFG
     */
private: 
    InstructionList retInstr;  // expressions update this

    void addInstructions(AST_node& expr);
    funcptr_t getFunction(AST_node& expr);

    // same as defined in lecture
    int allocConstant(constptr_t c);
    int allocName(std::string name);

    // helper that takes in a constant pointer, takes care of allocation
    // and creating the appropriate bb
    void loadConstant(constptr_t c);

    // takes care of writing assignments
    void addWriteInstructions(Expression* lhs);
    // writes variables (does not handle records)
    void addWriteVarInstructions(std::string varName);

    // helper to load builtins to a scope
    void loadBuiltIns();

    // Symbol Table state
    stvec_t symbolTables;
    stptr_t curTable;
    int stCounter = 0;

public:
    funcptr_t evaluate(Expression& exp); // entrypoint of the visitor
    funcptr_t retFunc;  // statements update this

    void visit(Block& exp) override;
    void visit(Global& exp) override;
    void visit(Assignment& exp) override;
    void visit(CallStatement& exp) override;
    void visit(IfStatement& exp) override;
    void visit(WhileLoop& exp) override;
    void visit(Return& exp) override;
    void visit(FunctionExpr& exp) override;
    void visit(BinaryExpr& exp) override;
    void visit(UnaryExpr& exp) override;
    void visit(FieldDeref& exp) override;
    void visit(IndexExpr& exp) override;
    void visit(Call& exp) override;
    void visit(RecordExpr& exp) override;
    void visit(Identifier& exp) override;
    void visit(IntConst& exp) override;
    void visit(StrConst& exp) override;
    void visit(BoolConst& exp) override;
    void visit(NoneConst& exp) override;
};
//    // Helper to return stored value
//    InstructionList evalInstructions(AST_node& expr) {
//        expr.accept(*this);
//        return instr;
//    }
//
//    // Helper to return a function
//    Function evalFunc(AST_node& expr) {
//        expr.accept(*this);
//        return f;
//    }
//
//    Function* getBytecode() {
//        Function* f;
//        return f;
//    }
//
//    void visit(Block& exp) {
//
//    }
//    void visit(Global& exp) {
//
//    }
//    void visit(Assignment& exp) {
//
//    }
//    void visit(CallStatement& exp) {
//
//    }
//    void visit(IfStatement& exp) {
//
//    }
//    void visit(WhileLoop& exp) {
//
//    }
//    void visit(Return& exp) {
//        // get the code to evalute the expression and leave it
//        // on the top of the stack
//        InstructionList iList = evalInstructions(exp);
//        // add a return instruciton
//        std::experimental::optional<int32_t> arg0;
//        Operation op = Operation::Call;
//        Instruction* i = new Instruction(op, arg0);
//        iList.push_back(*i);
//        instr = iList;
//    }
//    void visit(FunctionExpr& exp) {
//
//    }
//    void visit(BinaryExpr& exp) {
//
//    }
//    void visit(UnaryExpr& exp) {
//
//    }
//    void visit(FieldDeref& exp) {
//
//    }
//    void visit(IndexExpr& exp) {
//
//    }
//    void visit(Call& exp) {
//
//    }
//    void visit(RecordExpr& exp) {
//
//    }
//    void visit(Identifier& exp) {
//
//    }
//    void visit(IntConst& exp) {
//
//    }
//    void visit(StrConst& exp) {
//
//    }
//    void visit(BoolConst& exp) {
//
//    }
//    void visit(NoneConst& exp) {
//
//    }
//};
