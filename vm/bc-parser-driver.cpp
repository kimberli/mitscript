
#include "bc-parser.h"
#include "bc-lexer.h"
#include <iostream>
#include "prettyprinter.h"

using namespace std;

int main(int argc, char** argv)
{
    void* scanner;
    bclex_init(&scanner);

    if (argc < 2) {
        cout << "Expecting file name as argument" << endl;
        return 1;
    }
    FILE* infile = fopen(argv[1], "r");
    if (infile == NULL) {
        cout << "Cannot open file " << argv[1] << endl;
        return 1;
    }
    bcset_in(infile, scanner);

    Function* output;
    int rvalue = bcparse(scanner, output);
    if(rvalue == 1){
        cout<<"Parsing failed"<<endl;
        return 1;
    }

    Function* func(output);

    PrettyPrinter printer;

    printer.print(*func, std::cout);

    return 0;
}
