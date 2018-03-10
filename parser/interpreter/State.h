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
    Frame* callerFrame;
    Frame* globalFrame;
    set<string> globalVars;

    Frame() {};
    Frame(Frame* parent, Frame* caller, Frame* global):
        parentFrame(parent), callerFrame(caller), globalFrame(global) {};

    Value* getLocal(string var) {
        if (localVars.count(var)) {
            return localVars[var];
        }
        throw UninitializedVariableException(var + " is not initialized");
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
        return parentFrame->lookup_read(var);
    }

    void assign(string var, Value* val) {
        if (globalVars.count(var)) {
            LOG(1, "\tState: setting global " + var);
            globalFrame->setLocal(var, val);
            return;
        }
        LOG(1, "\tState: setting local " + var);
        setLocal(var, val);
        return;
    }
};
