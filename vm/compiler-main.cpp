#include "../parser/parser.h"
#include "../parser/lexer.h"
#include <iostream>
#include "prettyprinter.h"

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
    if(rvalue == 1){
        cout<<"Parsing failed"<<endl;
        return 1;
    }

    // TODO: traverse the AST and create a Function
    Function* rootFunc;

    std::shared_ptr<Function> func(rootFunc);

    PrettyPrinter printer;

    printer.print(*func, std::cout);

    return 0;
}
