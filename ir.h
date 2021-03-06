# pragma once

#include <experimental/optional>
#include "types.h"
#include "include/x64asm.h"

struct IrInstruction;
struct IrFunc;
struct Temp;

using namespace std;

typedef shared_ptr<IrInstruction> instptr_t;
typedef vector<instptr_t> IrInstList;
typedef experimental::optional<int32_t> optint_t;
typedef experimental::optional<string> optstr_t;
typedef int64_t offset_t;
typedef shared_ptr<Temp> tempptr_t;
typedef experimental::optional<tempptr_t> opttemp_t;
typedef vector<tempptr_t> TempList;
typedef shared_ptr<TempList> TempListPtr;
typedef experimental::optional<x64asm::R64> optreg_t;

struct Temp {
    int index;
	optreg_t reg = nullopt;
    optint_t stackOffset = nullopt;
	int startInterval = -1;
	int endInterval = -1;
	Temp(int i) : index(i) {}
};

static string asmRegToString(x64asm::R64 reg) {
	switch(reg) {
		case x64asm::rdi:
		{
			return "rdi";
		}
	    case x64asm::rsi:
		{
			return "rsi";
		}
	    case x64asm::rax:
		{
			return "rax";
		}
	    case x64asm::rbx: 
		{
			return "rbx";
		}
	    case x64asm::rcx:
		{
			return "rcx";
		}
		case x64asm::rdx:
		{
			return "rdx";
		}
	    case x64asm::r8: 
		{
			return "r8";
		}
	    case x64asm::r9:
		{
			return "r9";
		}
	    case x64asm::r10:
		{
			return "r10";
		}
	    case x64asm::r11:
		{
			return "r11";
		}
	    case x64asm::r12:
		{
			return "r12";
		}
	    case x64asm::r13:
		{
			return "r13";
		}
	    case x64asm::r14: 
		{
			return "r14";
		}
	    case x64asm::r15: 
		{
			return "r15";
		}
		default: 
			return "reg";
	}
}

enum class IrOp {
    // Description: Move a constant from constants array into temp var
    // op0: index into the constants array of the constant to move
    // temp0: temp index to store the constant in
    // Result: temp0 stores func.constants_[operand1]
    LoadConst,

    // Description: Load a function from functions list into temp
    // op0: index into functions array to move
    // temp0: temp index to store the funtion in
    // Result: temp0 stores the specified function
    LoadFunc,

    // Description: Load a global into a temp
    // op0: N/A
    // name0: name of the global to load
    // temp0: temp index to store into
    // Result: temp at temp0 contains the global var indicated by its name
    LoadGlobal,

    // Description: Store a value from a temp into a local
    // temp1: temp representing the val to store
    // temp0: temp representing the local to store into
    // Result: the local at index op0 contains the value stored by the temp at temp0
    StoreLocal,

    // Description: Store a value from a temp into a global
    // op0: N/A
    // name0: name of the global to store into
    // temp0: temp index containing the value to store
    // Result: the global named contains the value stored by temp0
    StoreGlobal,

    // Description: Store a reference to a free variable into a temp
    // op0: index of the free var to store the reference of
    // temp0: temp index to store reference into
    // Result: temp at temp0 contains reference to the local variable
    PushFreeRef,

    // Description: Load the value of a reference into a temp
    // temp0: temp index to store into
    // temp1: the temp storing the reference to load
    // Result: temp at temp0 contains value of the reference
    LoadReference,

    // Description: Store the value of a temp into a local ref
    // temp0: temp of the local to store into 
    // temp1: index of the temp w/ the val to store
    // Result: local temp0 contains the value in temp0
    StoreLocalRef,

    // Description: Allocate a record and store it to a temp
    // op0: N/A
    // temp0: temp index to allocate the record in
    // Result: temp at temp0 contains a new empty record
    AllocRecord,

    // Description: Load a value from a record and store it into a temp
    // op0: N/A
    // name0: name of the field to load from
    // temp0: temp index to store the field's value into
    // temp1: temp index containing the record to look in (must be record)
    // Result: temp at temp0 contains the value of the field
    FieldLoad,

