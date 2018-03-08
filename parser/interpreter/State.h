#pragma once

#include "Value.h"
#include <string>
#include <map>
#include <set>

using namespace std;

class Frame {
public:
    map<string, Value*> localVars;
    Frame* parentFrame;
    Frame* globalFrame;
    set<string> globalVars;

    Frame() {};
    Frame(Frame* parent, Frame* global):
        parentFrame(parent), globalFrame(global) {};

    Value* getLocal(string var) {
        auto search = localVars.find(var);
        if (search != localVars.end()) {
            return search->second;
        }
        throw UninitializedVariableException();
    }

    void setLocal(string var, Value* val) {
        localVars[var] = val;
    }

    Value* getGlobal(string var) {
        return globalFrame->getLocal(var);
    }

    void setGlobal(string var, Value* val) {
        globalVars.insert(var);
        globalFrame->setLocal(var, val);
    }

    Value* lookup(string var) {
        auto search = localVars.find(var);
        if (search != localVars.end()) {
            return search->second;
        }
        parentFrame->lookup(var);
    }
};
