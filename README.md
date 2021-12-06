# C++ Type Safe Allocator Aware Handles 

There is a design which is as old as time which is to use handles instead of pointers to access memory (roughly speaking).
However, that's not fancy and thus people either don't like it or complain about it not being fancy enough. It's not type safe! 
What if I assign one integer handle to another but they refer to different types? The horror!
What if I assign one handle to another and they are the same type but they use a different allocator? Even more horror!

So first and foremost this tackles that issue. Type safe, allocator aware handles that you don't accidentally assign to the wrong thing, that also have zero-overhead (LOL) and 
minimise memory safety problems issues (although not entirely obviously)

In practice i'd probably just stick with `int handle = 10`, but this way has more angle brackets so must be better right?

The other issue is smart pointer abuse. Smart pointers are BAD and encourage
bad design and sloppy code. Pointers should not be responsible for the memory they point to. Allocators should be responsible for that. But people didn't want to write allocators
and so they made their life way harder with smart pointers. Cyclic references, fragmented memory, confusing semantics and life times, all of that comes with smart pointers. And on 
top of that they are really slow. C++ with std::shared_ptr is bad, bad, bad!

This repo doesn't solve all the problems that smart pointers solve. It's just a different way to handle memory and a different way of thinking of things. 
It moves responsibilities away from smart pointers and places them with the allocators (where 
they should have been from the start). Allocators give you a type safe handle and with that you can access the data to your hearts content.

The big problem with handles is "zombie references". Handles that have a stale value. There are strategies to mitigate this, but it does mean that eventually the program will not be 
able to allocate any more handles (this could be by the time the universe ends though).

So with handles, dead references should be minimised as much as possible by your design. You get a similar problem with smart pointers where forgotten smart pointer will 
keep an object alive when it shouldn't. The only way to properly solve this is to make a custom allocator that does a sweep and clear

## Example 

Handles are templated and require you to specify the type they point to and the type of the allocator the data belongs to. In order to use 
the handles, the allocator class must have implemented the `T* handle_deref(nstd::handle<T, F> &handle)` function (where T is the type of the data and F is the type of the allocator (the class this function should belong to))

The handle deref function is exactly what it describes. It dererferences the handle. It returns the pointer the handle refers to. It's up to you how you implement this, but not that all handles are fundamental types, they are simple integral types in some way or another and can't be anything else.

You must also assign the static allocator pointer for you specific handle type. 

Lets say you have a handle to you type `struct MyData`. The handle for this could be `nstd::handle<MyData, MyDataAllocator>`. Where `MyDataAllocator` is your custom allocator type.
You must then set the static allocator pointer for this handle to a `MyDataAllocator` object.

```cpp
  struct MyData { int a; };
  struct MyDataAllocator { 
  
    MyData data;
    bool is_allocated = false;
  
    nstd::handle<MyData, MyDataAllocator> allocate() { 
        if (is_allocated) return 0; // if its allocated return 0, 0 being an invalid handle
        is_allocated = true;
        return 1;
    }
    
    MyData* handle_deref(nstd::handle<MyData, MyDataAllocator> &handle) { 
      if (handle.id != 1) return nullptr;
      if (is_allocated == false) return nullptr;
      return &data;
    }
  };

  int main() {
  
    // allocate the allocator for this type.
    nstd::handle<MyData, MyDataAllocator>::allocator = new MyDataAllocator()
    
    // allocate a value and get the handle 
    nstd::handle<MyData, MyDataAllocator> handle = nstd::handle<MyData, MyDataAllocator>::allocator->allocate();
    
    // get the pointer
    MyData *ptr = handle.val();
    
    // get the ref 
    MyData &val = handle.ref();
    
    // do checked execution 
     handle.checked_execution([](MyData *ptr) { ptr->a = 0; });
  }
```




