
#include "bc-parser.h"
#include "bc-lexer.h"
#include <iostream>
#include "prettyprinter.h"

using namespace std;

int main(int argc, char** argv)
{
  void* scanner;
  yylex_init(&scanner);
  yyset_in(stdin, scanner);
  
  Function* output;
  int rvalue = yyparse(scanner, output);
  if(rvalue == 1){

	  cout<<"Parsing failed"<<endl;
	  return 1;
  }

  std::shared_ptr<Function> func(output);

  PrettyPrinter printer;

  printer.print(*func, std::cout);

  return 0;
}
