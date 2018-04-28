# pragma once 

#include <experimental/optional>
#include "../vm/types.h"

struct IrInstruction;
struct IrFunc;

typedef std::vector<IrInstruction> IrInstList;
typedef std::experimental::optional<int32_t> optint_t;
typedef std::vector<IrFunc> IrProgram;

enum class IrOp {
   // TODO: finalize operation set
   
   // Description: Move a constant from constants array into temp var
   // Operand 1: index into the constants array of the constant to move
   // Operand 2: N/A
   // target: number of the temp to store the constant in 
   // Result: target stores func.constants_[operand1]
   LoadConst,

   // Description: Load a function from functions list into temp 
   // Operand 1: index into functions array to move
   // Operand 2: N/A
   // target: number of the temp to store the funtion in 
   // Result: target stores the specified function
   LoadFunc,  

   // Description: Load a local into a temp
   // Operand 1: index of the local to load
   // Operand 2: N/A
   // target: index of the temp to store into 
   // Result: target stores the desired local 
   LoadLocal,  

   // Description: Store a value from a temp into a local 
   // Operand 1: index of the temp containing the value to store
   // Operand 2: index of the local to store into 
   // target: N/A
   // Result: the local at index operand2 contains the value stored by operand1
   StoreLocal,  

   // Description: Load a global into a temp 
   // Operand 1: index of the global to load
   // Operand 2: N/A
   // target: index of the temp to load into 
   // Result: target contains the global var at index operand1
   LoadGlobal,

   // Description: Store a value from a temp into a global
   // Operand 1: index of the temp containing the value to store 
   // Operand 2: index of the global to store into 
   // target: N/A
   // Result: the global at index operand2 contains the value stored by operand1
   StoreGlobal,

   // Description: add the constants in two temps 
   // Operand 1: temp index holding right value
   // Operand 2: temp index holding left value
   // target: number of the temp to store the value in 
   // Result: target stores the result of doing operand1 + operand2
   Add,

   // Description: performs arithmetic operation on two temps 
   // Operand 1: temp index holding right value
   // Operand 2: temp index holding left value
   // target: number of the temp to store the value in 
   // Result: target stores the result of doing op(operand2, operand1)
   Sub, 
   Mul, 
   Div,

   // Description: computes unary minus
   // Operand 1: value to negate
   // Operand 2: N/A
   // target: number of the temp to store the result in 
   // Result: target stores -operand1
   Neg,

   // Description: computes a comparison on ints
   // Operand 1: right value
   // Operand 2: left value
   // target: number of the temp to store the result in 
   // Result: target stores bool indicating result of comparison
   Gt, 
   Geq,

   // Description: computes an equality between two vals (semantics from A2)
   // Operand 1: right value
   // Operand 2: left value
   // target: number of the temp to store the result in 
   // Result: target stores bool eq(operand2, operand1)
   Eq, 

   // Description: computes a boolean operation (semantics from A2)
   // Operand 1: right value
   // Operand 2: left value
   // target: number of the temp to store the result in
   // Result: target stores bool op(operand1, operand2)
   And, 
   Or,

   // Description: computes local negation
   // Operand 1: value to negate
   // Operand 2: N/A
   // target: number of the temp to store the result in
   // Result: target stores !operand1
   Not,

   // Description: move to a given label 
   // Operand 1: index of the label to jump to; should be unique 
   // Operand 2: N/A
   // target: N/A
   // Result: execution transfers to the label
   Goto,

   // Description: move to a given label 
   // Operand 1: index of the label to jump to; should be unique 
   // Operand 2: N/A
   // target: N/A
   // Result: execution transfers to the label
   If,

   // Description: swap the values in two temps 
   // Operand 1: index of one temp to swap
   // Operand 2: index of the other temp to swap
   // target: N/A
   // Result: values in the temps are swapped 
   Swap,

   // Description: pops and discards the top of the stack
   // Operand 1:   N/A
   // Operand 2:   N/A
   // target: N/A
   // Result: top element eliminated from stack
   Pop
};

struct IrInstruction {
    IrOp operation; 
    optint_t operand1;
    optint_t operand2;
    optint_t target;
    IrInstruction(const IrOp operation, optint_t operand1, optint_t operand2, optint_t target) : 
        operation(operation), 
        operand1(operand1), 
        operand2(operand2), 
        target(target) {};
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
