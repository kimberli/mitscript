#pragma once 

#include "cfg.h"
#include "types.h"
#include "instructions.h"

#include <memory>
#include <iostream>

std::shared_ptr<Function> CFGToFunc(cfgptr_t cfg);
