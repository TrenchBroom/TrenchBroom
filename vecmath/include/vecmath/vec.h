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

#ifndef TRENCHBROOM_VEC_H
#define TRENCHBROOM_VEC_H

#include "constants.h"
#include "scalar.h"

#include <cassert>
#include <cstddef>
#include <ostream>

namespace vm {
    template <typename T, size_t S>
    class vec {
    public:
        using float_type = vec<float, S>;
        using type = T;
        static const size_t size = S;

        static const vec<T,S> pos_x;
        static const vec<T,S> pos_y;
        static const vec<T,S> pos_z;
        static const vec<T,S> neg_x;
        static const vec<T,S> neg_y;
        static const vec<T,S> neg_z;
        static const vec<T,S> zero;
        static const vec<T,S> one;
        static const vec<T,S> NaN;
        static const vec<T,S> min;
        static const vec<T,S> max;
    protected:
        /**
         * The vector components.
         */
        T v[S];
    public:
        /**
         * Returns a vector with the component at the given index set to 1, and all others set to 0.
         *
         * @param index the index of the component to set to 1
         * @return the newly created vector
         */
        static vec<T,S> axis(size_t index) {
            vec<T,S> axis;
            axis[index] = static_cast<T>(1.0);
            return axis;
        }

        /**
         * Returns a vector where all components are set to the given value.
         *
         * @param value the value to set
         * @return the newly created vector
         */
        static vec<T,S> fill(T value) {
            vec<T,S> result;
            for (size_t i = 0; i < S; ++i) {
                result[i] = value;
            }
            return result;
        }

        /**
         * Parses the given string representation. The syntax of the given string is as follows
         *
         *   VEC ::= S * COMP;
         *     S ::= number of components
         *  COMP ::= WS, FLOAT;
         *    WS ::= " " | \\t | \\n | \\r | "(" | ")";
         * FLOAT ::= any floating point number parseable by std::atof
         *
         * Note that this method does not signal if the string could actually be parsed.
         *
         * @param str the string to parse
         * @return the vector parsed from the string
         */
        static vec<T,S> parse(const std::string& str) {
            size_t pos = 0;
            vec<T,S> result;
            doParse(str, pos, result);
            return result;
        }

        /**
         * Returns whether parse() can parse S components from the given string
         * 
         * @param str the string to parse
         * @return whether S components can be parsed
         */
        static bool canParse(const std::string& str) {
            size_t pos = 0;
            vec<T,S> result;
            return doParse(str, pos, result);
        }

