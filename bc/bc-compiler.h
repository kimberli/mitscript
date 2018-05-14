/*
 * compiler.h
 *
 * Defines a visitor that visits AST nodes generated in MITScript parsing
 * and outputs a root Function
 */

# pragma once

#include "../AST.h"
#include "../Visitor.h"
#include "../instructions.h"
#include "../types.h"
#include "symboltable.h"

#include <memory>
#include <functional>
#include <cassert>

using namespace std;

class BytecodeCompiler : public Visitor {
    /*
     * Visitor that converts the AST to a CFG
     */
private: 
    BcInstructionList retInstr;  // expressions update this
    int labelCounter = 0;

    void addInstructions(AST_node& expr);
    Function* getFunction(AST_node& expr);

    // same as defined in lecture
    int allocConstant(tagptr_t ptr);
    int allocName(string name);

    // helper that takes in a constant pointer, takes care of allocation
    // and creating the appropriate bb
    void loadConstant(tagptr_t ptr);

    // helper to add a single bytecode instruction
    void addInstruction(BcOp op, optint_t op0);
    // takes care of writing assignments
    void addWriteInstructions(Expression* lhs);
    // writes variables (does not handle records)
    void addWriteVarInstructions(string varName);

    // helper to load builtins to a scope
    void loadBuiltIns();

    // helper to move funcs from st to function
    bool putVarInFunc(string& varName, stptr_t table, Function* func);

    // Symbol Table state
    stvec_t symbolTables;
    stptr_t curTable;
    int stCounter = 0;

public:
    BytecodeCompiler() {};
    Function* evaluate(Expression& exp); // entrypoint of the visitor
    Function* retFunc;  // statements update this

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
