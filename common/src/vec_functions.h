/*
Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TRENCHBROOM_VEC_FUNCTIONS_H
#define TRENCHBROOM_VEC_FUNCTIONS_H

#include "vec_type.h"

#include <algorithm>
#include <vector>

/* ========== comparison operators ========== */

/**
 * Lexicographically compares the given components of the vectors using the given epsilon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @param epsilon the epsilon for component wise comparison
 * @return -1 if the left hand size is less than the right hand size, +1 if the left hand size is greater than the right hand size, and 0 if both sides are equal
 */
template <typename T, size_t S>
int compare(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
    for (size_t i = 0; i < S; ++i) {
        if (Math::lt(lhs[i], rhs[i], epsilon))
            return -1;
        if (Math::gt(lhs[i], rhs[i], epsilon))
            return 1;
    }
    return 0;
}

/**
 * Performs a pairwise lexicographical comparison of the pairs of vectors given by the two ranges. This function iterates over
 * both ranges in a parallel fashion, and compares the two current elements lexicagraphically until one range ends.
 *
 * @tparam I the range iterator type
 * @param lhsCur the beginning of the left hand range
 * @param lhsEnd the end of the left hand range
 * @param rhsCur the beginning of the right hand range
 * @param rhsEnd the end of the right hand range
 * @param epsilon the epsilon value for component wise comparison
 * @return -1 if the left hand range is less than the right hand range, +1 if the left hand range is greater than the right hand range, and 0 if both ranges are equal
 */
template <typename I>
int compare(I lhsCur, I lhsEnd, I rhsCur, I rhsEnd, const typename I::value_type::type epsilon = static_cast<typename I::value_type::type>(0.0)) {
    while (lhsCur != lhsEnd && rhsCur != rhsEnd) {
        const auto cmp = compare(*lhsCur, *rhsCur, epsilon);
        if (cmp < 0) {
            return -1;
        } else if (cmp > 0) {
            return 1;
        }
        ++lhsCur;
        ++rhsCur;
    }

    assert(lhsCur == lhsEnd && rhsCur == rhsEnd);
    return 0;
}

/**
 * Checks whether the given vectors are component wise equal up to the given epsilon.
 *
 * Unline the equality operator ==, this function takes an epsilon value into account.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param epsilon the epsilon value
 * @return true if the given vectors are component wise equal up to the given epsilon value
 */
template <typename T, size_t S>
bool equal(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon) {
    return compare(lhs, rhs, epsilon) == 0;
}

/**
 * Compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) == 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given vectors have equal values for each component, and false otherwise
 */
template <typename T, size_t S>
bool operator==(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return compare(lhs, rhs) == 0;
}

/**
 * Compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) != 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given vectors do not have equal values for each component, and false otherwise
 */
template <typename T, size_t S>
bool operator!=(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return compare(lhs, rhs) != 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) < 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is less than the given right hand vector
 */
template <typename T, size_t S>
bool operator<(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return compare(lhs, rhs) < 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) <= 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is less than or equal to the given right hand vector
 */
template <typename T, size_t S>
bool operator<=(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return compare(lhs, rhs) <= 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) > 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is greater than than the given right hand vector
 */
template <typename T, size_t S>
bool operator>(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return compare(lhs, rhs) > 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) >= 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is greater than or equal to than the given right hand vector
 */
template <typename T, size_t S>
bool operator>=(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return compare(lhs, rhs) >= 0;
}

/**
 * Checks whether the given vector has unit length (1).
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 1 and false otherwise
 */
template <typename T, size_t S>
bool isUnit(const vec<T,S>& v, const T epsilon = Math::Constants<T>::almostZero()) {
    return Math::one(length(v), epsilon);
}

/**
 * Checks whether the given vector has a length of 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 0 and false otherwise
 */
template <typename T, size_t S>
bool isZero(const vec<T,S>& v, const T epsilon = Math::Constants<T>::almostZero()) {
    for (size_t i = 0; i < S; ++i) {
        if (!Math::zero(v[i], epsilon)) {
            return false;
        }
    }
    return true;
}

/**
 * Checks whether the given vector NaN as any component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @return true if the given vector has NaN as any component
 */
