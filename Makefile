ms-parser-print: parser/ms/print_main.cpp
	g++ -O2 -g -std=gnu++11 parser/ms/print_main.cpp parser/ms/parser.cpp parser/ms/lexer.cpp -o parser/ms/mitscript-print

ms-parser: parser/ms/parser.yy
	bison --output=parser/ms/parser.cpp --defines=parser/ms/parser.h -v parser/ms/parser.yy
	
parser/ms/lexer.cpp: parser/ms/lexer.lex
	flex  --outfile=parser/ms/lexer.cpp --header-file=parser/ms/lexer.h parser/ms/lexer.lex

clean-ms-parser:
	rm -f parser/ms/lexer.cpp parser/ms/lexer.h parser/ms/parser.cpp parser/ms/parser.h parser/ms/parser.output

