MS_PARSER = parser/ms
MS_PARSER_SRC = parser/ms/parser.cpp parser/ms/lexer.cpp
BC_PARSER = parser/bc
BC_PARSER_SRC = parser/bc/parser.cpp parser/bc/lexer.cpp
BC_COMPILER_SRC = bc/bc-compiler.cpp bc/symboltable.cpp gc/gc.cpp frame.cpp types.cpp
VM_SRC = vm/interpreter.cpp $(BC_COMPILER_SRC)
REF = ref

clean: clean-ms-parser clean-bc-parser clean-ref clean-bc-compiler clean-vm

## EXECUTABLES

# make the MITScript pretty printer
ms-print: $(MS_PARSER)/print_main.cpp $(MS_PARSER_SRC)
	g++ -O2 -g -std=c++1y $(MS_PARSER)/print_main.cpp $(MS_PARSER_SRC) -o $(MS_PARSER)/ms-print

# make the bytecode pretty printer
bc-print: $(BC_PARSER)/print_main.cpp $(BC_PARSER_SRC)
	g++ -O2 -g -std=c++1y $(BC_PARSER)/print_main.cpp $(BC_PARSER_SRC) types.cpp frame.cpp gc/gc.cpp -o $(BC_PARSER)/bc-print

# MITScript -> bytecode compiler
bc-compiler: bc/* gc/* $(MS_PARSER)/parser.cpp $(BC_PARSER)/parser.cpp
	g++ -g -std=c++1y bc/compiler-main.cpp $(BC_COMPILER_SRC) $(MS_PARSER_SRC) -o mitscriptc

# MITScript -> bytecode -> vm
vm: vm/* bc/* gc/* $(MS_PARSER)/parser.cpp $(BC_PARSER)/parser.cpp
	g++ -g -std=c++1y vm/interpreter-main.cpp $(VM_SRC) $(BC_PARSER_SRC) $(MS_PARSER_SRC) -o mitscript

# reference interpreter (from a2)
ref: $(REF)/ref-main.cpp $(REF)/* Visitor.h AST.h $(MS_PARSER_SRC)
	g++ -O2 -g -std=c++1y $(REF)/*.cpp $(MS_PARSER_SRC) -o ref/mitscript

## OTHER FORMULAS:

# MITScript parser
$(MS_PARSER)/parser.cpp: $(MS_PARSER)/parser.yy $(MS_PARSER)/lexer.cpp
	bison --output=$(MS_PARSER)/parser.cpp --defines=$(MS_PARSER)/parser.h -v $(MS_PARSER)/parser.yy
	
$(MS_PARSER)/lexer.cpp: $(MS_PARSER)/lexer.lex
	flex  --outfile=$(MS_PARSER)/lexer.cpp --header-file=$(MS_PARSER)/lexer.h $(MS_PARSER)/lexer.lex

# bytecode parser
$(BC_PARSER)/parser.cpp: $(BC_PARSER)/parser.yy $(BC_PARSER)/lexer.cpp
	bison -Dapi.prefix={bc} --output=$(BC_PARSER)/parser.cpp --defines=$(BC_PARSER)/parser.h $(BC_PARSER)/parser.yy
	
$(BC_PARSER)/lexer.cpp: $(BC_PARSER)/lexer.lex
	flex -P bc --outfile=$(BC_PARSER)/lexer.cpp --header-file=$(BC_PARSER)/lexer.h $(BC_PARSER)/lexer.lex

# cleanup
clean-ms-parser:
	rm -f $(MS_PARSER)/lexer.cpp $(MS_PARSER)/lexer.h $(MS_PARSER)/parser.cpp $(MS_PARSER)/parser.h $(MS_PARSER)/parser.output $(MS_PARSER)/ms-print

clean-bc-parser:
	rm -f $(BC_PARSER)/lexer.cpp $(BC_PARSER)/lexer.h $(BC_PARSER)/parser.cpp $(BC_PARSER)/parser.h $(BC_PARSER)/parser.output $(BC_PARSER)/bc-print

clean-ref:
	rm -f ref/mitscript

clean-bc-compiler:
	rm -f mitscriptc

clean-vm:
	rm -f mitscript
