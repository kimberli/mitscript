
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include "include/x64asm.h"

using namespace std;
using namespace x64asm;

// Example Coden from class using x64asm

typedef void Value;

class dummy_function
{

public:
  dummy_function()
  {
    // Allocate buffer of constants
    constants.resize(10, 0);

    buffer_ = constants.data();
  }

  Value **buffer_;

private:
  std::vector<Value *> constants;
};

void assert_integer(Value *v)
{
  // check that value is integer using C++
}

void new_integer(int32_t v)
{
  // allocate new integer in C++
}

int main()
{
  Function test;

  dummy_function fn;

  // Create an assembler and a function to compile code to.
  Assembler assm;

  // Generate trampoline to handle non-static number of arguments
  assm.start(test);

  assm.push(rbp);
  assm.mov(rbp, rsp);
  assm.sub(rsp, Imm8(8));

  // Load y
  assm.push(rdi);
  
  // Load 2nd constant
  assm.mov(rdx, Imm64(&fn.buffer_[1]));
  assm.mov(rdx, M64(rdx));
  assm.push(rdx);

  // put y into first parameter register
  assm.mov(rdi, M64(rsp, Imm32(8)));

  // get address of assert_integer
  assm.assemble({MOV_R64_IMM64, {rax, Imm64(assert_integer)}});

  // call assert_integer
  assm.call(rax);

  // put constant into first parameter register
  assm.mov(rdi, M64(rsp, Imm32(0)));

  // get address of assert_integer
  assm.assemble({MOV_R64_IMM64, {rax, Imm64(assert_integer)}});

  // call assert_integer
  assm.call(rax);
  
  // pop constant
  assm.pop(rcx);

  // load 4 byte integer at address 
  assm.mov(ecx, M64(rcx));

  // pop y
  assm.pop(rdx);

  // load 4 byte integer at address 
  assm.mov(edx, M64(rdx));

  // move 4 byte value
  assm.sub(edx, ecx);

  // move result into first parameter
  assm.mov(edi, edx);

    // get address of assert_integer
  assm.assemble({MOV_R64_IMM64, {rax, Imm64(new_integer)}});

  // call assert_integer
  assm.call(rax);
  
  // push result onto stack
  assm.push(rax);

  assm.pop(rdx);

 // put result into first local variable
  assm.mov(M64(rbp, Imm32(-8)), rdx);

  // Load 2nd constant
  assm.mov(rdx, Imm64(&fn.buffer_[0]));
  assm.mov(rdx, M64(rdx));
  assm.push(rdx);

  // Set return value
  assm.pop(rax);

  // restore stack pointer
  assm.mov(rsp, rbp);
  
  // restore base pointer
  assm.pop(rbp);

  assm.ret();

  assm.finish();

  return 0;
}