template <typename T, size_t S>
bool isNaN(const vec<T,S>& v) {
    for (size_t i = 0; i < S; ++i) {
        if (Math::isnan(v[i])) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether each component of the given vector is within a distance of epsilon around an integral value.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if all components of the given vector are integral under the above definition
 */
template <typename T, size_t S>
bool isIntegral(const vec<T,S>& v, const T epsilon = static_cast<T>(0.0)) {
    for (size_t i = 0; i < S; ++i) {
        if (std::abs(v[i] - Math::round(v[i])) >= epsilon) {
            return false;
        }
    }
    return true;
}

/**
 * Checks whether the given three points are colinear.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param a the first point
 * @param b the second point
 * @param c the third point
 * @param epsilon the epsilon value
 * @return true if the given three points are colinear, and false otherwise
 */
template <typename T, size_t S>
bool colinear(const vec<T,S>& a, const vec<T,S>& b, const vec<T,S>& c, const T epsilon = Math::Constants<T>::colinearEpsilon()) {
    // see http://math.stackexchange.com/a/1778739

    T j = 0.0;
    T k = 0.0;
    T l = 0.0;
    for (size_t i = 0; i < S; ++i) {
        const T ac = a[i] - c[i];
        const T ba = b[i] - a[i];
        j += ac * ba;
        k += ac * ac;
        l += ba * ba;
    }

    return Math::zero(j * j - k * l, epsilon);
}

/**
 * Checks whether the given vectors are parallel. Two vectors are considered to be parallel if and only if they point
 * in the same or in opposite directions.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param epsilon the epsilon value
 * @return true if the given vectors are parallel, and false otherwise
 */
template <typename T, size_t S>
bool parallel(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon = Math::Constants<T>::colinearEpsilon()) {
    const T cos = dot(normalize(lhs), normalize(rhs));
    return Math::one(Math::abs(cos), epsilon);
}

/* ========== accessing major component / axis ========== */

/**
 * Comparator for a selection heap which compares the values of vector components.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, size_t S>
class selection_heap_cmp {
private:
    const vec<T,S>& m_v;
    bool m_abs;
public:
    selection_heap_cmp(const vec<T,S>& i_v, const bool abs) :
            m_v(i_v),
            m_abs(abs) {}

    bool operator()(size_t lhs, size_t rhs) const {
        assert(lhs < S);
        assert(rhs < S);
        if (m_abs)
            return std::abs(m_v[lhs]) < std::abs(m_v[rhs]);
        return m_v[lhs] < m_v[rhs];
    }
};

/**
 * Returns the index of the component with the k-highest absolute value. The k-highest component is the
 * index of the component that receives index k if the components are sorted descendent by their absolute
 * value.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @param k the value of k
 * @return the index of the k-highest component
 */
template <typename T, size_t S>
size_t majorComponent(const vec<T,S>& v, const size_t k) {
    assert(k < S);

    if (k == 0) {
        size_t index = 0;
        for (size_t i = 1; i < S; ++i) {
            if (std::abs(v[i]) > std::abs(v[index]))
                index = i;
        }
        return index;
    }

    // simple selection algorithm
    // we store the indices of the values in heap
    selection_heap_cmp<T,S> cmp(v, true);
    std::vector<size_t> heap;
    for (size_t i = 0; i < S; ++i) {
        heap.push_back(i);
        std::push_heap(std::begin(heap), std::end(heap), cmp);
    }

    std::sort_heap(std::begin(heap), std::end(heap), cmp);
    return heap[S - k - 1];
}

/**
 * Returns a vector indicating the axis of the k-largest component. The returning vector has all values
 * set to 0 except for the component that holds the k-largest value. The sign of the returned vector
 * depends on the sign of the value of the k-largest component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @param k the k value
 * @return the vector indicating the axis of the k-largest component.
 */
template <typename T, size_t S>
vec<T,S> majorAxis(const vec<T,S>& v, const size_t k) {
    const auto c = majorComponent(v, k);
    auto a = vec<T,S>::axis(c);
    if (v[c] < static_cast<T>(0.0)) {
        return -a;
    }
    return a;
}

/**
 * Returns a vector indicating the axis of the k-largest component. The returning vector has all values
 * set to 0 except for the compnent that holds the h-largest value. The sign of the returned vector is
 * always positive.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @param k the k value
 * @return the vector indicating the absolute axis of the k-largest component
 */
template <typename T, size_t S>
vec<T,S> absMajorAxis(const vec<T,S>& v, const size_t k) {
    const auto c = majorComponent(v, k);
    return vec<T,S>::axis(c);
}

/**
 * Returns the index of the largest component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @return the index of the largest component
 */
template <typename T, size_t S>
size_t firstComponent(const vec<T,S>& v) {
    return majorComponent(v, 0);
}

/**
 * Returns the index of the second largest component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @return the index of the second largest component
 */
template <typename T, size_t S>
size_t secondComponent(const vec<T,S>& v) {
    return majorComponent(v, 1);
}

/**
 * Returns the index of the third largest component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @return the index of the third largest component
 */
template <typename T, size_t S>
size_t thirdComponent(const vec<T,S>& v) {
    return majorComponent(v, 2);
}

/**
 * Returns the axis of the largest component.
 *
 * @tparam T the component type
 * @param v the vector
 * @return the axis of the largest component
 */
template <typename T>
vec<T,3> firstAxis(const vec<T,3>& v) {
    return majorAxis(v, 0);
}

/**
 * Returns the axis of the second largest component.
 *
 * @tparam T the component type
 * @param v the vector
 * @return the axis of the second largest component
 */
template <typename T>
vec<T,3> secondAxis(const vec<T,3>& v) {
    return majorAxis(v, 1);
}

/**
 * Returns the axis of the third largest component.
 *
 * @tparam T the component type
 * @param v the vector
 * @return the axis of the third largest component
 */
template <typename T>
vec<T,3> thirdAxis(const vec<T,3>& v) {
    return majorAxis(v, 2);
}

/* ========== arithmetic operators ========== */

/**
 * Returns an inverted copy of this vector. The copy is inverted by negating every component.
 *
 * @return the inverted copy
 */
template <typename T, size_t S>
vec<T,S> operator-(const vec<T,S>& vector) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = -vector[i];
    }
    return result;
}

