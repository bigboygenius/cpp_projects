#include <type_traits>
#include <utility>
#include <compare>

template <typename...>
class Tuple;

template <>
class Tuple<> {};

namespace detail {

    template <typename T>
    std::true_type test_copy_list_initializable(decltype(T {}, nullptr));

    template <typename...>
    std::false_type test_copy_list_initializable(...);

    template <typename, typename...>
    struct type_count;

    template <typename T, typename U, typename... Types>
    struct type_count<T, U, Types...> {
        static constexpr size_t value = static_cast<size_t>(std::is_same_v<T, U>) + type_count<T, Types...>::value;
    };

    template <typename T>
    struct type_count<T> {
        static constexpr size_t value = 0;
    };

    template <size_t, typename, typename...>
    struct find_type;

    template <size_t N, typename T, typename U, typename... Types>
    struct find_type<N, T, U, Types...> {
        static constexpr size_t value = std::is_same_v<T, U> ? N : find_type<N + 1, T, Types...>::value;
    };

    template <size_t N, typename T>
    struct find_type<N, T> {
        static constexpr size_t value = 0;
    };

    template <typename...>
    struct many_tuples;

    template <typename... A, typename... B, typename... OtherTuples>
    struct many_tuples<Tuple<A...>, Tuple<B...>, OtherTuples...> {
        using type = typename many_tuples<Tuple<A..., B...>, OtherTuples...>::type;
    };

    template <typename... A>
    struct many_tuples<Tuple<A...>> {
        using type = Tuple<A...>;
    };

    template <>
    struct many_tuples<> {
        using type = Tuple<>;
    };

}

template <typename T>
struct is_copy_list_initializable: decltype(detail::test_copy_list_initializable<T>(nullptr)) {};

template <std::size_t N, typename... Types>
struct TupleElement;

template <std::size_t N, typename Head, typename... Types>
struct TupleElement<N, Tuple<Head, Types...>> {
    using type = typename TupleElement<N - 1, Tuple<Types...>>::type;
};

template <typename Head, typename... Types>
struct TupleElement<0, Tuple<Head, Types...>> {
    using type = Head;
};

template <typename T, typename... Types>
constexpr size_t type_count_v = detail::type_count<T, Types...>::value;

template <typename T, typename... Types>
constexpr size_t find_type_v = detail::find_type<0, T, Types...>::value;

template <typename... Tuples>
using many_tuple_t = typename detail::many_tuples<Tuples...>::type;

template <typename>
struct is_tuple: std::false_type {};

template <typename... Types>
struct is_tuple<Tuple<Types...>>: std::true_type {};

template <typename Head, typename... Tail>
class Tuple<Head, Tail...> {
private:
    Head head;
    [[no_unique_address]] Tuple<Tail...> tail;

    template <typename... Utypes>
    friend class Tuple;

    template< typename T, typename... Types >
    requires (type_count_v<T, Types...> == 1)
    friend T& get(Tuple<Types...>&);

    template< typename T, typename... Types >
    requires (type_count_v<T, Types...> == 1)
    friend const T& get(const Tuple<Types...>&);

    template< typename T, typename... Types >
    requires (type_count_v<T, Types...> == 1)
    friend T&& get(Tuple<Types...>&&);

    template< typename T, typename... Types >
    requires (type_count_v<T, Types...> == 1)
    friend const T&& get(const Tuple<Types...>&&);

    template <std::size_t N, typename... Types>
    friend typename TupleElement<N, Tuple<Types...>>::type& get(Tuple<Types...>&);

    template <std::size_t N, typename... Types>
    friend typename TupleElement<N, Tuple<Types...>>::type&& get(Tuple<Types...>&&);

    template <std::size_t N, typename... Types>
    friend const typename TupleElement<N, Tuple<Types...>>::type& get(const Tuple<Types...>&);

    template <std::size_t N, typename... Types>
    friend const typename TupleElement<N, Tuple<Types...>>::type&& get(const Tuple<Types...>&&);

    template <typename... Tuples>
    friend auto tupleCat(Tuples&&... args);

    template <typename First, typename... Others>
    requires (std::conjunction_v<is_tuple<std::decay_t<First>>, is_tuple<std::decay_t<Others>>...>)
    explicit Tuple(First&& first, Others&&... others) : head(get<0>(std::forward<First>(first))),
                    tail(first.tail, std::forward<Others>(others)...) {}


    template <typename... Args>
    requires (std::conjunction_v<is_tuple<std::decay_t<Args>>...>)
    explicit Tuple(Tuple<>, Args&&... args): Tuple(std::forward<Args>(args)...) {}

public:

