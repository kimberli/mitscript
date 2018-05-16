#include "ir_to_asm.h"

/************************
 * LOCATION HELPERS
 ***********************/
uint32_t IrInterpreter::getRefArrayOffset() {
    return 8*(1 + numCalleeSaved);
}

uint32_t IrInterpreter::getTempOffset(tempptr_t temp) {
    return 8*(1 + numCalleeSaved + 1 + temp->index);
}

/************************
 * CALL HELPERS
 ***********************/
void IrInterpreter::Pop() {
    popCount -=1;
    assm.add(x64asm::rsp, x64asm::Imm32{8});
}
void IrInterpreter::Pop(x64asm::R64 reg) {
    popCount -=1;
    assm.pop(reg);
}
void IrInterpreter::Push(x64asm::R64 reg) {
    popCount +=1;
    assm.push(reg);
}

void IrInterpreter::callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, opttemp_t returnTemp) {
    callHelper(fn, args, temps, nullopt, returnTemp);
}

void IrInterpreter::callHelper(void* fn, vector<x64asm::Imm64> args, vector<tempptr_t> temps, optreg_t lastArg, opttemp_t returnTemp) {
    int totalArgs = args.size() + temps.size() + 1;
    assert (totalArgs <= numArgRegs);

    // IMPORTANT: the reg in lastArg should not be r10, rax, or any of the 6 argRegs
    // STEP 1: save caller-saved registers to stack, reverse order
    for (int i = 0; i < numCallerSaved; ++i) {
        Push(callerSavedRegs[numCallerSaved -1 - i]);
    }

    int pushed = 0;
    // push the opt reg and the arg temps if they're in regs
    if (lastArg) {
         pushed += 1;
         Push(lastArg.value());
    }
    for (auto it = temps.rbegin(); it != temps.rend(); it++) {
        tempptr_t t = *it;
        if (t->reg) {
            pushed += 1;
            Push(t->reg.value());
        }
    }

    // STEP 2: push first 6 arguments to registers, rest to stack
    // args + temps are put in that order as arguments for the function to call
    // on the stack
    // args should contain pointers to values or immediate numbers
    // temps are pointers to Temps which store stack offsets
    int numArgs = args.size() + temps.size();
    int argIndex = 0;
    LOG("  calling helper w/ " + to_string(numArgs) + " total args");
    while (argIndex < args.size()) {
        // put args 1 - 6 into regs (direct imm values)
        assm.mov(callerSavedRegs[argIndex], args[argIndex]);
        argIndex++;
    }
    while (argIndex < numArgs) {
        tempptr_t tempToPush = temps[argIndex - args.size()];
        x64asm::R64 regToStore = callerSavedRegs[argIndex];
        // load differently depending on where the arg is
        if (tempToPush->reg) {
            // can pop it 
            Pop(regToStore);
            pushed -=1;
        } else { // the value is stored on the stack; move from mem
            // move to the right reg from mem
            moveTemp(regToStore, tempToPush);
        }
        argIndex++;
    }
    // if the optional register argument is defined, push it as an arg
    if (lastArg) {
        Pop(callerSavedRegs[argIndex]);
        pushed -=1;
    }

    assert(pushed==0);

    assm.mov(x64asm::r10, x64asm::Imm64{fn});
    assm.call(x64asm::r10);

    // assume we do not have args to pop from the stack

    // restore caller-saved registers from stack, minus rax
    for (uint32_t i = 0; i < numCallerSaved - 1; ++i) {
        Pop(callerSavedRegs[i]);
    }

    // save return value
    if (returnTemp) {
        // move from rax to the temp
        bool usesRax = (returnTemp.value()->reg) && (returnTemp.value()->reg.value() == x64asm::rax);
        if (!usesRax) {
            moveTemp(returnTemp.value(), x64asm::rax);
            Pop(x64asm::rax);
        } else { // if it does use rax, nothing to do!
            Pop();
        }
    } else {
        Pop(x64asm::rax);
    }
}

/************************
 * LOCAL VAR/REF HELPERS
 ***********************/
