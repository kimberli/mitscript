# pragma once
#include "opt.h" 

class RegOpt : public IrOpt {
public: 
    IrFunc optimize(IrFunc func) override;
};
