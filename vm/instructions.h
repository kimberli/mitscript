#pragma once

#include <experimental/optional>
#include <vector>

enum class Operation
{
    // Description: push a constant onto the operand stack
    // Mnemonic:    load_const i
    // Operand 0:   index of constant in enclosing function's list of constants
    // Stack:       S =>  S :: f.constants()[i]
    LoadConst,

    // Description: push a  function onto the operand stack
    // Mnemonic:    load_func i
    // Operand 0:   index of function  in enclosing function's list of functions
    // Stack:       S => S :: f.functions()[i]
    LoadFunc,

    // Description: load value of local variable and push onto operand stack
    // Mnemonic:    load_local i
    // Operand 0:   index of local variable to read
    // Stack:       S => S :: value_of(f.local_vars[i])
    LoadLocal,

    // Description: store top of operand stack into local variable
    // Mnemonic:    store_local i
    // Operand 0:   index of local variable to store into
    // Operand 1:   value to store
    // Stack:       S :: operand 1==> S
    StoreLocal,

    // Description: load value of global variable
    // Mnemonic:    load_global i
    // Operand 0:   index of name of global variable in enclosing function's names list
    // Stack:       S =>  S :: global_value_of(f.names[i])
    LoadGlobal,

    // Description: store value into global variable
    // Mnemonic:    store_global i
    // Operand 0:   index of name of global variable in enclosing function's names list
    // Operand 1:   value to store
    // Stack:       S :: operand 1 ==> S
    StoreGlobal,

    // Description: push a reference to a local variable or free variable reference onto the operand stack
    // Mnemonic:    push_ref i
    // Operand 0:   index of local variable reference
    // Stack:       S ==>  address_of(var) :: S
    //                  where var = i < f.local_reference_vars.size() ? f.local_reference_vars[i]
    //                                                           :  f.free_vars[i - f.local_reference_vars.size()]
    PushReference,

    // Description: loads the value of a reference onto the operand stack
    // Mnemonic:    load_ref
    // Operand 0:   N/A
    // Operand 1:   reference to load from
    // Stack:       S :: operand 1 => S :: value_of(operand 1)
    LoadReference,

    // Description: loads the value of a reference onto the operand stack
    // Mnemonic:    store_ref
    // Operand 0:   N/A
    // Operand 1:   value to store
    // Operand 2:   reference to store to
    // Stack:       S :: operand 2 :: operand 1 => S
    StoreReference,

    // Description: allocates a record and pushes it on the operand stack
    // Mnemonic:    alloc_record
    // Operand 0:   N/A
    // Stack:       S => S :: record
    AllocRecord,

    // Description: load value of field from record
    // Mnemonic:    field_load i
    // Operand 0:   index of the field's name within the enclosing function's names list
    // Operand 1:   record from which to load
    // Stack:       S :: operand 1 => S :: record_value_of(operand, f.names[i])
    FieldLoad,

    // Description: store value into field of record
    // Mnemonic:    field_store i
    // Operand 0:   index of the field's name within the enclosing function's names list
    // Operand 1:   the value to store
    // Operand 2:   the record to store into
    // Stack:       S :: operand 2 :: operand 1 => S
    FieldStore,

    // Description: load value from index of record
    // Mnemonic:    index_load
    // Operand 0:   N/A
    // Operand 1:   the index to read from (can be arbitrary value. indexing adheres to semantics of Assignment #2)
    // Operand 2:   the record to read from
    // Stack:       S :: operand 2 :: operand 1 => S
    IndexLoad,

    // Description: store value into index of record
    // Mnemonic:    index_store
    // Operand 0:   N/A
    // Operand 1:   the value to store
    // Operand 2:   the index to store to (can be arbitrary value. indexing adheres to semantics of Assignment #2)
    // Operand 3:   the record to store into
    // Stack:       S :: operand 3 :: operand 2 :: operand 1 => S
    IndexStore,

    // Description: allocate a closure
    // Mnemonic:    alloc_closure m
    // Operand 0:   the number of free variable references passed to the closure
    // Operand i  (0 < i <= m) : free variable argument (m - i). Argument (m - i) maps to position (m - i)
    // Operand n=m+1 : function
    // Stack:       S :: operand n :: ... :: operand 3 :: operand 2 :: operand 1 => S :: closure
    AllocClosure,

