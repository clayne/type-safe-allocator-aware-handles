#include <type_traits>
#include <cstdlib>
#include <cstdint>

namespace nstd { // non-standard library

    template<typename T, typename Allocator, typename HandleType = uint64_t>
    struct handle {

        static_assert(std::is_fundamental<HandleType>());

        private:
            HandleType id; 
        public:
            friend Allocator;
            static Allocator *allocator;

        handle() = delete;

        inline handle(HandleType handle_in) : id(handle_in) { };

        inline T* ptr() {
            return allocator->handle_deref(*this);
        } 

        inline T& ref() {
            T *ptr = allocator->handle_deref(*this);
            if (ptr == nullptr) abort(); 
            return *ptr;
        }

        // Executes the function if, when this function is called, 
        // the handle is valid.
        template<typename Func>
        inline void checked_execution(Func f) {
            T *ptr = allocator->handle_deref(*this);
            if (ptr == nullptr) { return; }
            f(ptr);
        }
    };
    template<typename T, typename Allocator, typename HandleType>
    Allocator* handle<T, Allocator, HandleType>::allocator = nullptr;

    // eventually the counter in a simple allocator has to wrap 
    // back around. This means it will allocate you handles 
    // that you may have already stored (zombie references). So bear that in mind
    // Because the counter is a uint32_t it will take 4294967294 allocations 
    // before this starts to happen 
    template<typename T, uint32_t capacity>
    struct simple_allocator {

        T data[capacity];
        uint32_t slot[capacity];
        uint32_t counter = 1;

        simple_allocator() {
            for (uint64_t i = 0; i < capacity; i++) {
                slot[i] = 0;
            }
        }

        nstd::handle<T, simple_allocator> allocate() {
            
            for (uint32_t i = 0; i < capacity; i++) {
                if (slot[i] == 0) {

                    uint64_t handle = (uint64_t)i << 32;
                    handle |= (uint64_t)counter;
                    slot[i] = 1;

                    counter++;      
                    if (counter == 0) counter++; // it wrapped around, uh oh
                    
                    return handle;
                }
            }

            return 0; // 0 is an invalid handle
        }

        void deallocate(nstd::handle<T, simple_allocator> handle) {
            uint32_t index = handle.id >> 32;
            if (index >= capacity) return;
            slot[index] = 0;
        }

        T* handle_deref(nstd::handle<T, simple_allocator> &handle) {

            if (handle.id == 0) return nullptr; 
            uint32_t index = handle.id >> 32;

            if (index >= capacity) {
                handle.id = 0;
                return nullptr;
            }
            if (slot[index] == 0) {
                handle.id = 0;
                return nullptr;
            }
            return &data[index];
        }
    };
}