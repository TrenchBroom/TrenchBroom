/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef KDL_VECTOR_UTILS_H
#define KDL_VECTOR_UTILS_H

#include "collection_utils.h"

// Note: all except <cassert> are included by <vector> anyway, so there's no point in splitting this up further
#include <cassert>
#include <algorithm> // for std::sort, std::unique, std::find, std::find_if, std::remove, std::remove_if
#include <iterator> // std::back_inserter
#include <functional> // for std::less
#include <optional>
#include <type_traits> // for std::less
#include <vector>

namespace kdl {
    /**
     * Returns the vector element at the given index.
     *
     * Precondition: 0 <= index < v.size()
     *
     * @tparam T the type of the vector elements
     * @param v the vector
     * @param index the index
     * @return a const reference to the element at the given index
     */
    template <typename T, typename I>
    const T& vec_at(const std::vector<T>& v, const I index) {
        assert(index >= 0);
        const auto index_s = static_cast<typename std::vector<T>::size_type>(index);
        assert(index_s < v.size());
        return v[index_s];
    }

    /**
     * Returns the vector element at the given index.
     *
     * Precondition: 0 <= index < v.size()
     *
     * @tparam T the type of the vector elements
     * @param v the vector
     * @param index the index
     * @return a const reference to the element at the given index
     */
    template <typename T, typename I>
    T& vec_at(std::vector<T>& v, const I index) {
        assert(index >= 0);
        const auto index_s = static_cast<typename std::vector<T>::size_type>(index);
        assert(index_s < v.size());
        return v[index_s];
    }

    /**
     * Removes the last element of the given vector and returns it.
     *
     * Precondition: !v.empty()
     *
     * @tparam T the type of the vector elements
     * @param v the vector
     * @return the last element of the given vector
     */
    template <typename T>
    T vec_pop_back(std::vector<T>& v) {
        assert(!v.empty());
        T result = std::move(v.back());
        v.pop_back();
        return result;
    }

    /**
    * Removes the first element of the given vector and returns it.
    *
    * Precondition: !v.empty()
    *
    * @tparam T the type of the vector elements
    * @param v the vector
    * @return the first element of the given vector
    */
    template <typename T>
    T vec_pop_front(std::vector<T>& v) {
        assert(!v.empty());
        T result = std::move(v.front());
        v.erase(v.begin(), v.begin() + 1);
        return result;
    }

    /**
     * Returns a vector containing elements of type O, each of which is constructed by passing the corresponding
     * element of v to the constructor of o, e.g. result.push_back(O(e)), where result is the resulting vector, and e
     * is an element from v.
     *
     * Precondition: O must be constructible with an argument of type T
     *
     * @tparam O the type of the result vector elements
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @param v the vector to cast
     * @return a vector containing the elements of a, but with O as the element type
     */
    template<typename O, typename T, typename A>
    std::vector<O> vec_element_cast(const std::vector<T, A>& v) {
        if constexpr (std::is_same_v<T, O>) {
            return v;
        } else {
            std::vector<O> result;
            result.reserve(v.size());
            for (const auto& e : v) {
                result.push_back(O(e));
            }
            return result;
        }
    }
    
    /**
     * Finds the smallest index at which the given predicate is satisified in the given vector. If the given vector does
     * not such a value, an empty optional is returned.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam P the predicate type
     * @param v the vector to check
     * @param p the predicate
     * @return the smallest index at which the given predicate is satisfied in the given vector or an empty optional if
     * the given vector does not contain such a value
     */
    template<typename T, typename A, typename P,
    typename std::enable_if_t<
        std::is_invocable_r_v<bool, P, const T&>
    >* = nullptr>
    std::optional<typename std::vector<T, A>::size_type> vec_index_of(const std::vector<T, A>& v, P&& p) {
        using IndexType = typename std::vector<T, A>::size_type;
        for (IndexType i = 0; i < v.size(); ++i) {
            if (p(v[i])) {
                return i;
            }
        }
        return std::nullopt;
    }

