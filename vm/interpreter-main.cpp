/*
 * interpreter-main.cpp
 *
 * Main file used to make the MITScript interpreter
 * The interpreter has two capabilities:
 * 1) with the -b flag, it reads in bytecode and executes it
 * 2) with the -s flag, it reads in MITScript source, compiles it to bytecode, and
 *    executes the resulting bytecode
 */
#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "bc-parser.h"
#include "bc-lexer.h"

#include "compiler.h"
#include "interpreter.h"

#define MITSCRIPT 1
#define BYTECODE 2

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: interpreter [-b|-s] <FILENAME>" << endl;
        return 1;
    }

    FILE* infile;
    funcptr_t bc_output;
    int file_type = 0;
    int rvalue = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            file_type = BYTECODE;
        } else if (strcmp(argv[i], "-s") == 0) {
            file_type = MITSCRIPT;
        } else {
            infile = fopen(argv[i], "r");
            if (infile == NULL) {
                cout << "Cannot open file " << argv[i] << endl;
                return 1;
            }
        }
    }

    if (!file_type) {
        cout << "Usage: interpreter [-b|-s] <FILENAME>" << endl;
        return 1;
    }

    if (file_type == MITSCRIPT) {
        // parse mitscript, set output to bc_output
        void* scanner;
        yylex_init(&scanner);
        yyset_in(infile, scanner);
        Block* output;
        if (yyparse(scanner, output) == 1) {
            cout<<"Parsing MITScript failed"<<endl;
            return 1;
        }
        BytecodeCompiler* bc = new BytecodeCompiler();
        //output->accept(*bc);
        bc_output = bc->evaluate(*output);
    } else if (file_type == BYTECODE) {
        // parse bytecode, set bc_output
        Function* output;
        void* scanner;
        bclex_init(&scanner);
        bcset_in(infile, scanner);
        if (bcparse(scanner, output) == 1) {
            cout << "Parsing bytecode failed" << endl;
            return 1;
        }
        funcptr_t bc_output(output);
    }
  
    Interpreter* intp = new Interpreter(bc_output);
    try {
        intp->run();
    } catch (InterpreterException& exception) {
        cout << exception.toString() << endl;
        return 1;
    }

    return 0;
}
