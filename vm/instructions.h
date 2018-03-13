#pragma once

#include <experimental/optional>
#include <vector>

enum class Operation
{
    // Description: push a constant onto the operand stack
    // Operand 0: index of constant in enclosing function's list of constants 
    // Mnemonic:  load_const i
    // Stack:      S => f.constants()[i] :: S 
    LoadConst,

    // Description: push a  function onto the operand stack
    // Operand 0: index of function  in enclosing function's list of functions 
    // Mnemonic:  load_func i
    // Stack:      S => f.functions()[i] :: S 
    LoadFunc,
   
    // Description: load value of local variable and push onto operand stack
    // Operand 0: index of local variable to read
    // Mnemonic: load_local i
    // S => S :: value_of(f.local_vars[i]) 
    LoadLocal,

    // Description: store top of operand stack into local variable
    // Operand 0: index of local variable to store into
    // Operand 1: value to store
    // Mnemonic:  store_local i
    // Stack:     S :: operand 1==> S
    StoreLocal,

    // Description: load value of global variable
    // Operand 0: index of name of global variable in enclosing function's names list
    // Mnemonic:  load_global i
    // Stack:     S => global_value_of(f.names[i]) :: S
    LoadGlobal,

    // Description: store value into global variable
    // Operand 0: index of name of global variable in enclosing function's names list
    // Operand 1: value to store
    // Mnemonic:  store_global i
    // Stack:     S :: operand 1 ==> S
    StoreGlobal,

    // Description: push a reference to a local variable or free variable reference onto the operand stack
    // Operand 0: index of local variable reference 
    // Mnemonic:  push_ref i
    // Stack:     S ==>  address_of(var) :: S
    //            wehere var = i < f.local_reference_vars.size() ? f.local_reference_vars[i] 
    //                                                           :  f.free_vars[i - f.local_reference_vars.size()]
    PushReference,

    // Description: loads the value of a reference onto the operand stack
    // Operand 0: N/A
    // Operand 1: reference to load from
    // Mnemonic:  load_ref
    // Stack:     S :: operand 1 => S :: value_of(operand 1)
    LoadReference,

    // Description: loads the value of a reference onto the operand stack
    // Operand 0: N/A
    // Operand 1: value to store
    // Operand 2: reference to store to
    // Mnemonic:  load_ref
    // Stack:     S :: operand 2 :: operand 1 => S
    StoreReference,
    
    // Description: allocates a record and pushes it on the operand stack
    // Operand 0: N/A
    // Mnemonic:  alloc_record
    // Stack:     S => S :: record
    AllocRecord,

    // Description: load value of field from record
    // Operand 0: index of the field's name within the enclosing function's names list
    // Operand 1: record from which to load
    // Mnemonic: field_load i
    // Stack:     S :: operand 1 => S :: record_value_of(operand, f.names[i])
    FieldLoad,

    // Description: store value into field of record
    // Operand 0: index of the field's name within the enclosing function's names list
    // Operand 1: the value to store
    // Operand 2: the record to store into  
    // Mnemonic: field_store i
    // Stack:    S :: operand 2 :: operand 1 => S
    FieldStore,

    // Description: load value from index of record 
    // Operand 0: N/A
    // Operand 1: the index to read from (can be arbitrary value. indexing adheres to semantics of Assignment #2)
    // Operand 2: the record to read from
    // Stack:     S :: operand 2 :: operand 1 => S
    IndexLoad,

    // Description: store value into index of record
    // Operand 0: N/A
    // Operand 1: the value to store
    // Operand 2: the index to store to (can be arbitrary value. indexing adheres to semantics of Assignment #2)
    // Operand 3: the record to store into
    // Stack:     S :: operand 3 :: operand 2 :: operand 1 => S
    IndexStore,
    
    // Description: allocate a closure
    // Operand 0:       N/A
    // Operand 1:       function
    // Operand 2:       the number of free variable references passed to the closure
    // Operand 3:  - N: references to the function's free variables
    // Mnemonic:   alloc_closure
    // Stack:      S :: operand n :: ... :: operand 3 :: operand 2 :: operand 1 => S :: closure 
    AllocClosure, 
   
