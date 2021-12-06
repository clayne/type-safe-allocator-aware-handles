namespace nstd { // non-standard c++

    template<typename T, typename Allocator, typename HandleType = int>
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
}
