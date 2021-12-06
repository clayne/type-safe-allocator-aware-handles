#include "type_handle.h"
#include <new>

struct MyData {
    int a;
    double b;
    const char* c = "hello";
};

int main() {

    typedef simple_allocator nstd::simple_allocator<MyData, 1000>;

    // Create our allocator for this type handle type
    nstd::handle<MyData, simple_allocator>::allocator = new simple_allocator();

    // Allocate a handle to some data. The handle specifies the type it points to and the allocator responsible 
    // for the data.
    typedef HandleMyData nstd::handle<MyData, simple_allocator>;

    // typedefs are useful to get away from angle bracket madness
    HandleMyData handle = HandleMyData::allocator->allocate();

    // get can get data with ptr() 
    MyData *ptr_my_data = handle.ptr();
    // check if its null 
    if (ptr_my_data == nullptr) {
        printf("this shouldn't be null since we just allocated it!");
        delete HandleMyData::allocator;
        return;
    }

    // deallocate the handle 
    HandleMyData::allocator->deallocate(handle);
    ptr_my_data = handle.ptr();
    if (ptr_my_data == nullptr) {
        printf("this should be null since we deallocated it!")
    }

    // use ref() to get a reference. This will call abort() if its an invalid handle 
    handle = HandleMyData::allocator->allocate();
    HandleMyData& my_data = handle.ref();

    // or we can access it via checked execution.
    // this will check if valid and then execute the functor
    // if its not valid nothing will happen
    handle.checked_execution([](MyData *ptr){ *ptr.a = 10; });
    handle.checked_execution([](MyData *ptr){
        // pointer is guaranteed to be valid (by your allocator)
        // at this point
        *ptr.c = "Hello there"; 
    });

    delete HandleMyData::allocator;
}