        /**
         * Parses the given string for a list of vectors. The syntax of the given string is as follows:
         *
         * LIST ::= VEC, { SEP, VEC }
         *  SEP ::= " " | \\t | \\n |t \\r | "," | ";";
         *
         * Note that the list can be separated by whitespace or commas or semicolons, or a mix of these separators. Only
         * vectors which conform to the vector syntax are added to the given output iterator.
         *
         * @tparam O the type of the output iterator
         * @param str the string to parse
         * @param out the output iterator add the parsed vectors to
         */
        template <typename O>
        static void parseAll(const std::string& str, O out) {
            static const std::string blank(" \t\n\r,;");

            size_t pos = 0;
            while (pos != std::string::npos) {
                vec<T,S> temp;
                if (doParse(str, pos, temp)) {
                    out = temp;
                    ++out;
                }
                pos = str.find_first_of(blank, pos);
                pos = str.find_first_not_of(blank, pos);
            }
        }
    private:
        static bool doParse(const std::string& str, size_t& pos, vec<T,S>& result) {
            static const std::string blank(" \t\n\r()");

            const char* cstr = str.c_str();
            for (size_t i = 0; i < S; ++i) {
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) {
                    return false;
                }
                result[i] = static_cast<T>(std::atof(cstr + pos));
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) {
                    if (i < S-1) {
                        return false;
                    }
                }
            }
            return true;
        }
    public:
        /**
         * Creates a new vector with all components initialized to 0.
         */
        vec() {
            for (size_t i = 0; i < S; ++i) {
                v[i] = static_cast<T>(0.0);
            }
        }

        // Copy and move constructors
        vec(const vec<T,S>& other) = default;
        vec(vec<T,S>&& other) noexcept = default;

        // Assignment operators
        vec<T,S>& operator=(const vec<T,S>& other) = default;
        vec<T,S>& operator=(vec<T,S>&& other) noexcept = default;

        /**
         * Creates a new vector by copying the values from the given vector. If the given vector has a different component
         * type, the values are converted using static_cast. If the given vectors has a smaller size, then the remaining
         * elements of the newly created vector are filled with 0s. If the given vector has a greater size, then the
         * surplus components of the given vector are ignored.
         *
         * @tparam U the component type of the given vector
         * @tparam V the number of components of the given vector
         * @param other the vector to copy the values from
         */
        template <typename U, size_t V>
        explicit vec(const vec<U,V>& other) {
            for (size_t i = 0; i < std::min(S,V); ++i) {
                v[i] = static_cast<T>(other[i]);
            }
            for (size_t i = std::min(S,V); i < S; ++i) {
                v[i] = static_cast<T>(0.0);
            }
        }

        // Constructor with initializer list
        /**
         * Creates a new vector from the values in the given initializer list. If the given list has fewer elements than
         * the size of this vector, then the remaining components are set to 0. If the given list has more elements than
         * the size of this vector, then the surplus elements are ignored.
         *
         * @param values the values
         */
        vec(std::initializer_list<T> values) {
            auto it = std::begin(values);
            for (size_t i = 0; i < std::min(S, values.size()); ++i) {
                v[i] = *it++;
            }
            for (size_t i = values.size(); i < S; ++i) {
                v[i] = static_cast<T>(0.0);
            }
        }

        /**
         * Creates a new vector with the given component values. If the type of the given values differs from the
         * component type, then the values will be converted using static_cast. If fewer values are given than the number
         * of components of this vector, then the remaining components are set to 0. If more values are given than the
         * number of components of this vector, then the surplus values are ignored.
         *
         * @tparam U1 the type of the first given value
         * @tparam U2 the type of the second given value
         * @param i_x the value of the first component
         * @param i_y the value of the second component
         */
        template <typename U1, typename U2>
        vec(U1 i_x, U2 i_y) {
            if (S > 0) {
                v[0] = static_cast<T>(i_x);
                if (S > 1) {
                    v[1] = static_cast<T>(i_y);
                }
            }
            for (size_t i = 2; i < S; ++i) {
                v[i] = static_cast<T>(0.0);
            }
        }

        /**
         * Creates a new vector with the given component values. If the type of the given values differs from the
         * component type, then the values will be converted using static_cast. If fewer values are given than the number
         * of components of this vector, then the remaining components are set to 0. If more values are given than the
         * number of components of this vector, then the surplus values are ignored.
         *
         * @tparam U1 the type of the first given value
         * @tparam U2 the type of the second given value
         * @tparam U3 the type of the third given value
         * @param i_x the value of the first component
         * @param i_y the value of the second component
         * @param i_z the value of the third component
         */
        template <typename U1, typename U2, typename U3>
        vec(U1 i_x, U2 i_y, U3 i_z) {
            if (S > 0) {
                v[0] = static_cast<T>(i_x);
                if (S > 1) {
                    v[1] = static_cast<T>(i_y);
                    if (S > 2) {
                        v[2] = static_cast<T>(i_z);
                    }
                }
            }
            for (size_t i = 3; i < S; ++i) {
                v[i] = static_cast<T>(0.0);
            }
        }

        /**
         * Creates a new vector with the given component values. If the type of the given values differs from the
         * component type, then the values will be converted using static_cast. If fewer values are given than the number
         * of components of this vector, then the remaining components are set to 0. If more values are given than the
         * number of components of this vector, then the surplus values are ignored.
         *
         * @tparam U1 the type of the first given value
         * @tparam U2 the type of the second given value
         * @tparam U3 the type of the third given value
         * @tparam U4 the type of the fourth given value
         * @param i_x the value of the first component
         * @param i_y the value of the second component
         * @param i_z the value of the third component
         * @param i_w the value of the fourth component
         */
        template <typename U1, typename U2, typename U3, typename U4>
        vec(U1 i_x, U2 i_y, U3 i_z, U4 i_w) {
            if (S > 0) {
                v[0] = static_cast<T>(i_x);
                if (S > 1) {
                    v[1] = static_cast<T>(i_y);
                    if (S > 2) {
                        v[2] = static_cast<T>(i_z);
                        if (S > 3) {
                            v[3] = static_cast<T>(i_w);
                        }
                    }
                }
            }
            for (size_t i = 4; i < S; ++i) {
                v[i] = static_cast<T>(0.0);
            }
        }

        /**
         * Creates a vector with the values from the given vector, but sets the last component to the given scalar value.
         * Values which are not initialized by the given vector and value (due to the number of components of the given
         * vector being less than the number of components of the vector to create minus one) are set to 0. Surplus values
         * of the given vector are ignored.
         *
         * @tparam U the component type of the given vector and the type of the given scalar value
         * @tparam O the number of components of the given vector
         * @param i_v the vector to initialize components 0...S-2 with
         * @param last the value to initialize component S-1 with
         */
        template <typename U, size_t O>
        vec(const vec<U,O>& i_v, U last) {
            for (size_t i = 0; i < std::min(S-1,O); ++i) {
                v[i] = static_cast<T>(i_v[i]);
            }
            for (size_t i = std::min(S-1, O); i < S-1; ++i) {
                v[i] = static_cast<T>(0.0);
            }
            v[S-1] = static_cast<T>(last);
        }

        /**
         * Creates a vector with the values from the given vector, but sets the last two components to the given scalar values.
         * Values which are not initialized by the given vector and value (due to the number of components of the given
         * vector being less than the number of components of the vector to create minus two) are set to 0. Surplus values
         * of the given vector are ignored.
         *
         * @tparam U the component type of the given vector and the type of the given scalar value
         * @tparam O the number of components of the given vector
         * @param i_v the vector to initialize components 0...S-3 with
         * @param lastButOne the value to initialize component S-2 with
         * @param last the value to initialize component S-1 with
         */
        template <typename U, size_t O>
        vec(const vec<U,O>& i_v, U lastButOne, U last) {
            for (size_t i = 0; i < std::min(S-2,O); ++i) {
                v[i] = static_cast<T>(i_v[i]);
            }
            for (size_t i = std::min(S-2, O); i < S-2; ++i) {
                v[i] = static_cast<T>(0.0);
            }
            v[S-2] = static_cast<T>(lastButOne);
            v[S-1] = static_cast<T>(last);
        }

        /**
         * Returns a reference to the component at the given index. The index is not checked at runtime.
         *
         * @param index the index of the component
         * @return a reference to the compononent at the given index
         */
        T& operator[](size_t index) {
            assert(index < S);
            return v[index];
        }

        /**
         * Returns a const reference to the component at the given index. The index is not checked at runtime.
         *
         * @param index the index of the component
         * @return a const reference to the compononent at the given index
         */
        const T& operator[](size_t index) const {
            assert(index < S);
            return v[index];
        }

        /**
         * Returns the value of the first compnent.
         *
         * @return the value of the first component
         */
        T x() const {
            static_assert(S > 0);
            return v[0];
        }

        /**
         * Returns the value of the component compnent.
         *
         * @return the value of the component component
         */
        T y() const {
            static_assert(S > 1);
            return v[1];
        }

        /**
         * Returns the value of the third compnent.
         *
         * @return the value of the third component
         */
        T z() const {
            static_assert(S > 2);
            return v[2];
        }

        /**
         * Returns the value of the fourth compnent.
         *
         * @return the value of the fourth component
         */
        T w() const {
            static_assert(S > 3);
            return v[3];
        }

        /**
         * Returns a vector with the values of the first and second component.
         *
         * @return a vector with the values of the first and second component
         */
        vec<T,2> xy() const {
            static_assert(S > 1);
            return vec<T,2>(x(), y());
        }

        /**
         * Returns a vector with the values of the first and third component.
         *
         * @return a vector with the values of the first and third component
         */
        vec<T,2> xz() const {
            static_assert(S > 1);
            return vec<T,2>(x(), z());
        }

        /**
         * Returns a vector with the values of the second and third component.
         *
         * @return a vector with the values of the second and third component
         */
        vec<T,2> yz() const {
            static_assert(S > 1);
            return vec<T,2>(y(), z());
        }

        /**
         * Returns a vector with the values of the first three components.
         *
         * @return a vector with the values of the first three components
         */
        vec<T,3> xyz() const {
            static_assert(S > 2);
            return vec<T,3>(x(), y(), z());
        }

        /**
         * Returns a vector with the values of the first four components.
         *
         * @return a vector with the values of the first four components
         */
        vec<T,4> xyzw() const {
            static_assert(S > 3);
            return vec<T,4>(x(), y(), z(), w());
        }

        /**
        * Adds the given range of vertices to the given output iterator.
        *
        * @tparam I the range iterator type
        * @tparam O the output iterator type
        * @param cur the range start
        * @param end the range end
        * @param out the output iterator
        */
        template <typename I, typename O>
        static void getVertices(I cur, I end, O out) {
            while (cur != end) {
                out = *cur;
                ++out;
                ++cur;
            }
        }
    };

    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::pos_x = vec<T,S>::axis(0);
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::pos_y = vec<T,S>::axis(1);
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::pos_z = vec<T,S>::axis(2);
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::neg_x = -vec<T,S>::axis(0);
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::neg_y = -vec<T,S>::axis(1);
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::neg_z = -vec<T,S>::axis(2);
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::zero = vec<T,S>::fill(static_cast<T>(0.0));
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::one  = vec<T,S>::fill(static_cast<T>(1.0));
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::NaN  = vec<T,S>::fill(std::numeric_limits<T>::quiet_NaN());
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::min  = vec<T,S>::fill(std::numeric_limits<T>::min());
    template <typename T, size_t S>
    const vec<T,S> vec<T,S>::max  = vec<T,S>::fill(std::numeric_limits<T>::max());

    /* ========== comparison operators ========== */

    /**
     * Lexicographically compares the given components of the vectors using the given epsilon.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the left hand vector
     * @param rhs the right hand vector
     * @return -1 if the left hand size is less than the right hand size, +1 if the left hand size is greater than the right hand size, and 0 if both sides are equal
     */
    template <typename T, size_t S>
    int compare(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon = T(0.0)) {
        for (size_t i = 0; i < S; ++i) {
            // NaN handling: sort NaN's above non-NaN's, otherwise they would compare equal to any non-NaN value since
            // both the < and > tests below fail. Note that this function will compare NaN and NaN as equal.
            const bool lhsIsNaN = (lhs[i] != lhs[i]);
            const bool rhsIsNaN = (rhs[i] != rhs[i]);
            if (!lhsIsNaN && rhsIsNaN) {
                return -1;
            } else if (lhsIsNaN && !rhsIsNaN) {
                return 1;
            }

            if (lhs[i] < rhs[i] - epsilon) {
                return -1;
            } else if (lhs[i] > rhs[i] + epsilon) {
                return 1;
            }
        }
        return 0;
    }

    /**
     * Performs a pairwise lexicographical comparison of the pairs of vectors given by the two ranges. This function iterates over
     * both ranges in a parallel fashion, and compares the two current elements lexicagraphically until one range ends.
     * If the end of a range is reached, that range is considered less if the end of the other range has not yet been
     * reached. Otherwise, the two ranges of the same length, and are considered to be identical.
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
                return +1;
            }
            ++lhsCur;
            ++rhsCur;
        }

        if (rhsCur != rhsEnd) {
            return -1;
        } else if (lhsCur != lhsEnd) {
            return +1;
        } else {
           return 0;
        }
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
    bool isEqual(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon) {
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

    /* ========== accessing major component / axis ========== */

    /**
     * Helper struct to find the i'th largest component.
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
        size_t heap[S];
        for (size_t i = 0; i < S; ++i) {
            heap[i] = i;
            std::push_heap(&heap[0], &heap[i+1], cmp);
        }

        std::sort_heap(&heap[0], &heap[S], cmp);
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
     * Returns a copy of this vector.
     *
     * @return the copy
     */
    template <typename T, size_t S>
    vec<T,S> operator+(const vec<T,S>& vector) {
        return vector;
    }

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
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] + rhs[i];
        }
        return result;
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
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] - rhs[i];
        }
        return result;
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
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] * rhs[i];
        }
        return result;
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
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] * rhs;
        }
        return result;
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
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] / rhs[i];
        }
        return result;
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
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] / rhs;
        }
        return result;
    }

    /**
     * Returns the division of the given vector and scalar factor, which is computed by dividing the factor by each
     * component.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the scalar
     * @param rhs the vector
     * @return the scalar division of the given factor with the given vector
     */
    template <typename T, size_t S>
    vec<T,S> operator/(const T lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs / rhs[i];
        }
        return result;
    }

    /* ========== stream operators ========== */

    template <typename T, size_t S>
    std::ostream& operator<<(std::ostream& stream, const vec<T,S>& vec) {
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
            result[i] = min(lhs[i], rhs[i]);
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
            result[i] = max(lhs[i], rhs[i]);
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
            result[i] = absMin(lhs[i], rhs[i]);
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
            result[i] = absMax(lhs[i], rhs[i]);
        }
        return result;
    }

    /**
     * Returns a vector with each component clamped to the ranges defined in by the corresponding components of the
     * given minimum and maximum vectors.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the value to clamp
     * @param minVal the minimum values
     * @param maxVal the maximum values
     * @return the clamped vector
     */
    template <typename T, size_t S>
    vec<T,S> clamp(const vec<T,S>& v, const vec<T,S>& minVal, const vec<T,S>& maxVal) {
        return min(max(v, minVal), maxVal);
    }

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
            result[i] = abs(v[i]);
        }
        return result;
    }

    /**
     * Returns a vector where each component indicates the sign of the corresponding components of the given vector.
     *
     * For each component, the returned vector has a value of
     * - -1 if the corresponding component of the given vector is less than 0
     * - +1 if the corresponding component of the given vector is greater than 0
     * -  0 if the corresponding component of the given vector is equal to 0
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v a vector
     * @return a vector indicating the signs of the components of the given vector
     */
    template <typename T, size_t S>
    vec<T,S> sign(const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = sign(v[i]);
        }
        return result;
    }

    /**
     * Returns a vector where each component is set to 0 if the corresponding component of the given vector is less than
     * the corresponding component of the given edge vector, and 1 otherwise.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v a vector
     * @param e the edge vector
     * @return a vector indicating whether the given value is less than the given edge value or not
     */
    template <typename T, size_t S>
    vec<T,S> step(const vec<T,S>& e, const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = step(e[i], v[i]);
        }
        return result;
    }

    /**
     * Performs performs smooth Hermite interpolation for each component x of the given vector between 0 and 1 when
     * e0[i] < v[i] < e1[i].
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param e0 the lower edge values
     * @param e1 the upper edge values
     * @param v the vector to interpolate
     * @return the interpolated vector
     */
    template <typename T, size_t S>
    vec<T,S> smoothstep(const vec<T,S>& e0, const vec<T,S>& e1, const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = smoothstep(e0[i], e1[i], v[i]);
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
        auto result = static_cast<T>(0.0);
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
     * Rearranges the components of the given vector depending on the value of the axis parameter as follows:
     *
     * - 1: x y z -> y z x
     * - 2: x y z -> z x y
     * - 3: x y z -> x y z
     *
     * @tparam T the component type
     * @param point the point to swizzle
     * @param axis the axis
     * @return the swizzled point
     */
    template <typename T>
    vec<T,3> swizzle(const vec<T,3>& point, const size_t axis) {
        assert(axis <= 3);
        switch (axis) {
            case 0: // x y z -> y z x
                return vec<T,3>(point.y(), point.z(), point.x());
            case 1: // x y z -> z x y
                return vec<T,3>(point.z(), point.x(), point.y());
            default:
                return point;
        }
    }

    /**
     * Rearranges the components of the given vector depending on the value of the axis parameter so that it undoes
     * the effect of calling swizzle.
     *
     * @tparam T the component type
     * @param point the point to swizzle
     * @param axis the axis
     * @return the unswizzled point
     */
    template <typename T>
    vec<T,3> unswizzle(const vec<T,3>& point, const size_t axis) {
        assert(axis <= 3);
        switch (axis) {
            case 0:
                return vec<T,3>(point.z(), point.x(), point.y());
            case 1:
                return vec<T,3>(point.y(), point.z(), point.x());
            default:
                return point;
        }
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
    bool isUnit(const vec<T,S>& v, const T epsilon) {
        return isEqual(length(v), T(1.0), epsilon);
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
    bool isZero(const vec<T,S>& v, const T epsilon) {
        for (size_t i = 0; i < S; ++i) {
            if (!isZero(v[i], epsilon)) {
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
            if (isnan(v[i])) {
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
            if (std::abs(v[i] - round(v[i])) > epsilon) {
                return false;
            }
        }
        return true;
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
     * Returns a vector with each component set to the fractional part of the corresponding component of the given
     * vector.
     *
     * Note that this function differs from GLSL's fract, which behaves wrongly for negative values. Return 0.9 for
     * fract(-0.1). This function will correctly return -0.1 in this case.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the vector
     * @return the fractional vector
     */
    template <typename T, size_t S>
    vec<T,S> fract(const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = fract(v[i]);
        }
        return result;
    }

    /**
     * Returns a vector with each component set to the floating point remainder of the division of v over f. So for each
     * component i, it holds that result[i] = mod(v[i], f[i]).
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the dividend
     * @param f the divisor
     * @return the fractional remainder
     */
    template <typename T, size_t S>
    vec<T,S> mod(const vec<T,S>& v, const vec<T,S>& f) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = mod(v[i], f[i]);
        }
        return result;
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
    bool colinear(const vec<T,S>& a, const vec<T,S>& b, const vec<T,S>& c, const T epsilon = constants<T>::colinearEpsilon()) {
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

        return isZero(j * j - k * l, epsilon);
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
    bool parallel(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon = constants<T>::colinearEpsilon()) {
        const T cos = dot(normalize(lhs), normalize(rhs));
        return isEqual(abs(cos), T(1.0), epsilon);
    }

    /* ========== rounding and error correction ========== */

    /**
     * Returns a vector with each component set to the largest integer value not greater than the value of the
     * corresponding component of the given vector.
     *
     * @tparam T the component type, which must be a floating point type
     * @tparam S the number of components
     * @param v the value
     * @return a vector
     */
    template <typename T, size_t S>
    vec<T,S> floor(const vec<T,S>& v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = floor(v[i]);
        }
        return result;
    }

    /**
     * Returns a vector with each component set to the smallest integer value not less than the value of the
     * corresponding component of the given vector.
     *
     * @tparam T the component type, which must be a floating point type
     * @tparam S the number of components
     * @param v the value
     * @return a vector
     */
    template <typename T, size_t S>
    vec<T,S> ceil(const vec<T,S>& v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = ceil(v[i]);
        }
        return result;
    }

    /**
     * Returns a vector with each component set to the nearest integer which is not greater in magnitude than the
     * corresponding component of the given vector.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the vector to truncate
     * @return the truncated vector
     */
    template <typename T, size_t S>
    vec<T,S> trunc(const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = trunc(v[i]);
        }
        return result;
    }

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
            result[i] = round(v[i]);
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
    vec<T,S> snapDown(const vec<T,S>& v, const vec<T,S>& m) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = snapDown(v[i], m[i]);
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
    vec<T,S> snapUp(const vec<T,S>& v, const vec<T,S>& m) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = snapUp(v[i], m[i]);
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
    vec<T,S> snap(const vec<T,S>& v, const vec<T,S>& m) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = snap(v[i], m[i]);
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
    vec<T,S> correct(const vec<T,S>& v, const size_t decimals = 0, const T epsilon = constants<T>::correctEpsilon()) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = correct(v[i], decimals, epsilon);
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

        if (p == start || p == end) {
            return true;
        } else {
            const auto toStart = start - p;
            const auto toEnd   =   end - p;

            const auto d = dot(toEnd, normalize(toStart));
            return d < T(0.0);
        }
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
     template <typename I, typename G = identity>
     auto average(I cur, I end, const G& get = G()) -> typename std::remove_reference<decltype(get(*cur))>::type {
         assert(cur != end);

         using T = typename std::remove_reference<decltype(get(*cur))>::type::type;

         auto result = get(*cur++);
         auto count = T(1.0);
         while (cur != end) {
             result = result + get(*cur++);
             count = count + T(1.0);
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
    T measureAngle(const vec<T,3>& v, const vec<T,3>& axis, const vec<T,3>& up) {
        const auto cos = dot(v, axis);
        if (isEqual(cos, T(+1.0), vm::constants<T>::almostZero())) {
            return T(0.0);
        } else if (isEqual(cos, T(-1.0), vm::constants<T>::almostZero())) {
            return constants<T>::pi();
        } else {
            const auto perp = cross(axis, v);
            if (dot(perp, up) >= T(0.0)) {
                return std::acos(cos);
            } else {
                return constants<T>::twoPi() - std::acos(cos);
            }
        }
    }
}

#endif
