compiler: compiler-main.cpp compiler.h cfg.h
	g++ -g -std=c++1y compiler-main.cpp ../parser/parser.cpp ../parser/lexer.cpp -o compiler

interpreter: interpreter-main.cpp interpreter.cpp bc-parser
	g++ -g -std=c++1y interpreter-main.cpp interpreter.cpp types.cpp bc-parser.cpp bc-lexer.cpp ../parser/parser.cpp ../parser/lexer.cpp -o interpreter

bc-parser: bc-parser.cpp bc-lexer.cpp bc-parser-driver.cpp
	g++ -g -std=c++1y bc-parser-driver.cpp bc-parser.cpp bc-lexer.cpp -o bc-parser

bc-parser.cpp: bc-parser.yy
	bison -Dapi.prefix={bc} --output=bc-parser.cpp --defines=bc-parser.h bc-parser.yy

bc-lexer.cpp: bc-lexer.lex
	flex -P bc --outfile=bc-lexer.cpp --header-file=bc-lexer.h bc-lexer.lex

clean: 
	rm -f bc-lexer.cpp bc-lexer.h bc-parser.cpp bc-parser.h bc-parser.output bc-parser interpreter compiler *.exe
