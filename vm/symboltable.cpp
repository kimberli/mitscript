/*
 * symboltable.cpp
 *
 * Implements a visitor that takes charge of figuring out scoping descriptors
 */
#include "symboltable.h" 

VarDesc SymbolTable::resolve(std::string varName, stptr_t table) {
    // returns descriptor for a variable
    
    stptr_t curTable = table;

    while (table) {
        std::map<std::string, VarDesc> frameVars = table->vars;
        std::map<std::string, VarDesc>::iterator it = frameVars.find(varName);
        if (it != frameVars.end()) {
            return it->second;
        }
        curTable = curTable->parent;
    }

    // we did not find the var; this is an error. 
    assert(false);
}

stvec_t SymbolTableBuilder::eval(Expression& exp) {
    // create global symbol table
    stptr_t globalTable;
    tables.push_back(globalTable);

    // add names for the builtin functions
    globalTable->vars["print"] = VarDesc(GLOBAL);
    globalTable->vars["input"] = VarDesc(GLOBAL);
    globalTable->vars["intcast"] = VarDesc(GLOBAL);

    // run the visitor
    exp.accept(*this);

    // since this is the global frame, all vars are global. 
    VarDesc d;
    for (std::string var : local) {
        d = VarDesc(GLOBAL);
        globalTable->vars[var] = d;
    }
    for (std::string var : global) {
        d = VarDesc(GLOBAL);
        globalTable->vars[var] = d;
    }

    // post-processing: for each frame, for each referenced var, 
    // make an entry for global or free and mark parents. 
    for (stptr_t t : tables) {
        for (std::string var : t->referenced) {
            if (t->vars.count(var) == 0) { // it's not defined already
                VarDesc d = markLocalRef(var, t->parent);
                if (d.type == GLOBAL) {
                    t->vars[var] = VarDesc(GLOBAL);
                } else {
                    t->vars[var] = VarDesc(FREE);
                }
            }
        }
    }
    
    // now return the list of tables
    return tables;
}

VarDesc SymbolTableBuilder::markLocalRef(std::string varName, stptr_t child) {
    if (!child) {
        assert (false); // undefined var  
    }

    std::map<std::string, VarDesc> frameVars = child->vars;
    std::map<std::string, VarDesc>::iterator it = frameVars.find(varName);
    if (it != frameVars.end()) {
        VarDesc d = it->second;
        d.isReferenced = true;
        return d;
    } else {
        markLocalRef(varName, child->parent);
    }
}

void SymbolTableBuilder::visit(Block& exp) {
    for (Statement* s : exp.stmts) {
        s->accept(*this);
    }
}

void SymbolTableBuilder::visit(Global& exp) {
    global.insert(exp.name.name);
}

void SymbolTableBuilder::visit(Assignment& exp) {
    // visit the value
    exp.expr.accept(*this);

    Expression* lhsPtr = &(exp.lhs);
    
    // visit the lhs
    auto id = dynamic_cast<Identifier*>(lhsPtr);
    if (id != NULL) {
        local.insert(id->name);
    } else {
        // record derefs are handled normally
        exp.lhs.accept(*this);
    }
}

void SymbolTableBuilder::visit(CallStatement& exp) {
    exp.call.accept(*this);
}

void SymbolTableBuilder::visit(IfStatement& exp) {
    exp.condition.accept(*this);
    exp.thenBlock.accept(*this);
    if (exp.elseBlock) {
        exp.elseBlock->accept(*this);
    }
}

void SymbolTableBuilder::visit(WhileLoop& exp) {
    exp.condition.accept(*this);
    exp.body.accept(*this);
}

void SymbolTableBuilder::visit(Return& exp) {
    exp.expr.accept(*this);
}

void SymbolTableBuilder::visit(FunctionExpr& exp) {
    // make the symbol table for the func and push to the list
    stptr_t funcTable = std::make_shared<SymbolTable>(SymbolTable());
    tables.push_back(funcTable);

    // mark parent
    funcTable->parent = curTable;
    curTable = funcTable;

    // save the sets of the parent
    nameset_t parentGlobals = global;
    nameset_t parentLocals = local;
    nameset_t parentReferenced = referenced;
    
    // reset sets 
    global.clear();
    local.clear();
    referenced.clear();
    
    // args are local vars
    for (Identifier* arg : exp.args) {
        local.insert(arg->name);
    }
    // recurse on body
    exp.body.accept(*this);

    // for each var, add a map entry
    VarDesc d;
    for (std::string var : local) {
        d = VarDesc(LOCAL);
        funcTable->vars[var] = d;
    }
    for (std::string var : global) {
        d = VarDesc(GLOBAL);
        funcTable->vars[var] = d;
    }
    

    // store the referenced vars
    funcTable->referenced = referenced;

    // put the old ones back 
    global = parentGlobals;
    local = parentLocals;
    referenced = parentReferenced;

    // reset the curTable pointer
    curTable = funcTable->parent;
}

void SymbolTableBuilder::visit(BinaryExpr& exp) {
    exp.left.accept(*this);
    exp.right.accept(*this);
}

void SymbolTableBuilder::visit(UnaryExpr& exp) {
    exp.expr.accept(*this);
}

void SymbolTableBuilder::visit(FieldDeref& exp) {
    exp.base.accept(*this);
}

void SymbolTableBuilder::visit(IndexExpr& exp) {
    exp.base.accept(*this);
    exp.index.accept(*this);
}

void SymbolTableBuilder::visit(Call& exp) {
    exp.target.accept(*this);
    for (Expression* arg : exp.args) {
        arg->accept(*this);
    }
}

void SymbolTableBuilder::visit(RecordExpr& exp) {
    for (map<Identifier*, Expression*>::iterator it = exp.record.begin(); it != exp.record.end(); it++) {
        it->second->accept(*this);
    }
}

void SymbolTableBuilder::visit(Identifier& exp) {
    // this is a variable; add to referenced
    referenced.insert(exp.name);
}

void SymbolTableBuilder::visit(IntConst& exp) {
    // no-op
}

void SymbolTableBuilder::visit(StrConst& exp) {
    // no-op
}

void SymbolTableBuilder::visit(BoolConst& exp) {
    // no-op
}

void SymbolTableBuilder::visit(NoneConst& exp) {
    // no-op
}
