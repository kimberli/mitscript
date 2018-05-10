# a5 progress

Our design right now looks like:

    MITScript -----> bytecode -------> ir -------> asm
            bc-compiler       bc_to_ir    ir_to_asm

Our bytecode instructions can be found in `instructions.h`, and our IR instructions can be found in `ir.h`.

Key design changes in our bytecode:
* Labels are handled at this level now, so `BcOp::Goto` and `BcOp::If` take in "label indices" (just a global counter that gets incremented for every label) as their `op0`s. We added a new BC instruction called `BcOp::Label` which serves as the anchor to make the label at in the assembly layer.
* We also added a new field to `Function` called `labels_`, which maps these label indices to bytecode instruction indices inside the function. Then, when we encounter a `BcOp::Goto` or `BcOp::If` in the `Interpreter`'s `executeStep` function (when we choose NOT to dispatch to assembly), we set the instruction pointer to the value mapped by the BC instruction's `op0`.

Key design choices in our IR (basically a superset of BC instructions):
* We handle vanilla local vars and local ref vars differently. When we see a `LoadLocal` bytecode instruction, for example, we check to see whether the local we're trying to load is a vanilla local or a local ref. If it's the latter, we generate a `LoadLocalRef` IR instruction which, instead of putting a different `Constant` into a local var location on the stack, changes the value pointed to by the `ValWrapper` at that local var location on the stack.
* The IR instruction `PushLocalRef` takes in a local var ref that's already a `ValWrapper` and stores it into a temp, while `PushFreeRef` takes in a ref that's already a `ValWrapper` and stores it into a temp.
* We kept distinct `FieldLoad` and `IndexLoad` instructions for records--one takes in a `string` stored inside the `IrInstruction` object, and the other has its field as a `Constant` in a temp that it refers to.

Key design choices in our code generation:
* Temps are a generic notion for a location that `Value`s and primitives can be stored to. Right now, they are all stored as pointers on the stack, but later when we implement register allocation these can be registers as well.
* We have helper methods for assembly function prologues (and epilogues) that handle adjusting `rbp`/`rsp`, reserving space for locals/local ref vars, initializing locals to `None`, and saving/restoring callee-saved regs.
* We wrote a helper function that takes in lists of immediate values, temps, and an optional register, puts these as arguments in the appropriate locations, and calls a C++ helper function adhering to the calling convention.
* Code generation for the following IR instructions uses C++ helper functions:
  * `StoreGlobal` & `LoadGlobal`
  * `StoreLocalRef`
  * `Add` & `Eq`
  * `AllocClosure` and `Call`
  * `GarbageCollect`
  * checking for 0 when dividing
  * asserting, unboxing, reboxing, and casting types
  * getting and setting record fields

Bugs we've found and fixed:
* a bytecode generation bug where we have a variable that's both a global and an argument to the function
* handling implicit `return None`s at the end of a function
* catch exceptions thrown from helpers while we're in the assembly-running phase and print them gracefully
* allocating space for assembly instructions with `reserve()` (this could be done better; see remaining tasks)
* `jmp`/`je` bug to labels in the past (i.e. negative offsets)--we fixed this by redesigning labels to be included at the bytecode level
* ref var issues where we were passing in the underlying value when allocating the closure instead of the wrapper around the value--we fixed this by redesigning the way we handle ref vars
* checking for 0 with a helper when dividing to throw `IllegalArithmeticExceptions`
* binary comparisions (e.g. >, >=) with negative numbers--we were storing in 64-bit registers instead of 32-bit registers when our `Integer`s have 32-bit values
* and many more...

Remaining tasks to be tackled:
* debug a few remaining failing tests
* fix the bytecode parser to handle the `labels_` field OR rewrite our bytecode unit tests in MITScript and remove bytecode input altogether from our binary
* change the `reserve()` heuristic to be more accurate and performant
* figure out when to call the `GarbageCollect` IR instruction
* optimizations!

Our plan for optimization is to implement register allocation, then maybe other add-ons like constant propagation, generational garbage collection, selective assembly generation.
