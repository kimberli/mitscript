all: main codegen

main: main.cpp
	g++ main.cpp -std=c++11 -lstdc++ -Ix64asm -L x64asm/lib -lx64asm -o asm-test

codegen: codegen.cpp
	g++ codegen.cpp -std=c++11 -lstdc++ -Ix64asm -L x64asm/lib -lx64asm -o codegen

clean:  
	rm -f asm-test codegen