    constexpr explicit(!std::conjunction_v<is_copy_list_initializable<Head>, is_copy_list_initializable<Tail>...>)
    Tuple() requires (std::conjunction_v<std::is_default_constructible<Head>, std::is_default_constructible<Tail>...>)
            : head(), tail() {}

    explicit(!std::conjunction_v<std::is_convertible<const Head&, Head>, std::is_convertible<const Tail&, Tail>...>)
    Tuple(const Head& args_head, const Tail&... args_tail)
    requires (std::conjunction_v<std::is_copy_constructible<Head>, std::is_copy_constructible<Tail>...>)
            : head(args_head), tail(args_tail...) {}

    template <typename First, typename... UTypes>
    requires (sizeof...(UTypes) == sizeof...(Tail) &&
        std::conjunction_v<std::is_constructible<Head, First>, std::is_constructible<Tail, UTypes>...>)
    explicit(!std::conjunction_v<std::is_convertible<Head, First>, std::is_convertible<Tail, UTypes>...>)
    Tuple(First&& first, UTypes&&... args)
            : head(std::forward<First>(first)), tail(std::forward<UTypes>(args)...) {}

    template <typename First, typename... UTypes>
    explicit(!std::conjunction_v<std::is_convertible<const First&, Head>, std::is_convertible<const UTypes&, Tail>...>)
    Tuple(const Tuple<First, UTypes...>& other) requires (sizeof...(UTypes) == sizeof...(Tail)
        && std::conjunction_v<std::is_constructible<Head, const First&>, std::is_constructible<Tail, const UTypes&>...>
            && (sizeof...(Tail) != 0 || (!std::is_convertible_v<decltype(other), Head>
                && !std::is_constructible_v<Head, decltype(other)> && !std::is_same_v<Head, First>)))
    : head(get<0>(other)), tail(other.tail) {}

    template <typename First, typename... UTypes>
    explicit(!std::conjunction_v<std::is_convertible<First&&, Head>, std::is_convertible<UTypes&&, Tail>...>)
    Tuple(Tuple<First, UTypes...>&& other) requires (sizeof...(UTypes) == sizeof...(Tail)
    && std::conjunction_v<std::is_constructible<Head, First&&>, std::is_constructible<Tail, UTypes&&>...>
        && (sizeof...(Tail) != 0 || (!std::is_convertible_v<decltype(other), Head>
            && !std::is_constructible_v<Head, decltype(other)> && !std::is_same_v<Head, First>)))
    : head(get<0>(std::forward<decltype(other)>(other))), tail(std::move(other.tail)) {}

    template <typename T1, typename T2>
    requires (sizeof...(Tail) == 1
        && std::conjunction_v<std::is_constructible<Head, const T1&>, std::is_constructible<Tail, const T2&>...>)
    Tuple(const std::pair<T1, T2>& pair): head(pair.first), tail(pair.second) {}

    template <typename T1, typename T2>
    requires (sizeof...(Tail) == 1
        && std::conjunction_v<std::is_constructible<Head, T1&&>, std::is_constructible<Tail, T2&&>...>)
    Tuple(std::pair<T1, T2>&& pair)
            : head(std::move(pair.first)), tail(std::move(pair.second)) {}

    Tuple(const Tuple& other)
    requires (std::conjunction_v<std::is_copy_constructible<Head>, std::is_copy_constructible<Tail>...>)
            : head(other.head), tail(other.tail) {}

    Tuple(Tuple&& other)
    requires (std::conjunction_v<std::is_move_constructible<Head>, std::is_move_constructible<Tail>...>)
            : head(std::move(other.head)), tail(std::move(other.tail)) {}


    Tuple& operator=(const Tuple& other)
    requires (std::conjunction_v<std::is_copy_assignable<Head>, std::is_copy_assignable<Tail>...>)
    {
        if (this == &other) {
            return *this;
        }
        head = other.head;
        tail = other.tail;
        return *this;
    }

    Tuple& operator=(Tuple&& other)
    requires (std::conjunction_v<std::is_move_assignable<Head>, std::is_move_assignable<Tail>...>)
    {
        head = std::forward<Head>(get<0>(other));
        tail = std::move(other.tail);
        return *this;
    }

    template <typename First, typename... UTypes>
    Tuple& operator=(const Tuple<First, UTypes...>& other)
    requires (sizeof...(UTypes) == sizeof...(Tail)
        && std::conjunction_v<std::is_assignable<Head&, const First&>, std::is_assignable<Tail&, const UTypes&>...>)
    {
        head = other.head;
        tail = other.tail;
        return *this;
    }

