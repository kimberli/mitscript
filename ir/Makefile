all: ir-interpreter

ir-interpreter: main.cpp ir_to_asm.cpp machine_code_func.cpp bc_to_ir.cpp
	g++ -g main.cpp ir_to_asm.cpp bc_to_ir.cpp machine_code_func.cpp -std=c++1y -lstdc++ -Ix64asm -L x64asm/lib -lx64asm -o ir-interpreter

clean:
	rm -f ir-compiler ir-interpreter
