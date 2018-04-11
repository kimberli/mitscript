#pragma once
#include <cstdio>
#include <list>

class CollectedHeap;

//Any object that inherits from collectable can be created and tracked by the garbage collector.
class Collectable {
private:
	//Any private fields you add to the Collectable class will be accessible by the CollectedHeap 
	//(since it is declared as friend below). You can think of these fields as the header for the object, 
	//which will include metadata that is useful for the garbage collector.
    bool marked;
    int size;
protected:
	/*
	The mark phase of the garbage collector needs to follow all pointers from the collectable objects, check 
	if those objects have been marked, and if they have not, mark them and follow their pointers. 
	The simplest way to implement this is to require that collectable objects implement a follow method 
	that calls heap.markSuccessors( ) on all collectable objects that this object points to.
	markSuccessors() is the one responsible for checking if the object is marked and marking it.
	*/
	virtual void follow(CollectedHeap& heap)=0;
	friend CollectedHeap;
};

/*
This class keeps track of the garbage collected heap. The class must do all of the following: 
	- provide an interface to allocate objects that will be supported by garbage collection.
	- keep track of all currently allocated objects.
	- keep track of the number of currently allocated objects.
	- 
*/
class CollectedHeap {
private 
    int currentSize;
    std::list<Collectable> allocated;
public:

	/*
	The constructor should take as an argument the maximum size of the garbage collected heap. 
	You get to decide what the units of this value should be. Your VM should compute
	this value based on the -mem parameter passed to it. Keep in mind, however, that
	your VM could be using some extra memory that is not managed by the garbage collector, so 
	make sure you account for this.
	*/
	CollectedHeap(int maxmem) 
	{
	
	}


	/*
	return number of objects in the heap.
	This is different from the size of the heap, which should also be tracked
	by the garbage collector.
	*/
	int count() 
	{
	    // returns the size of allocated ll 
	}

	/*
	This method allocates an object of type T using the default constructor (with no parameters).
	T must be a subclass of Collectable. Before returning the object, it should be registered so that
	it can be deallocated later.	
	*/
	template<typename T>
	T* allocate() 
	{
	    // allocate an object using new 
        // estimate the size of the object 
        // register the object's size in the object's header and 
        //     by adding it to this.currentSize
        // put the object in the allocated ll so it can be garbage 
        //    collected
        // return the pointer
	}


	/*
	A variant of the method above; this version of allocate can be used to allocate objects whose constructor
	takes one parameter. Useful when allocating Integer or String objects.
	*/
	template<typename T, typename ARG>
	T* allocate(ARG a) 
	{
	    // see above
	}

	/*
	For performance reasons, you may want to implement specialized allocate methods to allocate particular kinds of objects.
	
	*/
    // TODO probably need custom allocators for functions, records, closures, etc
    // so that we can do a customized estimation of how much space they take


	/*
	This is the method that is called by the follow(...) method of a Collectable object. This 
	is how a Collectable object lets the garbage collector know about other Collectable otjects pointed to 
	by itself.
	*/
	inline void markSuccessors(Collectable* next) 
	{
    	// for each child, it not already marked, set marked to true and
        // call next.follow()
	}

	/*
	The gc method should be called by your VM (or by other methods in CollectedHeap) 
	whenever the VM decides it is time to reclaim memory. This method
	triggers the mark and sweep process.

	The ITERATOR type should support comparison, assignment and the ++ operator. 
	I should also be able to dereference an interator to get a Collectable object.
	This code will take iterators marking the [begin, end) range of the rootset
	
	*/
	template<typename ITERATOR>
	void gc(ITERATOR begin, ITERATOR end) 
	{
        // makes the root set (how???)
        // calls markSucessors on everything in the root set 
        // loop through the allocated ll. if marked = False, deallocate, decrement the size of the collector, and remove from ll. Else, set marked to False
	}
};


