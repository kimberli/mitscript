mitscript-parser: MITScriptLexer.cpp MITScriptParser.cpp main.cpp
	g++ -std=c++11 -Iinclude -Llib MITScriptLexer.cpp MITScriptParser.cpp MITScriptBaseVisitor.cpp MITScriptVisitor.cpp main.cpp -lantlr4-runtime -o mitscript-parser 

MITScriptLexer.cpp MITScriptParser.cpp: MITScript.g.intermediate
	@# Empty recipe

.INTERMEDIATE: MITScript.g.intermediate

MITScript.g.intermediate: MITScript.g
	java -Xmx500M -cp "bin/antlr-4.7.1-complete.jar:$CLASSPATH" org.antlr.v4.Tool -Dlanguage=Cpp -visitor -no-listener MITScript.g

clean:
	rm -f MITScriptLexer.h MITScriptLexer.cpp MITScriptParser.h MITScriptParser.cpp mitscript-parser MITScript.interp MITScriptLexer.interp MITScriptLexer.tokens MITScript.tokens MITScriptBaseVisitor.h MITScriptBaseVisitor.cpp MITScriptVisitor.h MITScriptVisitor.cpp 
