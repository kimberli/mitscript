# pragma once
# include "../ir.h"

class IrOpt {
protected: 
    IrFunc* func; 
    int32_t instructionIndex = 0;
    bool finished; 
public: 
    virtual IrFunc* optimize(IrFunc* irFunc) = 0;
};
