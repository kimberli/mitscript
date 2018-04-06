/*
 * cfgtofunc.h
 *
 * Defines a function that converts a CFG data structure to a Function
 * object (as defined in types.h)
 */
#pragma once 

#include "cfg.h"
#include "types.h"
#include "instructions.h"

#include <memory>
#include <iostream>

std::shared_ptr<Function> CFGToFunc(cfgptr_t cfg);
