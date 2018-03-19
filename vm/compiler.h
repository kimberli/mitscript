# include "../parser/AST.h"
# include "../parser/Visitor.h"
# include "instructions.h"

# include <memory>
# include <functional>
using namespace std;

class Block;
class Global;
class Assignment;
class CallStatement;
class IfStatement;
class WhileLoop;
class Return;
class Function;
class BinaryExpr;
class UnaryExpr;
class FieldDeref;
class IndexExpr;
class Call;
class Record;
class Identifier;
class IntConst;
class StrConst;
class BoolConst;
class NoneConst;

class BytecodeCompiler : public Visitor {
public:
    // Holder for code that has already been generated
    InstructionList ret;

    // Helper to return stored value
    InstructionList evaluate(AST_node& expr) {
        expr.accept(*this);
        return ret;
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
        InstructionList iList = evaluate(exp);
        // add a return instruciton 
        std::experimental::optional<int32_t> arg0;
        Operation op = Operation::Call;
        Instruction* i = new Instruction(op, arg0);
        iList.push_back(*i); 
        ret = iList;
    }
    void visit(Function& exp) {

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
    void visit(Record& exp) {

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

