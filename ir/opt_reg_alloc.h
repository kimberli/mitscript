# pragma once
#include "opt.h" 

class RegOpt : public IrOpt {
private: 
    IrInstList instructions;
    void run();
    void executeStep();
public: 
    IrFunc* optimize(IrFunc* irFunc) override;
};
