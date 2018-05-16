# pragma once
# include "../ir.h"
# include "../ir/bc_to_ir.h"

class IrOpt {
protected: 
    IrFunc* func; 
    int32_t instructionIndex = 0;
    bool finished; 
public: 
    virtual int optimize(IrFunc* irFunc) = 0;
};
