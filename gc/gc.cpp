#include "gc.h"
#include "../exception.h"
#include "../types.h"
#include "../frame.h"
#include "../opt/opt_tag_ptr.h"


using namespace std;

/* Collectable */
template<typename T>
size_t Collectable::getVecSize(vector<T> v) {
    size_t overhead = sizeof(v);
    size_t vecSize = v.capacity()*sizeof(T);
    return overhead + vecSize;
}
template<typename VAL>
size_t Collectable::getMapSize(map<string, VAL> m) {
    size_t result = sizeof(m);
    result += m.size()*(sizeof(string) + sizeof(VAL));
    for (auto it = m.begin(); it != m.end(); ++it) {
        result += it->first.size();
    }
    return result;
}
template<typename T>
size_t Collectable::getStackSize(list<T> s) {
    size_t overhead = sizeof(s);
    size_t stackSize = s.size() * sizeof(T);
    return overhead + stackSize;
}

size_t Collectable::getStringSize(string s) {
    return s.size();
}

/* CollectedHeap */
CollectedHeap::CollectedHeap(int maxmem, int currentSize, list<Collectable*>* rtset) {
    // maxmem is in MB
    maxSizeBytes = long(maxmem * 1000000);
    currentSizeBytes = currentSize;
    rootset = rtset;
}
void CollectedHeap::increment(int newMem) {
    // LOG("\tincreased size by " << newMem);
    currentSizeBytes += newMem;
}
int CollectedHeap::count() {
    return allocated.size();
}
long CollectedHeap::getSize() {
    return currentSizeBytes;
}
void CollectedHeap::checkSize() {
    if (currentSizeBytes < 0 || currentSizeBytes > maxSizeBytes) {
        throw RuntimeException("size OOB: " + to_string(currentSizeBytes) + " / " + to_string(maxSizeBytes));
    }
}
void CollectedHeap::registerCollectable(Collectable* c) {
    // LOG("\tincreased size by " << c->getSize());
    currentSizeBytes += c->getSize();
    allocated.push_back(c);
}
template<typename T>
tagptr_t CollectedHeap::allocate() {
    // to be used for None and Record
    T* ret = new T();
    registerCollectable(ret);
    return make_ptr(ret);
}
template<typename T>
T* CollectedHeap::allocate(tagptr_t ptr) {
    // to be used for ValWrapper and Frame
    T* ret = new T(ptr);
    registerCollectable(ret);
    return ret;
}
template<typename T>
T* CollectedHeap::allocate(Function* ptr) {
    // to be used for ValWrapper and Frame
    T* ret = new T(ptr);
    registerCollectable(ret);
    return ret;
}
template<typename T, typename KEY, typename VAL>
tagptr_t CollectedHeap::allocate(map<KEY, VAL> mapping) {
    T* ret = new T(mapping);
    registerCollectable(ret);
    return make_ptr(ret);
}
template<typename T>
T* CollectedHeap::allocate(vector<Function*> functions_,
            vector<tagptr_t> constants_,
            int32_t parameter_count_,
            vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
            vector<string> names_,
            vector<BcInstruction> instructions) {
    T* ret = new T(functions_, constants_, parameter_count_, local_vars_, local_reference_vars_, free_vars_, names_, instructions);
    registerCollectable(ret);
    return ret;
};
Closure* CollectedHeap::allocate(vector<ValWrapper*> refs, Function* func) {
    Closure* ret = new Closure(refs, func);
    registerCollectable(ret);
    return ret;
}
void CollectedHeap::gc() {
    // calls markSuccessors on everything in the root set
    // loop through the allocated ll. if marked = False, deallocate, decrement the size of the collector, and remove from ll. Else, set marked to False
    if (currentSizeBytes > maxSizeBytes / 2) {
        LOG("STARTING GC: size = " << currentSizeBytes << "/" << maxSizeBytes << ", count = " << count() << " rootset size = " << rootset->size());
        // mark stage
        for (auto item = rootset->begin(); item != rootset->end(); ++item) {
            markSuccessors(*item);
        }
        // sweep stage
        // we recount the data we are using to get a more accurate tally
        auto it = allocated.begin();
        while (it != allocated.end()) {
            Collectable* c = *it;
            if (!c->marked) {
                currentSizeBytes -= c->getSize();
                it = allocated.erase(it);
                delete c;
            } else {
                c->marked = false;
                ++it;
            }
        }
        LOG("ENDING GC: size = " << currentSizeBytes << ", count = " << count() << " rootset size = " << rootset->size());
    }
    checkSize();
}

// Declarations for vector size
template class vector<string>;
template size_t Collectable::getVecSize<string>(vector<string>);
template class vector<ValWrapper*>;
template size_t Collectable::getVecSize<ValWrapper*>(vector<ValWrapper*>);
template class vector<Function*>;
template size_t Collectable::getVecSize<Function*>(vector<Function*>);
template class vector<tagptr_t>;
template size_t Collectable::getVecSize<tagptr_t>(vector<tagptr_t>);
template class list<tagptr_t>;
template size_t Collectable::getStackSize(list<tagptr_t>);
template class map<string, tagptr_t>;
template size_t Collectable::getMapSize(map<string, tagptr_t>);
template class map<string, ValWrapper*>;
template size_t Collectable::getMapSize(map<string, ValWrapper*>);

// Declarations for allocate
template tagptr_t CollectedHeap::allocate<Function>();
template tagptr_t CollectedHeap::allocate<None>();
template tagptr_t CollectedHeap::allocate<Record>();
template ValWrapper* CollectedHeap::allocate<ValWrapper>(tagptr_t);
template Frame* CollectedHeap::allocate<Frame>(Function*);

// Declarations for native functions
template PrintNativeFunction* CollectedHeap::allocate<PrintNativeFunction>(vector<Function*>, vector<tagptr_t>, int, vector<string>, vector<string>, vector<string>, vector<string>, vector<BcInstruction>);
template InputNativeFunction* CollectedHeap::allocate<InputNativeFunction>(vector<Function*>, vector<tagptr_t>, int, vector<string>, vector<string>, vector<string>, vector<string>, vector<BcInstruction>);
template IntcastNativeFunction* CollectedHeap::allocate<IntcastNativeFunction>(vector<Function*>, vector<tagptr_t>, int, vector<string>, vector<string>, vector<string>, vector<string>, vector<BcInstruction>);
