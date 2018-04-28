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

   // Description: move to a given offset
   // Operand 1: stores the offset to move to 
   // Operand 2: N/A
   // target: N/A
   // Result: execution transfers to the offset
   Goto,
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

    IrFunc(IrInstList instructions, 
        vector<vptr<Constant>> constants_,
        int32_t parameter_count_) : 
        instructions(instructions), 
        constants_(constants_), 
        parameter_count_(parameter_count_) {};
};