    /**
     * Finds the smallest index at which the given value is found in the given vector. If the given vector does not
     * contain the given value, the size of the vector is returned.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam X the value type
     * @param v the vector to check
     * @param x the value to find
     * @return the smallest index at which the given value is found in the given vector or the vector's size if the
     * given vector does not contain the given value
     */
    template<typename T, typename A, typename X>
    std::optional<typename std::vector<T, A>::size_type> vec_index_of(const std::vector<T, A>& v, const X& x) {
        return vec_index_of(v, [&](const auto& e) { return e == x; });
    }
    
    /**
     * Finds the smallest index at which the given predicate is satisified in the given vector. If the given vector does
     * not such a value, an empty optional is returned.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam P the predicate type
     * @param v the vector to check
     * @param p the predicate
     * @return true if the given vector contains an element that satisfies the given predicate
     */
    template<typename T, typename A, typename P,
    typename std::enable_if_t<
        std::is_invocable_r_v<bool, P, const T&>
    >* = nullptr>
    bool vec_contains(const std::vector<T, A>& v, P&& p) {
        return vec_index_of(v, std::forward<P>(p)).has_value();
    }

    /**
     * Checks if the given value is contained in the given vector.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam X the value type
     * @param v the vector to check
     * @param x the value to check
     * @return true if the given vector contains the given value and false otherwise
     */
    template<typename T, typename A, typename X>
    bool vec_contains(const std::vector<T, A>& v, const X& x) {
        return vec_index_of(v, x).has_value();
    }

    namespace detail {
        template<typename T, typename A, typename... Args>
        void vec_append(std::vector<T, A>& v1, const Args& ... args) {
            (..., v1.insert(std::end(v1), std::begin(args), std::end(args)));
        }
    }

    /**
     * Appends the elements from the vectors in argument pack args to the end of v. Each element of function argument
     * pack args must be a vector with value_type T and allocator A.
     *
     * For each of the elements of args, the elements are copied into v in the order in which the appear.
     *
     * @tparam T the element type
     * @tparam A the allocator type
     * @tparam Args parameter pack containing the vectors to append to v
     * @param v the vector to append to
     * @param args the vectors to append to v
     */
    template<typename T, typename A, typename... Args>
    void vec_append(std::vector<T, A>& v, const Args& ... args) {
        v.reserve(kdl::col_total_size(v, args...));
        (..., v.insert(std::end(v), std::begin(args), std::end(args)));
    }

    /**
     * Returns a vector that contains all elements from the given vectors v and the elements of args in the order in
     * which they appear.
     *
     * @tparam T the element type
     * @tparam A the allocator type
     * @tparam Args parameter pack containing further vectors to append to v
     * @param v the first vector to concatenate
     * @param args further vectors to concatenate
     * @return a vector containing the elements from the given vectors
     */
    template<typename T, typename A, typename... Args>
    std::vector<T, A> vec_concat(const std::vector<T, A>& v, const Args& ... args) {
        std::vector<T, A> result;
        vec_append(result, v, args...);
        return result;
    }

    /**
     * Returns a slice of the given vector starting at offset and with count elements.
     *
     * If the given offset is not less than the number of elements of v, then an empty vector is returned. The returned
     * vector contains at most count elements from the given vector. If the given count is too large, i.e. it indicates
     * to include elements beyond the end of the given vector, then count is adjusted accordingly.
     *
     * The elements are copied into the returned vector.
     *
     * Precondition: offset + count does not exceed the number of elements in the given vector
     *
     * @tparam T the element type
     * @tparam A the allocator type
     * @param v the vector to return a slice of
     * @param offset the offset of the first element to return
     * @param count the number of elements to return
     * @return a vector containing the slice of the given vector
     */
    template <typename T, typename A>
    std::vector<T, A> vec_slice(const std::vector<T, A>& v, const std::size_t offset, const std::size_t count) {
        assert(offset + count <= v.size());

        std::vector<T, A> result;
        result.reserve(count);

        for (std::size_t i = 0u; i < count; ++i) {
            result.push_back(v[i + offset]);
        }

        return result;
    }

    /**
     * Returns a prefix of the given vector with count elements.
     *
     * The elements are copied into the returned vector.
     *
     * Precondition: count does not exceed the number of elements in the given vector
     *
     * @tparam T the element type
     * @tparam A the allocator type
     * @param v the vector to return a prefix of
     * @param count the number of elements to return
     * @return a vector containing the prefix of the given vector
     */
    template <typename T, typename A>
    std::vector<T, A> vec_slice_prefix(const std::vector<T, A>& v, const std::size_t count) {
        assert(count <= v.size());
        return vec_slice(v, 0u, count);
    }

