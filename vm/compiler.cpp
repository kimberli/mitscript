/*
 * compiler.cpp
 *
 * Implements a visitor that visits AST nodes generated in MITScript parsing
 * and outputs a root Function
 */
#include "compiler.h"

# include <memory>
# include <functional>

funcptr_t BytecodeCompiler::getFunction(AST_node& expr) {
    expr.accept(*this);
    return retFunc;
}

InstructionList BytecodeCompiler::getInstructions(AST_node& expr) {
    expr.accept(*this);
    return retInstr;
}

void BytecodeCompiler::loadConstant(constptr_t c) {
    // add the constant to the constants list
    retFunc->constants_.push_back(c);
    int constIdx = retFunc->constants_.size() - 1;
    // make the LoadConst instruction
    optint_t op0 = optint_t(constIdx);
    Instruction* instr = new Instruction(Operation::LoadConst, op0);
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::getWriteInstr(Expression* lhs) {
    auto id = dynamic_cast<Identifier*>(lhs);
    if (id != NULL) {
        // TODO: use a symbol table to figure out how to assign vars.
        return;
    }
    auto fieldD = dynamic_cast<FieldDeref*>(lhs);
    if (fieldD != NULL) {
        // TODO: handle record writes
        return;
    }
    auto indexE = dynamic_cast<IndexExpr*>(lhs);
    if (indexE != NULL) {
        // TODO: handle record writes
        return;
    }
}

void BytecodeCompiler::visit(Block& exp) {
    for (Statement* s : exp.stmts) {
        s->accept(*this);
        // add instruction to retFunc
    }
}
void BytecodeCompiler::visit(Global& exp) {
    // no-op
}
void BytecodeCompiler::visit(Assignment& exp) {
}
void BytecodeCompiler::visit(CallStatement& exp) {
}
void BytecodeCompiler::visit(IfStatement& exp) {
}
void BytecodeCompiler::visit(WhileLoop& exp) {
}
void BytecodeCompiler::visit(Return& exp) {
}
void BytecodeCompiler::visit(FunctionExpr& exp) {
}
void BytecodeCompiler::visit(BinaryExpr& exp) {
}
void BytecodeCompiler::visit(UnaryExpr& exp) {
}
void BytecodeCompiler::visit(FieldDeref& exp) {
}
void BytecodeCompiler::visit(IndexExpr& exp) {
}
void BytecodeCompiler::visit(Call& exp) {
}
void BytecodeCompiler::visit(RecordExpr& exp) {
}
void BytecodeCompiler::visit(Identifier& exp) {
}
void BytecodeCompiler::visit(IntConst& exp) {
}
void BytecodeCompiler::visit(StrConst& exp) {
}
void BytecodeCompiler::visit(BoolConst& exp) {
}
void BytecodeCompiler::visit(NoneConst& exp) {
}
