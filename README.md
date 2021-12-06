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
they should have been from the start). Allocators give you a type safe handle and with that handle you can copy it to your hearts content without wo

The big problem with handles is "zombie references". Handles that have a stale value. There are strategies to mitigate this, but it does mean that eventually the program will not be 
able to allocate any more handles (this could be by the time the universe ends though).

So with handles dead references should be minimsied as much as possible. You get a similar problem with smart pointers where forgotten smart pointer will 
keep an object alive when it shouldn't. You could make a custom allocator that does a sweep and clear