/**
 * Returns the sum of the given vectors, which is computed by adding all of their components.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the sum of the given two vectors
 */
template <typename T, size_t S>
vec<T,S> operator+(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result(lhs);
    return result += rhs;
}

/**
 * Adds the given right hand side to the given left hand side and returns a reference to the left hand side.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return a reference to the left hand vector after adding the right hand vector to it
 */
template <typename T, size_t S>
vec<T,S>& operator+=(vec<T,S>& lhs, const vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] += rhs[i];
    }
    return lhs;
}

/**
 * Returns the difference of the given vectors, which is computed by subtracting the corresponding components
 * of the right hand vector from the components of the left hand vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the difference of the given two vectors
 */
template <typename T, size_t S>
vec<T,S> operator-(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result(lhs);
    return result -= rhs;
}

/**
 * Subtracts the given right hand side from the given left hand side and returns a reference to the left hand side.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return a reference to the left hand vector after subtracting the right hand vector from it
 */
template <typename T, size_t S>
vec<T,S>& operator-=(vec<T,S>& lhs, const vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] -= rhs[i];
    }
    return lhs;
}

/**
 * Returns the product of the given vectors, which is computed by multiplying the corresponding components
 * of the right hand vector with the components of the left hand vector. Note that this does not compute
 * either the inner (or dot) product or the outer (or cross) product.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the product of the given two vectors
 */
template <typename T, size_t S>
vec<T,S> operator*(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result(lhs);
    return result *= rhs;
}

/**
 * Returns the product of the given vector and scalar factor, which is computed by multiplying each component of the
 * vector with the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar
 * @return the scalar product of the given vector with the given factor
 */
template <typename T, size_t S>
vec<T,S> operator*(const vec<T,S>& lhs, const T rhs) {
    vec<T,S> result(lhs);
    return result *= rhs;
}


/**
 * Returns the product of the given vector and scalar factor, which is computed by multiplying each component of the
 * vector with the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar
 * @param rhs the vector
 * @return the scalar product of the given vector with the given factor
 */
template <typename T, size_t S>
vec<T,S> operator*(const T lhs, const vec<T,S>& rhs) {
    return vec<T,S>(rhs) * lhs;
}

/**
 * Multiplies each component of the given left hand vector with the corresponding component of the given right hand
 * vector, stores the result in the left hand vector, and returns a reference to the left hand vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return a reference to the left hand vector after multiplying it with the right hand vector in a component wise fashion
 */
template <typename T, size_t S>
vec<T,S>& operator*=(vec<T,S>& lhs, const vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] *= rhs[i];
    }
    return lhs;
}

/**
 * Computes the scalar product of the vector with the scalar factor, stores the result in the vector, and returns a
 * reference to the vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the hand vector
 * @param rhs the scalar factor
 * @return a reference to the left hand vector after multiplying each of its components with the given scalar
 */
template <typename T, size_t S>
vec<T,S>& operator*=(vec<T,S>& lhs, const T rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] *= rhs;
    }
    return lhs;
}

