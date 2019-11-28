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

#ifndef TRENCHBROOM_COLUTILS_H
#define TRENCHBROOM_COLUTILS_H

#include <algorithm> // for std::remove
#include <functional> // for std::less

namespace ColUtils {
    template <typename T>
    struct Deleter {
        void operator()(T ptr) const {
            delete ptr;
        }
    };

    /**
     * Provides a notion of equivalence using a comparator. Two values are equivalent if they are mutually incomparable
     * by means of the comparator.
     *
     * @tparam T the value type
     * @tparam Compare the type of the comparator, defaults to std::less<T>
     */
    template <typename T, typename Compare = std::less<T>>
    struct Equivalence {
        Compare cmp;

        explicit Equivalence(const Compare& i_cmp = Compare()) :
        cmp(i_cmp) {}

        bool operator()(const T& lhs, const T& rhs) const {
            return !cmp(lhs, rhs) && !cmp(rhs, lhs);
        }
    };

    /**
     * Computes the sum of the sizes of the given containers.
     *
     * @tparam C the type of the first container
     * @tparam Args the type of the remaining containers
     * @param c the first container
     * @param args the remaining containers
     * @return the sum of the sizes of the given containers
     */
    template <typename C, typename... Args>
    auto size(const C& c, Args&&... args) {
        return c.size() + (args.size() + ...);
    }

    /**
     * Removes every element in range [first2, last2) from range [first1, last1). Removal is done by shifting the
     * removed elements to the end of the range [first1, last1) in such a way that the range [first1, result) contains
     * the retained elements and [result, last1) contains the removed elements. The relative order of the elements in
     * reach of the two resulting ranges is retained.
     *
     * @tparam I1 the type of the iterators of the range to remove from
     * @tparam I2 the type of the iterators of the range to remove
     * @param first1 the start of the range to remove from
     * @param last1 the end of the range to remove from (past-the-end iterator)
     * @param first2 the start of the range to remove
     * @param last2 the end of the range to remove (past-the-end iterator)
     * @return the position of the first removed element in the range [first1, last1) such that [first1, result)
     * contains the retained elements and [result, last1) contains the removed elements
     */
    template <typename I1, typename I2>
    I1 removeAll(I1 first1, I1 last1, I2 first2, I2 last2) {
        I1 result = last1;
        while (first2 != last2) {
            result = std::remove(first1, result, *first1++);
        }
        return result;
    }

    /**
     * Applies the given deleter to all elements in range [first, last).
     *
     * @tparam I the iterator type
     * @tparam D the deleter type, defaults to Deleter
     * @param first the start of the range of values to delete
     * @param last the end of the range of values to delete (past-the-end iterator)
     * @param deleter the deleter to apply
     */
    template <typename I, typename D = Deleter<typename I::value_type>>
    void deleteAll(I first, I last, const D& deleter = D()) {
        while (first != last) {
            deleter(*first++);
        }
    }

    /**
     * Applies the given deleter to all elements the given container.
     *
     * @tparam C the container type
     * @tparam D the deleter type, defaults to Deleter
     * @param c the container
     * @param deleter the deleter to apply
     */
    template <typename C, typename D = Deleter<typename C::value_type>>
    void deleteAll(C& c, const D& deleter = D()) {
        deleteAll(std::begin(c), std::end(c), deleter);
    }

