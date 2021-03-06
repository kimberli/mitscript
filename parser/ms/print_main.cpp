#include "parser.h"
#include "lexer.h"
#include "printer.h"
#include "../../AST.h"
#include <iostream>

using namespace std;

extern int yydebug;

int main(int argc, char** argv){
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

    PrettyPrinter printer;
    output->accept(printer);
    cout << "\n";
    return 0;
}
