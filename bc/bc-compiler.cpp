/*
 * compiler.cpp
 *
 * Implements a visitor that visits AST nodes generated in MITScript parsing
 * and outputs a root Function
 */
#include "bc-compiler.h"

Function* BytecodeCompiler::getFunction(AST_node& expr) {
    expr.accept(*this);
    return retFunc;
}

void BytecodeCompiler::addInstructions(AST_node& expr) {
    expr.accept(*this);
}


// TODO: These could be more efficient,
// we could definitely have a static value for None.
int BytecodeCompiler::allocConstant(tagptr_t ptr) {
    int i = retFunc->constants_.size();
    retFunc->constants_.push_back(ptr);
    return i;
}

int BytecodeCompiler::allocName(string name) {
    int i = retFunc->names_.size();
    retFunc->names_.push_back(name);
    return i;
}

void BytecodeCompiler::addInstruction(BcOp op, optint_t op0) {
    BcInstruction *instr = new BcInstruction(op, op0);
    retFunc->instructions.push_back(*instr);
}

void BytecodeCompiler::loadConstant(tagptr_t ptr) {
    // add the constant to the constants list
    int constIdx = allocConstant(ptr);
    // make the LoadConst instruction
    addInstruction(BcOp::LoadConst, constIdx);
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
        addInstruction(BcOp::Swap, nullopt);
        // FieldStore instruction
        addInstruction(BcOp::FieldStore, i);
        return;
    }
    auto indexE = dynamic_cast<IndexExpr*>(lhs);
    if (indexE != NULL) {
        // we need S :: record :: index :: value, so we need 2 swaps
        // load the record
        addInstructions(indexE->base);
        addInstruction(BcOp::Swap, nullopt);
        // load the index
        addInstructions(indexE->index);
        addInstruction(BcOp::Swap, nullopt);
        // store
        addInstruction(BcOp::IndexStore, nullopt);
        return;
    }
}

void BytecodeCompiler::addWriteVarInstructions(string varName) {
    desc_t d = curTable->vars.at(varName);
    BcInstructionList iList;
    switch (d->type) {
        case GLOBAL: {
            addInstruction(BcOp::StoreGlobal, d->index);
            return;
        }
        case LOCAL: {
            addInstruction(BcOp::StoreLocal, d->index);
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
    Function* printFunc = new Function();
    printFunc->parameter_count_ = 1;
    int printIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(printFunc);
    addInstruction(BcOp::LoadFunc, printIdx);
    addInstruction(BcOp::AllocClosure, 0);
    addWriteVarInstructions("print");

    // input
    Function* inputFunc = new Function();
    inputFunc->parameter_count_ = 0;
    int inputIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(inputFunc);
    addInstruction(BcOp::LoadFunc, inputIdx);
    addInstruction(BcOp::AllocClosure, 0);
    addWriteVarInstructions("input");

    // intcast
    Function* intcastFunc = new Function();
    intcastFunc->parameter_count_ = 1;
    int intcastIdx = retFunc->functions_.size();
    retFunc->functions_.push_back(intcastFunc);
    addInstruction(BcOp::LoadFunc, intcastIdx);
    addInstruction(BcOp::AllocClosure, 0);
    addWriteVarInstructions("intcast");
}

Function* BytecodeCompiler::evaluate(Expression& exp) {
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
    addInstruction(BcOp::Pop, nullopt);
}

void BytecodeCompiler::visit(IfStatement& exp) {
    // reserve 2 labels: 1 for then block, 1 for else block
    int ifLabel = labelCounter;
    labelCounter++;
    int endLabel = labelCounter;
    labelCounter++;

    // add instructions for evaluating the condition
    addInstructions(exp.condition);

    // add the IF instruction that will jump to the then block if true
    addInstruction(BcOp::If, ifLabel);

    // add in the else block
    if (exp.elseBlock) {
        addInstructions(*(exp.elseBlock));
    }

    // add the GOTO instruction that will break out of the if statement
    addInstruction(BcOp::Goto, endLabel);

    // add the LABEL instruction that marks where to jump to if condition is true
    addInstruction(BcOp::Label, ifLabel);
    retFunc->labels_[ifLabel] = retFunc->instructions.size();

    // add in the then block
    addInstructions(exp.thenBlock);

    // add the LABEL instruction that marks the end of the if statement
    addInstruction(BcOp::Label, endLabel);
    retFunc->labels_[endLabel] = retFunc->instructions.size();
}

void BytecodeCompiler::visit(WhileLoop& exp) {
    // add instructions for evaluating the condition
    int condLabel = labelCounter;
    labelCounter++;
    int bodyLabel = labelCounter;
    labelCounter++;

	// Mark start of while loop
	addInstruction(BcOp::StartWhile, nullopt);
    // add the GOTO instruction that will jump to the condition
    addInstruction(BcOp::Goto, condLabel);

    // add the LABEL instruction that marks where to jump to get to the body
    addInstruction(BcOp::Label, bodyLabel);
    retFunc->labels_[bodyLabel] = retFunc->instructions.size();

    // add the body
    addInstructions(exp.body);

    // add the LABEL instruction that marks where to jump to get to the condition
    addInstruction(BcOp::Label, condLabel);
    retFunc->labels_[condLabel] = retFunc->instructions.size();

    // add instructions for evaluating the condition
    addInstructions(exp.condition);

    // add the IF instruction that will jump to the body if true
    addInstruction(BcOp::If, bodyLabel);
	// Mark end of while loop
	addInstruction(BcOp::EndWhile, nullopt);
}

void BytecodeCompiler::visit(Return& exp) {
    addInstructions(exp.expr);
    addInstruction(BcOp::Return, nullopt);
}

bool BytecodeCompiler::putVarInFunc(string& varName, stptr_t table, Function* func) {
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
        default:
            throw "unknown vartype";
    }
}

void BytecodeCompiler::visit(FunctionExpr& exp) {
    // 1) get the corresponding symbol table
    stCounter += 1;
    stptr_t childTable = symbolTables.at(stCounter);

    // 2) make the new function object
    Function* childFunc = new Function();
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
    Function* parentFunc = retFunc;
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
    addInstruction(BcOp::LoadFunc, childFuncIdx);

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
        addInstruction(BcOp::PushReference, i);
    }

    // 7) allocate the closure
    int numRefs = childFunc->free_vars_.size();
    addInstruction(BcOp::AllocClosure, numRefs);
}

