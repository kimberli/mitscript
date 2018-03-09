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
        //auto search = localVars.find(var);
        //if (search != localVars.end()) {
        //    return search->second;
        //}
        if (localVars.count(var)) {
            return localVars[var];
        }
        return NULL;
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
        Value* r;
        if (globalVars.count(var)) {
            return getGlobal(var);
        }
        if ((r = getLocal(var)) != NULL) {
            return r;
        }
        // if (globalFrame->localVars.count(var)) {
        //     return getGlobal(var);
        // }
        // if (localVars.count(var)) {
        //     return getLocal(var);
        // }
        // auto gSearch = globalVars.find(var);
        // if (gSearch != globalVars.end()) {
        //     return getGlobal(var);
        // }
        // auto lSearch = localVars.find(var);
        // if (lSearch != localVars.end()) {
        //     return getLocal(var);
        // }
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