/**
 * Returns the division of the given vectors, which is computed by dividing the corresponding components
 * of the left hand vector by the components of the right hand vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the division of the given two vectors
 */
template <typename T, size_t S>
vec<T,S> operator/(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result(lhs);
    return result /= rhs;
}

/**
 * Returns the division of the given vector and scalar factor, which is computed by dividing each component of the
 * vector by the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar
 * @return the scalar division of the given vector with the given factor
 */
template <typename T, size_t S>
vec<T,S> operator/(const vec<T,S>& lhs, const T rhs) {
    vec<T,S> result(lhs);
    return result /= rhs;
}

/**
 * Computes the component wise division of the left hand vector by the right hand vector,
 * stores the result in the vector, and returns a reference to the vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar factor
 * @return a reference to the left hand vector after dividing each of its components by corresponding component of the right hand vector
 */
template <typename T, size_t S>
vec<T,S>& operator/=(vec<T,S>& lhs, const vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] /= rhs[i];
    }
    return lhs;
}

/**
 * Computes the scalar division of the vector by the scalar factor, stores the result in the vector, and returns a
 * reference to the vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar factor
 * @return a reference to the left hand vector after dividing each of its components by the given scalar
 */
template <typename T, size_t S>
vec<T,S>& operator/=(vec<T,S>& lhs, const T rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] /= rhs;
    }
    return lhs;
}

/**
 * Adds the given vector to each of the vectors in the given range.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the range of vectors
 * @param rhs the right hand vector
 * @return a range containing the sum of each of the vectors in the given range with the right hand vector
 */
template <typename T, size_t S>
typename vec<T,S>::List operator+(const typename vec<T,S>::List& lhs, const vec<T,S>& rhs) {
    typename vec<T,S>::List result;
    result.reserve(lhs.size());
    for (const auto& vec : lhs) {
        result.push_back(vec + rhs);
    }
    return result;
}

/**
 * Adds the given vector to each of the vectors in the given range.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the range of vectors
 * @return a range containing the sum of each of the vectors in the given range with the left hand vector
 */
template <typename T, size_t S>
typename vec<T,S>::List operator+(const vec<T,S>& lhs, const typename vec<T,S>::List& rhs) {
    return rhs + lhs;
}

/**
 * Multiplies each vector in the given range by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the range of vectors
 * @param rhs the scalar factor
 * @return a range containing the scalar product of each vector in the given range with the given scalar
 */
template <typename T, size_t S>
typename vec<T,S>::List operator*(const typename vec<T,S>::List& lhs, const T rhs) {
    typename vec<T,S>::List result;
    result.reserve(lhs.size());
    for (const auto& vec : lhs) {
        result.push_back(vec + rhs);
    }
    return result;
}

/**
 * Multiplies each vector in the given range by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar factor
 * @param rhs the range of vectors
 * @return a range containing the scalar product of each vector in the given range with the given scalar
 */
template <typename T, size_t S>
typename vec<T,S>::List operator*(const T lhs, const typename vec<T,S>::List& rhs) {
    return rhs * lhs;
}

/* ========== stream operators ========== */

template <typename T, size_t S>
std::ostream& operator<< (std::ostream& stream, const vec<T,S>& vec) {
    if (S > 0) {
        stream << vec[0];
        for (size_t i = 1; i < S; ++i) {
            stream << " " << vec[i];
        }
    }
    return stream;
}

/* ========== arithmetic functions ========== */

/**
 * Returns a vector where each component is the absolute value of the corresponding component of the the
 * given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to make absolute
 * @return the absolute vector
 */
template <typename T, size_t S>
vec<T,S> abs(const vec<T,S>& v) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::abs(v[i]);
    }
    return result;
}

/**
 * Returns a vector where each component is the minimum of the corresponding components of the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @return the component wise minimum of the given vectors
 */
template <typename T, size_t S>
vec<T,S> min(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::min(lhs[i], rhs[i]);
    }
    return result;
}

/**
 * Returns a vector where each component is the maximum of the corresponding components of the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @return the component wise maximum of the given vectors
 */
template <typename T, size_t S>
vec<T,S> max(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::max(lhs[i], rhs[i]);
    }
    return result;
}

/**
 * Returns a vector where each component is the absolute minimum of the corresponding components of the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @return the component wise absolute minimum of the given vectors
 */
template <typename T, size_t S>
vec<T,S> absMin(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::absMin(lhs[i], rhs[i]);
    }
    return result;
}