    // Description: Store a value into a record's field
    // op0: N/A
    // name0: name of the field to store into
    // temp0: temp index containing the record to store into (must be record)
    // temp1: temp index containing the field's new value
    // Result: temp at temp0 points to record with the new value set
    FieldStore,

    // Description: Load a value from a record and store it into a temp
    // op0: N/A
    // temp0: temp index to store the field's value into
    // temp1: temp index containing the record to look in (must be record)
    // temp2: temp index containing the field
    // Result: temp at temp0 contains the value of the field
    IndexLoad,

    // Description: Store a value into a record's field
    // op0: N/A
    // temp0: temp index containing the record to store into (must be record)
    // temp1: temp index containing the field's new value
    // temp2: temp index containing the index/field
    // Result: temp at temp0 points to record with the new value set
    IndexStore,

    // Description: Allocate a new closure
    // op0: number of free vars passed to the closure
    // temp0: temp index containing the new closure
    // temp1: temp index containing the function 
    // temps 2-m: temps containing references to the free vars we are going to pass
    // Result: temp at temp0 now contains the new closure
    AllocClosure,

    // Description: call a closure
    // op0: number of args passed to the function
    // temp0: temp index that will contain the return val
    // temp1: temp index containing the closure to call (must be closure)
    // temp 2-m: temps containing values of the args we are passing
    // Result: temp at temp0 contains the result of calling the function
    Call,

    // Description: return from a function
    // op0: N/A
    // temp0: temp index containing return value
    // Result: return from the function
    Return,

    // Description: add the constants in two temps (semantics from A2)
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: temp index holding right value
    // temp2: temp index holding left value
    // Result: temp at temp0 stores the result of doing temp2 + temp1
    Add,

    // Description: performs arithmetic operation on two temps
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: temp index holding right value (must be int)
    // temp2: temp index holding left value (must be int)
    // Result: temp at temp0 stores the result of doing op(temp2, temp1)
    Sub,
    Mul,
    Div,

    // Description: computes unary minus
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: value to negate (must be int)
    // Result: temp at temp0 stores -temp1
    Neg,

    // Description: computes a comparison on ints
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: temp index holding right value (must be int)
    // temp2: temp index holding left value (must be int)
    // Result: temp at temp0 stores bool indicating result of comparison
    Gt,
    Geq,

    // Description: computes an equality between two vals (semantics from A2)
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: temp index holding right value
    // temp2: temp index holding left value
    // Result: temp at temp0 stores bool eq(temp2, temp1)
    Eq,

    // Description: computes a boolean operation
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: temp index holding right value (must be bool)
    // temp2: temp index holding left value (must be bool)
    // Result: temp at temp0 stores bool op(temp2, temp1)
    And,
    Or,

    // Description: computes boolean negation
    // op0: N/A
    // temp0: temp index to store the result value in
    // temp1: temp index holding value to negate (must be bool)
    // Result: temp at temp0 stores !operand1
    Not,

    // Description: move to a given label
    // op0: index of the label to jump to; should be unique
    // Result: transfers execution to the label
    Goto,

    // Description: move to a given label
    // op0: index of the label to jump to conditionally; should be unique
    // temp0: temp index holding value to check (must be bool)
    // Result: transfers execution to the label if temp at temp0 is true
    If,

    // Description: asserts that a temp is an integer
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not an integer
    AssertInteger,

    // Description: asserts that a temp is a bool
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not a bool
    AssertBoolean,

    // Description: asserts that a temp is a string
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not a string
    AssertString,

    // Description: asserts that a temp is an record
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not an record
    AssertRecord,

    // Description: asserts that a temp is a function
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not a function
    AssertFunction,

    // Description: asserts that a temp is a closure
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not a closure
    AssertClosure,

    // Description: asserts that a temp is a val wrapper
    // op0: N/A
    // temp0: temp index of value to check
    // Result: throws RuntimeError if the temp is not a val wrapper
    AssertValWrapper,

    // Description: unboxes a temp to integer
    // op0: N/A
    // temp0: temp index where the int will be stored
    // temp1: temp index of the value to unbox; must be a tagged tagptr_t
    // Result: stores int(temp1) in temp0
    UnboxInteger,

    // Description: unboxes a temp to bool
    // op0: N/A
    // temp0: temp index where bool will be stored
    // temp1: temp index of the value to unbox; must be a tagged tagptr_t
    // Result: stores bool(temp1) in temp0
    UnboxBoolean,
    