    template <typename First, typename... UTypes>
    Tuple& operator=(Tuple<First, UTypes...>&& other)
    requires (sizeof...(UTypes) == sizeof...(Tail)
        && std::conjunction_v<std::is_assignable<Head&, First>, std::is_assignable<Tail&, UTypes>...>)
    {
        head = get<0>(std::move(other));
        tail = std::move(other.tail);
        return *this;
    }

    template <typename T1, typename T2>
    Tuple& operator=(const std::pair<T1, T2>& other) {
        head = other.first;
        tail = other.second;
        return *this;
    }

    template <typename T1, typename T2>
    Tuple& operator=(std::pair<T1, T2>&& other) {
        head = std::move(other.first);
        tail = std::move(other.second);
        return *this;
    }

    template <typename... A, typename... B>
    friend constexpr auto operator<=>(const Tuple<A...>&, const Tuple<B...>&);

};

template <typename T1, typename T2>
Tuple(std::pair<T1, T2>&&) -> Tuple<T1, T2>;

template <typename T1, typename T2>
Tuple(const std::pair<T1, T2>&) -> Tuple<T1, T2>;

template <typename... Args>
constexpr auto makeTuple(Args&&... args){
    return Tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
}

template <typename... Types>
Tuple<Types&...> tie(Types&... args) noexcept {
    return {args...};
}

template <typename... Types>
Tuple<Types&&...> forwardAsTuple(Types&&... args) noexcept {
    return {std::forward<Types>(args)...};
}

template <std::size_t N, typename... Types>
typename TupleElement<N, Tuple<Types...>>::type& get(Tuple<Types...>& tuple) {
    if constexpr (N == 0) {
        return tuple.head;
    } else {
        return get<N - 1>(tuple.tail);
    }
}

template <std::size_t N, typename... Types>
typename TupleElement<N, Tuple<Types...>>::type&& get(Tuple<Types...>&& tuple) {
    if constexpr (N == 0) {
        return std::forward<decltype(tuple.head)>(tuple.head);
    } else {
        return get<N - 1>(std::forward<decltype(tuple.tail)>(tuple.tail));
    }
}

template <std::size_t N, typename... Types>
const typename TupleElement<N, Tuple<Types...>>::type& get(const Tuple<Types...>& tuple) {
    if constexpr (N == 0) {
        return tuple.head;
    } else {
        return get<N - 1>(tuple.tail);
    }
}

template <std::size_t N, typename... Types>
const typename TupleElement<N, Tuple<Types...>>::type&& get(const Tuple<Types...>&& tuple) {
    if constexpr (N == 0) {
        return tuple.head;
    } else {
        return get<N - 1>(tuple.tail);
    }
}

template <typename T, typename... Types>
requires (type_count_v<T, Types...> == 1)
T& get(Tuple<Types...>& tuple) {
    return get<find_type_v<T, Types...>>(tuple);
}

template <typename T, typename... Types>
requires (type_count_v<T, Types...> == 1)
T&& get(Tuple<Types...>&& tuple) {
    return get<find_type_v<T, Types...>>(tuple);
}

template <typename T, typename... Types>
requires (type_count_v<T, Types...> == 1)
const T& get(const Tuple<Types...>& tuple) {
    return get<find_type_v<T, Types...>>(tuple);
}

template< typename T, typename... Types >
requires (type_count_v<T, Types...> == 1)
const T&& get(const Tuple<Types...>&& tuple) {
    return get<find_type_v<T, Types...>>(tuple);
}

template <typename... Tuples>
auto tupleCat(Tuples&&... args) {
    return many_tuple_t<std::decay_t<Tuples>...>(std::forward<Tuples>(args)...);
}

template <typename... A, typename... B>
requires (sizeof...(A) == sizeof...(B))
constexpr auto operator<=>(const Tuple<A...>& first, const Tuple<B...>& second) {
    if constexpr (sizeof...(A) == 0) {
        return std::strong_ordering::equal;
    } else {
        if (auto c = std::compare_three_way{}(get<0>(first), get<0>(second)); c != 0) {
            return c;
        }
        return first.tail <=> second.tail;
    }
}

template<class... A, class... B>
requires (sizeof...(A) == sizeof...(B))
constexpr bool operator==(const Tuple<A...>& a, const Tuple<B...>& b) {
    return std::is_eq(a <=> b);
}
