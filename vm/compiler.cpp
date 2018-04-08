/*
 * compiler.cpp
 *
 * Implements a visitor that visits AST nodes generated in MITScript parsing
 * and outputs a root Function
 */
#include "compiler.h"

funcptr_t BytecodeCompiler::getFunction(AST_node& expr) {
    expr.accept(*this);
    return retFunc;
}

void BytecodeCompiler::addInstructions(AST_node& expr) {
    expr.accept(*this);
    //retFunc->instructions.insert(retFunc->instructions.end(), retInstr.begin(), retInstr.end());
}


// TODO: These could be more efficient,
// we could definitely have a static value for None.
int BytecodeCompiler::allocConstant(constptr_t c) {
    int i = retFunc->constants_.size();
    retFunc->constants_.push_back(c);
    return i;
}

int BytecodeCompiler::allocName(std::string name) {
    int i = retFunc->names_.size();
    retFunc->names_.push_back(name);
    return i;
}

void BytecodeCompiler::loadConstant(constptr_t c) {
    // add the constant to the constants list
    int constIdx = allocConstant(c);
    // make the LoadConst instruction
    optint_t op0 = optint_t(constIdx);
    Instruction* instr = new Instruction(Operation::LoadConst, op0);
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::addWriteInstructions(Expression* lhs) {
    auto id = dynamic_cast<Identifier*>(lhs);
    if (id != NULL) {
        // use var load
        addWriteVarInstructions(id->name);
    }
    auto fieldD = dynamic_cast<FieldDeref*>(lhs);
    if (fieldD != NULL) {
        // load the record
        addInstructions(fieldD->base);
        // allocate a name 
        int i = allocName(fieldD->field.name);
        // we need to swap the order: we want s :: record :: value
        Instruction* swapInstr = new Instruction(Operation::Swap, optint_t());
        retFunc->instructions.push_back(*swapInstr);
        // FieldStore instruction
        Instruction* storeInstr = new Instruction(Operation::FieldStore, optint_t(i));
        retFunc->instructions.push_back(*storeInstr);
        return;
    }
    auto indexE = dynamic_cast<IndexExpr*>(lhs);
    if (indexE != NULL) {
        Instruction* swap = new Instruction(Operation::Swap, optint_t());
        // we need S :: record :: index :: value, so we need 2 swaps
        // load the record 
        addInstructions(indexE->base);
        retFunc->instructions.push_back(*swap);
        // load the index
        addInstructions(indexE->index);
        retFunc->instructions.push_back(*swap);
        // store
        Instruction* store = new Instruction(Operation::IndexStore, optint_t());
        retFunc->instructions.push_back(*store);
        return;
    }
}

void BytecodeCompiler::addWriteVarInstructions(std::string varName) {
    desc_t d = curTable->vars.at(varName);
    InstructionList iList;
    switch (d->type) {
        case GLOBAL: {
            optint_t i = optint_t(d->index);
            Instruction* instr = new Instruction(Operation::StoreGlobal, i);
            retFunc->instructions.push_back(*instr);
            return;
        }
        case LOCAL: {
            optint_t i = optint_t(d->index);
            Instruction* instr = new Instruction(Operation::StoreLocal, i);
            retFunc->instructions.push_back(*instr);
            return;
        }
        case FREE: {
            // recall d.index is an index into the free vars, so we have to 
            // jump over local ref vars. 
            optint_t i = optint_t(d->index + retFunc->local_reference_vars_.size());
            optint_t noArg0;
            Instruction* pushRefInstr = new Instruction(Operation::PushReference, i);
            // the stack is now S :: value :: ref, but we need 
            // S :: ref :: value, so add a swap instr. 
            Instruction* swapInstr = new Instruction(Operation::Swap, noArg0);
            Instruction* storeRefInstr = new Instruction(Operation::StoreReference, noArg0);
            retFunc->instructions.push_back(*pushRefInstr);
            retFunc->instructions.push_back(*swapInstr);
            retFunc->instructions.push_back(*storeRefInstr);
            return;
        }
    }
}

void BytecodeCompiler::loadBuiltIns() {
    // for each func, create a func /w right amount of args, 
    // add to the curent frame, 
    // then generate code to load globally
    funcptr_t printFunc = std::make_shared<Function>(Function());
    printFunc->parameter_count_ = 1;
    int printIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(printFunc);
    Instruction* loadPrint = new Instruction(Operation::LoadFunc, optint_t(printIdx));
    Instruction* allocPrint = new Instruction(Operation::AllocClosure, optint_t(0));
    retFunc->instructions.push_back(*loadPrint);
    retFunc->instructions.push_back(*allocPrint);
    addWriteVarInstructions("print");
    
    // intcast
    funcptr_t intcastFunc = std::make_shared<Function>(Function());
    intcastFunc->parameter_count_ = 1;
    int intcastIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(intcastFunc);
    Instruction* loadIntcast = new Instruction(Operation::LoadFunc, optint_t(intcastIdx));
    Instruction* allocIntcast = new Instruction(Operation::AllocClosure, optint_t(0));
    retFunc->instructions.push_back(*loadIntcast);
    retFunc->instructions.push_back(*allocIntcast);
    addWriteVarInstructions("intcast");
    
    // input 
    funcptr_t inputFunc = std::make_shared<Function>(Function());
    inputFunc->parameter_count_ = 0;
    int inputIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(inputFunc);
    Instruction* loadInp = new Instruction(Operation::LoadFunc, optint_t(inputIdx));
    Instruction* allocInp = new Instruction(Operation::AllocClosure, optint_t(0));
    retFunc->instructions.push_back(*loadInp);
    retFunc->instructions.push_back(*allocInp);
    addWriteVarInstructions("input");
}

funcptr_t BytecodeCompiler::evaluate(Expression& exp) {
    // set current function 
    // TODO: LOAD BUILT-INS
    retFunc = std::make_shared<Function>(Function());
    retFunc->parameter_count_ = 0;

    // generate a symbol table 
    SymbolTableBuilder stb = SymbolTableBuilder();
    symbolTables = stb.eval(exp);
    curTable = symbolTables.at(0);

    // load up retFunc with vars from symbol table
    for (std::map<std::string, desc_t>::iterator it = curTable->vars.begin(); it != curTable->vars.end(); it ++) {
        std::string varName = it->first;
        desc_t d = it->second;
        switch (d->type) {
            case GLOBAL: 
                d->index = retFunc->names_.size();
                retFunc->names_.push_back(varName);
                break;
            case LOCAL: 
                d->index = retFunc->local_vars_.size();
                retFunc->local_vars_.push_back(varName);
                if (d->isReferenced) {
                    d->refIndex = retFunc->local_reference_vars_.size();
                    retFunc->local_reference_vars_.push_back(varName);
                }
                break;
            case FREE: 
                d->index = retFunc->free_vars_.size();
                retFunc->free_vars_.push_back(varName); 
        }
    }

    // load built-in functions
    loadBuiltIns();

    // run this visitor
    exp.accept(*this);

    // return the function. 
    return retFunc;

}

void BytecodeCompiler::visit(Block& exp) {
    for (Statement* s : exp.stmts) {
        addInstructions(*s);
    }
}

void BytecodeCompiler::visit(Global& exp) {
    // no-op
}

void BytecodeCompiler::visit(Assignment& exp) {
    addInstructions(exp.expr);
    addWriteInstructions(&exp.lhs);
}

void BytecodeCompiler::visit(CallStatement& exp) {
    addInstructions(exp.call);
}

void BytecodeCompiler::visit(IfStatement& exp) {
    // add instructions for evaluating the condition
    addInstructions(exp.condition);

    int startSize = retFunc->instructions.size();
    // add in the else block first
    if (exp.elseBlock) {
        addInstructions(*(exp.elseBlock));
    }
    int elseSize = retFunc->instructions.size();
    // calculate offset needed to skip else block and insert If instruction before it
    int offsetElse = elseSize - startSize;
    InstructionList::iterator elsePos = retFunc->instructions.begin() + startSize;
    Instruction* ifInstr = new Instruction(Operation::If, offsetElse);
    retFunc->instructions.insert(elsePos, *ifInstr);
    LOG("added " + to_string(offsetElse) + " else instructions");

    // increment the count to the new end of the else block
    elseSize++;
    // add in the then block
    addInstructions(exp.thenBlock);
    int thenSize = retFunc->instructions.size();
    int offsetThen = thenSize - elseSize;
    // calculate offset needed to skip then block and insert Goto instruction before it
    InstructionList::iterator thenPos = retFunc->instructions.begin() + elseSize;
    Instruction* gotoInstr = new Instruction(Operation::Goto, offsetThen);
    retFunc->instructions.insert(thenPos, *gotoInstr);
    LOG("added " + to_string(offsetThen) + " then instructions");
}

void BytecodeCompiler::visit(WhileLoop& exp) {
    // add instructions for evaluating the condition
    addInstructions(exp.condition);
    int startSize = retFunc->instructions.size();
    // add in the body instructions
    addInstructions(exp.body);
    int bodySize = retFunc->instructions.size();
    int offsetBody = bodySize - startSize;
    // insert goto before the body
    InstructionList::iterator bodyPos = retFunc->instructions.begin() + startSize;
    Instruction* gotoInstr = new Instruction(Operation::Goto, offsetBody);
    retFunc->instructions.insert(bodyPos, *gotoInstr);
    // insert if after the body
    Instruction* ifInstr = new Instruction(Operation::If, -offsetBody);
    retFunc->instructions.push_back(*ifInstr);
}

void BytecodeCompiler::visit(Return& exp) {
    addInstructions(exp.expr);
    Instruction* instr = new Instruction(Operation::Return, optint_t());
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(FunctionExpr& exp) {
    // 1) get the corresponding symbol table
    stCounter += 1; 
    stptr_t childTable = symbolTables.at(stCounter);

    // 2) make the new function object 
    funcptr_t childFunc = std::make_shared<Function>(Function());
    childFunc->parameter_count_ = exp.args.size();

    // load up childFunc with vars from symbol table
    for (std::map<std::string, desc_t>::iterator it = childTable->vars.begin(); it != childTable->vars.end(); it ++) {
        std::string varName = it->first;
        desc_t d = it->second;
        switch (d->type) {
            case GLOBAL: 
                d->index = childFunc->names_.size();
                childFunc->names_.push_back(varName);
                break;
            case LOCAL: 
                d->index = childFunc->local_vars_.size();
                childFunc->local_vars_.push_back(varName);
                if (d->isReferenced) {
                    d->refIndex = childFunc->local_reference_vars_.size();
                    childFunc->local_reference_vars_.push_back(varName);
                }
                break;
            case FREE: 
                d->index = childFunc->free_vars_.size();
                childFunc->free_vars_.push_back(varName); 
                break;
        }
    }

    // 3) install the child's state and run
    funcptr_t parentFunc = retFunc;
    retFunc = childFunc; 
    stptr_t parentTable = curTable;
    curTable = childTable;

    // run
    exp.body.accept(*this);
    // if there is no explicit return statement, add a return None
    bool noInstr = retFunc->instructions.size() == 0;
    bool noRet = retFunc->instructions.size()>0 && retFunc->instructions.back().operation != Operation::Return;
    if (noInstr || noRet) {
        // add a return None
        constptr_t n = std::make_shared<None>(None());
        loadConstant(n);
        Instruction* ret = new Instruction(Operation::Return, optint_t());
        retFunc->instructions.push_back(*ret);
    }
    // reinstall parent state
    retFunc = parentFunc;
    curTable = parentTable;

    // 4) add the function to the function list with an index. 
    int childFuncIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(childFunc);

    // 5) load the recently created function onto the op stack
    Instruction* loadF = new Instruction(Operation::LoadFunc, optint_t(childFuncIdx));
    retFunc->instructions.push_back(*loadF);

    // 6) load refs to all the child's free vars. 
    for (std::string var : childFunc->free_vars_) {
        // get that var's description in the parent
        desc_t d = curTable->vars.at(var);
        int i;
        // can push a ref to a local ref var or a free var 
        if (d->type==LOCAL && d->isReferenced) {
            // instructions for ref variables
            int i = d->refIndex;
        } else if (d->type==FREE) {
            // instructions for free variables
            int i = d->index + retFunc->local_reference_vars_.size();
        } else {
            // this is an error, there is probably a bug. 
            assert(false);
        }
        Instruction* push = new Instruction(Operation::PushReference, optint_t(i));
        retFunc->instructions.push_back(*push);
    }
    
    // 7) allocate the closure 
    int numRefs = childFunc->free_vars_.size();
    Instruction* allocC = new Instruction(Operation::AllocClosure, optint_t(numRefs));
    retFunc->instructions.push_back(*allocC);
}

void BytecodeCompiler::visit(BinaryExpr& exp) {
    addInstructions(exp.left);
    addInstructions(exp.right);
    // concatenate two vecs
    Operation op; 
    optint_t noArg0;
    Instruction* swapOp = new Instruction(Operation::Swap, noArg0);
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
            retFunc->instructions.push_back(*swapOp);
            op = Operation::Gt;
            break;
        case Gt: 
            op = Operation::Gt;
            break;
        case Lt_eq: 
            // same logic as for lt
            retFunc->instructions.push_back(*swapOp);
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
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(UnaryExpr& exp) {
    addInstructions(exp.expr);
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
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(FieldDeref& exp) {
    // get instructions to load the record 
    addInstructions(exp.base);
    // add the field to the names list
    int i = allocName(exp.field.name);
    // compose instruction 
    Instruction* instr = new Instruction(Operation::FieldLoad, optint_t(i));
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(IndexExpr& exp) {
    // load the record 
    addInstructions(exp.base);
    // eval the index 
    addInstructions(exp.index);
    // instruction
    Instruction* instr = new Instruction(Operation::IndexLoad, optint_t());
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(Call& exp) {
    // load the closure
    addInstructions(exp.target);
    // push the args in order
    for (Expression* arg : exp.args) {
        arg->accept(*this);
        //InstructionList loadArg = retInstr;
        //retFunc->instructions.insert(retFunc->instructions.end(), loadArg.begin(), loadArg.end());
    }

    int numArgs = exp.args.size();
    Instruction* call = new Instruction(Operation::Call, optint_t(numArgs));
    retFunc->instructions.push_back(*call);
}

void BytecodeCompiler::visit(RecordExpr& exp) {
    // instr to allocate the record
    optint_t noArg0;
    Instruction* alloc = new Instruction(Operation::AllocRecord, noArg0);
    retFunc->instructions.push_back(*alloc);

    for (std::map<Identifier*, Expression*>::iterator it = exp.record.begin(); it != exp.record.end(); it ++) {
        std::cout << it->first->name << std::endl;
        // dup instruction 
        Instruction* dup = new Instruction(Operation::Dup, noArg0);
        retFunc->instructions.push_back(*dup);
        // eval the value and add those instructions
        addInstructions(*(it->second));
        // add the name to the names array
        std::string field = it->first->name;
        int i = allocName(field);
        // compose the instruction 
        Instruction* store = new Instruction(Operation::FieldStore, optint_t(i));
        retFunc->instructions.push_back(*store);
    }
}

void BytecodeCompiler::visit(Identifier& exp) {
    desc_t d = curTable->vars.at(exp.name);
    switch (d->type) {
        case GLOBAL: {
            // use load_global 
            optint_t i = optint_t(d->index);
            Instruction* instr = new Instruction(Operation::LoadGlobal, i);
            retFunc->instructions.push_back(*instr);
            break;
        }
        case LOCAL: {
            optint_t i = optint_t(d->index);
            Instruction* instr = new Instruction(Operation::LoadLocal, i);
            retFunc->instructions.push_back(*instr);
            break;
        }
        case FREE: {
            // recall d.index is an index into the free vars, so we have to 
            // jump over local ref vars. 
            optint_t i = optint_t(d->index + retFunc->local_reference_vars_.size());
            optint_t noArg0;
            Instruction* pushRefInstr = new Instruction(Operation::PushReference, i);
            Instruction* loadRefInstr = new Instruction(Operation::LoadReference, noArg0);
            retFunc->instructions.push_back(*pushRefInstr);
            retFunc->instructions.push_back(*loadRefInstr);
            break;
        }
    }

}

void BytecodeCompiler::visit(IntConst& exp) {
    constptr_t i = std::make_shared<Integer>(exp.val);
    loadConstant(i);
}

void BytecodeCompiler::visit(StrConst& exp) {
    constptr_t s = std::make_shared<String>(exp.val);
    loadConstant(s);
}

void BytecodeCompiler::visit(BoolConst& exp) {
    constptr_t b = std::make_shared<Boolean>(Boolean(exp.val));
    loadConstant(b);
}

void BytecodeCompiler::visit(NoneConst& exp) {
    constptr_t n = std::make_shared<None>(None());
    loadConstant(n);
}
