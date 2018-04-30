/*
 * symboltable.cpp
 *
 * Implements a visitor that takes charge of figuring out scoping descriptors
 */
#include "../exception.h"
#include "symboltable.h" 

stvec_t SymbolTableBuilder::eval(Expression& exp) {
    // create global symbol table
    stptr_t globalTable = make_shared<SymbolTable>(SymbolTable());
    tables.push_back(globalTable);

    // add names for the builtin functions
    globalTable->vars["print"] = make_shared<VarDesc>(VarDesc(GLOBAL));
    globalTable->vars["input"] = make_shared<VarDesc>(VarDesc(GLOBAL));
    globalTable->vars["intcast"] = make_shared<VarDesc>(VarDesc(GLOBAL));

    // install the global frame
    curTable = globalTable;

    // run the visitor
    exp.accept(*this);

    // since this is the global frame, all vars are global. 
    desc_t d;
    for (string var : local) {
        d = make_shared<VarDesc>(VarDesc(GLOBAL));
        globalTable->vars[var] = d;
    }
    for (string var : global) {
        d = make_shared<VarDesc>(VarDesc(GLOBAL));
        globalTable->vars[var] = d;
    }
    // takes care of global vars that were declared global in other scopes
    for (string var : sneakyGlobals) {
        d = make_shared<VarDesc>(VarDesc(GLOBAL));
        globalTable->vars[var] = d;
    }
    for (string var : referenced) {
        if (globalTable->vars.count(var) == 0) { // it's not defined 
            throw UninitializedVariableException(var + " is not initialized");
        }
    }

    // post-processing: for each frame, for each referenced var, 
    // make an entry for global or free and mark parents. 
    for (stptr_t t : tables) {
        for (string var : t->referenced) {
            if (t->vars.count(var) == 0) { // it's not defined in cur frame
                desc_t d = markLocalRef(var, t->parent);
                if (d->type == GLOBAL) {
                    t->vars[var] = make_shared<VarDesc>(VarDesc(GLOBAL));
                } else {
                    t->vars[var] = make_shared<VarDesc>(VarDesc(FREE));
                }
            }
        }
    }
    
    // now return the list of tables
    return tables;
}

desc_t SymbolTableBuilder::markLocalRef(string varName, stptr_t child) {
    // function to perform reference chasing 
    // given a pointer to a child and a varName, returns the appropriate
    // VarDesc, while keeping track of metadata like which parent vars 
    // were referenced
    if (!child) {
        throw UninitializedVariableException(varName + " is not initialized");
    }

    map<string, desc_t> frameVars = child->vars;
    map<string, desc_t>::iterator it = frameVars.find(varName);
    if (it != frameVars.end()) {
        desc_t d = it->second;
        d->isReferenced = true;
        return d;
    } else {
        // this means we are in a situation like childchild -> child ->parent
        // where there is sthg referenced in childchild that is in a more
        // distance ancestor, and we are currently at child. 
        // If we find the var and it is NOT global, this frame 
        // must contain a free variable to link childhild to parent
        desc_t retD = markLocalRef(varName, child->parent);
        if (retD->type != GLOBAL) {
            desc_t d = make_shared<VarDesc>(VarDesc(FREE));
            d->isReferenced = true;
            child->vars[varName] = d;
        }
        return retD;
    }
}

void SymbolTableBuilder::visit(Block& exp) {
    for (Statement* s : exp.stmts) {
        s->accept(*this);
    }
}

void SymbolTableBuilder::visit(Global& exp) {
    global.insert(exp.name.name);
    sneakyGlobals.insert(exp.name.name);
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
    stptr_t funcTable = make_shared<SymbolTable>(SymbolTable());
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
    desc_t d;
    for (string var : local) {
        d = make_shared<VarDesc>(VarDesc(LOCAL));
        funcTable->vars[var] = d;
    }
    for (string var : global) {
        d = make_shared<VarDesc>(VarDesc(GLOBAL));
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
