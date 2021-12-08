#include "type_handle.h"

#include <stdio.h>
#include <vector>
#include <set>

struct MyClass {
    int a;
    MyClass(int a_in) {
        a = a_in;
    }
};

int main() {

    typedef nstd::simple_allocator<std::vector<int>, 10>   AllocatorVec;
    typedef nstd::handle<std::vector<int>, AllocatorVec> HandleVec;

    typedef nstd::simple_allocator<std::set<HandleVec>, 5>  AllocatorSet;
    typedef nstd::handle<std::set<HandleVec>, AllocatorSet> HandleSet;

    typedef nstd::simple_allocator<MyClass, 4> AllocatorMyClass;
    typedef nstd::handle<MyClass, AllocatorMyClass> HandleMyClass;

    // Create a set of allocators for different types.
    // The simple allocator will allow this.
    AllocatorVec allocatorVec;
    AllocatorSet allocatorSet;
    AllocatorMyClass allocatorMyClass;

    // Allocate with the constructor
    HandleMyClass hMyClass = HandleMyClass::allocator->allocate(10);

    HandleVec hVec1 = HandleVec::allocator->allocate();
    HandleVec hVec2 = HandleVec::allocator->allocate();
    HandleSet hSet = HandleSet::allocator->allocate();

    // Can access the pointer of the hVec1
    std::vector<int>* ptr = hVec1.ptr();
    if (ptr != nullptr) {
        ptr->push_back(1);
    }
    // Can access via reference. This will abort if the reference is invalid
    hVec1.ref().push_back(10);

    // Can compose references inside other objects
    hSet.ref().insert(hVec1);   

    // Can encapuslate "unsafe" dereferences.
    // Checked execution executes code if the pointer is valid
    hSet.checked_execution([&hVec2](std::set<HandleVec> *setPtr) {
        setPtr->insert(hVec2);
    });

    // handles are just integers so we can do comparisons
    if (hVec1 == hVec2) {
       
    }
    else if (hVec1 != hVec2) {

    }
    else if (hVec1 <= hVec2) {

    }

    // But because the handles are type safe its impossible to assign
    // one handle to the other
    // hSet == hVec1; // <--- compile error 
}