    // Description: call a closure
    // Mnemonic:    call m
    // Operand 0:   number of arguments to the function (m)
    // Operand i  (0 < i <= m) : argument (m - i)
    // Operand n=m+1:   closure to call (closure reference)
    // Stack:       S::operand n :: .. :: operand 3 :: operand 2 :: operand 1 => S :: return value
    Call,

    // Description: ends the execution of the enclosing function and returns the top of the stack
    // Mnemonic:    return
    // Operand 0:   N/A
    // Operand 1:   value to return
    // Stack::      S :: operand 1 => S
    Return,

    // Description: implements addition (as given in the semantics of Assignment #2)
    // Mnemonic:    add
    // Operand 0:   N/A
    // Operand 1:   right value
    // Operand 2:   left value
    // Result:      value of the operation as specified by the semantics of Assignment #2
    // Stack:       S:: operand 2 :: operand 1 => S :: op(operand 2, operand 1)
    Add,

    // Description: performs an arithmetic operation on two integer operands
    // Mnemonic:    sub/mul/div
    // Operand 0:   N/A
    // Operand 1:   right value
    // Operand 2:   left value
    // Stack:       S:: operand 2 :: operand 1 => S :: op(operand 2, operand 1)
    Sub,
    Mul,
    Div,

    // Description: computes the unary minus of the integer operation
    // Mnemonic:    neg
    // Operand 0:   N/A
    // Operand 1:   value
    // Stack:       S :: operand 1 => S:: - operand 1
    Neg,

    // Description: computes a comparison operation on two integer operands
    // Mnemonic:    gt/geq
    // Operand 0:   N/A
    // Operand 1:   right value
    // Operand 2:   left value
    // Stack:       S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
    Gt,
    Geq,

    // Description: computes an equality between two values (semantics according to Assignment #2)
    // Mnemonic:    eq
    // Operand 0:   N/A
    // Operand 1:   right value
    // Operand 2:   left value
    // Stack:       S :: operand 2 :: operand 1 => S:: eq(operand 2, operand 1)
    Eq,

    // Description: computes a boolean operation on two boolean operands (semantics according to Assignment #2)
    // Mnemonic:    and/or
    // Operand 0:   N/A
    // Operand 1:   right value
    // Operand 2:   left value
    // Stack:       S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
    And,
    Or,

    // Description: computes the logical negation of a boolean operand
    // Mnemonic:    not
    // Operand 0:   N/A
    // Operand 1:   value
    // Stack:       S :: operand 1 => S:: op(operand 1)
    Not,

    // Description: transfers execution of the function to a new instruction offset within the current function
    // Mnemonic:    goto i
    // Example: goto 0 jumps to the current instruction. goto 1 jumps to the following instruction. goto -1 jumps to the preceding instruction
    // Operand 0: offset relative to the current instruction offset to jump to.
    // Stack:       S => S
    Goto,

    // Description: transfers execution of the function to a new instruction offset within the current function if the operand evaluates to true.
    // Mnemonic:    if i
    // If operand evaluates to false, then execution continues at the next instruction
    // Operand 0:   offset relative to the current instruction offset to jump to.
    // Operand 1:   value
    // Stack:       S :: operand 1 => S
    If,

    // Description: duplicates the element at the top of the stack.
    // Mnemonic:    dup
    // If this element is a reference to a record, function, or local variable, the operation only duplicates the reference.
    // Operand 0:   N/A
    // Operand 1:   value
    // Stack:       S :: operand 1 => S :: operand 1 :: operand 1
    Dup,

    // Description: swaps the two values at the top of the stack
    // Mnemonic:    swap
    // Operand 0:   N/A
    // Operand 1:   a value
    // Operand 2:   a value
    // Stack:       S :: operand 2 :: operand 1 => S :: operand 1 :: operand 2
    Swap,

    // Description: pops and discards the top of the stack
    // Mnemonic:    pop
    // Operand 0:   N/A
    // Operand 1:   a value
    // Stack:       S :: operand 1 => S
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
