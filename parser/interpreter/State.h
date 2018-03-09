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
        // only called when you're sure var is defined locally
        if (localVars.count(var)) {
            return localVars[var];
        }
        throw RuntimeException("should never get here");
    }

    void setLocal(string var, Value* val) {
        localVars[var] = val;
    }

    Value* getGlobal(string var) {
        if (parentFrame == NULL) {
            return getLocal(var);
        }
        return globalFrame->getLocal(var);
    }

    void setGlobal(string var) {
        globalVars.insert(var);
    }

    Value* lookup_read(string var) {
        if (globalVars.count(var)) {
            return getGlobal(var);
        }
        if (localVars.count(var)) {
            return getLocal(var);
        }
        if (parentFrame == NULL) {
            throw UninitializedVariableException(var + " is not initialized");
        }
        parentFrame->lookup_read(var);
    }

    void assign(string var, Value* val) {
        LOG(1, "\t starting assign");
        auto gSearch = globalVars.find(var);
        if (gSearch != globalVars.end()) {
            LOG(1, "\tState: setting global " + var);
            globalFrame->setLocal(var, val);
            return;
        }
        LOG(1, "\tState: setting local " + var);
        setLocal(var, val);
        return;
    }
};
