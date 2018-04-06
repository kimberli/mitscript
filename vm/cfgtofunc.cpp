/*
 * cfgtofunc.cpp
 *
 * Implements a function that converts a CFG data structure to a Function
 * object (as defined in types.h)
 */
#include "cfgtofunc.h"

std::shared_ptr<Function> CFGToFunc(cfgptr_t cfg) {

    // recurse on functions defined on this scope. 
    std::vector<std::shared_ptr<Function>> functions;
    for (cfgptr_t child : cfg->functions_) {
        std::shared_ptr<Function> f = CFGToFunc(child);
        functions.push_back(f);
    }
    
    // create flat instructions from the cfg
    InstructionList iList;
    bbptr_t next = cfg->codeEntry;
    // TODO: rn this ONLY handles serial blocks, no if/then or while. 
    while (next) {
        iList.insert(iList.end(), next->instructions.begin(), next->instructions.end());
        next = next->epsOut;
    }

    std::shared_ptr<Function> f = std::make_shared<Function>(Function(
        functions, 
        cfg->constants_,
        cfg->parameter_count,
        cfg->local_vars_,
        cfg->local_reference_vars_,
        cfg->free_vars_,
        cfg->names_,
        iList
    ));
    return f;
}
