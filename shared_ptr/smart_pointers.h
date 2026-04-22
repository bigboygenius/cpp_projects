#include <array>
template <typename T>
class WeakPtr;

struct BaseControlBlock {

    size_t shared_count = 1;
    size_t weak_count = 0;

    virtual void* get_ptr() = 0;

    virtual void delete_pointer() {}

    virtual void delete_control_block() {}

    virtual ~BaseControlBlock() = default;
};


template <typename T>
class SharedPtr {
private:

    template <typename U, typename Deleter = std::default_delete<U>, typename Alloc = std::allocator<U>>
    struct ControlBlockDeleterAllocator: BaseControlBlock {
        U* object;
        [[no_unique_address]] Deleter del;
        [[no_unique_address]] Alloc alloc;

        ControlBlockDeleterAllocator(U* ptr,
                 Deleter del = std::default_delete<U>(),
                 const Alloc& alloc = std::allocator<U>())
        : object(ptr), del(del), alloc(alloc) {}

        void* get_ptr() override {
            return object;
        }

        void delete_pointer() override {
            del(object);
        }

        void delete_control_block() override {
            using BlockAlloc = typename std::allocator_traits<Alloc>
                        ::template rebind_alloc<ControlBlockDeleterAllocator>;
            using BlockAllocTraits = std::allocator_traits<BlockAlloc>;
            BlockAlloc my_alloc;
            this->~BaseControlBlock();
            BlockAllocTraits::deallocate(my_alloc, this, 1);
        }
    };

    template <typename U, typename Alloc = std::allocator<U>>
    struct ControlBlockAllocateShared: BaseControlBlock {
        alignas(U) std::array<std::byte, sizeof(U)> object;
        [[no_unique_address]] Alloc alloc;


        template <typename... Args>
        ControlBlockAllocateShared(const Alloc& alloc, Args&&... args): alloc(alloc) {
            U* pointer = reinterpret_cast<U*>(object.data());
            new(pointer) U(std::forward<Args>(args)...);
        }

        void* get_ptr() override {
            return reinterpret_cast<U*>(object.data());;
        }

        void delete_pointer() override {
            U* pointer = reinterpret_cast<U*>(object.data());
            pointer->~U();
        }

        void delete_control_block() override {
            using BlockAlloc = typename std::allocator_traits<Alloc>
                        ::template rebind_alloc<ControlBlockAllocateShared>;
            using BlockAllocTraits = std::allocator_traits<BlockAlloc>;
            BlockAlloc my_alloc;
            BlockAllocTraits::destroy(my_alloc, this);
            BlockAllocTraits::deallocate(my_alloc, this, 1);
        }
    };

    T* ptr;
    BaseControlBlock* counter;

    SharedPtr(BaseControlBlock* block)
              : ptr(reinterpret_cast<T*>((block->get_ptr()))), counter(block) {}

    template <typename U>
    SharedPtr(const WeakPtr<U>& other): counter(other.counter) {
        if (counter) {
            ++(counter->shared_count);
        }
    }

    template <typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args&&... args);

    template <typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class SharedPtr;

