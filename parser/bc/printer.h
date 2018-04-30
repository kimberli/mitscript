/*
 * prettyprinter.h
 *
 * Defines a PrettyPrinter class that takes in a bytecode object (as a root
 * Function node) and prints it out in human-readable bytecode format
 */
#include "../../types.h"
#include "assert.h"
#include <memory>
#include <iostream>

class PrettyPrinter
{
public:
    PrettyPrinter()
        : indent_(0)
    {
    }

    void print(const Function &function, std::ostream &os)
    {
        print_indent(os) << "function\n";

        print_indent(os) << "{\n";

        auto old_indent = indent_;

        indent();

        print_indent(os) << "functions =";

        if (0 == function.functions_.size())
        {
            os << " [],\n";
        }
        else
        {
            os << "\n";

            print_indent(os) << "[\n";

            indent();

            for (size_t i = 0; i < function.functions_.size(); ++i)
            {
                print(*function.functions_[i], os);

                if (i != (function.functions_.size() - 1))
                {
                    os << ",\n";
                }
            }

            unindent();

            os <<"\n";
            print_indent(os) << "],\n";
        }


        {
            print_indent(os) << "constants = [";

            for (size_t i = 0; i < function.constants_.size(); ++i)
            {
                if (i != 0)
                {
                    os << ", ";
                }

                print(*function.constants_[i], os);
            }
        }

        os << "],\n";

        print_indent(os) << "parameter_count = " << function.parameter_count_ << ",\n";

        print("local_vars", function.local_vars_, os);
        print("local_ref_vars", function.local_reference_vars_, os);
        print("free_vars", function.free_vars_, os);
        print("names", function.names_, os);

        print_indent(os) << "instructions = \n";

        print_indent(os) << "[\n";

        indent();

        print(function.instructions, os);

        unindent();

        print_indent(os) << "]\n";

        unindent();
        print_indent(os) << "}";

        assert(indent_ == old_indent);
    }

private:
    void indent()
    {
        ++indent_;
    }

    void unindent()
    {
        --indent_;
    }

    std::ostream &print_indent(std::ostream &os)
    {
        for (size_t i = 0; i < indent_; ++i)
        {
            os << "\t";
        }

        return os;
    }

    void print(const std::string &name, const std::vector<std::string> &names, std::ostream &os)
    {
        print_indent(os) << name << " = [";

        for (size_t i = 0; i < names.size(); ++i)
        {
            if (i != 0)
            {
                os << ", ";
            }

            os << names[i];
        }
        os << "],\n";
    }

    void print(const Constant &constant, std::ostream &os)
    {
        if (const auto *value = dynamic_cast<const None *>(&constant))
        {
            os << "None";
        }
        else if (const auto *value = dynamic_cast<const Boolean *>(&constant))
        {
            os << (value->value ? "true" : "false");
        }
        else if (const auto *value = dynamic_cast<const Integer *>(&constant))
        {
            os << value->value;
        }
        else if (const auto *value = dynamic_cast<const String *>(&constant))
        {
            //TODO: escape
            os << '"' << value->value << '"';
        }
    }

    void print(const Instruction &inst, std::ostream &os)
    {

        switch (inst.operation)
        {
        case BcOp::LoadConst:
        {
            os << "load_const"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::LoadFunc:
        {
            os << "load_func"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::LoadLocal:
        {
            os << "load_local"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::StoreLocal:
        {
            os << "store_local"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::LoadGlobal:
        {
            os << "load_global"
               << "\t" << inst.operand0.value();

            break;
        }

        case BcOp::StoreGlobal:
        {
            os << "store_global"
               << "\t" << inst.operand0.value();

            break;
        }

        case BcOp::PushReference:
        {
            os << "push_ref"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::LoadReference:
        {
            os << "load_ref";

            break;
        }
        case BcOp::StoreReference:
        {
            os << "store_ref";

            break;
        }
         case BcOp::AllocRecord:
        {
            os << "alloc_record";

            break;
        }
        case BcOp::FieldLoad:
        {
            os << "field_load"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::FieldStore:
        {
            os << "field_store"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::IndexLoad:
        {
            os << "index_load";

            break;
        }
        case BcOp::IndexStore:
        {
            os << "index_store";

            break;
        }
        case BcOp::AllocClosure:
        {
            os << "alloc_closure"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::Call:
        {
            os << "call"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::Return:
        {
            os << "return";

            break;
        }
        case BcOp::Add:
        {
            os << "add";

            break;
        }
        case BcOp::Sub:
        {
            os << "sub";

            break;
        }

        case BcOp::Mul:
        {
            os << "mul";

            break;
        }
        case BcOp::Div:
        {
            os << "div";

            break;
        }
        case BcOp::Neg:
        {
            os << "neg";

            break;
        }
        case BcOp::Gt:
        {
            os << "gt";

            break;
        }
        case BcOp::Geq:
        {
            os << "geq";

            break;
        }
        case BcOp::Eq:
        {
            os << "eq";

            break;
        }
        case BcOp::And:
        {
            os << "and";

            break;
        }
        case BcOp::Or:
        {
            os << "or";

            break;
        }
        case BcOp::Not:
        {
            os << "not";
            break;
        }
        case BcOp::Goto:
        {
            os << "goto"
               << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::If:
        {
            os << "if"
            << "\t" << inst.operand0.value();

            break;
        }
        case BcOp::Dup:
        {
            os << "dup";

            break;
        }
        case BcOp::Swap:
        {
            os << "swap";

            break;
        }
        case BcOp::Pop:
        {
            os << "pop";

            break;
        }

        default:
            assert(false && "Unhandled BcOp");
        }
    }

    void print(const InstructionList &ilist, std::ostream &os)
    {
        for (size_t i = 0; i < ilist.size(); ++i)
        {

            print_indent(os);

            print(ilist[i], os);

            os << "\n";
        }
    }

private:
    size_t indent_;
};
