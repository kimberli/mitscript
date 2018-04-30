/*
 * compiler.cpp
 *
 * Implements a visitor that visits AST nodes generated in MITScript parsing
 * and outputs a root Function
 */
#include "bc-compiler.h"

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

int BytecodeCompiler::allocName(string name) {
    int i = retFunc->names_.size();
    retFunc->names_.push_back(name);
    return i;
}

void BytecodeCompiler::loadConstant(constptr_t c) {
    // add the constant to the constants list
    int constIdx = allocConstant(c);
    // make the LoadConst instruction
    optint_t op0 = optint_t(constIdx);
    BcInstruction* instr = new BcInstruction(BcOp::LoadConst, op0);
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
        BcInstruction* swapInstr = new BcInstruction(BcOp::Swap, optint_t());
        retFunc->instructions.push_back(*swapInstr);
        // FieldStore instruction
        BcInstruction* storeInstr = new BcInstruction(BcOp::FieldStore, optint_t(i));
        retFunc->instructions.push_back(*storeInstr);
        return;
    }
    auto indexE = dynamic_cast<IndexExpr*>(lhs);
    if (indexE != NULL) {
        BcInstruction* swap = new BcInstruction(BcOp::Swap, optint_t());
        // we need S :: record :: index :: value, so we need 2 swaps
        // load the record
        addInstructions(indexE->base);
        retFunc->instructions.push_back(*swap);
        // load the index
        addInstructions(indexE->index);
        retFunc->instructions.push_back(*swap);
        // store
        BcInstruction* store = new BcInstruction(BcOp::IndexStore, optint_t());
        retFunc->instructions.push_back(*store);
        return;
    }
}

void BytecodeCompiler::addWriteVarInstructions(string varName) {
    desc_t d = curTable->vars.at(varName);
    BcInstructionList iList;
    switch (d->type) {
        case GLOBAL: {
            optint_t i = optint_t(d->index);
            BcInstruction* instr = new BcInstruction(BcOp::StoreGlobal, i);
            retFunc->instructions.push_back(*instr);
            return;
        }
        case LOCAL: {
            optint_t i = optint_t(d->index);
            BcInstruction* instr = new BcInstruction(BcOp::StoreLocal, i);
            retFunc->instructions.push_back(*instr);
            return;
        }
        case FREE: {
            // you cannot write to free vars
            throw UninitializedVariableException(varName + " is not initialized");
        }
    }
}

void BytecodeCompiler::loadBuiltIns() {
    // for each func, create a func /w right amount of args,
    // add to the curent frame,
    // then generate code to load globally
    funcptr_t printFunc = new Function();
    printFunc->parameter_count_ = 1;
    int printIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(printFunc);
    BcInstruction* loadPrint = new BcInstruction(BcOp::LoadFunc, optint_t(printIdx));
    BcInstruction* allocPrint = new BcInstruction(BcOp::AllocClosure, optint_t(0));
    retFunc->instructions.push_back(*loadPrint);
    retFunc->instructions.push_back(*allocPrint);
    addWriteVarInstructions("print");

    // input
    funcptr_t inputFunc = new Function();
    inputFunc->parameter_count_ = 0;
    int inputIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(inputFunc);
    BcInstruction* loadInp = new BcInstruction(BcOp::LoadFunc, optint_t(inputIdx));
    BcInstruction* allocInp = new BcInstruction(BcOp::AllocClosure, optint_t(0));
    retFunc->instructions.push_back(*loadInp);
    retFunc->instructions.push_back(*allocInp);
    addWriteVarInstructions("input");

    // intcast
    funcptr_t intcastFunc = new Function();
    intcastFunc->parameter_count_ = 1;
    int intcastIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(intcastFunc);
    BcInstruction* loadIntcast = new BcInstruction(BcOp::LoadFunc, optint_t(intcastIdx));
    BcInstruction* allocIntcast = new BcInstruction(BcOp::AllocClosure, optint_t(0));
    retFunc->instructions.push_back(*loadIntcast);
    retFunc->instructions.push_back(*allocIntcast);
    addWriteVarInstructions("intcast");
}