    /**
     * Returns a suffix of the given vector with count elements.
     *
     * The elements are copied into the returned vector.
     *
     * Precondition: count does not exceed the number of elements in the given vector
     *
     * @tparam T the element type
     * @tparam A the allocator type
     * @param v the vector to return a prefix of
     * @param count the number of elements to return
     * @return a vector containing the prefix of the given vector
     */
    template <typename T, typename A>
    std::vector<T, A> vec_slice_suffix(const std::vector<T, A>& v, const std::size_t count) {
        assert(count <= v.size());
        return vec_slice(v, v.size() - count, count);
    }


    /**
     * Erases every element from the given vector which is equal to the given value using the erase-remove idiom.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam X the value type
     * @param v the vector
     * @param x the value to erase
     */
    template<typename T, typename A, typename X>
    void vec_erase(std::vector<T, A>& v, const X& x) {
        v.erase(std::remove(std::begin(v), std::end(v), x), std::end(v));
    }

    /**
     * Erases every element from the given vector for which the given predicate evaluates to true using the erase-remove
     * idiom.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam P the predicate type
     * @param v the vector
     * @param predicate the predicate
     */
    template<typename T, typename A, typename P>
    void vec_erase_if(std::vector<T, A>& v, const P& predicate) {
        v.erase(std::remove_if(std::begin(v), std::end(v), predicate), std::end(v));
    }

    /**
     * Erases the element at the given index from the given vector. The element is swapped with the last element of the
     * vector, and then the last element is erased.
     *
     * Precondition: i < v.size()
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @param v the vector
     * @param i the index of the element to erase, which must be less than the given vector's size
     */
    template<typename T, typename A>
    void vec_erase_at(std::vector<T, A>& v, const typename std::vector<T, A>::size_type i) {
        assert(i < v.size());
        auto it = std::next(std::begin(v), static_cast<typename std::vector<T, A>::difference_type>(i));
        v.erase(it);
    }

    /**
     * Erases every value from the given vector which is equal to any value in the given collection.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam C the collection type
     * @param v the vector to erase elements from
     * @param c the collection of values to erase
     */
    template<typename T, typename A, typename C>
    void vec_erase_all(std::vector<T, A>& v, const C& c) {
        for (const auto& x : c) {
            vec_erase(v, x);
        }
    }

    /**
     * Sorts the elements of the given vector according to the given comparator.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam Compare the type of the comparator to use
     * @param v the vector to sort
     * @param cmp the comparator to use for comparisons
     */
    template<typename T, typename A, typename Compare = std::less<T>>
    void vec_sort(std::vector<T, A>& v, const Compare& cmp = Compare()) {
        std::sort(std::begin(v), std::end(v), cmp);
    }

    /**
     * Sorts the elements of the given vector and removes all duplicate values. A value is a duplicate if it is
     * equivalent to its predecessor in the vector.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam Compare the type of the comparator to use
     * @param v the vector to sort and remove duplicates from
     * @param cmp the comparator to use for sorting and for determining equivalence
     */
    template<typename T, typename A, typename Compare = std::less<T>>
    void vec_sort_and_remove_duplicates(std::vector<T, A>& v, const Compare& cmp = Compare()) {
        std::sort(std::begin(v), std::end(v), cmp);
        v.erase(std::unique(std::begin(v), std::end(v), kdl::equivalence<T, Compare>(cmp)), std::end(v));
    }

    /**
     * Returns a vector containing every element of the given vector that passes the given filter.
     * The elements are copied into the returned vector in the same order as they are in the given vector.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam F the type of the filter to apply, must be of type `bool(const T&)`
     * @param v the vector
     * @param filter the filter to apply
     * @return a vector containing the elements that passed the filter
     */
    template<typename T, typename A, typename F,
        typename std::enable_if_t<
            std::is_invocable_v<F, const T&>
        >* = nullptr>
    std::vector<T, A> vec_filter(const std::vector<T, A>& v, F&& filter) {
        std::vector<T, A> result;
        result.reserve(v.size());

        for (const auto& x : v) {
            if (filter(x)) {
                result.push_back(x);
            }
        }

        return result;
    }

