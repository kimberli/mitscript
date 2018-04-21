#include "gc.h"
#include "../vm/exception.h"
#include "../vm/frame.h"
#include "../vm/types.h"

using namespace std;

/* Collectable */
template<class T>
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
CollectedHeap::CollectedHeap(int maxmem, int currentSize, list<Frame*>* frames) {
    // maxmem is in MB
    maxSizeBytes = long(maxmem * 1000000);
    currentSizeBytes = currentSize;
    rootset = frames;
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
T* CollectedHeap::allocate() {
    T* ret = new T();
    registerCollectable(ret);
    return ret;
}
template<typename T, typename ARG>
T* CollectedHeap::allocate(ARG a) {
    // to be used for strings, ints, bools, ValWrapper, empty records
    T* ret = new T(a);
    registerCollectable(ret);
    return ret;
}
template<typename T, typename KEY, typename VAL>
T* CollectedHeap::allocate(map<KEY, VAL> mapping) {
    T* ret = new T(mapping);
    registerCollectable(ret);
    return ret;
}
template<typename T>
T* CollectedHeap::allocate(vector<Function*> functions_,
            vector<Constant*> constants_,
            int32_t parameter_count_,
            vector<string> local_vars_,
            vector<string> local_reference_vars_,
            vector<string> free_vars_,
            vector<string> names_,
            vector<Instruction> instructions) {
    T* ret = new T(functions_, constants_, parameter_count_, local_vars_, local_reference_vars_, free_vars_, names_, instructions);
    registerCollectable(ret);
    return ret;
};
template<typename T>
T* CollectedHeap::allocate(vector<ValWrapper*> refs, Function* func) {
    T* ret = new T(refs, func);
    registerCollectable(ret);
    return ret;
}
void CollectedHeap::gc() {
    // calls markSuccessors on everything in the root set
    // loop through the allocated ll. if marked = False, deallocate, decrement the size of the collector, and remove from ll. Else, set marked to False
    if (currentSizeBytes > maxSizeBytes / 2) {
        LOG("STARTING GC: size = " << currentSizeBytes << "/" << maxSizeBytes << ", count = " << count());
        // mark stage
        for (auto frame = rootset->begin(); frame != rootset->end(); ++frame) {
            markSuccessors(*frame);
        }
        // sweep stage
        // we recount the data we are using to get a more accurate tally
        auto it = allocated.begin();
        while (it != allocated.end()) {
            Collectable* c = *it;
            if (!c->marked) {
                // LOG("\tdecreased size by " << c->getSize() << " @ " << c);
                currentSizeBytes -= c->getSize();
                it = allocated.erase(it);
                delete c;
            } else {
                // LOG("\tskipped @ " << c);
                c->marked = false;
                ++it;
            }
        }
        for (auto frame = rootset->begin(); frame != rootset->end(); ++frame) {
            (*frame)->func->marked = false;
        }
        LOG("ENDING GC: size = " << currentSizeBytes << ", count = " << count());
    }
    checkSize();
}

// Declarations for vector size
template class vector<string>;
template size_t Collectable::getVecSize<string>(vector<string>);
template class vector<ValWrapper*>;
template size_t Collectable::getVecSize<ValWrapper*>(vector<ValWrapper*>);
template class vector<Constant*>;
template size_t Collectable::getVecSize<Constant*>(vector<Constant*>);
template class vector<Function*>;
template size_t Collectable::getVecSize<Function*>(vector<Function*>);
template class list<Value*>;
template size_t Collectable::getStackSize<Value*>(list<Value*>);
template class map<string, Value*>;
template size_t Collectable::getMapSize(map<string, Value*>);
//template size_t Collectable::getStringMapSize(map<string, Value*>);
template class map<string, ValWrapper*>;
template size_t Collectable::getMapSize(map<string, ValWrapper*>);

// Declarations for allocate
template Function* CollectedHeap::allocate<Function>();
template None* CollectedHeap::allocate<None>();
template Boolean* CollectedHeap::allocate<Boolean>(bool);
template String* CollectedHeap::allocate<String>(string);
template Integer* CollectedHeap::allocate<Integer>(int);
template Record* CollectedHeap::allocate<Record>();
template Closure* CollectedHeap::allocate<Closure>(vector<ValWrapper*>, Function*);
template Frame* CollectedHeap::allocate<Frame, Function*>(Function* func);
template ValWrapper* CollectedHeap::allocate<ValWrapper, Constant*>(Constant*);

// Declarations for native functions
template PrintNativeFunction* CollectedHeap::allocate<PrintNativeFunction>(std::vector<Function*, std::allocator<Function*> >, std::vector<Constant*, std::allocator<Constant*> >, int, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<Instruction, std::allocator<Instruction> >);
template InputNativeFunction* CollectedHeap::allocate<InputNativeFunction>(std::vector<Function*, std::allocator<Function*> >, std::vector<Constant*, std::allocator<Constant*> >, int, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<Instruction, std::allocator<Instruction> >);
template IntcastNativeFunction* CollectedHeap::allocate<IntcastNativeFunction>(std::vector<Function*, std::allocator<Function*> >, std::vector<Constant*, std::allocator<Constant*> >, int, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::vector<Instruction, std::allocator<Instruction> >);
