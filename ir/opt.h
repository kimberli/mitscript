# pragma once
# include "../ir.h"

class IrOpt {
public: 
    virtual IrFunc optimize(IrFunc func); 
};
