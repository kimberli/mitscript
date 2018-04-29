# pragma once

#include <experimental/optional>
#include "../vm/types.h"

struct IrInstruction;
struct IrFunc;

typedef std::vector<IrInstruction> IrInstList;
typedef std::experimental::optional<int32_t> optint_t;
typedef std::vector<int32_t> TempList;

enum class IrOp {
    // Description: Move a constant from constants array into temp var
    // op0: index into the constants array of the constant to move
    // temp0: number of the temp to store the constant in
    // Result: temp0 stores func.constants_[operand1]
    LoadConst,

    // Description: Load a function from functions list into temp
    // op0: index into functions array to move
    // temp0: number of the temp to store the funtion in
    // Result: temp0 stores the specified function
    LoadFunc,

    // Description: Load a local into a temp
    // op0: index of the local to load
    // temp0: index of the temp to store into
    // Result: temp0 stores the desired local
    LoadLocal,

    // Description: Store a value from a temp into a local
    // op0: index of the local to store into
    // temp0: index of the temp containing the value to store
    // Result: the local at index operand2 contains the value stored by operand1
    StoreLocal,

    // Description: Load a global into a temp
    // op0: index of the global to load
    // temp0: index of the temp to load into
    // Result: temp0 contains the global var at index operand1
    LoadGlobal,

    // Description: Store a value from a temp into a global
    // temp0: index of the temp containing the value to store
    // op0: index of the global to store into
    // Result: the global at index operand2 contains the value stored by operand1
    StoreGlobal,

    // Description: Allocate a record and store it to a temp
    // op0: N/A
    // temp0: the index of the temp to store the record into
    // Result: the temp0 contains a new empty record
    AllocRecord,

    // Description: Load a field from a record and store it into a temp
    // op0: N/A
    // temp0: the index of the temp to store the field value into
    // temp1: index of the temp containing the record to look in
    // temp2: index of the temp containing the field name
    // Result: the temp0 contains the value of the field
    FieldLoad,

    // Description: Store a value into the field of a record
    // op0: N/A
    // temp0: the index of the temp containing the record to store into
    // temp1: index of the temp containing the field name
    // temp2: index of the temp containing the field value
    // Result: the temp0 record now has the new field set
    FieldStore,

    // Description: Load an index from a record and store it into a temp
    // temp0: the index of the temp to store the index value into
    // temp1: index of the temp containing the record to look in
    // temp2: index of the temp containing the index
    // Result: the temp0 contains the value of the index
    IndexLoad,

    // Description: Store a value into the index of a record
    // temp0: the index of the temp containing the record to store into
    // temp1: index of the temp containing the index
    // temp2: index of the temp containing the index's new value
    // Result: the temp0 record now has the new index set
    IndexStore,

    // Description: Allocate a new closure
    // op0: Number of free vars passed to the closure
    // temp0: index of temp containing the new closure
    // temps 1-m: Contains references to the free vars we are going to pass
    // Result: the temp0 now contains the new closure
    AllocClosure,

    // Description: call a closure
    // op0: Number of args passed to the function
    // temp0: the index of the temp to contain the return val
    // temp1: index of temp storing the closure to call
    // temp 2-m: Contains values of the args we are passing
    // Result: the temp0 contains the result of calling the function
    Call,

    // Description: return from a function
    // temp0: index of temp containing return value
    // Result: return from the function
    Return,

    // Description: add the constants in two temps
    // temp0: number of the temp to store the value in
    // temp1: temp index holding right value
    // temp2: temp index holding left value
    // Result: temp0 stores the result of doing operand1 + operand2
    Add,

    // Description: performs arithmetic operation on two temps
    // temp0: number of the temp to store the value in
    // temp1: temp index holding right value
    // temp2: temp index holding left value
    // Result: temp0 stores the result of doing op(operand2, operand1)
    Sub,
    Mul,
    Div,

    // Description: computes unary minus
    // temp0: number of the temp to store the result in
    // temp1: value to negate
    // Result: temp0 stores -operand1
    Neg,