void IrInterpreter::installLocalVar(tempptr_t temp, uint32_t localIdx) {
    // IMPORTANT! make sure you order these calls in a way that you do not
    // overrwrite rdi or rsi before you finish needing them
    x64asm::M64 src = x64asm::M64{x64asm::rdi, x64asm::Imm32{localIdx*8}};
    if (temp->reg) {
        // move directly to the reg
        assm.mov(temp->reg.value(), src);
    } else {
        // the arg is stored on the stack; use rdi as scratch space
        Push(x64asm::rdi);
        // move the value into rdi
        assm.mov(x64asm::rdi, src);
        // move rdi onto the right stack location
        moveTemp(temp, x64asm::rdi);
        // restore rdi
        Pop(x64asm::rdi);
    }
}

void IrInterpreter::installLocalNone(tempptr_t temp) {
    x64asm::Imm64 src = x64asm::Imm64{(uint64_t)vmPointer->NONE};
    if (temp->reg) {
        assm.mov(temp->reg.value(), src);
    } else {
        // put None in a scratch reg
        Push(x64asm::rdi);
        assm.mov(x64asm::rdi, src);
        moveTemp(temp, x64asm::rdi);
        Pop(x64asm::rdi);
    }
}

void IrInterpreter::installLocalRefVar(tempptr_t temp, uint32_t localIdx) {
    x64asm::R64 reg = x64asm::rdi;
    bool usedScratch = false;
    if (temp->reg) {
        reg = temp->reg.value();
    } else {
        // use rdi as scratch space
        Push(reg);
        usedScratch = true;
    }
    // move the value to the target reg if applicable else a scratch reg
    assm.mov(
         reg,
         x64asm::M64{x64asm::rdi, x64asm::Imm32{8*localIdx}}
    );
    // convert to a valwrapper
    vector<tempptr_t> noTempArgs;
    vector<x64asm::Imm64> args = {
        x64asm::Imm64{vmPointer},
    };
    tempptr_t returnTemp = temp;
    // this will put the result back inside the right temp
    callHelper((void*) &(helper_new_valwrapper), args, noTempArgs, reg, returnTemp);

    // cleanup
    if (usedScratch) {
        // restore rdi
        Pop(reg);
    }
}

void IrInterpreter::installLocalRefNone(tempptr_t temp) {
    // convert to ValWrapper
    vector<tempptr_t> noTempArgs;
    vector<x64asm::Imm64> args = {
        x64asm::Imm64{vmPointer},
        x64asm::Imm64{(uint64_t)vmPointer->NONE},
    };
    tempptr_t returnTemp = temp;
    // this will put the result back inside the right temp
    callHelper((void*) &(helper_new_valwrapper), args, noTempArgs, returnTemp);
}

/************************
 * REG ALLOCATION HELPERS
 ***********************/

x64asm::R64 IrInterpreter::getScratchReg() {
    x64asm::R64 toSpill = x64asm::r10;
    Push(toSpill);
    regPopCount += 1; // metadata to check for errors
    LOG("getting scratch reg: " + asmRegToString(toSpill));
    return toSpill;
};

void IrInterpreter::returnScratchReg(x64asm::R64 reg) {
    Pop(reg);
    regPopCount -= 1;
}

/************************
 * TEMP HELPERS
 ***********************/
void IrInterpreter::moveTemp(x64asm::R64 dest, tempptr_t src) {
    moveTemp(dest, src, TempOp::MOVE);
}

void IrInterpreter::moveTemp(tempptr_t dest, tempptr_t src) {
    moveTemp(dest, src, TempOp::MOVE);
}

void IrInterpreter::moveTemp(tempptr_t dest, x64asm::R64 src) {
    if (dest->reg) {
        assm.mov(dest->reg.value(), src);
    } else {
        // dest in mem, src is a reg
        uint32_t destOffset = getTempOffset(dest);
        assm.mov(
            x64asm::M64{
                x64asm::rbp,
                x64asm::Imm32{-destOffset}
            },
            src
        );
    }
}