    /**
     * Returns a vector containing every element of the given vector that passes the given filter.
     * The elements are copied into the returned vector in the same order as they are in the given vector.
     *
     * This version passes the vector element indices to the filter function.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam F the type of the filter to apply, must be of type `bool(const T&, std::size_t)`
     * @param v the vector
     * @param filter the filter to apply
     * @return a vector containing the elements that passed the filter
     */
    template<typename T, typename A, typename F,
        typename std::enable_if_t<
            std::is_invocable_v<F, const T&, std::size_t>
        >* = nullptr>
    std::vector<T, A> vec_filter(const std::vector<T, A>& v, F&& filter) {
        std::vector<T, A> result;
        result.reserve(v.size());

        for (std::size_t i = 0u; i < v.size(); ++i) {
            if (filter(v[i], i)) {
                result.push_back(v[i]);
            }
        }

        return result;
    }

    /**
     * Returns a vector containing every element of the given vector that passes the given filter.
     * The elements are moved into the returned vector in the same order as they are in the given vector.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam F the type of the filter to apply, must be of type `bool(const T&)`
     * @param v the vector
     * @param filter the filter to apply
     * @return a vector containing the elements that passed the filter
     */
    template<typename T, typename A, typename F,
        typename std::enable_if_t<
            std::is_invocable_r_v<bool, F, const T&>
        >* = nullptr>
    std::vector<T, A> vec_filter(std::vector<T, A>&& v, F&& filter) {
        std::vector<T, A> result;
        result.reserve(v.size());

        for (auto&& x : v) {
            if (filter(x)) {
                result.push_back(std::move(x));
            }
        }

        return result;
    }

    /**
     * Returns a vector containing every element of the given vector that passes the given filter.
     * The elements are moved into the returned vector in the same order as they are in the given vector.
     *
     * This version passes the vector element indices to the filter function.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam F the type of the filter to apply, must be of type `bool(const T&, std::size_t)`
     * @param v the vector
     * @param filter the filter to apply
     * @return a vector containing the elements that passed the filter
     */
    template<typename T, typename A, typename F,
        typename std::enable_if_t<
            std::is_invocable_r_v<bool, F, const T&, std::size_t>
        >* = nullptr>
    std::vector<T, A> vec_filter(std::vector<T, A>&& v, F&& filter) {
        std::vector<T, A> result;
        result.reserve(v.size());

        for (std::size_t i = 0u; i < v.size(); ++i) {
            if (filter(v[i], i)) {
                result.push_back(std::move(v[i]));
            }
        }

        return result;
    }

    /**
     * Applies the given lambda to each element of the given vector and returns a vector containing the resulting
     * values, in order in which their original elements appeared in v.
     *
     * The elements are passed to the given lambda as const lvalue references.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam L the type of the lambda to apply
     * @param v the vector
     * @param transform the lambda to apply, must be of type `auto(const T&)`
     * @return a vector containing the transformed values
     */
    template<typename T, typename A, typename L,
        typename std::enable_if_t<
            std::is_invocable_v<L, const T&>
        >* = nullptr>
    auto vec_transform(const std::vector<T, A>& v, L&& transform) {
        using ResultType = decltype(transform(std::declval<const T&>()));

        std::vector<ResultType> result;
        result.reserve(v.size());
        for (const auto& x : v) {
            result.push_back(transform(x));
        }

        return result;
    }

    /**
     * Applies the given lambda to each element of the given vector and returns a vector containing the resulting
     * values, in order in which their original elements appeared in v.
     *
     * The elements are passed to the given lambda as const lvalue references.
     *
     * This version passes the vector element indices to the filter function.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam L the type of the lambda to apply
     * @param v the vector
     * @param transform the lambda to apply, must be of type `auto(const T&, std::size_t)`
     * @return a vector containing the transformed values
     */
    template<typename T, typename A, typename L,
        typename std::enable_if_t<
            std::is_invocable_v<L, const T&, std::size_t>
        >* = nullptr>
    auto vec_transform(const std::vector<T, A>& v, L&& transform) {
        using ResultType = decltype(transform(std::declval<const T&>(), std::declval<std::size_t>()));

        std::vector<ResultType> result;
        result.reserve(v.size());
        
        for (std::size_t i = 0u; i < v.size(); ++i) {
            result.push_back(transform(v[i], i));
        }

        return result;
    }
    
