#include "cfg.h"

BB::BB(bool epsOutput, InstructionList instr) {
    hasEpsOutput = epsOutput;
    instructions = instr;
}

void CFGBuilder::appendInstr(Instruction instr) {
    retExit->instructions.push_back(instr);
}

int CFGBuilder::allocConstant(constptr_t c) {
    curFunc->constants_.push_back(c);
    int i = curFunc->constants_.size() -1;
    return i;
}

void CFGBuilder::loadConstant(constptr_t c) {
    int constIdx = allocConstant(c);
    // make the LoadConst instruction
    optint_t op0 = optint_t(constIdx);
    Instruction loadconst = Instruction(Operation::LoadConst, op0);
    InstructionList instr;
    instr.push_back(loadconst);
    bbptr_t b = std::make_shared<BB>(BB(true, instr));
    retEnter = b;
    retExit = b;
}

void CFGBuilder::write(Expression* lhs, Value* rhs) {
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

//void CFGBuilder::visit(BinaryExpr& exp) {
//    InstructionList instr;
//    Operation op; 
//    optint_t noArg0;
//    // eval the operands in the correct order
//    exp.left.accept(*this);
//    exp.right.accept(*this);
//    // choose the correct instruction
//    switch (exp.op) {
//        case Or: 
//            op = Operation::Or;
//            break;
//        case And: 
//            op = Operation::And;
//            break;
//        case Lt: 
//            break;
//        case Gt: 
//            op = Operation::Gt;
//            break;
//        case Lt_eq: 
//            break;
//        case Gt_eq: 
//            op = Operation::Geq;
//            break;
//        case Eq_eq: 
//            op = Operation::Eq;
//            break;
//        case Plus: 
//            op = Operation::Add;
//            break;
//        case Minus:
//            op = Operation::Sub;
//            break;
//        case Times: 
//            op = Operation::Mul;
//            break;
//        case Divide:
//            op = Operation::Div;
//            break;
//    }
//}
//
void CFGBuilder::visit(UnaryExpr& exp) {
    Operation op;
    optint_t noArg0;
    // eval the operand
    exp.expr.accept(*this);
    // choose the correct instruction
    switch (exp.op) {
        case Not: 
            op = Operation::Not;
            break;
        case Neg: 
            op = Operation::Neg;
            break;
    }
    Instruction instr = Instruction(op, noArg0);
    // add the new instruction to the same basic block
    appendInstr(instr);
}

void CFGBuilder::visit(Identifier& exp) {
    // TODO: use a symbol table to figure out how to load variables. 
}

void CFGBuilder::visit(IntConst& exp) {
    constptr_t i = std::make_shared<Integer>(exp.val);
    loadConstant(i);
}

void CFGBuilder::visit(StrConst& exp) {
    constptr_t s = std::make_shared<String>(exp.val);
    loadConstant(s);
}

void CFGBuilder::visit(BoolConst& exp) {
    constptr_t b = std::make_shared<Boolean>(Boolean(exp.val));
    loadConstant(b);
}

void CFGBuilder::visit(NoneConst& exp) {
    constptr_t n = std::make_shared<None>(None());
    loadConstant(n);
}
