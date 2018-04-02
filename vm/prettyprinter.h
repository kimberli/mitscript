#include "types.h"
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
        case Operation::LoadConst:
        {
            os << "load_const"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::LoadFunc:
        {
            os << "load_func"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::LoadLocal:
        {
            os << "load_local"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::StoreLocal:
        {
            os << "store_local"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::LoadGlobal:
        {
            os << "load_global"
               << "\t" << inst.operand0.value();

            break;
        }

        case Operation::StoreGlobal:
        {
            os << "store_global"
               << "\t" << inst.operand0.value();

            break;
        }

        case Operation::PushReference:
        {
            os << "push_ref"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::LoadReference:
        {
            os << "load_ref";

            break;
        }
        case Operation::StoreReference:
        {
            os << "store_ref";

            break;
        }
         case Operation::AllocRecord:
        {
            os << "alloc_record";

            break;
        }
        case Operation::FieldLoad:
        {
            os << "field_load"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::FieldStore:
        {
            os << "field_store"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::IndexLoad:
        {
            os << "index_load";

            break;
        }
        case Operation::IndexStore:
        {
            os << "index_store";

            break;
        }
        case Operation::AllocClosure:
        {
            os << "alloc_closure"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::Call:
        {
            os << "call"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::Return:
        {
            os << "return";

            break;
        }
        case Operation::Add:
        {
            os << "add";

            break;
        }
        case Operation::Sub:
        {
            os << "sub";

            break;
        }

        case Operation::Mul:
        {
            os << "mul";

            break;
        }
        case Operation::Div:
        {
            os << "div";

            break;
        }
        case Operation::Neg:
        {
            os << "neg";

            break;
        }
        case Operation::Gt:
        {
            os << "gt";

            break;
        }
        case Operation::Geq:
        {
            os << "geq";

            break;
        }
        case Operation::Eq:
        {
            os << "eq";

            break;
        }
        case Operation::And:
        {
            os << "and";

            break;
        }
        case Operation::Or:
        {
            os << "or";

            break;
        }
        case Operation::Not:
        {
            os << "not";
            break;
        }
        case Operation::Goto:
        {
            os << "goto"
               << "\t" << inst.operand0.value();

            break;
        }
        case Operation::If:
        {
            os << "if"
            << "\t" << inst.operand0.value();

            break;
        }
        case Operation::Dup:
        {
            os << "dup";

            break;
        }
        case Operation::Swap:
        {
            os << "swap";

            break;
        }
        case Operation::Pop:
        {
            os << "pop";

            break;
        }

        default:
            assert(false && "Unhandled Operation");
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
