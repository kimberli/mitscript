#pragma once

#include <memory>

using namespace std;


// You will need to a virtual visitor class with a
// visit method for each different type of expression and statement
// as defined in AST.h

class Visitor {
public:
// For each AST node, you need a virtual method of the form
// virtual void visit(EXPRESSION_TYPE& exp)=0;
};


