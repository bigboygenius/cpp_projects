#include <iostream>
#include <array>
#include <iterator>
#include <limits>
#include <type_traits>
#include <memory>
#include <cstddef>

constexpr size_t DYNAMIC_CAPACITY = std::numeric_limits<std::size_t>::max();

template<typename T, size_t Capacity, bool DYNAMIC>
struct BufferStrorage;

template<typename T, size_t Capacity>
struct BufferStrorage<T, Capacity, false> {
    alignas(T) std::array<std::byte, (Capacity + 1) * sizeof(T)> temporary;
    size_t sz = 0;
    size_t head = 0;
    size_t tail = 1;
    T* arr = reinterpret_cast<T*>(temporary.data());

    BufferStrorage() = default;

    BufferStrorage(size_t) {}

    BufferStrorage(const BufferStrorage& other): sz(other.sz), head(other.head), tail(other.tail) {}

    constexpr size_t capacity() const {
        return Capacity + 1;
    }

    ~BufferStrorage() {
        for (size_t i = 0; i < sz; ++i) {
            std::destroy_at(arr + (head + i) % capacity());
        }
    }
};

template<typename T, size_t Capacity>
struct BufferStrorage<T, Capacity, true> {
    size_t cap;
    size_t sz = 0;
    size_t head = 0;
    size_t tail = 1;
    T* arr;

    BufferStrorage(size_t capac) : cap(capac + 1) {
        arr = reinterpret_cast<T*>(new std::byte[cap * sizeof(T)]);
    }

    BufferStrorage(const BufferStrorage& other): cap(other.cap), sz(other.sz), head(other.head), tail(other.tail) {
        arr = reinterpret_cast<T*>(new std::byte[cap * sizeof(T)]);
    }

    constexpr size_t capacity() const {
        return cap;
    }

    ~BufferStrorage() {
        for (size_t i = 0; i < sz; ++i) {
            std::destroy_at(arr + (head + i) % capacity());
        }
        delete[] reinterpret_cast<std::byte*>(arr);
    }
};

template<typename T, size_t Capacity, bool IsConst, bool DYNAMIC>
class BasicIterator;

template<typename T, size_t Capacity, bool IsConst>
class BasicIterator<T, Capacity, IsConst, true> {
public:
    size_t capacity;

    BasicIterator(size_t cap): capacity(cap) {}

    size_t cap() const {
        return capacity;
    }
};

template<typename T, size_t Capacity, bool IsConst>
class BasicIterator<T, Capacity, IsConst, false> {
public:
    BasicIterator(size_t) {}

    size_t cap() const {
        return Capacity + 1;
    }
};

template<typename T, size_t Capacity = DYNAMIC_CAPACITY>
class CircularBuffer {
public:
    static constexpr bool IsDynamicCapacity = Capacity == DYNAMIC_CAPACITY;

    BufferStrorage<T, Capacity, IsDynamicCapacity> storage;

    template<bool IsConst>
    struct base_iterator: BasicIterator<T, Capacity, IsConst, IsDynamicCapacity> {
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        pointer ptr;
        pointer start;
        pointer beginarr;

        base_iterator(T* _ptr, T* _begin, T* _buffer_begin, size_t _cap): BasicIterator<T, Capacity, IsConst,
                IsDynamicCapacity>(_cap), ptr(_ptr), start(_begin), beginarr(_buffer_begin) {
            if constexpr (IsDynamicCapacity) {
                this->capacity = _cap;
            }
        }

        base_iterator(const base_iterator<false>& other): BasicIterator<T, Capacity, IsConst, IsDynamicCapacity>(other.cap()),
                                                          ptr(other.ptr), start(other.start), beginarr(other.beginarr) {
            if constexpr (IsDynamicCapacity) {
                this->capacity = other.cap();
            }
        }

        base_iterator& operator=(const base_iterator<false>& other) {
            ptr = other.ptr;
            start = other.start;
            beginarr = other.beginarr;
            if constexpr (IsDynamicCapacity) {
                this->capacity = other.cap();
            } *this;
            return *this;
        }

        base_iterator& operator+=(difference_type n) {
            if (n < 0) {
                *this -= -n;
            } else {
                ptr = beginarr + ((ptr - beginarr) + n) % this->cap();
            }
            return *this;
        }

        base_iterator& operator-=(difference_type n) {
            if (n < 0) {
                *this += -n;
            } else {
                *this += this->cap() - n;
            }
            return *this;
        }

