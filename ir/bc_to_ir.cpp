/*
 * bc-to-ir.cpp
 *
 * Convert bytecode to IR representation
 */
#include "../vm/types.h"
#include "bc_to_ir.h"
#include "ir.h"
#include <algorithm>
#include <stack>

using namespace std;

IrCompiler::IrCompiler(vptr<Function> mainFunc) {
	globalFunc = mainFunc;
};

IrFunc IrCompiler::toIrFunc() {
	int i = 0;
    Instruction inst = globalFunc->instructions[i];
    switch (inst.operation) {
        case Operation::LoadConst:
            {
                break;
            }
        case Operation::LoadFunc:
            {
                break;
            }
        case Operation::LoadLocal:
            {
                break;
            }
        case Operation::StoreLocal:
            {
                break;
            }
        case Operation::LoadGlobal:
            {
                break;
            }
        case Operation::StoreGlobal:
            {
                break;
            }
        case Operation::PushReference:
            {
                break;
            }
        case Operation::LoadReference:
            {
                break;
            }
        case Operation::StoreReference:
            {
                break;
            }
        case Operation::AllocRecord:
            {
                break;
            }
        case Operation::FieldLoad:
            {
                break;
            }
        case Operation::FieldStore:
            {
                break;
            }
        case Operation::IndexLoad:
            {
                break;
            }
        case Operation::IndexStore:
            {
                break;
            }
        case Operation::AllocClosure:
            {
                break;
            }
        case Operation::Call:
            {
                break;
            }
        case Operation::Return:
            {
                break;
            }
        case Operation::Add:
            {
                break;
            }
        case Operation::Sub:
            {
                break;
            }
        case Operation::Mul:
            {
                break;
            }
        case Operation::Div:
            {
                break;
            }
        case Operation::Neg:
            {
                break;
            }
        case Operation::Gt:
            {
                break;
            }
        case Operation::Geq:
            {
                break;
            }
        case Operation::Eq:
            {
                break;
            }
        case Operation::And:
            {
                break;
            }
        case Operation::Or:
            {
                break;
            }
        case Operation::Not:
            {
                break;
            }
        case Operation::Goto:
            {
                break;
            }
        case Operation::If:
            {
                break;
            }
        case Operation::Dup:
            {
                break;
            }
        case Operation::Swap:
            {
                break;
            }
        case Operation::Pop:
            {
                break;
            }
        default:
            throw RuntimeException("should never get here - invalid instruction");
    }
};

IrProgram IrCompiler::toIr() {
};
