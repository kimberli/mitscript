# pragma once
#include "opt.h" 

struct LiveInterval {
	int start;
	int end;
	int tempIndex;
};

class RegOpt : public IrOpt {
private: 
    IrInstList instructions;
    void run();
    void executeStep();
public: 
    IrFunc* optimize(IrFunc* irFunc) override;
};
