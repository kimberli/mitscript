# pragma once 

#include <experimental/optional>
#include "../vm/types.h"

struct IrInstruction;
struct IrFunc;

typedef std::vector<IrInstruction> IrInstList;
typedef std::experimental::optional<int32_t> optint_t;

enum class IrOp {
   // TODO: add other ops  
   
   // Description: subtract the constants in two temps 
   // Operand 1: temp index holding left value
   // Operand 2: temp index holding right value
   // target: number of the temp to store the value in 
   // Result: tmp stores the result of doing op1 - op2
   Sub
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
    vector<vptr<IrFunc>> functions_;
    vector<vptr<Constant>> constants_;
    int32_t parameter_count_;
    IrFunc(IrInstList instructions, 
        vector<vptr<IrFunc>> functions_,
        vector<vptr<Constant>> constants_,
        int32_t parameter_count_) : 
        instructions(instructions), 
        functions_(functions_),
        constants_(constants_), 
        parameter_count_(parameter_count_) {};
};
