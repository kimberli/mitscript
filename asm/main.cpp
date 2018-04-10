/*
Copyright 2013 eric schkufza

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#define __STDC_FORMAT_MACROS 1

#include <fstream>
#include <iostream>
#include <inttypes.h>

#include "include/x64asm.h"

using namespace std;
using namespace x64asm;

// This example demonstrates three different methods of assembling and
// invoking a toy implementation of the memcpy function in this directory,
// memcpy.s

// Example 1: Read from file
Function from_file()
{
  // Create an assembler.
  Assembler assm;

  // Create a code object and read its contents using the >> operator.
  // In addition to fstreams, this works for all istream types.
  Code c;
  ifstream ifs("x64asm/examples/memcpy.s");
  ifs >> c;

  // Assemble the code and return the result.
  auto result = assm.assemble(c);
  return result.second;
}

// Example 2: Write code using in-memory RTL.
Function from_code()
{
  // Create an assembler.
  Assembler assm;

  // Create code using the in-memory RTL.
  // In addition to the initializer list constructor method shown below,
  // the Code class supports all all STL sequence container operations
  // (ie: resize, find, clear...).
  Code c{
      {XOR_R64_R64, {rcx, rcx}},
      {LABEL_DEFN, {Label{"loop"}}},
      {CMP_R64_R64, {rcx, rdx}},
      {JE_LABEL, {Label{"done"}}},
      {MOV_R8_M8, {al, M8{rsi, rcx, Scale::TIMES_1}}},
      {MOV_M8_R8, {M8{rdi, rcx, Scale::TIMES_1}, al}},
      {INC_R64, {rcx}},
      {JMP_LABEL, {Label{"loop"}}},
      {LABEL_DEFN, {Label{"done"}}},
      {RET}};

  // Assemble the code and return the result.

  auto result = assm.assemble(c);
  return result.second;
}

// Example 3: Use the assembler API.
Function from_api()
{
  // Create an assembler and a function to compile code to.
  Assembler assm;
  Function memcpy;

  // The assembler is stateful, this method specializes it for a function.
  assm.start(memcpy);

  // Instructions are inserted using type-safe API calls.
  // Note that as above, labels can be referenced before being bound.
  assm.xor_(rcx, rcx);
  assm.bind(Label{"loop"});
  assm.cmp(rcx, rdx);
  assm.je(Label{"done"});

  assm.mov(al, M8{rsi, rcx, Scale::TIMES_1});
  assm.mov(M8{rdi, rcx, Scale::TIMES_1}, al);
  assm.inc(rcx);
  assm.jmp(Label{"loop"});

  assm.bind(Label{"done"});
  assm.ret();

  // Finish assembly (ie: patch up label references) and return the result.
  assm.finish();
  return memcpy;
}

// Example 4: Unbounded Number of parameters

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

typedef void Value;

class MachineCodeFunction
{
public:
  MachineCodeFunction(const size_t parameter_count, Function body)
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
    const static x64asm::R64 registers[] = {rdi, rsi, rdx, rcx, r8, r9};

    // Should be able to use static_assert instead
    assert((sizeof(registers) / sizeof(x64asm::R64)) == NUM_PARAMETER_REGS);

    assert(!compiled_);

    // Allocate the parameter buffer
    buffer_.resize(parameter_count_);

    // Create an assembler and a function to compile code to.
    Assembler assm;

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
      assm.sub(rsp, Imm8{8});

      pushed_alignment = true;
    }

    // Push arguments 7-n in reverse order
    for (size_t i = 6; i < parameter_count_; ++i)
    {
      // Load address of parameter buffer slot for this value
      assm.assemble({MOV_R64_IMM64, {rax, Imm64{&buffer_[parameter_count_ - (i - NUM_PARAMETER_REGS) - 1]}}});

      // Load parameter value from buffer
      assm.assemble({MOV_R64_M64, {rax, M64{rax}}});

      // Push on stack
      assm.push(rax);
    }

    // Pass arguments 1 to 6
    for (size_t i = 0; i < parameter_count_ && i < NUM_PARAMETER_REGS; ++i)
    {
      // Load address of parameter buffer slot for this value
      // Be careful to use a register (rax) that is not in the set of argument registers
      assm.assemble({MOV_R64_IMM64, {rax, Imm64{&buffer_[i]}}});

      // Load parameter value from buffer
      assm.assemble({MOV_R64_M64, {registers[i], M64{rax}}});
    }

    // Call body of function
    assm.assemble({MOV_R64_IMM64, {rax, Imm64{body_}}});

    assm.call(rax);

    // Pop arguments 7-n off stack
    for (size_t i = parameter_count_; NUM_PARAMETER_REGS < i; --i)
    {
      // Pop stack into unused register
      assm.pop(rdx);
    }

    if (pushed_alignment)
    {
      assm.add(rsp, Imm8{8});
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

  Function trampoline_;
  Function body_;
};

// Example: Invokes an assembled version of the memcpy function.
void test(const Function &memcpy)
{
  const char *source = "Hello, world!";
  char *target = new char[32];

  // Functions can be passed to ostream objects to view their hex encoding.
  cout << "Hex source: " << endl;
  cout << memcpy << endl;

  // A function can be called with up to six arguments, each of which must
  // be or be castable to a native type. No explicit cast is necessary.
  memcpy.call<void, char *, const char *, int>(target, source, 14);
  cout << "After return target = \"" << target << "\"" << endl;
  cout << endl;

  delete target;
}

void test(MachineCodeFunction &memcpy)
{
  const char *source = "Hello, world!";
  char *target = new char[32];

  std::vector<Value *> args({target, const_cast<char *>(source), (void *)(14)});

  cout << "test4" << endl;

  memcpy.call(args);

  cout << "After return target = \"" << target << "\"" << endl;
  cout << endl;

  delete target;
}

void print6(int64_t A, int64_t B, int64_t C, int64_t D, int64_t E, int64_t F)
{
  printf("received: ");

  cout << A << " ";
  cout << B << " ";
  cout << C << " ";
  cout << D << " ";
  cout << E << " ";
  cout << F << "\n";
}

void print7(int64_t A, int64_t B, int64_t C, int64_t D, int64_t E, int64_t F, int64_t G)
{
  printf("received: ");

  cout << A << " ";
  cout << B << " ";
  cout << C << " ";
  cout << D << " ";
  cout << E << " ";
  cout << F << " ";
  cout << G << "\n";
}

void print8(int64_t A, int64_t B, int64_t C, int64_t D, int64_t E, int64_t F, int64_t G, int64_t H)
{
  printf("received: ");

  cout << A << " ";
  cout << B << " ";
  cout << C << " ";
  cout << D << " ";
  cout << E << " ";
  cout << F << " ";
  cout << G << " ";
  cout << H << "\n";
}

void testArgs()
{
  const std::vector<Value *> testInputs[] =
  {
          {(Value *)1, (Value *)2, (Value *)3, (Value *)4, (Value *)5, (Value *)6},
          {(Value *)1, (Value *)2, (Value *)3, (Value *)4, (Value *)5, (Value *)6, (Value *)7},
          {(Value *)1, (Value *)2, (Value *)3, (Value *)4, (Value *)5, (Value *)6, (Value *)7, (Value *)8}
  };

  void* testFunctions[] ={(void *)&print6, (void *)&print7, (void *)&print8};

  for (size_t testId = 0; testId < 3; ++testId)
  {
    Function printAsm;

    Assembler assm;

    assm.start(printAsm);

    assm.assemble({MOV_R64_IMM64, {rax, Imm64{testFunctions[testId]}}});

    assm.jmp(rax);

    assm.finish();

    auto &args = testInputs[testId];

    printf("expected: ");
    for (Value *arg : args)
    {
      cout << (int64_t)arg << " ";
    }
    printf("\n");

    MachineCodeFunction testFunction(args.size(), printAsm);
    testFunction.compile();

    testFunction.call(args);

    printf("\n");
  }
}

int main()
{
  Function f1 = from_file();
  test(f1);

  Function f2 = from_code();
  test(f2);

  Function f3 = from_api();
  test(f3);

  MachineCodeFunction f4(3, f3);
  f4.compile();
  test(f4);

  testArgs();

  return 0;
}
