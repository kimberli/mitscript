parser: parser.cpp lexer.cpp main.cpp PrettyPrinter.h Visitor.h
	g++ -O2 -g -std=gnu++11 main.cpp parser.cpp lexer.cpp -o mitscript

parser.cpp: parser.yy
	bison --output=parser.cpp --defines=parser.h -v parser.yy

lexer.cpp: lexer.lex
	flex  --outfile=lexer.cpp --header-file=lexer.h lexer.lex

clean:
	rm -f lexer.cpp lexer.h parser.cpp parser.h parser.output *.exe mitscript