        base_iterator operator+(difference_type n) const {
            base_iterator copy = *this;
            copy += n;
            return copy;
        }

        base_iterator operator-(difference_type n) const {
            base_iterator copy = *this;
            copy -= n;
            return copy;
        }

        base_iterator& operator++() {
            *this += 1;
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator copy = *this;
            *this += 1;
            return copy;
        }

        base_iterator& operator--() {
            *this -= 1;
            return (*this);
        }

        base_iterator operator--(int) {
            base_iterator copy = *this;
            *this -= 1;
            return copy;
        }

        pointer operator->() const {
            return ptr;
        }

        reference operator*() const {
            return *ptr;
        }

        difference_type operator-(const base_iterator& other) const {
            size_t dif1 = (ptr - start) >= 0 ? ptr - start :
                          this->cap() - (start - ptr);
            size_t dif2 = (other.ptr - other.start) >= 0 ? other.ptr - other.start :
                          other.cap() - (other.start - other.ptr);
            return dif1 - dif2;
        }

        friend base_iterator operator+(difference_type n, const base_iterator& it) {
            return it + n;
        }

        bool operator==(const base_iterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const base_iterator& other) const {
            return !(ptr == other.ptr);
        }

        bool operator<(const base_iterator& other) const {
            return *this - other < 0;
        }

        bool operator>(const base_iterator& other) const {
            return other < *this;
        }

        bool operator<=(const base_iterator& other) const {
            return !(*this > other);
        }

