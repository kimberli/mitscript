#include "parser.h"
#include "lexer.h"
#include "AST.h"
#include "PrettyPrinter.h"
#include <iostream>

using namespace std;

extern int yydebug;

int main(int argc, char** argv){
  void* scanner;
  yydebug = 1;  // set to nonzero for parser debug statements
  yylex_init(&scanner);
  yyset_in(stdin, scanner);
  Block* output;
  int rvalue = yyparse(scanner, output);
  if(rvalue == 1){
    cout<<"Parsing failed"<<endl;
    return 1;
  }
  PrettyPrinter printer;
  output->accept(printer);
  cout<<"\n";
}