funcptr_t BytecodeCompiler::evaluate(Expression& exp) {
    // set current function
    retFunc = new Function();
    retFunc->parameter_count_ = 0;

    // generate a symbol table
    SymbolTableBuilder stb = SymbolTableBuilder();
    symbolTables = stb.eval(exp);
    curTable = symbolTables.at(0);

    // load up retFunc with vars from symbol table
    for (map<string, desc_t>::iterator it = curTable->vars.begin(); it != curTable->vars.end(); it ++) {
        string varName = it->first;
        putVarInFunc(varName, curTable, retFunc);
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
    BcInstruction* popInstr = new BcInstruction(BcOp::Pop, optint_t());
    retFunc->instructions.push_back(*popInstr);
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

    // increment the count to the new end of the else block
    elseSize++;
    int offsetElse = elseSize - startSize;
    BcInstructionList::iterator elsePos = retFunc->instructions.begin() + startSize;
    BcInstruction* ifInstr = new BcInstruction(BcOp::If, offsetElse + 1);
    retFunc->instructions.insert(elsePos, *ifInstr);
    LOG("added " + to_string(offsetElse) + " else instructions");

    // add in the then block
    addInstructions(exp.thenBlock);
    int thenSize = retFunc->instructions.size();
    int offsetThen = thenSize - elseSize;
    // calculate offset needed to skip then block and insert Goto instruction before it
    BcInstructionList::iterator thenPos = retFunc->instructions.begin() + elseSize;
    BcInstruction* gotoInstr = new BcInstruction(BcOp::Goto, offsetThen + 1);
    retFunc->instructions.insert(thenPos, *gotoInstr);
    LOG("added " + to_string(offsetThen) + " then instructions");
}

void BytecodeCompiler::visit(WhileLoop& exp) {
    // add instructions for evaluating the condition
    int startsize = retFunc->instructions.size();

    // add the body
    addInstructions(exp.body);
    int endBody = retFunc->instructions.size();

    // insert the goto to skip to the condition
    int bodySize = endBody - startsize;
    BcInstructionList::iterator startPos = retFunc->instructions.begin() + startsize;
    BcInstruction* goInstr = new BcInstruction(BcOp::Goto, optint_t(bodySize + 1));
    retFunc->instructions.insert(startPos, *goInstr);

     // add the condition
    addInstructions(exp.condition);
    int endCondition = retFunc->instructions.size();

    int conditionAndBodySize = endCondition-startsize-1; // -1 for the goto
    // add the if which takes you back to the start of the body
    BcInstruction* ifInstr = new BcInstruction(BcOp::If, optint_t(-conditionAndBodySize));
    retFunc->instructions.push_back(*ifInstr);
}

void BytecodeCompiler::visit(Return& exp) {
    addInstructions(exp.expr);
    BcInstruction* instr = new BcInstruction(BcOp::Return, optint_t());
    retFunc->instructions.push_back(*instr);
}

bool BytecodeCompiler::putVarInFunc(string& varName, stptr_t table, funcptr_t func) {
    // returns true if the bool was put in the local array, false else
    desc_t d = table->vars.at(varName);
    switch (d->type) {
        case GLOBAL:
            d->index = func->names_.size();
            func->names_.push_back(varName);
            return false;
        case LOCAL:
            d->index = func->local_vars_.size();
            func->local_vars_.push_back(varName);
            if (d->isReferenced) {
                d->refIndex = func->local_reference_vars_.size();
                func->local_reference_vars_.push_back(varName);
            }
            return true;
        case FREE:
            d->index = func->free_vars_.size();
            func->free_vars_.push_back(varName);
            return false;
    }
}

void BytecodeCompiler::visit(FunctionExpr& exp) {
    // 1) get the corresponding symbol table
    stCounter += 1;
    stptr_t childTable = symbolTables.at(stCounter);

    // 2) make the new function object
    funcptr_t childFunc = new Function();
    childFunc->parameter_count_ = exp.args.size();

    // load up childFunc with vars from symbol table
    // start with args in order
    set<string> argNames;
    for (Identifier* arg : exp.args) {
        string argName = arg->name;
        argNames.insert(argName);
        bool wasLocal = putVarInFunc(argName, childTable, childFunc);
        // if the arg was global, we need to push its name as a placeholder
        if (!wasLocal) { 
            childFunc->local_vars_.push_back(argName);
        }
    }

    for (map<string, desc_t>::iterator it = childTable->vars.begin(); it != childTable->vars.end(); it ++) {
        string varName = it->first;
        bool isArg = argNames.find(varName) != argNames.end();
        if (!isArg) {
            putVarInFunc(varName, childTable, childFunc);
        }
    }

    // 3) install the child's state and run
    funcptr_t parentFunc = retFunc;
    retFunc = childFunc;
    stptr_t parentTable = curTable;
    curTable = childTable;

    // run
    exp.body.accept(*this);

    // reinstall parent state
    retFunc = parentFunc;
    curTable = parentTable;

    // 4) add the function to the function list with an index.
    int childFuncIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(childFunc);

    // 5) load the recently created function onto the op stack
    BcInstruction* loadF = new BcInstruction(BcOp::LoadFunc, optint_t(childFuncIdx));
    retFunc->instructions.push_back(*loadF);

    // 6) load refs to all the child's free vars.
    // THESE NEED TO GO ON BACKWARDS
    //for (string var : childFunc->free_vars_) {
    vector<string> freeVars = childFunc->free_vars_;
    for (vector<string>::reverse_iterator it = freeVars.rbegin();
         it != freeVars.rend();
         it++) {
        string var = *it;
        // get that var's description in the parent
        desc_t d = curTable->vars.at(var);
        int i;
        // can push a ref to a local ref var or a free var
        if (d->type==LOCAL && d->isReferenced) {
            // instructions for ref variables
            i = d->refIndex;
        } else if (d->type==FREE) {
            // instructions for free variables
            i = d->index + retFunc->local_reference_vars_.size();
        } else {
            // this is an error, there is probably a bug.
            assert(false);
        }
        BcInstruction* push = new BcInstruction(BcOp::PushReference, optint_t(i));
        retFunc->instructions.push_back(*push);
    }

    // 7) allocate the closure
    int numRefs = childFunc->free_vars_.size();
    BcInstruction* allocC = new BcInstruction(BcOp::AllocClosure, optint_t(numRefs));
    retFunc->instructions.push_back(*allocC);
}

void BytecodeCompiler::visit(BinaryExpr& exp) {
    addInstructions(exp.left);
    addInstructions(exp.right);
    // concatenate two vecs
    BcOp op;
    optint_t noArg0;
    BcInstruction* swapOp = new BcInstruction(BcOp::Swap, noArg0);
    // choose the correct instruction
    switch (exp.op) {
        case Or:
            op = BcOp::Or;
            break;
        case And:
            op = BcOp::And;
            break;
        case Lt:
            // no lt instr provided, so first switch op order.
            retFunc->instructions.push_back(*swapOp);
            op = BcOp::Gt;
            break;
        case Gt:
            op = BcOp::Gt;
            break;
        case Lt_eq:
            // same logic as for lt
            retFunc->instructions.push_back(*swapOp);
            op = BcOp::Geq;
            break;
        case Gt_eq:
            op = BcOp::Geq;
            break;
        case Eq_eq:
            op = BcOp::Eq;
            break;
        case Plus:
            op = BcOp::Add;
            break;
        case Minus:
            op = BcOp::Sub;
            break;
        case Times:
            op = BcOp::Mul;
            break;
        case Divide:
            op = BcOp::Div;
            break;
    }
    BcInstruction* instr = new BcInstruction(op, noArg0);
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(UnaryExpr& exp) {
    addInstructions(exp.expr);
    BcOp op;
    optint_t noArg0;
    // choose the correct instruction
    switch (exp.op) {
        case Not:
            op = BcOp::Not;
            break;
        case Neg:
            op = BcOp::Neg;
            break;
    }
    BcInstruction* instr = new BcInstruction(op, noArg0);
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(FieldDeref& exp) {
    // get instructions to load the record
    addInstructions(exp.base);
    // add the field to the names list
    int i = allocName(exp.field.name);
    // compose instruction
    BcInstruction* instr = new BcInstruction(BcOp::FieldLoad, optint_t(i));
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::visit(IndexExpr& exp) {
    // load the record
    addInstructions(exp.base);
    // eval the index
    addInstructions(exp.index);
    // instruction
    BcInstruction* instr = new BcInstruction(BcOp::IndexLoad, optint_t());
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
    BcInstruction* call = new BcInstruction(BcOp::Call, optint_t(numArgs));
    retFunc->instructions.push_back(*call);
}

void BytecodeCompiler::visit(RecordExpr& exp) {
    // instr to allocate the record
    optint_t noArg0;
    BcInstruction* alloc = new BcInstruction(BcOp::AllocRecord, noArg0);
    retFunc->instructions.push_back(*alloc);

    for (map<Identifier*, Expression*>::iterator it = exp.record.begin(); it != exp.record.end(); it ++) {
        // dup instruction
        BcInstruction* dup = new BcInstruction(BcOp::Dup, noArg0);
        retFunc->instructions.push_back(*dup);
        // eval the value and add those instructions
        addInstructions(*(it->second));
        // add the name to the names array
        string field = it->first->name;
        int i = allocName(field);
        // compose the instruction
        BcInstruction* store = new BcInstruction(BcOp::FieldStore, optint_t(i));
        retFunc->instructions.push_back(*store);
    }
}

void BytecodeCompiler::visit(Identifier& exp) {
    desc_t d = curTable->vars.at(exp.name);
    switch (d->type) {
        case GLOBAL: {
            // use load_global
            optint_t i = optint_t(d->index);
            BcInstruction* instr = new BcInstruction(BcOp::LoadGlobal, i);
            retFunc->instructions.push_back(*instr);
            break;
        }
        case LOCAL: {
            optint_t i = optint_t(d->index);
            BcInstruction* instr = new BcInstruction(BcOp::LoadLocal, i);
            retFunc->instructions.push_back(*instr);
            break;
        }
        case FREE: {
            // recall d.index is an index into the free vars, so we have to
            // jump over local ref vars.
            optint_t i = optint_t(d->index + retFunc->local_reference_vars_.size());
            optint_t noArg0;
            BcInstruction* pushRefInstr = new BcInstruction(BcOp::PushReference, i);
            BcInstruction* loadRefInstr = new BcInstruction(BcOp::LoadReference, noArg0);
            retFunc->instructions.push_back(*pushRefInstr);
            retFunc->instructions.push_back(*loadRefInstr);
            break;
        }
    }

}

void BytecodeCompiler::visit(IntConst& exp) {
    constptr_t i = new Integer(exp.val);
    loadConstant(i);
}

void BytecodeCompiler::visit(StrConst& exp) {
    constptr_t s = new String(exp.val);
    loadConstant(s);
}

void BytecodeCompiler::visit(BoolConst& exp) {
    constptr_t b = new Boolean(exp.val);
    loadConstant(b);
}

void BytecodeCompiler::visit(NoneConst& exp) {
    constptr_t n = new None();
    loadConstant(n);
}