    // Description: call a closure
    // Operand 0:     N/A
    // Operand 1:     closure to call (closure reference)
    // Operand 2:     number of arguments
    // Operand 3 - N: argument ((N - 3) - i)
    // Mnemonic:      call
    // Stack:         S::operand n :: .. :: operand 3 :: operand 2 :: operand 1 => S :: value
    Call,

    // Description: ends the execution of the enclosing function and returns the top of the stack
    // Operand 0:   N/A
    // Operand 1:   value to return
    // Mnemonic:    return
    // Stack::      S :: operand 1 => S
    Return,

    // Description: implements addition (as given in the semantics of Assignment #2)
    // Operand 0: N/A
    // Operand 1: right value 
    // Operand 2: left value
    // Result:    value of the operation as specified by the semantics of Assignment #2
    // Mnemonic:  sub/mul/div
    // Stack:     S:: operand 2 :: operand 1 => S :: op(operand 2, operand 1)
   
    Add,

    // Description: performs an arithmetic operation on two integer operands
    // Operand 0: N/A
    // Operand 1: right value 
    // Operand 2: left value
    // Mnemonic:  sub/mul/div
    // Stack:     S:: operand 2 :: operand 1 => S :: op(operand 2, operand 1)
    Sub,
    Mul,
    Div,

    // Description: computes the unary minus of the integer operation
    // Operand 0: N/A
    // Operand 1: value 
    // Mnemonic:  neg
    // Stack:     S :: operand 1 => S:: - operand 1
    Neg,


    // Description: computes a comparison operation on two integer operands
    // Operand 0: N/A
    // Operand 1: right value 
    // Operand 2: left value
    // Mnemonic:  gt/geq
    // Stack:     S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
    Gt,
    Geq,


    // Description: computes an equality between two values (semantics according to Assignment #2)
    // Operand 0: N/A
    // Operand 1: right value 
    // Operand 2: left value
    // Mnemonic:  gt/geq
    // Stack:     S :: operand 2 :: operand 1 => S:: eq(operand 2, operand 1)
    Eq,

    // Description: computes a boolean operation on two boolean operands
    // Operand 0: N/A
    // Operand 1: right value 
    // Operand 2: left value
    // Mnemonic:  and/or
    // Stack:     S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
    And,
    Or,
    
    // Description: computes the logical negation of a boolean operand
    // Operand 0: N/A
    // Operand 1: value
    // Mnemonic:  and/or
    // Stack:     S :: operand 1 => S:: op(operand 1)
    Not,
    
    
    // Description: transfers execution of the function to a new instruction offset within the current function
    // Example: goto 0 jumps to the current instruction. goto 1 jumps to the following instruction. goto -1 jumps to the preceeding instruction
    // Operand 0: offset relative to the current instruction offset to jump to.
    // Mnemonic:  goto i
    // Stack:     S => S
    Goto,

    // Description: transfers execution of the function to a new instruction offset within the current function if the operand evaluates to true
    // Operand 0: offset relative to the current instruction offset to jump to.
    // Operand 1: value
    // Mnemonic:  if i
    // Stack:     S :: operand 1 => S
    If,

    // Description: duplicates the element at the top of the stack. 
    // If this element is a reference to a record, function, or local variable, the operation only depulicates the reference
    // Operand 0: N/A
    // Operand 1: value
    // Mnemonic:  dup
    // Stack:     S :: operand 1 => S :: operand 1 :: operand 1
    Dup,

    // Description: swaps the two values at the top of the stack
    // Operand 0: N/A
    // Operand 1: a value
    // Operand 2: a value
    // Mnemonic:  swap
    // Stack:     S :: operand 2 :: operand 1 => S :: operand 1 :: operand 2
    Swap,

    // Description: pops and discards the top of the stack 
    // Operand 0: N/A
    // Operand 1: a value
    // Mnemonic:  swap
    // Stack:     S :: operand 1 => S
    Pop
};


struct Instruction
{   
    Instruction(const Operation operation, std::experimental::optional<int32_t> operand0) 
    : operation(operation),
    operand0(operand0)
    {

    }

    Operation operation;
    std::experimental::optional<int32_t> operand0;
};

typedef std::vector<Instruction> InstructionList;
