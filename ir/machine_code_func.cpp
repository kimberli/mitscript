#include "include/x64asm.h"
#include "../vm/types.h"

/**
* This class is to help you get started with jumping into machine code from your VM interpreter
* x64asm naturally supports calling function with up to six parameters (see other examples 
* in  this file). However, it does not naturally support calling functions with more than 
* six parameters.  This function implements calling functions with more than six parameters 
* as you may need to support for MITScript
*
* This is an illustrative example. You may be able to change this implementation to achieve
* better performance
*/

class MachineCodeFunction
{
public:
  MachineCodeFunction(const size_t parameter_count, x64asm::Function body)
      : parameter_count_(parameter_count),
        body_(body),
        compiled_(false)
  {
  }

  void compile()
  {
    // Number of parameters passed in registers
    const static size_t NUM_PARAMETER_REGS = 6;

    // Parameter Registers according to SysV calling convention
    const static x64asm::R64 registers[] = {x64asm::rdi, x64asm::rsi, x64asm::rdx, x64asm::rcx, x64asm::r8, x64asm::r9};

    // Should be able to use static_assert instead
    assert((sizeof(registers) / sizeof(x64asm::R64)) == NUM_PARAMETER_REGS);

    assert(!compiled_);

    // Allocate the parameter buffer
    buffer_.resize(parameter_count_);

    // Create an assembler and a function to compile code to.
    x64asm::Assembler assm;

    // Generate trampoline to handle non-static number of arguments
    assm.start(trampoline_);

    // At this point the stack is 8-byte aligned.
    // However, we need it to be 16-byte aligned for the call instruction
    // If we push an even number of arguments on the stack
    // or push none at all, then we need to adjust the stack
    // by 8 bytes
    bool pushed_alignment = false;

    if ((parameter_count_ <= NUM_PARAMETER_REGS) || ((parameter_count_ % 2) == 0))
    {
      assm.sub(x64asm::rsp, x64asm::Imm8{8});

      pushed_alignment = true;
    }

    // Push arguments 7-n in reverse order
    for (size_t i = 6; i < parameter_count_; ++i)
    {
      // Load address of parameter buffer slot for this value
      assm.assemble({x64asm::MOV_R64_IMM64, {x64asm::rax, x64asm::Imm64{&buffer_[parameter_count_ - (i - NUM_PARAMETER_REGS) - 1]}}});

      // Load parameter value from buffer
      assm.assemble({x64asm::MOV_R64_M64, {x64asm::rax, x64asm::M64{x64asm::rax}}});

      // Push on stack
      assm.push(x64asm::rax);
    }

    // Pass arguments 1 to 6
    for (size_t i = 0; i < parameter_count_ && i < NUM_PARAMETER_REGS; ++i)
    {
      // Load address of parameter buffer slot for this value
      // Be careful to use a register (rax) that is not in the set of argument registers
      assm.assemble({x64asm::MOV_R64_IMM64, {x64asm::rax, x64asm::Imm64{&buffer_[i]}}});

      // Load parameter value from buffer
      assm.assemble({x64asm::MOV_R64_M64, {registers[i], x64asm::M64{x64asm::rax}}});
    }

    // Call body of function
    assm.assemble({x64asm::MOV_R64_IMM64, {x64asm::rax, x64asm::Imm64{body_}}});

    assm.call(x64asm::rax);

    // Pop arguments 7-n off stack
    for (size_t i = parameter_count_; NUM_PARAMETER_REGS < i; --i)
    {
      // Pop stack into unused register
      assm.pop(x64asm::rdx);
    }

    if (pushed_alignment)
    {
      assm.add(x64asm::rsp, x64asm::Imm8{8});
    }

    assm.ret();

    assm.finish();

    compiled_ = true;
  }

  Value *call(const std::vector<Value *> args)
  {
    assert(compiled_);
    assert(args.size() == parameter_count_);

    // copy contents into buffer
    std::copy(args.begin(), args.end(), buffer_.begin());

    Value *result = trampoline_.call<Value *>();

    return result;
  }

private:
  bool compiled_;
  std::vector<Value *> buffer_;
  size_t parameter_count_;

  x64asm::Function trampoline_;
  x64asm::Function body_;
};


