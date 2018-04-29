# pragma once 

#include "ir.h"

class IrInterpreter {
private: 
    vptr<IrProgram> program;
    vptr<IrFunc> curFunc;
    int instructionIndex;
    bool finished;

    void executeStep();
public: 
    IrInterpreter(vptr<IrProgram> irProgram); 
    void run(); // runs the program 
};