void IrInterpreter::moveTemp(x64asm::R64 dest, tempptr_t src, TempOp tempOp) {
    x64asm::Opcode op;
    if (src->reg) {
        switch (tempOp) {
            case TempOp::MOVE:
                op = x64asm::MOV_R64_R64;
                break;
            case TempOp::SUB:
                op = x64asm::SUB_R64_R64;
                break;
            case TempOp::MUL:
                op = x64asm::IMUL_R64_R64;
                break;
            case TempOp::CMP:
                op = x64asm::CMP_R64_R64;
                break;
            case TempOp::AND:
                op = x64asm::AND_R64_R64;
                break;
            case TempOp::OR:
                op = x64asm::OR_R64_R64;
                break;
        }
        assm.assemble({op, {dest, src->reg.value()}});
    } else {
        uint32_t offset = getTempOffset(src);
        switch (tempOp) {
            case TempOp::MOVE:
                op = x64asm::MOV_R64_M64;
                break;
            case TempOp::SUB:
                op = x64asm::SUB_R64_M64;
                break;
            case TempOp::MUL:
                op = x64asm::IMUL_R64_M64;
                break;
            case TempOp::CMP:
                op = x64asm::CMP_R64_M64;
                break;
            case TempOp::AND:
                op = x64asm::AND_R64_M64;
                break;
             case TempOp::OR:
                op = x64asm::OR_R64_M64;
                break;
        }
        assm.assemble({op, {
            dest,
            x64asm::M64{
                x64asm::rbp,
                x64asm::Imm32{-offset}
            }
        }});
    }
}

void IrInterpreter::moveTemp(tempptr_t dest, tempptr_t src, TempOp tempOp) {
    if (dest->reg) {
        moveTemp(dest->reg.value(), src, tempOp);
    } else {
        x64asm::Opcode op;
        switch (tempOp) {
            case TempOp::MOVE:
                op = x64asm::MOV_M64_R64;
                break;
            case TempOp::SUB:
                op = x64asm::SUB_M64_R64;
                break;
            case TempOp::MUL:
                // multiplication can only do R->M, reverse order of args.
                assert (false);
                break;
            case TempOp::CMP:
                op = x64asm::CMP_M64_R64;
                break;
            case TempOp::AND:
                op = x64asm::AND_M64_R64;
                break;
            case TempOp::OR:
                op = x64asm::OR_M64_R64;
                break;
        }

        if (src->reg) {
           // dest is in memory, src in a reg
            uint32_t offset = getTempOffset(dest);
            assm.assemble({op, {
                x64asm::M64{
                    x64asm::rbp,
                    x64asm::Imm32{-offset}
                },
                src->reg.value()
            }});
        } else { // both are in memory :'(
            // we need a scratch reg
            x64asm::R64 reg = getScratchReg();
            // move src into the reg
            uint32_t srcOffset = getTempOffset(src);
            assm.mov(
                reg,
                x64asm::M64{
                    x64asm::rbp,
                    x64asm::Imm32{-srcOffset}
                }
            );
            // move the reg into dest
            uint32_t destOffset = getTempOffset(dest);
            assm.assemble({op, {
                x64asm::M64{
                    x64asm::rbp,
                    x64asm::Imm32{-destOffset}
                },
                reg
            }});
            // give back the reg
            returnScratchReg(reg);
        }
    }
}

void IrInterpreter::moveTemp(x64asm::R32 dest, tempptr_t src) {
    moveTemp(dest, src, TempOp::MOVE);
}

void IrInterpreter::moveTemp(tempptr_t dest, x64asm::R32 src) {
    if (dest->reg) {
        assm.mov(getRegBottomHalf(dest->reg.value()), src);
    } else {
        // dest in mem, src is a reg
        uint32_t destOffset = getTempOffset(dest);
        assm.mov(
            x64asm::M32{
                x64asm::rbp,
                x64asm::Imm32{-destOffset}
            },
            src
        );
    }
}

void IrInterpreter::moveTemp(x64asm::R32 dest, tempptr_t src, TempOp tempOp) {
    x64asm::Opcode op;
    if (src->reg) {
        switch (tempOp) {
            case TempOp::MOVE:
                op = x64asm::MOV_R32_R32;
                break;
            case TempOp::SUB:
                op = x64asm::SUB_R32_R32;
                break;
            case TempOp::MUL:
                op = x64asm::IMUL_R32_R32;
                break;
            case TempOp::CMP:
                op = x64asm::CMP_R32_R32;
                break;
            case TempOp::AND:
                op = x64asm::AND_R32_R32;
                break;
            case TempOp::OR:
                op = x64asm::OR_R32_R32;
                break;
        }
        assm.assemble({op, {dest, getRegBottomHalf(src->reg.value())}});
    } else {
        uint32_t offset = getTempOffset(src);
        switch (tempOp) {
            case TempOp::MOVE:
                op = x64asm::MOV_R32_M32;
                break;
            case TempOp::SUB:
                op = x64asm::SUB_R32_M32;
                break;
            case TempOp::MUL:
                op = x64asm::IMUL_R32_M32;
                break;
            case TempOp::CMP:
                op = x64asm::CMP_R32_M32;
                break;
            case TempOp::AND:
                op = x64asm::AND_R32_M32;
                break;
             case TempOp::OR:
                op = x64asm::OR_R32_M32;
                break;
        }
        assm.assemble({op, {
            dest,
            x64asm::M32{
                x64asm::rbp,
                x64asm::Imm32{-offset}
            }
        }});
    }
}

