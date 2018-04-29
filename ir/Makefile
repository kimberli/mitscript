all: ir-compiler ir-interpreter

ir-compiler: main.cpp 
	g++ -g -std=c++1y main.cpp -o ir-compiler

ir-interpreter: main.cpp
	g++ -g -std=c++1y main.cpp ir_to_asm.cpp -lstdc++ -Ix64asm -L ../asm/x64asm/lib -lx64asm -o ir-interpreter

clean: 
	rm -f ir-compiler ir-interpreter