    /**
     * Applies the given lambda to each element of the given vector and returns a vector containing the resulting
     * values, in order in which their original elements appeared in v.
     *
     * The elements are passed to the given lambda as lvalue references.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam L the type of the lambda to apply
     * @param v the vector
     * @param transform the lambda to apply, must be of type `auto(const T&)`
     * @return a vector containing the transformed values
     */
    template<typename T, typename A, typename L,
        typename std::enable_if_t<
            std::is_invocable_v<L, T&>
        >* = nullptr>
    auto vec_transform(std::vector<T, A>& v, L&& transform) {
        using ResultType = decltype(transform(std::declval<T&>()));

        std::vector<ResultType> result;
        result.reserve(v.size());
        for (auto& x : v) {
            result.push_back(transform(x));
        }

        return result;
    }

    /**
     * Applies the given lambda to each element of the given vector and returns a vector containing the resulting
     * values, in order in which their original elements appeared in v.
     *
     * The elements are passed to the given lambda as lvalue references.
     *
     * This version passes the vector element indices to the filter function.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam L the type of the lambda to apply
     * @param v the vector
     * @param transform the lambda to apply, must be of type `auto(const T&, std::size_t)`
     * @return a vector containing the transformed values
     */
    template<typename T, typename A, typename L,
        typename std::enable_if_t<
            std::is_invocable_v<L, T&, std::size_t>
        >* = nullptr>
    auto vec_transform(std::vector<T, A>& v, L&& transform) {
        using ResultType = decltype(transform(std::declval<T&>(), std::declval<std::size_t>()));

        std::vector<ResultType> result;
        result.reserve(v.size());
        
        for (std::size_t i = 0u; i < v.size(); ++i) {
            result.push_back(transform(v[i], i));
        }

        return result;
    }

    /**
     * Applies the given lambda to each element of the given vector and returns a vector containing the resulting
     * values, in order in which their original elements appeared in v.
     *
     * The elements are passed to the given lambda as rvalue references.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam L the type of the lambda to apply, must be of type auto(T&&)
     * @param v the vector
     * @param transform the lambda to apply
     * @return a vector containing the transformed values
     */
    template<typename T, typename A, typename L,
        typename std::enable_if_t<
            std::is_invocable_v<L, T&&>
        >* = nullptr>
    auto vec_transform(std::vector<T, A>&& v, L&& transform) {
        using ResultType = decltype(transform(std::declval<T&&>()));

        std::vector<ResultType> result;
        result.reserve(v.size());
        for (auto&& x : v) {
            result.push_back(transform(std::move(x)));
        }

        return result;
    }

    /**
     * Applies the given lambda to each element of the given vector and returns a vector containing the resulting
     * values, in order in which their original elements appeared in v.
     *
     * The elements are passed to the given lambda as rvalue references.
     *
     * This version passes the vector element indices to the filter function.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam L the type of the lambda to apply
     * @param v the vector
     * @param transform the lambda to apply, must be of type auto(T&&, std::size_t)
     * @return a vector containing the transformed values
     */
    template<typename T, typename A, typename L,
        typename std::enable_if_t<
            std::is_invocable_v<L, T&&, std::size_t>
        >* = nullptr>
    auto vec_transform(std::vector<T, A>&& v, L&& transform) {
        using ResultType = decltype(transform(std::declval<T&&>(), std::declval<std::size_t>()));

        std::vector<ResultType> result;
        result.reserve(v.size());
        
        for (std::size_t i = 0u; i < v.size(); ++i) {
            result.push_back(transform(std::move(v[i]), i));
        }

        return result;
    }

