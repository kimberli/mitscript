# include "../parser/AST.h"
# include "../parser/Visitor.h"
# include "instructions.h"
# include "types.h"

# include <memory>
# include <functional>

using namespace std;

class BytecodeCompiler : public Visitor {
public: // Holder for code that has already been generated
    InstructionList instr;
    // Holder for a function
    Function f;

    // Helper to return stored value
    InstructionList evalInstructions(AST_node& expr) {
        expr.accept(*this);
        return instr;
    }

    // Helper to return a function
    Function evalFunc(AST_node& expr) {
        expr.accept(*this);
        return f;
    }

    Function* getBytecode() {
        Function* f;
        return f;
    }

    void visit(Block& exp) {

    }
    void visit(Global& exp) {

    }
    void visit(Assignment& exp) {

    }
    void visit(CallStatement& exp) {

    }
    void visit(IfStatement& exp) {

    }
    void visit(WhileLoop& exp) {

    }
    void visit(Return& exp) {
        // get the code to evalute the expression and leave it
        // on the top of the stack
        InstructionList iList = evalInstructions(exp);
        // add a return instruciton
        std::experimental::optional<int32_t> arg0;
        Operation op = Operation::Call;
        Instruction* i = new Instruction(op, arg0);
        iList.push_back(*i);
        instr = iList;
    }
    void visit(FunctionExpr& exp) {

    }
    void visit(BinaryExpr& exp) {

    }
    void visit(UnaryExpr& exp) {

    }
    void visit(FieldDeref& exp) {

    }
    void visit(IndexExpr& exp) {

    }
    void visit(Call& exp) {

    }
    void visit(RecordExpr& exp) {

    }
    void visit(Identifier& exp) {

    }
    void visit(IntConst& exp) {

    }
    void visit(StrConst& exp) {

    }
    void visit(BoolConst& exp) {

    }
    void visit(NoneConst& exp) {

    }
};