public:

    constexpr SharedPtr(): ptr(nullptr), counter(nullptr) {}

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other): ptr(other.ptr), counter(other.counter) {
        if (counter) {
            ++(counter->shared_count);
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr): ptr(ptr), counter(other.counter) {
        if (counter) {
            ++(counter->shared_count);
        }
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other): ptr(other.ptr), counter(other.counter) {
        other.counter = nullptr;
        other.ptr = nullptr;
    }

    SharedPtr(const SharedPtr& other): ptr(other.ptr), counter(other.counter) {
        if (counter) {
            ++(counter->shared_count);
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : SharedPtr(other) {
        other.reset();
    }

    template <typename Y, typename Deleter = std::default_delete<Y>,
            typename Alloc = std::allocator<T>, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr(Y* ptr, Deleter del = std::default_delete<Y>(),
                    Alloc alloc = std::allocator<Y>()): ptr(ptr) {
        using Block = ControlBlockDeleterAllocator<Y, Deleter, Alloc>;
        using BlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Block>;
        using BlockAllocTraits = std::allocator_traits<BlockAlloc>;
        BlockAlloc my_alloc;
        Block* block = BlockAllocTraits::allocate(my_alloc, 1);
        new (block) Block(ptr, del, alloc);
        counter = block;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        SharedPtr copy(other);
        swap(copy);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        reset();
        ptr = other.ptr;
        counter = other.counter;
        other.ptr = nullptr;
        other.counter = nullptr;
        return *this;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        reset();
        ptr = other.ptr;
        counter = other.counter;
        if (counter) {
            ++(counter->shared_count);
        }
        return *this;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        reset();
        ptr = other.ptr;
        counter = other.counter;
        other.ptr = nullptr;
        other.counter = nullptr;
        return *this;
    }

    T& operator*() const noexcept {
        return *get();
    }

    T* operator->() const noexcept {
        return get();
    }

    T* get() const noexcept {
        return ptr;
    }

    size_t use_count() const {
        return counter ? counter->shared_count : 0;
    }

    void swap(SharedPtr& other) {
        std::swap(ptr, other.ptr);
        std::swap(counter, other.counter);
    }

    void reset() noexcept {
        if (!counter) {
            ptr = nullptr;
            return;
        }
        if (--(counter->shared_count) == 0) {
            counter->delete_pointer();
            if (counter->weak_count == 0) {
                counter->delete_control_block();
            }
        }
        counter = nullptr;
        ptr = nullptr;
    }

    template <typename Y>
    void reset(Y* p) {
        SharedPtr<T>(p).swap(*this);
    }

    ~SharedPtr() {
        if (!counter) {
            return;
        }
        if (--(counter->shared_count) == 0) {
            counter->delete_pointer();
            if (counter->weak_count == 0) {
                counter->delete_control_block();
            }
        }
    }
};

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    using ControlBlock = typename SharedPtr<T>
    ::template ControlBlockAllocateShared<T, Alloc>;
    using BlockAlloc = typename std::allocator_traits<Alloc>
    ::template rebind_alloc<ControlBlock>;
    using BlockAllocTraits = std::allocator_traits<BlockAlloc>;
    BlockAlloc alloc_mine = alloc;
    ControlBlock* block = BlockAllocTraits::allocate(alloc_mine, 1);
    BlockAllocTraits::construct(alloc_mine, block,
                                alloc_mine, std::forward<Args>(args)...);
    return SharedPtr<T>(block);
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return allocateShared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

template <typename T>
class WeakPtr {
private:
    BaseControlBlock* counter;

    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class SharedPtr;

public:

    WeakPtr(): counter(nullptr) {}

    WeakPtr(const WeakPtr& other): counter(other.counter) {
        if (counter) {
            ++(counter->weak_count);
        }
    }

    WeakPtr(WeakPtr&& other): counter(other.counter) {
        other.counter = nullptr;
    }

    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other): counter(other.counter) {
        other.counter = nullptr;
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other): counter(other.counter) {
        if (counter) {
            ++(counter->weak_count);
        }
    }

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other): counter(other.counter) {
        if (counter) {
            ++(counter->weak_count);
        }
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        reset();
        counter = other.counter;
        if (counter) {
            ++(counter->weak_count);
        }
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        reset();
        counter = other.counter;
        if (counter) {
            ++(counter->weak_count);
        }
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        reset();
        counter = other.counter;
        if (counter) {
            ++(counter->weak_count);
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        reset();
        counter = other.counter;
        other.counter = nullptr;
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        reset();
        counter = other.counter;
        other.counter = nullptr;
        return *this;
    }

    size_t use_count() const noexcept {
        return counter ? counter->shared_count : 0;
    }

    bool expired() const noexcept {
        return use_count() == 0;
    }

    void reset() noexcept {
        if (!counter) {
            return;
        }
        if (--(counter->weak_count) == 0 && counter->shared_count == 0) {
            counter->delete_control_block();
        }
        counter = nullptr;
    }

    SharedPtr<T> lock() const noexcept {
        return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    }

    ~WeakPtr() {
        if (!counter) {
            return;
        }
        if (--(counter->weak_count) == 0 && counter->shared_count == 0) {
            counter->delete_control_block();

        }
    }
};

template <typename T>
class EnableSharedFromThis {
private:
    SharedPtr<T> sptr;

public:
    SharedPtr<T> shared_from_this() {
        return sptr;
    }
};

