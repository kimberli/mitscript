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
    Function* bc_output;
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
        BytecodeCompiler* c = new BytecodeCompiler();
        output->accept(*c);
        bc_output = c->getBytecode();
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

    std::shared_ptr<Function> func(bc_output);
  
    Interpreter* intp = new Interpreter(bc_output); 
    intp->run();

    return 0;
}
