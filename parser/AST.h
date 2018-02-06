#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include "Visitor.h"

#define Assert(cond, msg) if(!(cond)){ std::cerr<<msg<<endl; throw SystemException("Bad stuff"); }

using namespace std;

class SystemException {
	string msg_;
public:
	SystemException(const string& msg) :msg_(msg) {}
};


class AST_node {

public:
	virtual void accept(Visitor& v) = 0;
};

class Expression : public AST_node {
public:

};

class Statement: public AST_node {

};

// You need to define classes that inherit from Expression and Statement
// for all the different kinds of expressions and statements in the language.



