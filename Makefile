CXX = g++
CXXFLAGS ?= -O0 -g -std=c++1y -Wreturn-type -Ix64asm

SRC_DIRS = ./parser/ms ./parser/bc ./asm ./bc ./gc ./ir ./vm ./
SRCS := $(shell find $(SRC_DIRS) -name \"*.cpp\")
OBJS := $(SRCS)/%.o
DEPS := $(OBJS:.o=.d)

MS_PARSER = parser/ms
MS_PARSER_OBJS = parser/ms/parser.o parser/ms/lexer.o
BC_PARSER = parser/bc
BC_PARSER_OBJS = parser/bc/parser.o parser/bc/lexer.o
BC_COMPILER_OBJS = bc/bc-compiler.o bc/symboltable.o gc/gc.o frame.o types.o
BC_COMPILER_HEADERS = bc/*.h gc/*.h frame.h types.h exception.h instructions.h parser/bc/printer.h
VM_OBJS = vm/interpreter.o ir/bc_to_ir.o asm/ir_to_asm.o asm/helpers.o machine_code_func.o $(BC_COMPILER_OBJS)
VM_HEADERS = vm/*.h ir/*.h asm/*.h ir.h $(BC_COMPILER_HEADERS)
ROOT_FILES = $(shell find . -name \"*.o\")
REF = ref
REF_OBJS = ref/Value.o

.PHONY: clean-ms-parser
.PHONY: clean-bc-parser
.PHONY: clean-ref
.PHONY: clean-bc-compiler
.PHONY: clean-interpreter

default: interpreter

clean: clean-ms-parser clean-bc-parser clean-ref clean-bc-compiler clean-interpreter

## EXECUTABLES

# make the MITScript pretty printer
ms-print: $(MS_PARSER)/ms-print
$(MS_PARSER)/ms-print: $(MS_PARSER)/print_main.cpp $(MS_PARSER_OBJS)
	$(CXX) $(CXXFLAGS) $(MS_PARSER)/print_main.cpp $(MS_PARSER_OBJS) -o $@

# make the bytecode pretty printer
bc-print: $(BC_PARSER)/bc-print
$(BC_PARSER)/bc-print: $(BC_PARSER)/print_main.cpp $(BC_PARSER_OBJS)
	$(CXX) $(CXXFLAGS) $(BC_PARSER)/print_main.cpp $(BC_PARSER_OBJS) types.cpp frame.cpp gc/gc.cpp -o $@

# MITScript -> bytecode compiler
bc-compiler: mitscriptc
mitscriptc: $(MS_PARSER_OBJS) $(BC_COMPILER_OBJS) $(BC_COMPILER_HEADERS) bc/compiler-main.cpp
	$(CXX) $(CXXFLAGS) bc/compiler-main.cpp $(BC_COMPILER_OBJS) $(MS_PARSER_OBJS) -o $@

# MITScript -> bytecode -> IR -> vm
interpreter: mitscript
mitscript: $(MS_PARSER_OBJS) $(BC_PARSER_OBJS) $(ROOT_FILES) $(VM_OBJS) $(VM_HEADERS) vm/interpreter-main.cpp
	$(CXX) $(CXXFLAGS) vm/interpreter-main.cpp $(VM_OBJS) $(BC_PARSER_OBJS) $(MS_PARSER_OBJS) -lstdc++ -L x64asm/lib -lx64asm -o $@
	
# reference interpreter (from a2)
ref: ref/mitscript
ref/mitscript: $(REF)/ref-main.cpp $(REF_OBJS) $(MS_PARSER_OBJS)
	$(CXX) $(CXXFLAGS) $(REF)/ref-main.cpp $(REF_OBJS) $(MS_PARSER_OBJS) -o $@


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
	rm -f $(MS_PARSER)/lexer.cpp $(MS_PARSER)/lexer.h $(MS_PARSER)/parser.cpp $(MS_PARSER)/parser.h $(MS_PARSER)/parser.output $(MS_PARSER_OBJS) $(MS_PARSER)/ms-print

clean-bc-parser:
	rm -f $(BC_PARSER)/lexer.cpp $(BC_PARSER)/lexer.h $(BC_PARSER)/parser.cpp $(BC_PARSER)/parser.h $(BC_PARSER)/parser.output $(BC_PARSER_OBJS) $(BC_PARSER)/bc-print

clean-ref:
	rm -f ref/mitscript $(REF_OBJS)

clean-bc-compiler:
	rm -f mitscriptc $(BC_COMPILER_OBJS)

clean-interpreter:
	rm -f mitscript $(VM_OBJS)

-include $(DEPS)