    // Description: computes a comparison on ints
    // temp1: right value
    // temp2: left value
    // temp0: number of the temp to store the result in
    // Result: temp0 stores bool indicating result of comparison
    Gt,
    Geq,

    // Description: computes an equality between two vals (semantics from A2)
    // temp1: right value
    // temp2: left value
    // temp0: number of the temp to store the result in
    // Result: temp0 stores bool eq(operand2, operand1)
    Eq,

    // Description: computes a boolean operation (semantics from A2)
    // temp1: right value
    // temp2: left value
    // temp0: number of the temp to store the result in
    // Result: temp0 stores bool op(operand1, operand2)
    And,
    Or,

    // Description: computes local negation
    // temp1: value to negate
    // temp0: number of the temp to store the result in
    // Result: temp0 stores !operand1
    Not,

    // Description: move to a given label
    // op0: index of the label to jump to; should be unique
    // Result: execution transfers to the label
    Goto,

    // Description: move to a given label
    // op0: index of the label to jump to; should be unique
    // Result: execution transfers to the label
    If,

    // Description: swap the values in two temps
    // temp0: index of one temp to swap
    // temp1: index of the other temp to swap
    // Result: values in the temps are swapped
    Swap,

    // Description: pops and discards the top of the stack
    // Result: top element eliminated from stack
    Pop,

    // Description: asserts that the first temp is an integer
    // temp0: index of value to check
    // Result: throw RuntimeError if the temp is not an integer
    AssertInteger,

    // Description: asserts that the first temp is a bool
    // temp0: index of value to check
    // Result: throw RuntimeError if the temp is not a bool
    AssertBool,

    // Description: asserts that the first temp is a string
    // temp0: index of value to check
    // Result: throw RuntimeError if the temp is not a string
    AssertString,

    // Description: asserts that the first temp is an record
    // temp0: index of value to check
    // Result: throw RuntimeError if the temp is not an record
    AssertRecord,

    // Description: asserts that the first temp is a function
    // temp0: index of value to check
    // Result: throw RuntimeError if the temp is not a function
    AssertFunction,

    // Description: asserts that the first temp is a closure
    // temp0: index of value to check
    // Result: throw RuntimeError if the temp is not a closure
    AssertClosure,

    // Description: casts the first temp to integer
    // temp0: index of value to cast
    // Result: stores integer in temp0
    CastInteger,

    // Description: casts the first temp to bool
    // temp0: index of value to cast
    // Result: stores bool in temp0
    CastBool,

    // Description: casts the first temp to string
    // temp0: index of value to cast
    // Result: stores string in temp0
    CastString,

    // Description: casts the first temp to record
    // temp0: index of value to cast
    // Result: stores record in temp0
    CastRecord,

    // Description: casts the first temp to function
    // temp0: index of value to cast
    // Result: stores function in temp0
    CastFunction,

    // Description: casts the first temp to closure
    // temp0: index of value to cast
    // Result: stores closure in temp0
    CastClosure,
};


// Each instruction contains an optional op0 and an array of temps storing its
// other operands.
// By convention, if the operation stores a value somewhere, it is put in
// temp 0.
struct IrInstruction {
    IrOp operation;
    optint_t op0;
    TempList templist;
    IrInstruction(const IrOp operation, optint_t op0, TempList templist):
        operation(operation),
        op0(op0),
        templist(templist) {};
};

struct IrFunc {
    IrInstList instructions;
    vector<vptr<Constant>> constants_;
    int32_t parameter_count_;
    int32_t local_count_;

    IrFunc(IrInstList instructions,
        vector<vptr<Constant>> constants_,
        int32_t parameter_count_,
        int32_t local_count_) :
        instructions(instructions),
        constants_(constants_),
        parameter_count_(parameter_count_),
        local_count_(local_count_) {};
};

struct IrProgram {
    vector<vptr<IrFunc>> functions;
    IrProgram(vector<vptr<IrFunc>> functions): functions(functions) {};
};
