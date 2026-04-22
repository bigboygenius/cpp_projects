#include <memory>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <cmath>
#include <stdexcept>


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
        size_t hash = 0;

        Node(BaseNode* prev_, BaseNode* next_, const T& value): value(value) {
            this->prev = prev_;
            this->next = next_;
            prev_->next = this;
            next_->prev = this;
        }

        Node(BaseNode* prev_, BaseNode* next_, T&& value): value(value) {
            this->prev = prev_;
            this->next = next_;
            prev_->next = this;
            next_->prev = this;
        }

        template <typename... Args>
        Node(BaseNode* prev_, BaseNode* next_, Args&&... args): value(std::forward<Args>(args)...) {
            this->prev = prev_;
            this->next = next_;
            prev_->next = this;
            next_->prev = this;
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

    List(const List& other): nodeAllocator(NodeAllocatorTraits::select_on_container_copy_construction(other.nodeAllocator)) {
        for (const_iterator iter = other.cbegin(); iter != other.cend(); ++iter) {
            Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
            try {
                NodeAllocatorTraits::construct(nodeAllocator, node,
                                               end().current->prev, end().current, *iter);
                node->hash = get_hash(iter);
            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
                clear();
                throw;
            }
            ++sz;
        }
    }

    List(List&& other) noexcept : sz(other.sz), end_(other.end_), nodeAllocator(std::move(other.nodeAllocator)) {
        other.sz = 0;
        end_.prev->next = end_.next->prev = &end_;
        other.end_.prev = other.end_.next = &other.end_;
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }
        if constexpr (NodeAllocatorTraits::propagate_on_container_copy_assignment::value) {
            clear();
            nodeAllocator = other.nodeAllocator;
            for (const_iterator iter = other.cbegin(); iter != other.cend(); ++iter) {
                push_back(*iter);
                get_hash(--end()) = get_hash(iter);
            }
            return *this;
        }
        List copy(other);
        swap(copy);
        return *this;
    }

    List& operator=(List&& other) noexcept {
        clear();
        if constexpr (NodeAllocatorTraits::propagate_on_container_move_assignment::value) {
            nodeAllocator = std::move(other.nodeAllocator);
            end_ = other.end_;
            end_.prev->next = end_.next->prev = &end_;
            other.end_.prev = other.end_.next = &other.end_;
            sz = other.sz;
            other.sz = 0;
            return *this;
        }
        for (iterator iter = other.begin(); iter != other.end(); ++iter) {
            emplace(end(), std::move(*iter));
            get_hash(--end()) = get_hash(iter);
        }
        other.end_.prev = other.end_.next = &other.end_;
        other.sz = 0;
        return *this;
    }

    void swap(List& other) {
        std::swap(end_.next, other.end_.next);
        std::swap(end_.prev, other.end_.prev);
        std::swap(sz, other.sz);
        if constexpr (NodeAllocatorTraits::propagate_on_container_swap::value) {
            std::swap(nodeAllocator, other.nodeAllocator);
        }
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

        base_iterator() : current(nullptr) {}

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

        template <bool IsOtherConst>
        bool operator==(const base_iterator<IsOtherConst>& other) const {
            return current == other.current;
        }

        template <bool IsOtherConst>
        bool operator!=(const base_iterator<IsOtherConst>& other) const {
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

    void push_back(T&& value) {
        insert(end(), value);
    }

    void pop_back() {
        erase(--end());
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void push_front(const T&& value) {
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

    template<typename ... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(nodeAllocator, node, pos.current->prev, pos.current, std::forward<Args>(args)...);
        ++sz;
        return iterator(pos.current->prev);
    }

    template<typename InputIt>
    void insert(const_iterator pos, InputIt first, InputIt last) {
        for (; first != last; ++first) {
            auto iter = insert(pos, *first);
            get_hash(iter) = get_hash(first);
        }
    }

    iterator insert(const_iterator it, const T& value) {
        Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(nodeAllocator, node, it.current->prev, it.current, value);
        ++sz;
        return iterator(it.current->prev);
    }

    iterator insert(const_iterator it, T&& value) {
        Node* node = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(nodeAllocator, node, it.current->prev, it.current, value);
        ++sz;
        return iterator(it.current->prev);
    }

    iterator erase(const_iterator it) {
        Node* node = static_cast<Node*>(it.current);
        iterator iter = iterator(node->next);
        node->prev->next = node->next;
        node->next->prev = node->prev;
        NodeAllocatorTraits::destroy(nodeAllocator, node);
        NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
        --sz;
        return iter;
    }

    ~List() {
        clear();
    }

    size_t& get_hash(const_iterator iter) const {
        return static_cast<Node*>(iter.current)->hash;
    }

};


template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;

private:
    using ListType = List<NodeType, Alloc>;
    using AllocTraits = std::allocator_traits<Alloc>;
    using ListIterator = typename ListType::iterator;

    [[no_unique_address]] Hash hash;
    [[no_unique_address]] Equal equal;
    [[no_unique_address]] Alloc alloc;
    ListType buckets;
    float max_load = 1.0;

public:
    template <bool IsConst>
    struct base_iterator {
        typename ListType::template base_iterator<IsConst> iter;

        using pointer = std::conditional_t<IsConst, const NodeType*, NodeType*>;
        using reference = std::conditional_t<IsConst, const NodeType&, NodeType&>;
        using value_type = std::conditional_t<IsConst, const NodeType, NodeType>;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        base_iterator() : iter() {}

        base_iterator(typename ListType::iterator other): iter(other) {}

        base_iterator(typename ListType::const_iterator other): iter(other) {}

        template <bool IsOtherConst, typename = std::enable_if_t<IsConst && !IsOtherConst>>
        base_iterator(const base_iterator<IsOtherConst>& other): iter(other.iter) {}


        base_iterator& operator++() {
            ++iter;
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator copy = *this;
            ++iter;
            return copy;
        }

        pointer operator->() const {
            return &(*iter);
        }

        reference operator*() const {
            return *iter;
        }

        template <bool IsOtherConst>
        bool operator==(const base_iterator<IsOtherConst>& other) const {
            return iter == other.iter;
        }

        template <bool IsOtherConst>
        bool operator!=(const base_iterator<IsOtherConst>& other) const {
            return !(iter == other.iter);
        }

        bool is_default() {
            return iter.current == nullptr;
        }

        void make_default() {
            iter.current = nullptr;
        }
    };

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

private:
    std::vector<iterator> bucket_pointers;

public:

    UnorderedMap(const UnorderedMap& other) :
            hash(other.hash), equal(other.equal), alloc(AllocTraits::select_on_container_copy_construction(other.alloc)),
            buckets(other.buckets), max_load(other.max_load),
            bucket_pointers(other.bucket_pointers) {}

    UnorderedMap(UnorderedMap&& other) noexcept :
                hash(std::move(other.hash)), equal(std::move(other.equal)), alloc(std::move(other.alloc)),
                buckets(std::move(other.buckets)), max_load(other.max_load), bucket_pointers(std::move(other.bucket_pointers))
    {}

    UnorderedMap(size_t bucket_count = 16,
                 const Hash& hash = Hash(), const Equal& equal = Equal(),
                 const Alloc& alloc = Alloc()):
                 hash(hash), equal(equal), alloc(alloc), buckets(), bucket_pointers(bucket_count) {}

    UnorderedMap& operator=(const UnorderedMap& other) {
        if (this == &other) {
            return *this;
        }
        hash = other.hash;
        equal = other.equal;
        if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
                alloc = other.alloc;
        }
        buckets = other.buckets;
        bucket_pointers = other.bucket_pointers;
        max_load = other.max_load;
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        hash = std::move(other.hash);
        equal = std::move(other.equal);
        if constexpr (AllocTraits::propagate_on_container_move_assignment::value || alloc == other.alloc) {
            alloc = other.alloc;
        }
        buckets.clear();
        std::vector<iterator> new_bucket_pointers(other.bucket_pointers.size());
        bucket_pointers = std::move(new_bucket_pointers);
        for (iterator iter = other.begin(); iter != other.cend(); ++iter) {
            insert(std::move(*iter));
        }
        max_load = other.max_load;
        return *this;
    }

    void swap(UnorderedMap& other) {
        std::swap(hash, other.hash);
        std::swap(equal, other.equal);
        std::swap(max_load, other.max_load);
        if constexpr (AllocTraits::propagate_on_container_swap::value) {
            std::swap(alloc, other.alloc);
        }
        std::swap(bucket_pointers, other.bucket_pointers);
        buckets.swap(other.buckets);
    }


    iterator begin() {
        return iterator(buckets.begin());
    }

    const_iterator cbegin() const {
        return const_iterator(buckets.cbegin());
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(buckets.end());
    }

    const_iterator cend() const {
        return const_iterator(buckets.cend());
    }

    const_iterator end() const {
        return cend();
    }


    size_t size() const {
        return buckets.size();
    }

    bool empty() const {
        return size() == 0;
    }

    Value& operator[](const Key& key) {
        if (auto iter = find(key); iter != end()) {
            return iter->second;
        }
        if (load_factor() >= max_load_factor()) {
            rehash(2 * size());
        }
        size_t hash_ = hash(key);
        auto old_iter = bucket_pointers[hash_ % bucket_pointers.size()];
        auto new_iter = buckets.emplace(
                old_iter.is_default() ? buckets.end() : old_iter.iter,
                key, Value());
        buckets.get_hash(new_iter) = hash_;
        bucket_pointers[hash_ % bucket_pointers.size()] = iterator(new_iter);
        return bucket_pointers[hash_ % bucket_pointers.size()]->second;
    }

    Value& operator[](Key&& key) {
        if (auto iter = find(key); iter != end()) {
            return iter->second;
        }
        if (load_factor() >= max_load_factor()) {
            rehash(2 * size());
        }
        size_t hash_ = hash(key);
        auto old_iter = bucket_pointers[hash_ % bucket_pointers.size()];
        auto new_iter = buckets.emplace(
                old_iter.is_default() ? buckets.end() : old_iter.iter,
                key, Value());
        buckets.get_hash(new_iter) = hash_;
        bucket_pointers[hash_ % bucket_pointers.size()] = iterator(new_iter);
        return bucket_pointers[hash_ % bucket_pointers.size()]->second;
    }

    Value& at(const Key& key) {
        if (auto it = find(key); it != end()) {
            return it->second;
        }
        throw std::out_of_range("No element with this key");
    }

    const Value& at(const Key& key) const {
        if (auto it = find(key); it != end()) {
            return it->second;
        }
        throw std::out_of_range("No element with this key");
    }

    std::pair<iterator, bool> insert(const NodeType& node) {
        if (auto iter = find(node.first); iter != end()) {
            return std::make_pair(iter, false);
        }
        if (load_factor() >= max_load_factor()) {
            rehash(2 * size());
        }
        size_t hash_ = hash(node.first);
        auto old_iter = bucket_pointers[hash_ % bucket_pointers.size()];
        auto new_iter = buckets.insert(
                old_iter.is_default() ? buckets.end() : old_iter.iter, node);
        iterator it(new_iter);
        buckets.get_hash(new_iter) = hash_;
        bucket_pointers[hash_ % bucket_pointers.size()] = it;
        return std::make_pair(it, true);
    }

    std::pair<iterator, bool> insert(NodeType&& node) {
        if (auto iter = find(node.first); iter != end()) {
            return std::make_pair(iter, false);
        }
        if (load_factor() >= max_load_factor()) {
            rehash(2 * size());
        }
        size_t hash_ = hash(node.first);
        auto old_iter = bucket_pointers[hash_ % bucket_pointers.size()];
        auto new_iter = buckets.emplace(
                old_iter.is_default() ? buckets.end() : old_iter.iter,
                std::piecewise_construct,
                std::forward_as_tuple(std::move(const_cast<std::remove_const_t<Key>&>(node.first))),
                std::forward_as_tuple(std::move(node.second)));
        iterator it(new_iter);
        buckets.get_hash(new_iter) = hash_;
        bucket_pointers[hash_ % bucket_pointers.size()] = it;
        return std::make_pair(it, true);
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        NodeType* p = AllocTraits::allocate(alloc, 1);
        AllocTraits::construct(alloc, p, std::forward<Args>(args)...);
        auto ans = insert(std::move(*p));
        AllocTraits::destroy(alloc, p);
        AllocTraits::deallocate(alloc, p, 1);
        return ans;
    }

    iterator erase(iterator pos) {
        size_t hash_ = buckets.get_hash(pos.iter);
        if (bucket_pointers[hash_ % bucket_pointers.size()] != pos) {
            return iterator(buckets.erase(pos.iter));
        }
        if (typename ListType::iterator iter = bucket_pointers[hash_ % bucket_pointers.size()].iter;
                    hash_ == buckets.get_hash(++iter)) {
            ++bucket_pointers[hash_ % bucket_pointers.size()];
            return iterator(buckets.erase(pos.iter));
        }
        bucket_pointers[hash_ % bucket_pointers.size()].make_default();
        return iterator(buckets.erase(pos.iter));
    }

    iterator erase(const_iterator pos) {
        size_t hash_ = buckets.get_hash(pos.iter);
        if (bucket_pointers[hash_ % bucket_pointers.size()] != pos) {
            return iterator(buckets.erase(pos.iter));
        }
        if (typename ListType::iterator iter = bucket_pointers[hash_ % bucket_pointers.size()].iter;
                ++iter != end().iter && hash_ == buckets.get_hash(iter)) {
            ++bucket_pointers[hash_ % bucket_pointers.size()];
            return iterator(buckets.erase(pos.iter));
        }
        bucket_pointers[hash_ % bucket_pointers.size()].make_default();
        return iterator(buckets.erase(pos.iter));
    }

    iterator erase(const_iterator first, const_iterator last) {
        iterator it;
        for (; first != last; ) {
            it = erase(first++);
        }
        return it;
    }

    iterator find(const Key& key) {
        size_t hash_ = hash(key);
        if (!bucket_pointers[hash_ % bucket_pointers.size()].is_default()) {
            for (iterator iter = bucket_pointers[hash_ % bucket_pointers.size()];
                 iter != end() && buckets.get_hash(iter.iter) == hash_; ++iter) {
                if (equal(key, iter->first)) {
                    return iter;
                }
            }
        }
        return iterator(buckets.end());
    }

    const_iterator find(const Key& key) const {
        size_t hash_ = hash(key);
        if (!bucket_pointers[hash_ % bucket_pointers.size()].is_default()) {
            for (iterator iter = bucket_pointers[hash_ % bucket_pointers.size()];
                 iter != end() && buckets.get_hash(iter.iter) == hash_; ++iter) {
                if (equal(key, iter->first)) {
                    return const_iterator(iter);
                }
            }
        }
        return const_iterator(buckets.end());
    }

    float load_factor() const {
        return size() * 1. / bucket_pointers.size();
    }

    float max_load_factor() const {
        return max_load;
    }

    void max_load_factor(float new_ml) {
        max_load = new_ml;
        if (load_factor() > max_load) {
            rehash(2 * size() - 1);
        }
    }

    void reserve(size_t count) {
        auto necessary = static_cast<size_t>(std::ceil(count * 1. / max_load));
        if (necessary > size()) {
            rehash(necessary);
        }
    }

    void rehash(size_t count) {
        auto n = static_cast<size_t>(std::ceil(size() * 1. / max_load_factor()));
        if (count > n) {
            n = count;
        }
        UnorderedMap new_map(n);
        for (iterator iter = begin(); iter != end(); ++iter) {
            new_map.insert(std::move(*iter));
        }
        *this = std::move(new_map);
    }

    ~UnorderedMap() = default;
};