void BytecodeCompiler::visit(BinaryExpr& exp) {
    addInstructions(exp.left);
    addInstructions(exp.right);
    // concatenate two vecs
    BcOp op;
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
            addInstruction(BcOp::Swap, nullopt);
            op = BcOp::Gt;
            break;
        case Gt:
            op = BcOp::Gt;
            break;
        case Lt_eq:
            // same logic as for lt
            addInstruction(BcOp::Swap, nullopt);
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
    addInstruction(op, nullopt);
}

void BytecodeCompiler::visit(UnaryExpr& exp) {
    addInstructions(exp.expr);
    BcOp op;
    // choose the correct instruction
    switch (exp.op) {
        case Not:
            op = BcOp::Not;
            break;
        case Neg:
            op = BcOp::Neg;
            break;
    }
    addInstruction(op, nullopt);
}

void BytecodeCompiler::visit(FieldDeref& exp) {
    // get instructions to load the record
    addInstructions(exp.base);
    // add the field to the names list
    int i = allocName(exp.field.name);
    // compose instruction
    addInstruction(BcOp::FieldLoad, i);
}

void BytecodeCompiler::visit(IndexExpr& exp) {
    // load the record
    addInstructions(exp.base);
    // eval the index
    addInstructions(exp.index);
    // instruction
    addInstruction(BcOp::IndexLoad, nullopt);
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
    addInstruction(BcOp::Call, numArgs);
}

void BytecodeCompiler::visit(RecordExpr& exp) {
    // instr to allocate the record
    addInstruction(BcOp::AllocRecord, nullopt);

    for (map<Identifier*, Expression*>::iterator it = exp.record.begin(); it != exp.record.end(); it ++) {
        // dup instruction
        addInstruction(BcOp::Dup, nullopt);
        // eval the value and add those instructions
        addInstructions(*(it->second));
        // add the name to the names array
        string field = it->first->name;
        int i = allocName(field);
        // compose the instruction
        addInstruction(BcOp::FieldStore, i);
    }
}

void BytecodeCompiler::visit(Identifier& exp) {
    desc_t d = curTable->vars.at(exp.name);
    switch (d->type) {
        case GLOBAL: {
            // use load_global
            addInstruction(BcOp::LoadGlobal, d->index);
            break;
        }
        case LOCAL: {
            addInstruction(BcOp::LoadLocal, d->index);
            break;
        }
        case FREE: {
            // recall d.index is an index into the free vars, so we have to
            // jump over local ref vars.
            int i = d->index + retFunc->local_reference_vars_.size();
            addInstruction(BcOp::PushReference, i);
            addInstruction(BcOp::LoadReference, nullopt);
            break;
        }
    }

}

void BytecodeCompiler::visit(IntConst& exp) {
    tagptr_t ptr = make_ptr(exp.val);
    loadConstant(ptr);
}

void BytecodeCompiler::visit(StrConst& exp) {
    tagptr_t ptr = make_ptr(&exp.val);
    loadConstant(ptr);
}

void BytecodeCompiler::visit(BoolConst& exp) {
    tagptr_t ptr = make_ptr(exp.val);
    loadConstant(ptr);
}

void BytecodeCompiler::visit(NoneConst& exp) {
    Constant* n = new None();
    loadConstant(make_ptr(n));
}