void IrInterpreter::moveTemp32(tempptr_t dest, tempptr_t src) {
    moveTemp32(dest, src, TempOp::MOVE);
}

void IrInterpreter::moveTemp32(tempptr_t dest, tempptr_t src, TempOp tempOp) {
    if (dest->reg) {
        moveTemp(getRegBottomHalf(dest->reg.value()), src, tempOp);
    } else {
        x64asm::Opcode op;
        switch (tempOp) {
            case TempOp::MOVE:
                op = x64asm::MOV_M32_R32;
                break;
            case TempOp::SUB:
                op = x64asm::SUB_M32_R32;
                break;
            case TempOp::MUL:
                // multiplication can only do R->M, reverse order of args.
                assert (false);
                break;
            case TempOp::CMP:
                op = x64asm::CMP_M32_R32;
                break;
            case TempOp::AND:
                op = x64asm::AND_M32_R32;
                break;
            case TempOp::OR:
                op = x64asm::OR_M32_R32;
                break;
        }

        if (src->reg) {
           // dest is in memory, src in a reg
            uint32_t offset = getTempOffset(dest);
            assm.assemble({op, {
                x64asm::M32{
                    x64asm::rbp,
                    x64asm::Imm32{-offset}
                },
                getRegBottomHalf(src->reg.value())
            }});
        } else { // both are in memory :'(
            // we need a scratch reg
            x64asm::R64 scratch = getScratchReg();
			x64asm::R32 reg = getRegBottomHalf(scratch);	
            // move src into the reg
            uint32_t srcOffset = getTempOffset(src);
            assm.mov(
                reg,
                x64asm::M32{
                    x64asm::rbp,
                    x64asm::Imm32{-srcOffset}
                }
            );
            // move the reg into dest
            uint32_t destOffset = getTempOffset(dest);
            assm.assemble({op, {
                x64asm::M32{
                    x64asm::rbp,
                    x64asm::Imm32{-destOffset}
                },
                reg
            }});
            // give back the reg
            returnScratchReg(scratch);
        }
    }
}


/************************
 * ASSM HELPERS
 ***********************/
x64asm::R32 IrInterpreter::getRegBottomHalf(x64asm::R64 reg) {
    switch (reg) {
        case x64asm::rdi:
        {
            return x64asm::edi;
        }
        case x64asm::rsi:
        {
            return x64asm::esi;
        }
        case x64asm::rax:
        {
            return x64asm::eax;
        }
        case x64asm::rbx:
        {
            return x64asm::ebx;
        }
        case x64asm::rcx:
        {
            return x64asm::ecx;
        }
        case x64asm::rdx:
        {
            return x64asm::edx;
        }
        case x64asm::r8:
        {
            return x64asm::r8d;
        }
        case x64asm::r9:
        {
            return x64asm::r9d;
        }
        case x64asm::r10:
        {
            return x64asm::r10d;
        }
        case x64asm::r11:
        {
            return x64asm::r11d;
        }
        case x64asm::r12:
        {
            return x64asm::r12d;
        }
        case x64asm::r13:
        {
            return x64asm::r13d;
        }
        case x64asm::r14:
        {
            return x64asm::r14d;
        }
        case x64asm::r15:
        {
            return x64asm::r15d;
        }
        default:
            throw RuntimeException("reg does not have bottom half");
    }
};
const x64asm::R64 IrInterpreter::callerSavedRegs[] = {
    // the first 6 are the arg regs
    x64asm::rdi,
    x64asm::rsi,
    x64asm::rdx,
    x64asm::rcx,
    x64asm::r8,
    x64asm::r9,
    // end arg regs
    x64asm::r10,
    x64asm::r11,
    x64asm::rax
};
const x64asm::R64 IrInterpreter::calleeSavedRegs[] = {
    // rbp and rsp are handled a little differently, so will not consider
    // callee-saved
    x64asm::rbx,
    x64asm::r12,
    x64asm::r13,
    x64asm::r14,
    x64asm::r15
};