/**
 * Returns a vector where each component is the absolute maximum of the corresponding components of the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @return the component wise absolute maximum of the given vectors
 */
template <typename T, size_t S>
vec<T,S> absMax(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::absMax(lhs[i], rhs[i]);
    }
    return result;
}

/**
 * Returns the dot product (also called inner product) of the two given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the dot product of the given vectors
 */
template <typename T, size_t S>
T dot(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    T result = static_cast<T>(0.0);
    for (size_t i = 0; i < S; ++i) {
        result += (lhs[i] * rhs[i]);
    }
    return result;
}

/**
 * Returns the cross product (also called outer product) of the two given 3d vectors.
 *
 * @tparam T the component type
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the cross product of the given vectors
 */
template <typename T>
vec<T,3> cross(const vec<T, 3>& lhs, const vec<T, 3>& rhs) {
    return vec<T,3>(lhs[1] * rhs[2] - lhs[2] * rhs[1],
                    lhs[2] * rhs[0] - lhs[0] * rhs[2],
                    lhs[0] * rhs[1] - lhs[1] * rhs[0]);
}

/**
 * Mixes the given two vectors using the given factors. For each component i of the given vectors, the corresponding
 * component of the result is computed as
 *
 *   (1 - f[i]) * lhs[i] + f * rhs[i]
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param f the mixing factors
 * @return the mixed vector
 */
template <typename T, size_t S>
vec<T,S> mix(const vec<T,S>& lhs, const vec<T,S>& rhs, const vec<T,S>& f) {
    return (vec<T,S>::one - f) * lhs + f * rhs;
}

/**
 * Computes the distance between two given points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first point
 * @param rhs the second point
 * @return the distance between the given points
 */
template <typename T, size_t S>
T distance(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return length(lhs - rhs);
}

/**
 * Computes the squared distance between two given points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first point
 * @param rhs the second point
 * @return the squared distance between the given points
 */
template <typename T, size_t S>
T squaredDistance(const vec<T,S>& lhs, const vec<T,S>& rhs) {
    return squaredLength(lhs - rhs);
}

/**
 * Normalizes the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the squared length of
 * @return the normalized vector
 */
template <typename T, size_t S>
vec<T,S> normalize(const vec<T,S>& vec) {
    return vec / length(vec);
}

/**
 * Returns the length of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the length of
 * @return the length of the given vector
 */
template <typename T, size_t S>
T length(const vec<T,S>& vec) {
    return std::sqrt(squaredLength(vec));
}

/**
 * Returns the squared length of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to normalize
 * @return the squared length of the given vector
 */
template <typename T, size_t S>
T squaredLength(const vec<T,S>& vec) {
    return dot(vec, vec);
}

/**
 * Converts the given point in cartesian coordinates to homogeneous coordinates by embedding the point into
 * a vector with a size increased by 1 and setting the last component to 1.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point in cartesian coordinates
 * @return the point in homogeneous coordinates
 */
template <typename T, size_t S>
vec<T,S+1> toHomogeneousCoords(const vec<T,S>& point) {
    return vec<T,S+1>(point, static_cast<T>(1.0));
}

/**
 * Converts the given point in homogeneous coordinates to cartesian coordinates by dividing all but the last
 * component by the value of the last component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point in homogeneous coordinates
 * @return the point in cartesian coordinates
 */
template <typename T, size_t S>
vec<T,S-1> toCartesianCoords(const vec<T,S>& point) {
    vec<T,S-1> result;
    for (size_t i = 0; i < S-1; ++i) {
        result[i] = point[i] / point[S-1];
    }
    return result;
}

/* ========== rounding and error correction ========== */

/**
 * Returns a vector where each component is the rounded value of the corresponding component of the given
 * vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round
 * @return the rounded vector
 */
