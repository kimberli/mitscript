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
        throw UninitializedVariableException(" is not initialized");
    }

    void setLocal(string var, Value* val) {
        localVars[var] = val;
    }

    Value* getGlobal(string var) {
        return globalFrame->getLocal(var);
    }

    void setGlobal(string var) {
        globalVars.insert(var);
    }

    Value* lookup_read(string var) {
        auto gSearch = globalVars.find(var);
        if (gSearch != globalVars.end()) {
            return getGlobal(var);
        }
        auto lSearch = localVars.find(var);
        if (lSearch != localVars.end()) {
            return lSearch->second;
        }
        if (parentFrame == NULL) {
            throw UninitializedVariableException(var + " is not initialized");
        }
        parentFrame->lookup_read(var);
    }

    void assign(string var, Value* val) {
        auto gSearch = globalVars.find(var);
        if (gSearch != globalVars.end()) {
            globalFrame->setLocal(var, val);
            return;
        }
        auto lSearch = localVars.find(var);
        if (lSearch != localVars.end()) {
            setLocal(var, val);
            return;
        }
    }
};
