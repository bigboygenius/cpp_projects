#include <array>
#include <memory>
#include <cstddef>
#include <type_traits>

template <size_t N>
class StackStorage {
private:
    std::byte store[N];
    size_t current = 0;

public:
    StackStorage() = default;

    StackStorage(const StackStorage&) = delete;

    StackStorage& operator=(const StackStorage&) = delete;

    void* allocate(size_t size, size_t alignment) {
        void* ptr = store + current;
        size_t space = N - current;
        if (std::align(alignment, size, ptr, space)) {
            current = reinterpret_cast<std::byte*>(ptr) - store + size;
        }
        if (current >= N) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    ~StackStorage() = default;
};


template <typename T, size_t N>
class StackAllocator {
private:
    StackStorage<N>* storage;
public:
    using value_type = T;

    StackAllocator(): storage() {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other): storage(other.get_storage()) {}


    StackAllocator(StackStorage<N>& other): storage(&other) {}

    template <typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& other) {
        storage = other.get_storage();
        return *this;
    }

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    T* allocate(size_t n) {
        return reinterpret_cast<T*>(storage->allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(void*, size_t) {}

    StackStorage<N>* get_storage() const {
        return storage;
    }

    bool operator==(const StackAllocator& other) const {
        return storage == other.storage;
    }

    bool operator!=(const StackAllocator& other) const {
        return !(storage == other.storage);
    }

    StackAllocator select_on_container_copy_construction() const {
        return *this;
    }

    ~StackAllocator() = default;

};


template <typename T, typename Alloc = std::allocator<T>>
class List {
private:
    struct BaseNode {
        BaseNode* prev = this;
        BaseNode* next = this;

        BaseNode() = default;

    };

    struct Node: BaseNode {
        T value;

        Node(BaseNode* prev_, BaseNode* next_, const T& val): value(val) {
            this->prev = prev_;
            this->next = next_;
            prev_->next = this;
            next_->prev = this;
        }

        Node(BaseNode* prev_, BaseNode* next_): value() {
            this->prev = prev_;
            this->next = next_;
            prev_->next = this;
            next_->prev = this;
        }

        Node(const Node* other): value(other->value) {
            this->prev = other->prev;
            this->next = other->next;
        }
    };

    using NodeAllocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;
    using value_type = T;
    size_t sz = 0;
    BaseNode end_;
    [[no_unique_address]] NodeAllocator nodeAllocator;

public:
    List() = default;

    List(size_t n, const T& value = T()) {
        for (size_t i = 0; i < n; ++i) {
            Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
            try {
                NodeAllocatorTraits::construct(nodeAllocator, node,
                                                                begin().current->prev, begin().current, value);
            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
                clear();
                throw;
            }
            ++sz;
        }
    }

    List(const Alloc& allocator): nodeAllocator(NodeAllocatorTraits::select_on_container_copy_construction(allocator)) {}

    List(size_t n, const Alloc& allocator): nodeAllocator(NodeAllocatorTraits::select_on_container_copy_construction(allocator)) {
        for (size_t i = 0; i < n; ++i) {
            Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
            try {
                NodeAllocatorTraits::construct(nodeAllocator, node,
                                                                begin().current->prev, begin().current);

            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
                clear();
                throw;
            }
            ++sz;
        }
    }

    List(size_t n, const T& value, const Alloc& allocator):
                            nodeAllocator(NodeAllocatorTraits::select_on_container_copy_construction(allocator)) {
        for (size_t i = 0; i < n; ++i) {
            Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
            try {
                NodeAllocatorTraits::construct(nodeAllocator, node,
                                                                begin().current->prev, begin().current, value);

            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
                clear();
                throw;
            }
            ++sz;
        }
    }

    List(const List& other): nodeAllocator(NodeAllocatorTraits::select_on_container_copy_construction(other.nodeAllocator)) {
        for (const T& val: other) {
            Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
            try {
                NodeAllocatorTraits::construct(nodeAllocator, node,
                                                                end().current->prev, end().current, val);
            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
                clear();
                throw;
            }
            ++sz;
        }
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }
        if constexpr (NodeAllocatorTraits::propagate_on_container_copy_assignment::value) {
            clear();
            nodeAllocator = other.nodeAllocator;
            for (const T& val : other) {
                push_back(val);
            }
            return *this;
        }
        List copy(other);
        swap(copy);
        return *this;
    }

    void swap(List& other) {
        std::swap(end_.next, other.end_.next);
        std::swap(end_.prev, other.end_.prev);
        std::swap(sz, other.sz);
        std::swap(nodeAllocator, other.nodeAllocator);
        if (end_.next != &end_) {
            end_.next->prev = &end_;
            end_.prev->next = &end_;
        } else {
            end_.next = end_.prev = &end_;
        }
        if (other.end_.next != &other.end_) {
            other.end_.next->prev = &other.end_;
            other.end_.prev->next = &other.end_;
        } else {
            other.end_.next = other.end_.prev = &other.end_;
        }
    }

    void clear() {
        while (!empty()) {
            pop_back();
        }
    }

    template <bool IsConst>
    struct base_iterator {
        BaseNode* current;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        base_iterator(const BaseNode* other): current(const_cast<BaseNode*>(other)) {}

        template <bool IsOtherConst, typename = std::enable_if_t<IsConst && !IsOtherConst>>
        base_iterator(const base_iterator<IsOtherConst>& other): current(other.current) {}

        base_iterator& operator++() {
            current = current->next;
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator copy = *this;
            ++*this;
            return copy;
        }

        base_iterator& operator--() {
            current = current->prev;
            return *this;
        }

        base_iterator operator--(int) {
            base_iterator copy = *this;
            --*this;
            return copy;
        }

        pointer operator->() const {
            return &static_cast<Node*>(current)->value;
        }

        reference operator*() const {
            return static_cast<Node*>(current)->value;
        }

        bool operator==(const base_iterator& other) const {
            return current == other.current;
        }

        bool operator!=(const base_iterator& other) const {
            return !(*this == other);
        }

    };

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(end_.next);
    }

    const_iterator begin() const {
        return const_iterator(end_.next);
    }

    const_iterator cbegin() const {
        return const_iterator(end_.next);
    }

    iterator end() {
        return iterator(&end_);
    }

    const_iterator end() const {
        return const_iterator(&end_);
    }

    const_iterator cend() const {
        return const_iterator(&end_);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const {
        return rbegin();
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator rend() const {
        return crend();
    }


    void push_back(const T& value) {
        insert(end(), value);
    }

    void pop_back() {
        erase(--end());
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void pop_front() {
        erase(begin());
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    NodeAllocator get_allocator() const {
        return nodeAllocator;
    }

    void insert(const_iterator it, const T& value) {
        Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(nodeAllocator, node, it.current->prev, it.current, value);
        ++sz;
    }

    void erase(const_iterator it) {
        Node* node = static_cast<Node*>(it.current);
        node->prev->next = node->next;
        node->next->prev = node->prev;
        NodeAllocatorTraits::destroy(nodeAllocator, node);
        NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
        --sz;
    }

    ~List() {
        clear();
    }

};