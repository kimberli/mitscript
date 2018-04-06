/*
 * cfg.cpp
 *
 * Implements the basic block (BB) data structure, as well as classes for the CFG
 * and CFGBuilder.
 */
#include "cfg.h"

BB::BB(bool epsOutput, InstructionList instr) {
    hasEpsOutput = epsOutput;
    instructions = instr;
}

CFG::CFG() {
    parameter_count = 0;
}

CFGBuilder::CFGBuilder() {
}

cfgptr_t CFGBuilder::evaluate(Expression& exp) {
    std::cout << "Running" << std::endl;
    // set current function 
    // TODO: LOAD BUILT-INS
    curFunc = std::make_shared<CFG>(CFG());
    curFunc->parameter_count = 0;

    // generate a symbol table 
    SymbolTableBuilder stb = SymbolTableBuilder();
    symbolTable = stb.eval(exp);
    curTable = symbolTable.at(0);

    // load up curFunc with vars from symbol table
    for (std::map<std::string, desc_t>::iterator it = curTable->vars.begin(); it != curTable->vars.end(); it ++) {
        std::string varName = it->first;
        desc_t d = it->second;
        switch (d->type) {
            case GLOBAL: 
                d->index = curFunc->names_.size();
                curFunc->names_.push_back(varName);
                break;
            case LOCAL: 
                d->index = curFunc->local_vars_.size();
                curFunc->local_vars_.push_back(varName);
                if (d->isReferenced) {
                    curFunc->local_reference_vars_.push_back(varName);
                }
                break;
            case FREE: 
                d->index = curFunc->free_vars_.size();
                curFunc->free_vars_.push_back(varName); 
        }
    }

    // run this visitor
    exp.accept(*this);

    // return the function. 
    return curFunc;
}

InstructionList CFGBuilder::getInstructions(AST_node& expr) {
    expr.accept(*this);
    return retInstr;
}

int CFGBuilder::allocConstant(constptr_t c) {
    int i = curFunc->constants_.size();
    curFunc->constants_.push_back(c);
    return i;
}

int CFGBuilder::allocName(std::string name) {
    int i = curFunc->names_.size();
    curFunc->names_.push_back(name);
    return i;
}

void CFGBuilder::loadConstant(constptr_t c) {
    int constIdx = allocConstant(c);
    // make the LoadConst instruction
    optint_t op0 = optint_t(constIdx);
    Instruction* instr = new Instruction(Operation::LoadConst, op0);
    InstructionList iList;
    iList.push_back(*instr);
    retInstr = iList;
}

InstructionList CFGBuilder::getLoadVarInstr(std::string varName) {
    desc_t d = curTable->vars.at(varName);
    InstructionList iList;
    switch (d->type) {
        case GLOBAL: {
            // use load_global 
            optint_t i = optint_t(d->index);
            Instruction* instr = new Instruction(Operation::LoadGlobal, i);
            iList.push_back(*instr);
            return iList;
        }
        case LOCAL: {
            optint_t i = optint_t(d->index);
            Instruction* instr = new Instruction(Operation::LoadLocal, i);
            iList.push_back(*instr);
            return iList;
        }
        case FREE: 
            // recall d.index is an index into the free vars, so we have to 
            // jump over local ref vars. 
            optint_t i = optint_t(d->index + curFunc->local_reference_vars_.size());
            optint_t noArg0;
            Instruction* pushRefInstr = new Instruction(Operation::PushReference, i);
            Instruction* loadRefInstr = new Instruction(Operation::LoadReference, noArg0);
            iList.push_back(*pushRefInstr);
            iList.push_back(*loadRefInstr);
            return iList;
    }
}

