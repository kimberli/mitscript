#include "opt_reg_alloc.h"
#include <set>
#include "include/x64asm.h"

void RegOpt::optimize(IrFunc* irFunc) {
	linearScan(irFunc);
};

struct compareIntervalEnd {
	bool operator()(const tempptr_t &a, const tempptr_t &b) {
		return a->endInterval <= b->endInterval;
	};
};

void RegOpt::linearScan(IrFunc* irFunc) {
	set<tempptr_t, compareIntervalEnd> active; // ordered by endInterval
	int stackOffset = 0;
	int numRegisters = freeRegisters.size();
//	for (tempptr_t temp_i: irFunc->temps) {
//		// modifying all regs to be long
//		temp_i->startInterval = 0;
//		temp_i->endInterval = irFunc->instructions.size();
//	}
	for (tempptr_t temp_i: irFunc->temps) {
		if (temp_i->startInterval == -1 && temp_i->endInterval == -1) {
			// TODO: creating global temps that do nothing
			continue;
		}
		std::set<tempptr_t, compareIntervalEnd>::iterator it;
		for (it = active.begin(); it != active.end();) { // Expire old intervals
			tempptr_t temp_j = *it;
			if (temp_j->endInterval >= temp_i->startInterval) {
				break;
			}
			if (temp_j->reg) {
				freeRegisters.push_back(temp_j->reg.value());
			}
			active.erase(it++);
		}
		if (active.size() == numRegisters) {
			// Spill this interval
			std::set<tempptr_t, compareIntervalEnd>::iterator end = active.end();
			end--;
			tempptr_t spill = *(end);
			if (spill->endInterval > temp_i->endInterval) {
				temp_i->reg = spill->reg;
				spill->reg = nullopt;
				spill->stackOffset = stackOffset;
				stackOffset++;
				active.insert(temp_i);
				active.erase(end);
			} else {
				temp_i->stackOffset = stackOffset;
				stackOffset++;
			}
		} else {
			// Allocate register to this temp
			temp_i->reg = freeRegisters.back();
			freeRegisters.pop_back();
			active.insert(temp_i);
		}
		LOG(to_string(temp_i->index) + " start: " + to_string(temp_i->startInterval) + ", end: " + to_string(temp_i->endInterval));
	}
};

void RegOpt::run() {
    LOG("Starting reg alloc optimization");
    if (func->instructions.size() > 0) {
        while (!finished) {
            executeStep();
        }
    }
};

void RegOpt::executeStep() {
    instptr_t inst = func->instructions.at(instructionIndex);
    switch(inst->op) {
        case IrOp::LoadConst:
            {
                LOG(to_string(instructionIndex) + ": LoadConst");
                break;
            }
        case IrOp::LoadFunc:
            {
                LOG(to_string(instructionIndex) + ": LoadFunc");
                break;
            }
       case IrOp::LoadGlobal:
            {
                LOG(to_string(instructionIndex) + ": LoadGlobal");
                break;
            }
        case IrOp::StoreLocal:
            {
                LOG(to_string(instructionIndex) + ": StoreLocal");
                break;
            }
       case IrOp::StoreGlobal:
            {
                LOG(to_string(instructionIndex) + ": StoreGlobal");
                break;
            }
		case IrOp::StoreLocalRef: 
			{
				LOG(to_string(instructionIndex) + ": StoreLocalRef");
                break;
			}
       case IrOp::PushFreeRef: 
            {
                LOG(to_string(instructionIndex) + ": PushFreeRef");
                break;
            }
        case IrOp::LoadReference:
            {
                LOG(to_string(instructionIndex) + ": LoadReference");
                break;
            }
        case IrOp::AllocRecord:
            {
                LOG(to_string(instructionIndex) + ": AllocRecord");
                break;
            };
        case IrOp::FieldLoad:
            {
                LOG(to_string(instructionIndex) + ": FieldLoad");
                break;
            };
        case IrOp::FieldStore:
            {
                LOG(to_string(instructionIndex) + ": FieldStore");
                break;
            };
        case IrOp::IndexLoad:
            {
                LOG(to_string(instructionIndex) + ": IndexLoad");
                break;
            };
        case IrOp::IndexStore:
            {
                LOG(to_string(instructionIndex) + ": IndexStore");
                break;
            };
        case IrOp::AllocClosure:
            {
                LOG(to_string(instructionIndex) + ": AllocClosure");
                break;
            };
        case IrOp::Call:
            {
                LOG(to_string(instructionIndex) + ": Call");
                break;
            };
        case IrOp::Return:
            {
                LOG(to_string(instructionIndex) + ": Return");
                break;
            };
        case IrOp::Add:
            {
                LOG(to_string(instructionIndex) + ": Add");
                break;
            };
        case IrOp::Sub:
            {
                LOG(to_string(instructionIndex) + ": Sub");
                break;
            };
        case IrOp::Mul:
            {
                LOG(to_string(instructionIndex) + ": Mul");
                break;
            };
        case IrOp::Div:
            {
                LOG(to_string(instructionIndex) + ": Div");
                break;
            };
        case IrOp::Neg: {
                LOG(to_string(instructionIndex) + ": Neg");
                break;
            };
        case IrOp::Gt:
            {
                LOG(to_string(instructionIndex) + ": Gt");
                break;
            };
        case IrOp::Geq:
            {
                LOG(to_string(instructionIndex) + ": Geq");
                break;
            };
        case IrOp::Eq:
            {
                LOG(to_string(instructionIndex) + ": Eq");
                break;
            };
        case IrOp::And:
            {
                LOG(to_string(instructionIndex) + ": And");
                break;
            };
        case IrOp::Or:
            {
                LOG(to_string(instructionIndex) + ": Or");
                break;
            };
        case IrOp::Not:
            {
                LOG(to_string(instructionIndex) + ": Not");
                break;
            };
        case IrOp::Goto:
            {
                LOG(to_string(instructionIndex) + ": Goto");
                break;
            };
        case IrOp::If:
            {
                LOG(to_string(instructionIndex) + ": If");
                break;
            };
       case IrOp::AssertInteger:
            {
                LOG(to_string(instructionIndex) + ": AssertInteger");
                break;
            };
        case IrOp::AssertBoolean:
            {
                LOG(to_string(instructionIndex) + ": AssertBool");
                break;
            };
        case IrOp::AssertString:
            {
                LOG(to_string(instructionIndex) + ": AssertString");
                break;
            };
        case IrOp::AssertRecord:
            {
                LOG(to_string(instructionIndex) + ": AssertRecord");
                break;
            };
        case IrOp::AssertFunction:
            {
                LOG(to_string(instructionIndex) + ": AssertFunction");
                break;
            };
        case IrOp::AssertClosure:
            {
                LOG(to_string(instructionIndex) + ": AssertClosure");
                break;
            };
        case IrOp::AssertValWrapper: 
            {
                LOG(to_string(instructionIndex) + ": AssertValWrapper");
                break;
            };
        case IrOp::CastString:
            {
                LOG(to_string(instructionIndex) + ": CastString");
                break;
            };
        case IrOp::AddLabel:
            {
                LOG(to_string(instructionIndex) + ": AddLabel");
                break;
            };
        case IrOp::GarbageCollect:
            {
                LOG(to_string(instructionIndex) + ": GarbageCollect");
                break;
            };
        default:
            throw RuntimeException("Should not get here: invalid ir inst");
    }
    instructionIndex += 1; 
    if (instructionIndex >= func->instructions.size()) {
        finished = true;
    }
}
