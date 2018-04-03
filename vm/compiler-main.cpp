#include "../parser/parser.h"
#include "../parser/lexer.h"
#include <iostream>
#include "prettyprinter.h"
#include "compiler.h"
#include "cfg.h"
#include "cfgtofunc.h"

using namespace std;

int main(int argc, char** argv)
{
    void* scanner;
    yylex_init(&scanner);

    if (argc < 2) {
        cout << "Expecting file name as argument" << endl;
        return 1;
    }
    FILE* infile = fopen(argv[1], "r");
    if (infile == NULL) {
        cout << "Cannot open file " << argv[1] << endl;
        return 1;
    }
    yyset_in(infile, scanner);

    Block* output;
    int rvalue = yyparse(scanner, output);
    if (rvalue == 1) {
        cout << "Parsing MITScript failed" << endl;
        return 1;
    }

    CFGBuilder* builder = new CFGBuilder();
    output->accept(*builder);
    cfgptr_t rootCFG = builder->curFunc;

    std::shared_ptr<Function> rootFunc = CFGToFunc(rootCFG);

    PrettyPrinter printer;

    printer.print(*rootFunc, std::cout);

    return 0;
}
