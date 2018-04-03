#include "cfg.h"

BB::BB(bool epsOutput, InstructionList instr) {
    hasEpsOutput = epsOutput;
    instructions = instr;
}

CFG::CFG() {
    parameter_count = 0;
}

CFGBuilder::CFGBuilder() {
    curFunc = std::make_shared<CFG>(CFG());
}

InstructionList CFGBuilder::getInstructions(AST_node& expr) {
    expr.accept(*this);
    return retInstr;
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
    retInstr = instr;
}

InstructionList CFGBuilder::getWriteInstr(Expression* lhs) {
    InstructionList iList;
    auto id = dynamic_cast<Identifier*>(lhs);
    if (id != NULL) {
        // TODO: use a symbol table to figure out how to assign vars.
        return iList;
    }
    auto fieldD = dynamic_cast<FieldDeref*>(lhs);
    if (fieldD != NULL) {
        // TODO: handle record writes
        return iList;
    }
    auto indexE = dynamic_cast<IndexExpr*>(lhs);
    if (indexE != NULL) {
        // TODO: handle record writes
        return iList;
    }
}

void CFGBuilder::visit(Block& exp) {
    std::cout << "visiting a block" << std::endl;
    // make a cfg for each statement in the list. 
    // every statement returns a cfg with a single entrance and exit, 
    // so we can just chain them together. 
    // TODO: we can actually squish them down into a single basic block. 
    bbptr_t entrance = nullptr;
    bbptr_t exit = nullptr;
    for (Statement* s : exp.stmts) {
        s->accept(*this);
        if (!entrance) {
            // establish the entrance 
            entrance = retEnter;
        } else {
            // Add the new block to the end of the chain 
            exit->epsOut = retEnter;
        }
        exit = retExit;
    }
    retEnter = entrance;
    retExit = exit;
    // set the just created block as the entrypoint of the cur func 
    curFunc->codeEntry = entrance;
}

void CFGBuilder::visit(Assignment& exp) {
    // eval rhs 
    InstructionList iList = getInstructions(exp.expr);
    InstructionList write = getWriteInstr(&(exp.lhs));
    // concat the two lists 
    iList.insert(iList.end(), write.begin(), write.end());

    // because this is a statement it should return a BB
    bbptr_t ret = std::make_shared<BB>(BB(true, iList));
    retEnter = ret;
    retExit = ret;
}

void CFGBuilder::visit(BinaryExpr& exp) {
    InstructionList iList = getInstructions(exp.left);
    InstructionList evalR = getInstructions(exp.right);
    // concatenate two vecs
    iList.insert(iList.end(), evalR.begin(), evalR.end());
    Operation op; 
    optint_t noArg0;
    Instruction swapOp = Instruction(Operation::Swap, noArg0);
    // choose the correct instruction
    switch (exp.op) {
        case Or: 
            op = Operation::Or;
            break;
        case And: 
            op = Operation::And;
            break;
        case Lt: 
            // no lt instr provided, so first switch op order.
            iList.push_back(swapOp);
            op = Operation::Gt;
            break;
        case Gt: 
            op = Operation::Gt;
            break;
        case Lt_eq: 
            // same logic as for lt
            iList.push_back(swapOp);
            op = Operation::Geq;
            break;
        case Gt_eq: 
            op = Operation::Geq;
            break;
        case Eq_eq: 
            op = Operation::Eq;
            break;
        case Plus: 
            op = Operation::Add;
            break;
        case Minus:
            op = Operation::Sub;
            break;
        case Times: 
            op = Operation::Mul;
            break;
        case Divide:
            op = Operation::Div;
            break;
    }
    Instruction instr = Instruction(op, noArg0);
    iList.push_back(instr);
    retInstr = iList;
}


void CFGBuilder::visit(UnaryExpr& exp) {
    InstructionList iList = getInstructions(exp);
    Operation op;
    optint_t noArg0;
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
    iList.push_back(instr);
    retInstr = iList;
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