    // Description: takes a raw int and creates an Integer object
    // temp0: temp to hold the new Integer object
    // temp1: holds the int value 
    // Result: stores tagptr_t(temp1) in temp0
    NewInteger,
 
    // Description: takes a raw bool and creates a Boolean object
    // temp0: temp to hold the new Boolean object
    // temp1: holds the bool value 
    // Result: stores tagptr_t(temp1) in temp0
    NewBoolean,

    // Description: takes an object and casts it to a string
    // temp0: temp to hold the new String object
    // temp1: holds the thing to cast
    // Result: stores String(temp1) in temp0
    CastString,
    
    // Description: add a label at this point in the generated asm
    // op0: index of label to add
    // Result: adds label_op0 to this point in asm execution
    AddLabel,

    // Description: runs the garbage collector
    GarbageCollect
};


// Each instruction contains an optional op0 and an array of temps storing its
// return value and other operands.
// By convention, if the operation stores a value somewhere, it is put in
// extraTemps[0].
struct IrInstruction {
    IrOp op;
    optint_t op0;
    optstr_t name0;
    TempListPtr tempIndices;
    IrInstruction(const IrOp op, optint_t op0):
        op(op),
        name0(),
        op0(op0),
        tempIndices(make_shared<TempList>()) {};
    IrInstruction(const IrOp op, tempptr_t temp0, tempptr_t temp1): 
        op(op),
        name0(),
        op0(),
        tempIndices(make_shared<TempList>(TempList{temp0, temp1})) {};
    IrInstruction(const IrOp op, optstr_t name0, tempptr_t temp):
        op(op),
        name0(name0),
        op0(),
        tempIndices(make_shared<TempList>(TempList{temp})) {};
    IrInstruction(const IrOp op, optstr_t name0, TempListPtr tempIndices):
        op(op),
        name0(name0),
        op0(),
        tempIndices(tempIndices) {};
    IrInstruction(const IrOp op, optint_t op0, TempListPtr tempIndices):
        op(op),
        op0(op0),
        name0(),
        tempIndices(tempIndices) {};
    IrInstruction(const IrOp op, optint_t op0, tempptr_t temp):
        op(op),
        op0(op0),
        name0(),
        tempIndices(make_shared<TempList>(TempList{temp})) {};
    IrInstruction(const IrOp op, tempptr_t temp):
        op(op),
        op0(),
        name0(),
        tempIndices(make_shared<TempList>(TempList{temp})) {};
    IrInstruction(const IrOp op, TempListPtr tempIndices):
        op(op),
        op0(),
        name0(),
        tempIndices(tempIndices) {};
    string getInfo() {
        string s;
        if (op0) {
            s += "\top0: " + to_string(op0.value()) + "\n";
        }
        if (name0) {
            s += "\tname0: " + name0.value() + "\n";
        }
        s += "\ttemps:";
        for (tempptr_t t : *tempIndices) {
            s += " #";
            s += to_string(t->index) + " ";
			if (t->reg) {
				s += "(" + asmRegToString(t->reg.value()) + ")";
			} 
			if (t->stackOffset) {
				s += "(stack " + to_string(t->stackOffset.value()) + ")";
			}
        }
        return s;
    }
};

struct IrFunc {
    IrInstList instructions;
    vector<tagptr_t> constants_;
    vector<Function*> functions_;
	vector<tempptr_t> temps;
	int temp_count;
    int32_t parameter_count_;
    int32_t local_count_;
    // TODO: rn hard-coding this at 0, but we need to actually keep track 
    // of our refs somehow in the ir-interpreter
    int32_t ref_count_ = 0;

    IrFunc(IrInstList instructions,
        vector<tagptr_t> constants_,
        vector<Function*> functions_,
		vector<tempptr_t> temps,
		int temp_count,
        int32_t parameter_count_,
        int32_t local_count_,
        int32_t ref_count_) :
        instructions(instructions),
        constants_(constants_),
		temps(temps),
		temp_count(temp_count),
        functions_(functions_),
        parameter_count_(parameter_count_),
        local_count_(local_count_),
        ref_count_(ref_count_) {};
};