        bool operator>=(const base_iterator& other) const {
            return !(*this < other);
        }

    };

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(storage.arr + storage.head, storage.arr + storage.head, storage.arr, storage.capacity());
    }

    const_iterator cbegin() const {
        return const_iterator(storage.arr + storage.head, storage.arr + storage.head, storage.arr, storage.capacity());
    }

    const_iterator begin() const {
        return const_iterator(storage.arr + storage.head, storage.arr + storage.head, storage.arr, storage.capacity());;
    }

    iterator end() {
        return begin() + storage.sz;
    }

    const_iterator cend() const {
        return cbegin() + storage.sz;
    }

    const_iterator end() const {
        return cbegin() + storage.sz;
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
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

    CircularBuffer() : storage() {
        static_assert(!IsDynamicCapacity);
    }

    explicit CircularBuffer(size_t capacity) : storage(capacity) {
        if constexpr (!IsDynamicCapacity) {
            if (capacity != Capacity) {
                throw std::invalid_argument("Invalid argument");
            }
        }
    }

    CircularBuffer(const CircularBuffer& other) : storage(other.storage) {
        try {
            for (size_t i = 0; i < storage.sz; ++i) {
                std::construct_at(storage.arr + (storage.head + i) % storage.capacity(),
                                  other.storage.arr[(other.storage.head + i) % other.storage.capacity()]);
            }
        } catch (...) {
            for (size_t i = 0; i < storage.sz; ++i) {
                std::destroy_at(storage.arr + (storage.head + i) % storage.capacity());
            }
            storage.~BufferStrorage();
            throw;
        }
    }

    CircularBuffer& operator=(const CircularBuffer& other) {
        if (this != &other) {
            for (size_t i = 0; i < storage.sz; ++i) {
                std::destroy_at(storage.arr + (storage.head + i) % storage.capacity());
            }
            storage.sz   = other.storage.sz;
            storage.head = other.storage.head;
            storage.tail = other.storage.tail;
            for (size_t i = 0; i < storage.sz; ++i) {
                std::construct_at(storage.arr + (storage.head + i) % storage.capacity(),
                                  other.storage.arr[(other.storage.head + i) % other.storage.capacity()]
                );
            }
        }
        return *this;
    }

    void push_back(const T& value) {
        if (empty()) {
            std::construct_at(storage.arr, value);
            storage.head = 0;
            storage.tail = 1;
            ++storage.sz;
            return;
        }
        if (!full()) {
            std::construct_at(storage.arr + storage.tail, value);
            storage.tail = (storage.tail + 1) % storage.capacity();
            ++storage.sz;
            return;
        }
        std::destroy_at(storage.arr + (storage.tail + 1) % storage.capacity());
        std::construct_at(storage.arr + storage.tail, value);
        storage.tail = (storage.tail + 1) % storage.capacity();
        storage.head = (storage.head + 1) % storage.capacity();
    }

    void push_front(const T& value) {
        if (empty()) {
            std::construct_at(storage.arr, value);
            storage.tail = 1;
            storage.head = 0;
            ++storage.sz;
            return;
        }
        if (!full()) {
            std::construct_at(storage.arr + (storage.head - 1 + storage.capacity()) % storage.capacity(), value);
            storage.head = (storage.head - 1 + storage.capacity()) % storage.capacity();
            ++storage.sz;
            return;
        }
        std::destroy_at(storage.arr + (storage.head + storage.capacity() - 2) % storage.capacity());
        std::construct_at(storage.arr + (storage.head - 1 + storage.capacity()) % storage.capacity(), value);
        storage.tail = (storage.tail + storage.capacity() - 1) % storage.capacity();
        storage.head = (storage.head + storage.capacity() - 1) % storage.capacity();
    }

    void pop_back() {
        if (empty()) {
            return;
        }
        std::destroy_at(storage.arr + (storage.tail + storage.capacity() - 1) % storage.capacity());
        --storage.sz;
        storage.tail = (storage.tail - 1 + storage.capacity()) % storage.capacity();
    }

    void pop_front() {
        if (empty()) {
            return;
        }
        std::destroy_at(storage.arr + storage.head);
        --storage.sz;
        storage.head = (storage.head + 1) % storage.capacity();
    }

    size_t size() const {
        return storage.sz;
    }

    size_t capacity() const {
        return storage.capacity() - 1;
    }

    bool empty() const {
        return storage.sz == 0;
    }

    bool full() const {
        return storage.sz == capacity();
    }

    T& operator[](size_t index) {
        return storage.arr[(storage.head + index) % storage.capacity()];
    }

    const T &operator[](size_t index) const {
        return storage.arr[(storage.head + index) % storage.capacity()];
    }

    T& at(size_t index) {
        if (index >= storage.sz) {
            throw std::out_of_range("Out of range");
        }
        return storage.arr[(storage.head + index) % storage.capacity()];
    }

    const T& at(size_t index) const {
        if (index >= storage.sz) {
            throw std::out_of_range("Out of range");
        }
        return storage.arr[(storage.head + index) % storage.capacity()];
    }

    void swap(CircularBuffer& other) {
        if constexpr (!IsDynamicCapacity) {
            std::swap(storage.temporary, other.storage.temporary);
            storage.arr = reinterpret_cast<T*>(storage.temporary.data());
            other.storage.arr = reinterpret_cast<T*>(other.storage.temporary.data());
        } else {
            std::swap(storage.arr, other.storage.arr);
        }
        std::swap(storage.head, other.storage.head);
        std::swap(storage.tail, other.storage.tail);
        std::swap(storage.sz, other.storage.sz);
    }

    void insert(const iterator& it, const T& value) {
        std::ptrdiff_t diff = it - begin();
        if (diff == 0 && full()) {
            return;
        }
        if (full()) {
            pop_front();
            --diff;
        }
        if (diff == 0 && !full()) {
            push_front(value);
            return;
        }
        for (size_t i = storage.sz; i > static_cast<size_t>(diff); --i) {
            std::construct_at(storage.arr + (storage.head + i) % storage.capacity(), storage.arr[(storage.head + i + storage.capacity() - 1) % storage.capacity()]);
            std::destroy_at(storage.arr + (storage.head + i + storage.capacity() - 1) % storage.capacity());
        }
        std::construct_at(storage.arr + (storage.head + diff) % storage.capacity(), value);
        ++storage.sz;
        storage.tail = (storage.head + storage.sz) % storage.capacity();
    }

    void erase(const iterator& it) {
        std::ptrdiff_t diff = it - begin();
        if (diff == 0) {
            pop_front();
            return;
        }
        if (static_cast<size_t>(diff) == storage.sz - 1) {
            pop_back();
            return;
        }
        storage.arr[(storage.head + diff) % storage.capacity()].~T();
        for (size_t i = diff; i < storage.sz - 1; ++i) {
            std::construct_at(storage.arr + (storage.head + i) % storage.capacity(), storage.arr[(storage.head + i + 1) % storage.capacity()]);
            std::destroy_at(storage.arr + (storage.head + i + 1) % storage.capacity());
        }
        --storage.sz;
        storage.tail = (storage.head + storage.sz) % storage.capacity();
    }

    ~CircularBuffer() = default;
};