# include "gc.h" 

/* Collectable */
template<class T>
size_t Collectable::getVecSize(vector<T> v) {
    size_t overhead = sizeof(v);
    size_t vecSize = v.capacity()*sizeof(T);
    return overhead + vecSize;
}

template<typename KEY, typename VAL>
size_t Collectable::getMapSize(map<KEY, VAL> m) {
    size_t overhead = sizeof(m);
    size_t mapSize = m.size()*(sizeof(KEY) + sizeof(VAL));
    return overhead + mapSize;
}

template<typename T>
size_t Collectable::getStackSize(stack<T> s) {
    size_t overhead = sizeof(s);
    size_t stackSize = s.size()*sizeof(T);
    return overhead + stackSize;
}

/* CollectedHeap */
CollectedHeap::CollectedHeap(int maxmem) {
    long maxSizeBytes = maxmem*1000000;
}

int CollectedHeap::count() {
    // returns the size of allocated ll
    throw "unimplemented";
}

void CollectedHeap::registerCollectable(Collectable* c) {
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
    // to be used for strings, ints, bools, ValuePtr, empty records
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
T* CollectedHeap::allocate(vector<ValuePtr*> refs, Function* func) {
    T* ret = new T(refs, func);
    registerCollectable(ret);
    return ret;
}

template<typename ITERATOR>
void CollectedHeap::gc(ITERATOR begin, ITERATOR end) {
    // makes the root set (how???)
    // calls markSucessors on everything in the root set
    // loop through the allocated ll. if marked = False, deallocate, decrement the size of the collector, and remove from ll. Else, set marked to False
    throw "unimplemented";
}

// Declarations so templates are compiled
template class vector<string>;
template size_t Collectable::getVecSize<string>(vector<string>);
template class vector<ValuePtr*>;
template size_t Collectable::getVecSize<ValuePtr*>(vector<ValuePtr*>);
template class vector<Constant*>;
template size_t Collectable::getVecSize<Constant*>(vector<Constant*>);
template class vector<Function*>;
template size_t Collectable::getVecSize<Function*>(vector<Function*>);
//template class vector<Instruction*>;
//template size_t Collectable::getVecSize<Instruction>(vector<Instruction>);
template class stack<Value*>;
template size_t Collectable::getStackSize<Value*>(stack<Value*>);
template class map<string, Value*>;
template size_t Collectable::getMapSize(map<string, Value*>);
template class map<string, ValuePtr*>;
template size_t Collectable::getMapSize(map<string, ValuePtr*>);
