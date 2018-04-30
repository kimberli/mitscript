CC = g++
CC_FLAGS = -O2 -g -std=c++1y

MS_PARSER = parser/ms
MS_PARSER_SRC = parser/ms/parser.cpp parser/ms/lexer.cpp
BC_PARSER = parser/bc
BC_PARSER_SRC = parser/bc/parser.cpp parser/bc/lexer.cpp
BC_COMPILER_SRC = bc/bc-compiler.cpp bc/symboltable.cpp gc/gc.cpp frame.cpp types.cpp
VM_SRC = vm/interpreter.cpp $(BC_COMPILER_SRC)
IR_SRC = ir/bc_to_ir.cpp asm/ir_to_asm.cpp asm/helpers.cpp machine_code_func.cpp
ROOT_FILES = *.h *.cpp
REF = ref

default: interpreter

clean: clean-ms-parser clean-bc-parser clean-ref clean-bc-compiler clean-interpreter

## EXECUTABLES

# make the MITScript pretty printer
ms-print: $(MS_PARSER)/ms-print
$(MS_PARSER)/ms-print: $(MS_PARSER)/print_main.cpp $(MS_PARSER_SRC)
	$(CC) $(CC_FLAGS) $(MS_PARSER)/print_main.cpp $(MS_PARSER_SRC) -o $@

# make the bytecode pretty printer
bc-print: $(BC_PARSER)/bc-print
$(BC_PARSER)/bc-print: $(BC_PARSER)/print_main.cpp $(BC_PARSER_SRC)
	$(CC) $(CC_FLAGS) $(BC_PARSER)/print_main.cpp $(BC_PARSER_SRC) types.cpp frame.cpp gc/gc.cpp -o $@

# MITScript -> bytecode compiler
bc-compiler: mitscriptc
mitscriptc: bc/* gc/* $(MS_PARSER)/parser.cpp $(BC_PARSER)/parser.cpp $(ROOT_FILES)
	$(CC) $(CC_FLAGS) bc/compiler-main.cpp $(BC_COMPILER_SRC) $(MS_PARSER_SRC) -o $@

# MITScript -> bytecode -> IR -> vm
interpreter: mitscript
mitscript: bc/* gc/* ir/* asm/* vm/* $(MS_PARSER)/parser.cpp $(BC_PARSER)/parser.cpp $(ROOT_FILES)
	$(CC) $(CC_FLAGS) vm/interpreter-main.cpp $(IR_SRC) $(VM_SRC) $(BC_PARSER_SRC) $(MS_PARSER_SRC) -lstdc++ -Ix64asm -L x64asm/lib -lx64asm -o $@
	
# reference interpreter (from a2)
ref: ref/mitscript
ref/mitscript: $(REF)/ref-main.cpp $(REF)/*.cpp $(REF)/*.h Visitor.h AST.h $(MS_PARSER_SRC)
	$(CC) $(CC_FLAGS) $(REF)/*.cpp $(MS_PARSER_SRC) -o $@


## TESTS
test: test-compiler test-vm test-interpreter

test-compiler: mitscriptc
	tests/vm/test_compiler.sh

test-vm: mitscript
	tests/vm/test_vm.sh

test-interpreter: mitscript
	tests/vm/test_interpreter.sh

test-mem: mitscript
	tests/gc/test_massif.sh

test-parser: $(MS_PARSER)/ms-print
	tests/parser/test_parser.sh


## OTHER FORMULAS

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


## CLEAN UP
clean-ms-parser:
	rm -f $(MS_PARSER)/lexer.cpp $(MS_PARSER)/lexer.h $(MS_PARSER)/parser.cpp $(MS_PARSER)/parser.h $(MS_PARSER)/parser.output $(MS_PARSER)/ms-print

clean-bc-parser:
	rm -f $(BC_PARSER)/lexer.cpp $(BC_PARSER)/lexer.h $(BC_PARSER)/parser.cpp $(BC_PARSER)/parser.h $(BC_PARSER)/parser.output $(BC_PARSER)/bc-print

clean-ref:
	rm -f ref/mitscript

clean-bc-compiler:
	rm -f mitscriptc

clean-interpreter:
	rm -f mitscript