    /**
     * Performs lexicographical comparison of the given ranges [first1, last1) and [first2, last2) using the given
     * comparator. Returns -1 if the first range is less than the second range, or +1 in the opposite case, or 0 if both
     * ranges are equivalent.
     *
     * @tparam I1 the iterator type of the first range
     * @tparam I2 the iterator type of the second range
     * @tparam Compare the comparator type, defaults to std::less<T>, where T both I1::value_type and I2::value_type
     * must be convertible to T
     * @param first1 the start iterator of the first range
     * @param last1 the end iterator of the first range (past-the-end iterator)
     * @param first2 the start iterator of the second range
     * @param last2 the end iterator of the second range (past-the-end iterator)
     * @param cmp the comparator to use
     * @return an int indicating the result of the comparison
     */
    template <typename I1, typename I2, typename Compare = std::less<typename std::common_type<typename I1::value_type, typename I2::value_type>::type>>
    int lexicographicalCompare(I1 first1, I1 last1, I2 first2, I2 last2, const Compare& cmp = Compare()) {
        while (first1 < last1 && first2 < last2) {
            if (cmp(*first1, *first2)) {
                return -1;
            } else if (cmp(*first2, *first1)) {
                return 1;
            } else {
                ++first1;
                ++first2;
            }
        }

        if (first1 != last1 && first2 == last2) {
            return -1;
        } else if (first1 == last1 && first2 != last2) {
            return 1;
        } else {
            return 0;
        }
    }

    /**
     * Performs lexicographical comparison of the given collections c1 and c2 using the given comparator. Returns -1 if
     * the first collection is less than the second collection, or +1 in the opposite case, or 0 if both collections are
     * equivalent.
     *
     * @tparam C1 the type of the first collection
     * @tparam C2 the type of the second collection
     * @tparam Compare the comparator type, defaults to std::less<T>, where T both C1::value_type and C2::value_type
     * must be convertible to T
     * @param c1 the first collection
     * @param c2 the second collection
     * @param cmp the comparator to use
     * @return an int indicating the result of the comparison
     */
    template <typename C1, typename C2, typename Compare = std::less<typename std::common_type<typename C1::value_type, typename C2::value_type>::type>>
    int lexicographicalCompare(const C1& c1, const C2& c2, const Compare& cmp = Compare()) {
        return lexicographicalCompare(std::begin(c1), std::end(c1), std::begin(c2), std::end(c2), cmp);
    }

    /**
     * Checks whether the given ranges [first1, last1) and [first2, last2) are equivalent according to using the given
     * comparator. Two ranges are considered equivalent according to a comparator if the ranges have the same number of
     * elements, and each pair of corresponding elements of the ranges is equivalent.
     *
     * @tparam I1 the iterator type of the first range
     * @tparam I2 the iterator type of the second range
     * @tparam Compare the comparator type, defaults to std::less<T>, where T both I1::value_type and I2::value_type
     * must be convertible to T
     * @param first1 the start iterator of the first range
     * @param last1 the end iterator of the first range (past-the-end iterator)
     * @param first2 the start iterator of the second range
     * @param last2 the end iterator of the second range (past-the-end iterator)
     * @param cmp the comparator to use
     * @return true if the given ranges are equivalent and false otherwise
     */
    template <typename I1, typename I2, typename Compare = std::less<typename std::common_type<typename I1::value_type, typename I2::value_type>::type>>
    bool equivalent(I1 first1, I1 last1, I2 first2, I2 last2, const Compare& cmp = Compare()) {
        return lexicographicalCompare(first1, last1, first2, last2, cmp) == 0;
    }

    /**
     * Checks whether the given collections are equivalent according to using the given comparator. Two collections are
     * considered equivalent according to a comparator if they have the same number of elements, and each pair of
     * corresponding elements of the collections is equivalent.
     *
     * @tparam C1 the type of the first collection
     * @tparam C2 the type of the second collection
     * @tparam Compare the comparator type, defaults to std::less<T>, where T both C1::value_type and C2::value_type
     * must be convertible to T
     * @param c1 the first collection
     * @param c2 the second collection
     * @param cmp the comparator to use
     * @return true if the given collections are equivalent and false otherwise
     */
    template <typename C1, typename C2, typename Compare = std::less<typename std::common_type<typename C1::value_type, typename C2::value_type>::type>>
    bool equivalent(const C1& c1, const C2& c2, const Compare& cmp = Compare()) {
        if (c1.size() != c2.size()) {
            return false;
        } else {
            return equivalent(c1, c2, cmp);
        }
    }
}

#endif //TRENCHBROOM_COLUTILS_H
