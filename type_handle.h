#include <type_traits>
#include <cstdlib>
#include <cstdint>

namespace nstd { // non-standard library

    template<typename T, typename Allocator, typename HandleType = uint64_t>
    struct handle {

        static_assert(std::is_fundamental<HandleType>(), "The handle type of a handle must be a fundamental type");

    private:
        HandleType id;
    public:
        friend Allocator;
        static Allocator* allocator;

        handle() {}

        inline handle(HandleType handle_in) : id(handle_in) {};

        // I hate having to do this in C++. I just want this class to
        // behave like an integer
        friend bool operator <  (handle lhs, handle rhs) { return lhs.id < rhs.id; }
        friend bool operator <= (handle lhs, handle rhs) { return lhs.id <= rhs.id; }
        friend bool operator >  (handle lhs, handle rhs) { return lhs.id < rhs.id; }
        friend bool operator >= (handle lhs, handle rhs) { return lhs.id <= rhs.id; }
        friend bool operator == (handle lhs, handle rhs) { return lhs.id == rhs.id; }
        friend bool operator != (handle lhs, handle rhs) { return lhs.id != rhs.id; }

        inline T* ptr() {
            return allocator->handle_deref(*this);
        }

        inline T& ref() {
            T* ptr = allocator->handle_deref(*this);
            if (ptr == nullptr) abort();
            return *ptr;
        }

        // Executes the function if, when this function is called, 
        // the handle is valid.
        template<typename Func>
        inline void checked_execution(Func f) {
            T* ptr = allocator->handle_deref(*this);
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

        T* data = nullptr;
        bool* slot = nullptr;
        uint32_t counter = 1;

        simple_allocator() {

            // we can assign the typed handle allocator here in the constructor.
            // there can only be one allocator per type. may rethink this design choice
            nstd::handle<T, simple_allocator>::allocator = this;

            data = (T*)malloc(sizeof(T) * capacity);
            if (data == nullptr) abort();
            slot = (bool*)calloc(capacity, sizeof(*slot));
        }

        ~simple_allocator() {

            for (uint32_t i = 0; i < capacity; i++) {
                if (slot[i] == true) {
                    data[i].~T();
                }
            }

            free(data);
            free(slot);
            data = nullptr;
            slot = nullptr;
        }


        template<class... Args>
        nstd::handle<T, simple_allocator> allocate(Args... args) {

            for (uint32_t i = 0; i < capacity; i++) {

                if (slot[i] == false) {


                    uint64_t handle = (uint64_t)i << 32;
                    handle |= (uint64_t)counter;
                    slot[i] = true;

                    // placement new our data 
                    new (&data[i]) T(args...);

                    counter++;
                    if (counter == 0) counter++; // it wrapped around, uh oh

                    return handle;
                }
            }

            return 0; // 0 is an invalid handle
        }

        void deallocate(nstd::handle<T, simple_allocator>& handle) {
            if (handle.id == 0) return;
            uint32_t index = handle.id >> 32;
            if (index >= capacity) {
                handle.id = 0;
                return;
            }

            // call data's destructor;
            data[index].~T();

            slot[index] = false;
        }

        T* handle_deref(nstd::handle<T, simple_allocator>& handle) {

            if (handle.id == 0) return nullptr;
            uint32_t index = handle.id >> 32;

            if (index >= capacity) {
                handle.id = 0;
                return nullptr;
            }
            if (slot[index] == false) {
                handle.id = 0;
                return nullptr;
            }
            return &data[index];
        }
    };
}