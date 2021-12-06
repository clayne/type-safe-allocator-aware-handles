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

Handles are templated and require you to specify the type they point to and the type of the allocator the data belongs to at compile time (hence type safety). In order to use 
the handles, the allocator class must have implemented the `T* handle_deref(nstd::handle<T, F> &handle)` function (where T is the type of data and F is the type of the allocator (the class this function should belong to))

The handle deref function is exactly what it says on the tin. It dereferences the handle. It returns the pointer to the data the handle refers to. It's up to you how you implement this, you can even alter the handle since its passed by reference (this is good for invalidating a handle). Note, all handles are fundamental types, they are simple integral types in some way or another and can't be anything else although you can specifiy what kind of integer you want (unsigned, signed etc)

Before you can use an allocator you must assign the static allocator pointer for the handle.

For instance, let's say you have a handle to a type `struct MyData`. The handle for this could be `nstd::handle<MyData, MyDataAllocator>`. Where `MyDataAllocator` is your custom allocator type.
You must then set the static allocator pointer for this handle to a valid `MyDataAllocator` object. See below

```cpp
  struct MyData { int a; };
  struct MyDataAllocator { // my custom allocator. note that handle_deref() is implemented
  
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
  
    // allocate the allocator for this type. for all handles of this type, this is their allocator.
    nstd::handle<MyData, MyDataAllocator>::allocator = new MyDataAllocator()
  }
```
We can get the value a handle refers to in 3 ways. 

```cpp
    // allocate a value and get the handle 
    nstd::handle<MyData, MyDataAllocator> handle = nstd::handle<MyData, MyDataAllocator>::allocator->allocate();
    
    // get the pointer. will return nullptr if this is an invalid handle
    MyData *ptr = handle.val();
    
    // get the ref. will abort the program is its invalid
    MyData &val = handle.ref();
    
    // executes the functor if the handle is valid when the function is called. this is fancy but impractical
     handle.checked_execution([](MyData *ptr) { ptr->a = 0; });
```

We get type safety because every handle is a different type. So we don't end up making this mistake: 

```cpp
  nstd::handle<int, simple_allocator<int, 100>>       a = 1;
  nstd::handle<double, simple_allocator<double, 100>> b = handle; // compile error 
```

Do we really need this repo? Not really, but the plan is to win smart pointer people over so that they eventually write nice stuff. This is supposed to help them.

## Creating allocators

Handles present a problem which is you end up (eventually or possibly not within the lifespan of the computer if you do it right) zombie references. These are old references that point to memory that is now being reused. You can get around this by being smart about allocations and writing good allocators. You can have an allocator that could do reference counting if so desire or you can make allocators thread safe. The beauty of handles is you can switch out the allocator and it doesn't change the usage of your handles. This is why smart pointers are a bad strategy. Handles shift you back to thinking about allocations (which is what people should have been thinking about at the beginning)

