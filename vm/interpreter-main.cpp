/*
 * interpreter-main.cpp
 *
 * Main file used to make the MITScript interpreter
 * The interpreter has two capabilities:
 * 1) with the -b flag, it reads in bytecode and executes it
 * 2) with the -s flag, it reads in MITScript source, compiles it to bytecode, and
 *    executes the resulting bytecode
 */
#include <stdexcept>

#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "bc-parser.h"
#include "bc-lexer.h"

#include "compiler.h"
#include "interpreter.h"
#include "../gc/gc.h"

#define MITSCRIPT 1
#define BYTECODE 2

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: interpreter [-b|-s] <FILENAME> -mem <mem in MB>" << endl;
        return 1;
    }

    FILE* infile;
    funcptr_t bc_output;
    int file_type = 0;
    int rvalue = 0;
    int maxmem = 1000;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            file_type = BYTECODE;
        } else if (strcmp(argv[i], "-s") == 0) {
            file_type = MITSCRIPT;
        } else if (strcmp(argv[i], "-mem") == 0) {
            string memError = "-mem takes an integer specifying max data usage in MB";
            if ((i+1) >= argc) {
                cout << memError << endl;
                return 1;
            }
            try {
                maxmem = stoi(argv[i+1]);
            } catch (std::invalid_argument& ia) {
                cout << memError << endl;
                return 1;
            }
            i++; // skip an arg
        }
        else {
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

    // allocate a garbage collector
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
        try {
            bc_output = bc->evaluate(*output);
        } catch (InterpreterException& exception) {
            cout << exception.toString() << endl;
            return 1;
        }
    } else if (file_type == BYTECODE) {
        // parse bytecode, set bc_output
        void* scanner;
        bclex_init(&scanner);
        bcset_in(infile, scanner);
        if (bcparse(scanner, bc_output) == 1) {
            cout << "Parsing bytecode failed" << endl;
            return 1;
        }
    }
  
    try {
        Interpreter* intp = new Interpreter(bc_output, maxmem);
        intp->run();
    } catch (InterpreterException& exception) {
        cout << exception.toString() << endl;
        return 1;
    }

    return 0;
}