template <typename T, size_t S>
vec<T,S> round(const vec<T,S>& v) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::round(v[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector down to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round down
 * @param m the multiples to round down to
 * @return the rounded vector
 */
template <typename T, size_t S>
vec<T,S> roundDownToMultiple(const vec<T,S>& v, const vec<T,S>& m) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundDownToMultiple(v[i], m[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector up to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round down
 * @param m the multiples to round up to
 * @return the rounded vector
 */
template <typename T, size_t S>
vec<T,S> roundUpToMultiple(const vec<T,S>& v, const vec<T,S>& m) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundUpToMultiple(v[i], m[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round down
 * @param m the multiples to round to
 * @return the rounded vector
 */
template <typename T, size_t S>
vec<T,S> roundToMultiple(const vec<T,S>& v, const vec<T,S>& m) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundToMultiple(v[i], m[i]);
    }
    return result;
}

/**
 * Corrects the given vector's components to the given number of decimal places.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to correct
 * @param decimals the number of decimal places to keep
 * @param epsilon the epsilon value
 * @return the corrected vector
 */
template <typename T, size_t S>
vec<T,S> correct(const vec<T,S>& v, const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon()) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::correct(v[i], decimals, epsilon);
    }
    return result;
}

/**
 * Given three colinear points, this function checks whether the first point is contained in a segment formed by the
 * other two points.
 *
 * The result is undefined for the case of non-colinear points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param p the point to check
 * @param start the segment start
 * @param end the segment end
 * @return true if the given point is contained within the segment
 */
template <typename T, size_t S>
bool between(const vec<T,S>& p, const vec<T,S>& start, const vec<T,S>& end) {
    assert(colinear(p, start, end));
    const vec<T,S> toStart = start - p;
    const vec<T,S> toEnd   =   end - p;

    const T d = dot(toEnd, normalize(toStart));
    return !Math::pos(d);
}

/**
 * Computes the average of the given range of elements, using the given function to transform an element into a vector.
 *
 * @tparam I the type of the range iterators
 * @tparam G the type of the transformation function from a range element to a vector type
 * @param cur the start of the range
 * @param end the end of the range
 * @param get the transformation function, defaults to identity
 * @return the average of the vectors obtained from the given range of elements
 */
template <typename I, typename G>
auto average(I cur, I end, const G& get = Math::Identity()) -> typename std::remove_reference<decltype(get(*cur))>::type {
    assert(cur != end);

    auto result = get(*cur++);
    auto count = 1.0;
    while (cur != end) {
        result += get(*cur++);
        count += 1.0;
    }
    return result / count;
}

/**
 * Computes the CCW angle between axis and vector in relation to the given up vector. All vectors are expected to be
 * normalized. The CCW angle is the angle by which the given axis must be rotated in CCW direction about the given up
 * vector so that it becomes identical to the given vector.
 *
 * @tparam T the coordinate type
 * @param v the vector
 * @param axis the axis
 * @param up the up vector
 * @return the CCW angle
 */
template <typename T>
T angleBetween(const vec<T,3>& v, const vec<T,3>& axis, const vec<T,3>& up) {
    const auto cos = dot(v, axis);
    if (Math::one(+cos)) {
        return static_cast<T>(0.0);
    } else if (Math::one(-cos)) {
        return Math::Constants<T>::pi();
    } else {
        const auto perp = cross(axis, v);
        if (!Math::neg(dot(perp, up))) {
            return std::acos(cos);
        } else {
            return Math::Constants<T>::twoPi() - std::acos(cos);
        }
    }
}

/**
 * Return type for the distanceToSegment function. Contains the point on a segment which is closest to some given
 * point, and the distance between that segment point and the given point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, size_t S>
struct EdgeDistance {
    /**
     * The closest point on a given segment to a given point.
     */

    const vec<T,S> point;
    /**
     * The distance between the closest segment point and a given point.
     */
    const T distance;

    /**
     * Constructs a new instance with the given info.
     *
     * @param i_point the closest point on the segment
     * @param i_distance the distance between the closest point and the given point
     */
    EdgeDistance(const vec<T,S>& i_point, const T i_distance) :
            point(i_point),
            distance(i_distance) {}
};

/**
 * Given a point X and a segment represented by two points A and B, this function computes the closest point P on the
 * segment AB and the given point X, as well as the distance between X and P.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point
 * @param start the start point of the segment
 * @param end the end point of the segment
 * @return a struct containing the closest point on the segment and the distance between that point and the given point
 */
template <typename T, size_t S>
EdgeDistance<T,S> distanceOfPointAndSegment(const vec<T,S>& point, const vec<T,S>& start, const vec<T,S>& end) {
    const vec<T,S> edgeVec = end - start;
    const vec<T,S> edgeDir = normalize(edgeVec);
    const T scale = dot(point - start, edgeDir);

    // determine the closest point on the edge
    vec<T,S> closestPoint;
    if (scale < 0.0) {
        closestPoint = start;
    } else if ((scale * scale) > squaredLength(edgeVec)) {
        closestPoint = end;
    } else {
        closestPoint = start + edgeDir * scale;
    }

    const T distance = length(point - closestPoint);
    return EdgeDistance<T,S>(closestPoint, distance);
}

#endif
