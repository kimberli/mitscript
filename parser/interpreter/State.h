#pragma once

#include "Value.h"
#include <string>
#include <map>
#include <set>

#define LOG(lvl, msg) { if (lvl > 10) std::cerr << msg << endl; }

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
