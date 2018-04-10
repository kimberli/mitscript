all: compiler interpreter

sources = compiler.cpp types.cpp symboltable.cpp frame.cpp

compiler: compiler-main.cpp $(sources)
	g++ -g -std=c++1y compiler-main.cpp $(sources) ../parser/parser.cpp ../parser/lexer.cpp -o mitscriptc

interpreter: interpreter-main.cpp interpreter.cpp $(sources) bc-parser
	g++ -g -std=c++1y interpreter-main.cpp interpreter.cpp $(sources) bc-parser.cpp bc-lexer.cpp ../parser/parser.cpp ../parser/lexer.cpp -o mitscript

bc-parser: bc-parser.cpp bc-lexer.cpp bc-parser-driver.cpp
	g++ -g -std=c++1y types.cpp bc-parser-driver.cpp bc-parser.cpp bc-lexer.cpp -o bc-parser

bc-parser.cpp: bc-parser.yy
	bison -Dapi.prefix={bc} --output=bc-parser.cpp --defines=bc-parser.h bc-parser.yy

bc-lexer.cpp: bc-lexer.lex
	flex -P bc --outfile=bc-lexer.cpp --header-file=bc-lexer.h bc-lexer.lex

clean: 
	rm -f bc-lexer.cpp bc-lexer.h bc-parser.cpp bc-parser.h bc-parser.output bc-parser mitscript mitscriptc *.exe
