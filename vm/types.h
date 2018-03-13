#pragma once

#include "instructions.h"

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

struct Constant
{

  virtual ~Constant() { }
};

struct None : public Constant
{

  virtual ~None() { }
};

struct Integer : public Constant
{
  Integer(int64_t value) 
  : value(value)
  {

  }

   int64_t value;

   virtual ~Integer() { }
};

struct String : public Constant
{
    String(std::string value)
    : value(value)
    {

    }

    std::string value;

    virtual ~String() { }
};

struct Boolean : public Constant
{
    Boolean(bool value) 
    : value(value)
    {

    }

    bool value;

    virtual ~Boolean() { }
};

struct Function 
{
  // List of functions defined within this function (but not functions defined inside of nested functions)
  std::vector<std::shared_ptr<Function>> functions_;
 
 // List of constants used by the instructions within this function (but not nested functions)
  std::vector<std::shared_ptr<Constant>> constants_;

 // The number of parameters to the function
  uint32_t parameter_count_;
  
  // List of local variables
  // The first parameter_count_ variables are the function's parameters
  // in their order as given in the paraemter list
  std::vector<std::string> local_vars_;
  
    // List of local variables accessed by reference (LocalReference)
  std::vector<std::string> local_reference_vars_;

  // List of the names of non-global and non-local variables accessed by the function
  std::vector<std::string> free_vars_;

  // List of global variable and field names used inside the function
  std::vector<std::string> names_;
    
  InstructionList instructions;
};