InstructionList CFGBuilder::getWriteInstr(Expression* lhs) {
    // the value is ALREADY ON THE STACK. 
    InstructionList iList;

    auto id = dynamic_cast<Identifier*>(lhs);
    if (id != NULL) {
        // use var load
        desc_t d = curTable->vars.at(id->name);
        InstructionList iList;
        switch (d->type) {
            case GLOBAL: {
                optint_t i = optint_t(d->index);
                Instruction* instr = new Instruction(Operation::StoreGlobal, i);
                iList.push_back(*instr);
                return iList;
            }
            case LOCAL: {
                optint_t i = optint_t(d->index);
                Instruction* instr = new Instruction(Operation::StoreLocal, i);
                iList.push_back(*instr);
                return iList;
            }
            case FREE: {
                // recall d.index is an index into the free vars, so we have to 
                // jump over local ref vars. 
                optint_t i = optint_t(d->index + curFunc->local_reference_vars_.size());
                optint_t noArg0;
                Instruction* pushRefInstr = new Instruction(Operation::PushReference, i);
                // the stack is now S :: value :: ref, but we weed 
                // S :: ref :: value, so add a swap instr. 
                Instruction* swapInstr = new Instruction(Operation::Swap, noArg0);
                Instruction* storeRefInstr = new Instruction(Operation::StoreReference, noArg0);
                iList.push_back(*pushRefInstr);
                iList.push_back(*swapInstr);
                iList.push_back(*storeRefInstr);
                return iList;
            }
        }
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

void CFGBuilder::visit(Global& exp) {
     // no-op
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

void CFGBuilder::visit(IfStatement& exp) {
    // generate a t-f bb for the condition 
    InstructionList conditionList = getInstructions(exp.condition);
    bbptr_t cond = std::make_shared<BB>(BB(false, conditionList));

    // generate a graph for the then block
    exp.thenBlock.accept(*this);
    bbptr_t thenEnter = retEnter;
    bbptr_t thenExit = retExit;

    // generate empty bb for the endpoint
    InstructionList endList;
    bbptr_t end = std::make_shared<BB>(BB(true, endList));

    // connect condition true to then entrance
    cond->trueOut = thenEnter;

    // connect then exit to the exit
    thenExit->epsOut = end;

    // handle the else block 
    if (exp.elseBlock) {
        // generate a graph for the else block
        exp.elseBlock->accept(*this);
        bbptr_t elseEnter = retEnter;
        bbptr_t elseExit = retExit;

        // connect condition false to else entrance
        cond->falseOut = elseEnter;

        // connect else exit to end
        elseExit->epsOut = end;

    } else {
        // connect condition false directly to end
        cond->falseOut = end;
    }
        
    // return entrance and exit
    retEnter = cond; 
    retExit = end;
}

void CFGBuilder::visit(WhileLoop& exp) {
    // empty bb as a starting point
    InstructionList startList;
    bbptr_t start = std::make_shared<BB>(BB(true, startList));

    // empty bb as endpoint
    InstructionList endList;
    bbptr_t end = std::make_shared<BB>(BB(true, startList));

    // make a T/F bb for the condition
    InstructionList conditionList = getInstructions(exp.condition);
    bbptr_t cond = std::make_shared<BB>(BB(false, conditionList)); 

    // make a graph for the body
    exp.body.accept(*this);
    bbptr_t bodyEnter = retEnter;
    bbptr_t bodyExit = retExit;
    
    // connect starting block to expression block 
    start->epsOut = cond; 

    // connect the condition true output to the body entrance
    cond->trueOut = bodyEnter;

    // connect the condition condition false output to the end
    cond->falseOut = end;

    // connect the body exit to condition 
    bodyExit->epsOut = cond; 

    // return start and end 
    retEnter = start;
    retExit = end;
}

void CFGBuilder::visit(Return& exp) {
    // generate instructions for the return val 
    InstructionList iList = getInstructions(exp.expr);
    optint_t noArg0; 
    Operation op = Operation::Return;
    Instruction* instr = new Instruction(op, noArg0);
    iList.push_back(*instr);
    // leave the instructions 
    retInstr = iList;
}

void CFGBuilder::visit(FunctionExpr& exp) {
    // Symbol Table maintenance 
    stCounter += 1;
    curTable = symbolTable.at(stCounter);
    
    // TODO: all the logic to build the function 

    // reset the symbol table pointer
    curTable = curTable->parent;
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
    Instruction* instr = new Instruction(op, noArg0);
    iList.push_back(*instr);
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
    Instruction* instr = new Instruction(op, noArg0);
    // add the new instruction to the same basic block
    iList.push_back(*instr);
    retInstr = iList;
}

void CFGBuilder::visit(RecordExpr& exp) {
    // should leave the filled-out record at the top of the stack.
    InstructionList iList;
    // instr to allocate the record
    optint_t noArg0;
    Instruction* alloc = new Instruction(Operation::AllocRecord, noArg0);
    iList.push_back(*alloc);

    for (std::map<Identifier*, Expression*>::iterator it = exp.record.begin(); it != exp.record.end(); it ++) {
        // dup instruction 
        Instruction* dup = new Instruction(Operation::Dup, noArg0);
        iList.push_back(*dup);
        // eval the value and add those instructions
        InstructionList value = getInstructions(*(it->second));
        iList.insert(iList.end(), value.begin(), value.end());
        // add the name to the names array
        std::string field = it->first->name;
        int i = allocName(field);
        // compose the instruction 
        Instruction* store = new Instruction(Operation::FieldStore, optint_t(i));
        iList.push_back(*store);
    }
    // leave the instructions 
    retInstr = iList;
}

void CFGBuilder::visit(Identifier& exp) {
    retInstr = getLoadVarInstr(exp.name);
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
