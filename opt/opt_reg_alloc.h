# pragma once
#include "opt.h" 
#include <vector>

class RegOpt : public IrOpt {
private: 
	vector<x64asm::R64> freeRegisters = {
	    x64asm::rdi,
	    x64asm::rsi,
	    x64asm::rax,
	    x64asm::rbx,
	    x64asm::rcx,
	    x64asm::rdx,
	    x64asm::r8,
	    x64asm::r9,
	    x64asm::r10,
	    x64asm::r11,
	    x64asm::r12,
	    x64asm::r13,
	    x64asm::r14,
	    x64asm::r15,
	};
    IrInstList instructions;
    void run();
    void executeStep();
	void linearScan(IrFunc* irFunc);
public: 
    void optimize(IrFunc* irFunc) override;
};
