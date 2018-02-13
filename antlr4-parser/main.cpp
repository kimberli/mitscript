#include <iostream>

#include "antlr4-runtime.h"
#include "MITScriptLexer.h"
#include "MITScriptParser.h"

int main(int argc, const char* argv[]) {
  std::ifstream stream;
  stream.open(argv[1]);

  antlr4::ANTLRInputStream input(stream);
  MITScriptLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  MITScriptParser parser(&tokens);
  
  auto tree = parser.program();
  
  std::cout <<tree->toStringTree(&parser) << std::endl; 

  return 0;
}