    /**
     * Returns a vector containing those values from s1 which are not also in s2. Values from s1 and s2 are compared
     * using the common comparator from both sets.
     *
     * Expects that both S1 and S2 declare the types of their values with ::value_type and the comparator used to
     * compare the values with ::value_compare. Additionally, the type of value_compare must be identical in both sets.
     *
     * The value type of the returned vector is the common type of S1 and S2's member types. The values from s1 which
     * are not also in s2 are added to the returned vector in the order in which they appear in s2.
     *
     * @tparam S1 the type of the first set
     * @tparam S2 the type of the second set
     * @param s1 the first set
     * @param s2 the second set
     * @return a vector containing the set difference of s1 and s2.
     */
    template<typename S1, typename S2>
    auto set_difference(const S1& s1, const S2& s2) {
        using T1 = typename S1::value_type;
        using T2 = typename S2::value_type;
        using C1 = typename S1::value_compare;
        using C2 = typename S2::value_compare;
        static_assert(std::is_same<C1, C2> ::value, "incompatible comparators");

        using T = std::common_type_t<T1, T2>;
        using C = C1;

        std::vector<T> result;
        result.reserve(s1.size());
        std::set_difference(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2), std::back_inserter(result),
            C());
        return result;
    }

    /**
     * Returns a vector containing all values from s1 and s2 without duplicates. A pair of values from s1 and s2 is a
     * duplicate if the values are equivalent according to the common comparator of s1 and s2.
     *
     * Expects that both S1 and S2 declare the types of their values with ::value_type and the comparator used to
     * compare the values with ::value_compare. Additionally, the type of value_compare must be identical in both sets.
     *
     * The value type of the returned vector is the common type of S1 and S2's member types. The order of the values in
     * the returned vector complies with the common comparator of s1 and s2.
     *
     * @tparam S1 the type of the first set
     * @tparam S2 the type of the second set
     * @param s1 the first set
     * @param s2 the second set
     * @return a vector containing the set union of s1 and s2.
     */
    template<typename S1, typename S2>
    auto set_union(const S1& s1, const S2& s2) {
        using T1 = typename S1::value_type;
        using T2 = typename S2::value_type;
        using C1 = typename S1::value_compare;
        using C2 = typename S2::value_compare;
        static_assert(std::is_same<C1, C2> ::value, "incompatible comparators");

        using T = typename std::common_type<T1, T2>::type;
        using C = C1;

        std::vector<T> result;
        result.reserve(s1.size() + s2.size());
        std::set_union(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2), std::back_inserter(result), C());
        return result;
    }


    /**
     * Returns a vector containing all values from s1 and s2 which are present in both sets.
     *
     * Expects that both S1 and S2 declare the types of their values with ::value_type and the comparator used to
     * compare the values with ::value_compare. Additionally, the type of value_compare must be identical in both sets.
     *
     * The value type of the returned vector is the common type of S1 and S2's member types. The order of the values in
     * the returned vector complies with the common comparator of s1 and s2.
     *
     * @tparam S1 the type of the first set
     * @tparam S2 the type of the second set
     * @param s1 the first set
     * @param s2 the second set
     * @return a vector containing the set union of s1 and s2.
     */
    template<typename S1, typename S2>
    auto set_intersection(const S1& s1, const S2& s2) {
        using T1 = typename S1::value_type;
        using T2 = typename S2::value_type;
        using C1 = typename S1::value_compare;
        using C2 = typename S2::value_compare;
        static_assert(std::is_same<C1, C2> ::value, "incompatible comparators");

        using T = typename std::common_type<T1, T2>::type;
        using C = C1;

        std::vector<T> result;
        result.reserve(s1.size() + s2.size());
        std::set_intersection(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2),
            std::back_inserter(result), C());
        return result;
    }

    /**
     * Clears the given vector and ensures that it has a capacity of 0 afterwards.
     *
     * @tparam T the type of the vector elements
     * @param v the vector
     */
    template<typename T>
    void vec_clear_to_zero(std::vector<T>& v) {
        v.clear();
        v.shrink_to_fit();
    }

    /**
     * Applies the given deleter to every element of the given vector, and clears the vector afterwards.
     *
     * @tparam T the type of the vector elements
     * @param v the vector
     */
    template<typename T, typename D = deleter<T*>>
    void vec_clear_and_delete(std::vector<T*>& v, const D& deleter = D()) {
        kdl::col_delete_all(v, deleter);
        v.clear();
    }
}

#endif //KDL_VECTOR_UTILS_H
