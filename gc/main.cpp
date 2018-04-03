#include "vm/types.h"
#include "timerclass.h"
#include <cstdlib>

using namespace std;

void allocate_objects(int N,  CollectedHeap& heap, vector<string>& names, vector<Value*>& rootset) {
	// The loop below allocates a large number of objects and connects them together in a semi-random manner, 
	// adding some subset of them to the rootset.
	Record* r = heap.allocate<Record>();
	Record* q = heap.allocate<Record>();
	for (int i = 0; i < N; ++i) {

		Integer* ii = heap.allocate<Integer>(5);
		String* s = heap.allocate<String>("name");


		int t = rand() % 3;
		if (t == 0) { r->set(names[i % names.size()], ii); }
		if (t == 1) { r->set(names[i % names.size()], s); }
		if (t == 2) { r->set(names[i % names.size()], q); }

		if (rand() % 7 == 0) {
			q = r;
		}

		if (rand() % 7 == 0) {
			r = heap.allocate<Record>();
		}

		if (rand() % 10 == 0) {
			rootset.push_back(r);
		}
	}
}


/*
This is a simple randomized tester for your garbage collector which can serve as a basic sanity check 
and can be helpful when debugging.
*/
int main(int argc, char** argv) {


	if (argc > 1) {
		// If you pass an argument to the main function, that argument is expected to be a number, and the 
		// random number generator will be seeded from this argument. If you run twice with the same seed you should
		// get see repeatable results.
		int s = atoi(argv[1]);
		cout << " seed " << s << endl;
		srand(s);
	}

	//A few names that will be used as part of the test.
	vector<string> names;
	names.push_back("foo");
	names.push_back("baz");
	names.push_back("x");
	names.push_back("y");
	names.push_back("z");

	
	
	//initialize the timer to keep track of time.
	timerclass timer("Total Time");

	timer.start();

	//Define a Collected heap.
	CollectedHeap heap;

	vector<Value*> rootset;
	

	int N = 100000;
	
	allocate_objects(N, heap, names, rootset);

	timer.stop().print("After allocation"); // stops the timer and prints the current time.

	timer.restart(); // starts a new phase of the timer.
	
	int before = heap.getSize();
	heap.gc(rootset.begin(), rootset.end()); //garbage collect on the current rootset.
	int after1 = heap.getSize();
	rootset.clear(); // clears the rootset.
	heap.gc(rootset.begin(), rootset.end()); // garbage collects again; this time we expect to get an empty heap.
	timer.stop(); // stops the second phase of the timer.

	cout << "Heap size before gc: " << before << endl;
	cout << "Heap size after first gc: " << after1 << endl;
	cout << "Heap size after gc: " << heap.getSize() << endl;
	

	timer.print(); // prints the time for both the second phase and the sum of all phases.



}
