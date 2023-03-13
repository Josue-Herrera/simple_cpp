#pragma once

#include <algorithm>
#include <numeric>
#include <type_traits>

namespace calgo {
    template <class, class = void>
    constexpr bool is_iterable{};
    
    template <class T>
    constexpr bool is_iterable
    <   T, 
        std::void_t 
        < 
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end())
        >
    > = true;

    template<class, class = void>
    constexpr bool is_container{};

    template<class T>
    constexpr bool is_container
    <   T,
        std::void_t
        <
            typename T::value_type,
            typename T::size_type,
            typename T::iterator,
            typename T::const_iterator,
            decltype(std::declval<T>().size()),
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end()),
            decltype(std::declval<T>().cbegin()),
            decltype(std::declval<T>().cend())
        >
    > = true;

    template<class... Ts> struct overload: Ts... { using Ts::operator()...; };
    template<class... Ts> overload(Ts...) -> overload<Ts...>;

    template<class Iterator>
    struct iterable {
        using iterator = Iterator;
        Iterator begin_, end_;
        constexpr iterable() = default;
        constexpr explicit iterable(std::pair<Iterator,Iterator> p) : begin_{p.first}, end_{p.second}{}
        constexpr Iterator begin() const { return begin_; }
        constexpr Iterator end()   const { return end_; }
        constexpr Iterator begin() { return begin_; }
        constexpr Iterator end()   { return end_; }
    };
    template<class Iterator>
    iterable(Iterator begin, Iterator end) -> iterable<Iterator>;
    template<class Iterator>
    iterable(std::pair<Iterator,Iterator>) -> iterable<Iterator>;

    template<class I>
    constexpr auto distance(std::pair<I,I> p) noexcept {
        return std::distance(p.first, p.second);
    }

    template<class Container>
    constexpr auto distance(Container const& c) noexcept {
        return std::distance(std::begin(c), std::end(c));
    }

    template<class T>
    constexpr auto equals_one (T t) noexcept -> bool { return t == T{1}; }

    template<class T>
    constexpr auto only_one_exists (T t) noexcept -> bool { return equals_one(distance(t)); }
   
    template<class T>
    constexpr auto none_exists (T t) noexcept -> bool { return not distance(t); }

    template<class Tuple>
    constexpr auto value(Tuple const& val) noexcept  { return std::get<1>(val); }

    template<class Tuple>
    constexpr auto key(Tuple const& val) noexcept { return std::get<0>(val); }

     /**
     * @brief Reverse the order of the elemaents of a tuple like object.
     * 
     * @tparam T1 type 1
     * @tparam T2 type 2
     * @tparam Tuple like object.
     * @param t type to be reversed.
     * @return Tuple<T2,T1> 
     */
    template< class T1, class T2, template<class, class> class Tuple>
    constexpr auto reverse(Tuple<T1,T2> const& t) noexcept -> Tuple<T2,T1> {
        return { std::get<1>(t), std::get<0>(t) };
    }

     /**
     * @brief finds an Elements in the @p container that equals the @p value.
     * 
     * @tparam Container STL like container.
     * @tparam Value type that is equality comparable to the value type of the container.
     * @param container  mutable reference to a container.
     * @param value moves the elements to the end to be erased.
     * @return Iterator of the elements that equals the value.
     */
    template<class Container, class Value>
    constexpr auto find(Container const& container, Value&& value) noexcept -> typename Container::const_iterator {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::find(std::begin(container), std::end(container), std::forward<Value>(value));
    }

    template<class Container, class Predicate>
    constexpr auto find_if (Container const& container, Predicate predicate) noexcept -> typename Container::const_iterator {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        return std::find_if(std::begin(container), std::end(container), predicate);
    }

    template<class Container, class Predicate>
    constexpr auto contains_if(Container& container, Predicate predicate) noexcept -> bool {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return find_if(container, predicate) != std::end(container);
    }

    /**
     * @brief Checks to see if the @p container contains @p value in it.
     * 
     * @tparam Container STL like container.
     * @tparam Value type that is equality comparable to the value type of the container.
     * @param container  mutable reference to a container.
     * @param value moves the elements to the end to be erased.
     * @return bool if the elements that equals the value was found.
     */
    template<class Container, class Value>
    constexpr auto contains(Container& container, Value&& value) noexcept -> bool {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return find(container, std::forward<Value>(value)) != std::end(container);
    }
    
    /**
     * @brief Moves Elements to the end of @p container that equals the @p value.
     * 
     * @tparam Container STL like container.
     * @tparam Value type that is equality comparable to the value type of the container.
     * @param container  mutable reference to a container.
     * @param value moves the elements to the end to be erased.
     * @return Iterator of the start of the elements that need to be erased.
     */
    template<class Container, class Value>
    constexpr auto remove(Container& container, Value&& value) noexcept -> typename Container::iterator {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::remove(std::begin(container), std::end(container), std::forward<Value>(value));
    }

    /**
     * @brief Moves Elements to the end of @p container that don't pass the @p predicate.
     * 
     * @tparam Container Container STL like container.
     * @tparam Predicate Unary Predicate Callable that takes the container's value type.
     * @param container  mutable reference to a container.
     * @param predicate moves the elements to the end to be erased.
     * @return Iterator of the start of the elements that need to be erased.
     */
    template<class Container, class Predicate>
    constexpr auto remove_if (Container& container, Predicate predicate) noexcept -> typename Container::iterator {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::remove_if(std::begin(container), std::end(container), predicate);
    }

    /**
     * @brief Moves Elements to the end of @p container that equal adjacent Elements.
     * 
     * @tparam Container STL like container.
     * @param container  mutable reference to a container.
     * @return Iterator of the start of the elements that need to be erased.
     */
    template<class Container>
    constexpr auto unique(Container& container) noexcept -> typename Container::iterator {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::unique(std::begin(container), std::end(container));
    }

    /**
     * @brief Moves Elements to the end of @p container that equal adjacent Elements.
     * 
     * @tparam Container STL like container.
     * @tparam Predicate Binary Predicate Callable that takes two container's value type.
     * @param container  mutable reference to a container.
     * @param predicate moves the elements to the end to be erased.
     * @return Iterator of the start of the elements that need to be erased.
     */
    template<class Container, class Predicate>
    constexpr auto unique_if(Container& container, Predicate predicate) noexcept -> typename Container::iterator {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::unique(std::begin(container), std::end(container), predicate);
    }

    template <class Iterator, class Out, class Predicate, class Transform>
    constexpr Out unique_transform(Iterator begin, Iterator end, Out out, Predicate predicate, Transform transform) noexcept
    {
        if (begin != end)
        {
            typename Iterator::value_type temp(*begin);
            *out = transform(temp);
            ++out;
            while (++begin != end)
            {
                if (not predicate(temp, *begin))
                {
                    temp = *begin;
                    *out = transform(temp);
                    ++out;
                }
            }
        }

        return out;
    }
       /**
     * @brief Moves Elements to the end of @p container that equal adjacent Elements.
     * 
     * @tparam Container STL like container.
     * @tparam Predicate Binary Predicate Callable that takes two container's value type.
     * @param container  mutable reference to a container.
     * @param predicate moves the elements to the end to be erased.
     * @return Iterator of the start of the elements that need to be erased.
     */
    template<class Container, class Out, class Transform>
    constexpr auto unique_transform(Container const& container, Out out, Transform transform) noexcept -> Out {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        using value_t = typename Container::value_type;
        return calgo::unique_transform(std::begin(container), std::end(container), out, std::equal_to<value_t>{}, transform);
    }

    /**
     * @brief Moves Elements to the end of @p container that equal adjacent Elements.
     * 
     * @tparam Container STL like container.
     * @tparam Predicate Binary Predicate Callable that takes two container's value type.
     * @param container  mutable reference to a container.
     * @param predicate moves the elements to the end to be erased.
     * @return Iterator of the start of the elements that need to be erased.
     */
    template<class Container, class Out>
    constexpr auto unique_copy(Container const& container, Out out) noexcept -> Out {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::unique_copy(std::begin(container), std::end(container), out);
    }

    /**
     * @brief Erase an Element in the container at the iterator.
     * 
     * @tparam Container STL like container.
     * @tparam Iterator from the Container passed in.
     * @param container mutable reference to a container.
     * @param location location that gets erased.
     */
    template<class Container>
    constexpr auto erase(Container& container, typename Container::key_type&& key) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        container.erase(std::forward<typename Container::key_type>(key));    
    }

    /**
     * @brief Erase an Element in the container at the iterator.
     * 
     * @tparam Container STL like container.
     * @tparam Iterator from the Container passed in.
     * @param container mutable reference to a container.
     * @param location location that gets erased.
     */
    template<class Container>
    constexpr auto erase(Container& container, typename Container::iterator b, typename Container::iterator e) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        container.erase(b, e);    
    }

    /**
     * @brief Erase an Element in the container at the iterator.
     * 
     * @tparam Container STL like container.
     * @tparam Iterator from the Container passed in.
     * @param container mutable reference to a container.
     * @param location location that gets erased.
     */
    template<class Container>
    constexpr auto erase_at (Container& container, typename Container::iterator location) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        container.erase(location);    
    }

    /**
     * @brief Erase a @p container from a @p start iterator to the end.
     * 
     * @tparam Container STL like container.
     * @tparam Iterator from the Container passed in.
     * @param container mutable reference to a container.
     * @param start location to the end that gets erased.
     */
    template<class Container, class Iterator>
    constexpr auto erase_to_end (Container& container, Iterator start) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        static_assert(std::is_same_v<typename Container::iterator, Iterator>, "Must be an Iterator of passed in Container.");
        container.erase(start, std::end(container));    
    }

    /**
     * @brief Erase Elements of @p container that don't pass the @p predicate.
     * 
     * @tparam Container Container STL like container.
     * @tparam Predicate Unary Predicate Callable.
     * @param container  mutable reference to a container.
     * @param predicate that returns true if the element should be erased.
     */
    template<class Container, class Predicate>
    constexpr auto erase_if (Container& container, Predicate predicate) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        erase_to_end(container, remove_if(container, predicate));
    }

    /**
     * @brief Erase Elements to the end of @p container that follow equal adjacent Elements.
     * 
     * @tparam Container Container STL like container.
     * @param container  mutable reference to a container.
     */
    template<class Container>
    constexpr auto erase_duplicates(Container& container) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        erase_to_end(container, unique(container));
    }

    /**
     * @brief Erase Elements to the end of @p container that follow equal adjacent Elements.
     * 
     * @tparam Container STL like container.
     * @tparam Predicate Binary Predicate Callable that takes two container's value type.
     * @param container  mutable reference to a container.
     * @param predicate  returns true if two adjacent elements are equal.
     */
    template<class Container, class Predicate>
    constexpr auto erase_duplicates_if(Container& container, Predicate predicate) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        erase_to_end(container, unique_if(container, predicate));
    }

    /**
     * @brief Erases an Element of the @p container that is found by the @p predicate.
     * 
     * @tparam Container Container STL like container.
     * @tparam Predicate Unary Predicate Callable.
     * @param container mutable reference to a container.
     * @param predicate that returns true if an element is found.
     */
    template<class Container, class Predicate>
    constexpr auto erase_if_found (Container& container, Predicate predicate) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        erase_at(container, std::find_if(std::begin(container), std::end(container), predicate));
    }

   /**
     * @brief Erase Elements to the end of @p container that equals the @p value.
     * 
     * @tparam Container STL like container.
     * @tparam Value type that is equality comparable to the value type of the container.
     * @param container  mutable reference to a container.
     * @param value elements to the end to be erased.
     */
    template<class Container, class Value>
    constexpr auto erase_value(Container& container, Value&& value) noexcept -> void {
        static_assert(is_container<Container>, "Must be an STL like Container. ");
        erase_to_end(container, remove(container, std::forward<Value>(value)));
    }

    /**
     * @brief 
     * 
     * @tparam Container 
     * @tparam Out 
     * @tparam TransformFunc 
     * @param container 
     * @param out 
     * @param transform 
     * @return Out 
     */
    template<class Container, class Out, class TransformFunc>
    constexpr auto transform(Container const& container, Out out, TransformFunc transform) noexcept -> Out {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::transform(std::begin(container), std::end(container), out, transform);
    }

    /**
     * @brief Symmetric Cartesian Product Mininmum, Upper Right traingle Iteration.
     * 
     * @param begin Iterator to the start of range 
     * @param end Iterator to the end of range.
     * @param op Binary Operation that takes 2 elememts.
     * 
     * Example Collection : [(1, 0), (2, 0), (3, 0), (4,0)]
     * iterations : 
     *      (1, 0) (2, 0) 
     *      (1, 0) (3, 0)
     *      (1, 0) (4, 0)
     *      (2, 0) (3, 0)
     *      (2, 0) (4, 0)
     *      (3, 0) (4, 0)     
     *   
     * complexity : O(n) + O(n-1) + ... while n > 0.
     */
    template<class Iterator, class BinaryOperation>
    constexpr auto triangle_product(Iterator begin, Iterator end, BinaryOperation op) noexcept -> void {
       while(begin != end) {
            auto next = std::next(begin);
            while (next != end) {
                op(*begin,*next);
                ++next;
            }
            ++begin;
       }
    }

    /**
     * @brief 
     * 
     * @tparam Container 
     * @tparam BinaryOperation 
     * @param container 
     * @param op 
     */
    template<class Container, class BinaryOperation>
    constexpr auto triangle_product(Container const& container, BinaryOperation op) noexcept -> void {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        calgo::triangle_product(std::begin(container), std::end(container), op);
    }

    template<class Container>
    constexpr auto sort(Container& container) noexcept -> void {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        std::sort(std::begin(container), std::end(container));
    }

    template<class Container, class BinaryPredicate>
    constexpr auto sort(Container& container, BinaryPredicate binary_predicate) noexcept -> void {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        std::sort(std::begin(container), std::end(container), binary_predicate);
    }

    template<class Container, class Contianer2, class BinaryPredicate>
    constexpr auto equal(Container const& container, Contianer2 const& container2, BinaryPredicate binary_predicate) noexcept -> bool {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::equal(std::begin(container), std::end(container),
               std::begin(container2), std::end(container2),
               binary_predicate);
    }

    template<class Container, class Container2>
    constexpr auto equal_values(Container const& container, Container2 const& container2) noexcept -> bool {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        static_assert(is_iterable<Container2>, "Must be iterable [have begin() and end() functions.]");
        return std::equal(std::begin(container), std::end(container),
               std::begin(container2), std::end(container2),
               [](auto&a, auto&b){ return calgo::value(a) == calgo::value(b);});
    }

    template<class I1, class I2, class BinaryPredicate>
    constexpr auto equal(std::pair<I1,I1> const& pair_range1, std::pair<I2,I2> const& pair_range2, BinaryPredicate binary_predicate) noexcept -> bool {
       return calgo::equal(iterable{pair_range1}, iterable{pair_range2}, binary_predicate);
    }

    template<class I1, class I2>
    constexpr auto equal_values(std::pair<I1,I1> const& pair_range1, std::pair<I2,I2> const& pair_range2) noexcept -> bool {
       return calgo::equal_values(iterable{pair_range1}, iterable{pair_range2});
    }

    template<class Container>
    constexpr auto back_inserter(Container& container) noexcept  {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::inserter(container, std::end(container));
    }

    template<class Container>
    constexpr auto front_inserter(Container& container) noexcept  {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        return std::inserter(container, std::begin(container));
    }

    template<class InputIt1, class InputIt2, class Operation>
    constexpr void set_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, Operation operation) noexcept {
        while (first1 != last1 && first2 != last2)
        {
            if (*first1 < *first2)
                ++first1;
            else
            {
                if (not(*first2 < *first1))
                    operation(*first1++); // *first1 and *first2 are equivalent.
                ++first2;
            }
        }
    }

    template<class InputIt1, class InputIt2, class Operation, class Compare>
    constexpr void set_intersection(InputIt1 first1, InputIt1 last1,
                                    InputIt2 first2, InputIt2 last2, Operation operation, Compare comp)
    {
        while (first1 != last1 && first2 != last2)
        {
            if (comp(*first1, *first2))
                ++first1;
            else
            {
                if (not comp(*first2, *first1))
                   operation(*first1++); // *first1 and *first2 are equivalent.
                ++first2;
            }
        }
    }

    template<class Container, class Container2, class Operation>
    constexpr void set_intersection(Container& container, Container2& container2, Operation operation) noexcept {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        static_assert(is_iterable<Container2>, "Must be iterable [have begin() and end() functions.]");
        calgo::set_intersection(std::begin(container), std::end(container),
                                std::begin(container2), std::end(container2), operation);
    }

    template<class Container, class Container2, class Operation, class Compare>
    constexpr void set_intersection(Container& container, Container2& container2, Operation operation, Compare comp) noexcept {
        static_assert(is_iterable<Container>, "Must be iterable [have begin() and end() functions.]");
        static_assert(is_iterable<Container2>, "Must be iterable [have begin() and end() functions.]");
        calgo::set_intersection(std::begin(container), std::end(container),
                                std::begin(container2), std::end(container2), 
                                operation, comp);
    }

    
}

