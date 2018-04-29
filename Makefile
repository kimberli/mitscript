MS_PARSER = parser/ms
BC_PARSER = parser/bc

# make the MITScript pretty printer
ms-print: $(MS_PARSER)/print_main.cpp $(MS_PARSER)/parser.cpp $(MS_PARSER)/lexer.cpp
	g++ -O2 -g -std=gnu++11 $(MS_PARSER)/print_main.cpp $(MS_PARSER)/parser.cpp $(MS_PARSER)/lexer.cpp -o $(MS_PARSER)/ms-print

# MITScript parser
$(MS_PARSER)/parser.cpp: $(MS_PARSER)/parser.yy
	bison --output=$(MS_PARSER)/parser.cpp --defines=$(MS_PARSER)/parser.h -v $(MS_PARSER)/parser.yy
	
$(MS_PARSER)/lexer.cpp: $(MS_PARSER)/lexer.lex
	flex  --outfile=$(MS_PARSER)/lexer.cpp --header-file=$(MS_PARSER)/lexer.h $(MS_PARSER)/lexer.lex

clean-ms-parser:
	rm -f $(MS_PARSER)/lexer.cpp $(MS_PARSER)/lexer.h $(MS_PARSER)/parser.cpp $(MS_PARSER)/parser.h $(MS_PARSER)/parser.output $(MS_PARSER)/ms-print

# make the bytecode pretty printer
bc-print: $(BC_PARSER)/print_main.cpp $(BC_PARSER)/parser.cpp $(BC_PARSER)/lexer.cpp
	g++ -O2 -g -std=c++1y $(BC_PARSER)/print_main.cpp $(BC_PARSER)/parser.cpp $(BC_PARSER)/lexer.cpp types.cpp vm/frame.cpp gc/gc.cpp -o $(BC_PARSER)/bc-print

# bytecode parser
$(BC_PARSER)/parser.cpp: $(BC_PARSER)/parser.yy
	bison -Dapi.prefix={bc} --output=$(BC_PARSER)/parser.cpp --defines=$(BC_PARSER)/parser.h $(BC_PARSER)/parser.yy
	
$(BC_PARSER)/lexer.cpp: $(BC_PARSER)/lexer.lex
	flex -P bc --outfile=$(BC_PARSER)/lexer.cpp --header-file=$(BC_PARSER)/lexer.h $(BC_PARSER)/lexer.lex

clean-bc-parser:
	rm -f $(BC_PARSER)/lexer.cpp $(BC_PARSER)/lexer.h $(BC_PARSER)/parser.cpp $(BC_PARSER)/parser.h $(BC_PARSER)/parser.output $(BC_PARSER)/bc-print

clean: clean-ms-parser clean-bc-parser
