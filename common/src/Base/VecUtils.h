/*
 Copyright (C) 2010-2019 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_VECUTILS_H
#define TRENCHBROOM_VECUTILS_H

#include "ColUtils.h"

#include <cassert>
#include <algorithm>
#include <iterator>
#include <vector>

namespace VecUtils {
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
    template <typename O, typename T, typename A>
    std::vector<O> cast(const std::vector<T,A>& v) {
        std::vector<O> result;
        result.reserve(v.size());
        for (const auto& e : v) {
            result.push_back(O(e));
        }
        return result;
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
    template <typename T, typename A, typename X>
    typename std::vector<T,A>::size_type indexOf(const std::vector<T,A>& v, const X& x) {
        using IndexType = typename std::vector<T,A>::size_type;
        for (IndexType i = 0; i < v.size(); ++i) {
            if (v[i] == x) {
                return i;
            }
        }
        return v.size();
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
    template <typename T, typename A, typename X>
    bool contains(const std::vector<T,A>& v, const X& x) {
        return indexOf(v, x) < v.size();
    }

    namespace detail {
        template <typename T, typename A, typename... Args>
        void append(std::vector<T,A>& v1, const Args&... args) {
            (... , v1.insert(std::end(v1), std::begin(args), std::end(args)));
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
    template <typename T, typename A, typename... Args>
    void append(std::vector<T,A>& v, const Args&... args) {
        v.reserve(ColUtils::size(v, args...));
        (... , v.insert(std::end(v), std::begin(args), std::end(args)));
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
    template <typename T, typename A, typename... Args>
    std::vector<T,A> concat(const std::vector<T,A>& v, const Args&... args) {
        std::vector<T,A> result;
        append(result, v, args...);
        return result;
    }

    /**
     * Erases every element from the given vector which is equal to the given value.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam X the value type
     * @param v the vector
     * @param x the value to erase
     */
    template <typename T, typename A, typename X>
    void erase(std::vector<T,A>& v, const X& x) {
        v.erase(std::remove(std::begin(v), std::end(v), x), std::end(v));
    }

    /**
     * Erases every element from the given vector for which the given predicate evaluates to true.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @tparam X the predicate type
     * @param v the vector
     * @param x the predicate
     */
    template <typename T, typename A, typename P>
    void eraseIf(std::vector<T,A>& v, const P& predicate) {
        v.erase(std::remove_if(std::begin(v), std::end(v), predicate), std::end(v));
    }

    /**
     * Erases the element at the given index from the given vector. The element is swapped with the last element of the
     * vector, and then the last element is erased.
     *
     * @tparam T the type of the vector elements
     * @tparam A the vector's allocator type
     * @param v the vector
     * @param i the index of the element to erase, which must be less than the given vector's size
     */
    template <typename T, typename A>
    void eraseAt(std::vector<T,A>& v, const typename std::vector<T,A>::size_type i) {
        assert(i < v.size());
        auto it = std::next(std::begin(v), i);
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
    template <typename T, typename A, typename C>
    void eraseAll(std::vector<T,A>& v, const C& c) {
        for (const auto& x : c) {
            erase(v, x);
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
    template <typename T, typename A, typename Compare = std::less<T>>
    void sort(std::vector<T,A>& v, const Compare& cmp = Compare()) {
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
    template <typename T, typename A, typename Compare = std::less<T>>
    void sortAndMakeUnique(std::vector<T,A>& v, const Compare& cmp = Compare()) {
        std::sort(std::begin(v), std::end(v), cmp);
        v.erase(std::unique(std::begin(v), std::end(v), ColUtils::Equivalence<T,Compare>(cmp)), std::end(v));
    }
}

#endif //TRENCHBROOM_VECUTILS_H
