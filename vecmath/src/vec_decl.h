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

#ifndef TRENCHBROOM_VEC_DECL_H
#define TRENCHBROOM_VEC_DECL_H

#include "utils.h"
#include "constants.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <vector>

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

        // TODO 2201: make lowercase or move out
        using List = std::vector<vec<T,S>>;
        static const List EmptyList;
    protected:
        /**
         * The vector components.
         */
        std::array<T, S> v;
    public:
        /**
         * Returns a vector with the component at the given index set to 1, and all others set to 0.
         *
         * @param index the index of the component to set to 1
         * @return the newly created vector
         */
        static vec<T,S> axis(size_t index);

        /**
         * Returns a vector where all components are set to the given value.
         *
         * @param value the value to set
         * @return the newly created vector
         */
        static vec<T,S> fill(T value);

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
        static vec<T,S> parse(const std::string& str);

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
            }
        }
    private:
        static bool doParse(const std::string& str, size_t& pos, vec<T,S>& result);
    public:
        /**
         * Creates a new vector with all components initialized to 0.
         */
        vec();

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
        vec(std::initializer_list<T> values);

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
        T& operator[](size_t index);

        /**
         * Returns a const reference to the component at the given index. The index is not checked at runtime.
         *
         * @param index the index of the component
         * @return a const reference to the compononent at the given index
         */
        const T& operator[](size_t index) const;

        /**
         * Returns the value of the first compnent.
         *
         * @return the value of the first component
         */
        T x() const;

        /**
         * Returns the value of the component compnent.
         *
         * @return the value of the component component
         */
        T y() const;

        /**
         * Returns the value of the third compnent.
         *
         * @return the value of the third component
         */
        T z() const;

        /**
         * Returns the value of the fourth compnent.
         *
         * @return the value of the fourth component
         */
        T w() const;

        /**
         * Returns a vector with the values of the first and second component.
         *
         * @return a vector with the values of the first and second component
         */
        vec<T,2> xy() const;

        /**
         * Returns a vector with the values of the first and third component.
         *
         * @return a vector with the values of the first and third component
         */
        vec<T,2> xz() const;

        /**
         * Returns a vector with the values of the second and third component.
         *
         * @return a vector with the values of the second and third component
         */
        vec<T,2> yz() const;

        /**
         * Returns a vector with the values of the first three components.
         *
         * @return a vector with the values of the first three components
         */
        vec<T,3> xyz() const;

        /**
         * Returns a vector with the values of the first four components.
         *
         * @return a vector with the values of the first four components
         */
        vec<T,4> xyzw() const;
    };

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
    int compare(const vec<T,S>& lhs, const vec<T,S>& rhs, T epsilon = static_cast<T>(0.0));

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
    int compare(I lhsCur, I lhsEnd, I rhsCur, I rhsEnd, typename I::value_type::type epsilon = static_cast<typename I::value_type::type>(0.0));

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
    bool isEqual(const vec<T,S>& lhs, const vec<T,S>& rhs, T epsilon);

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
    bool operator==(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    bool operator!=(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    bool operator<(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    bool operator<=(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    bool operator>(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    bool operator>=(const vec<T,S>& lhs, const vec<T,S>& rhs);

    /* ========== accessing major component / axis ========== */

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
    size_t majorComponent(const vec<T,S>& v, size_t k);

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
    vec<T,S> majorAxis(const vec<T,S>& v, size_t k);

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
    vec<T,S> absMajorAxis(const vec<T,S>& v, size_t k);

    /**
     * Returns the index of the largest component.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the vector
     * @return the index of the largest component
     */
    template <typename T, size_t S>
    size_t firstComponent(const vec<T,S>& v);

    /**
     * Returns the index of the second largest component.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the vector
     * @return the index of the second largest component
     */
    template <typename T, size_t S>
    size_t secondComponent(const vec<T,S>& v);

    /**
     * Returns the index of the third largest component.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the vector
     * @return the index of the third largest component
     */
    template <typename T, size_t S>
    size_t thirdComponent(const vec<T,S>& v);

    /**
     * Returns the axis of the largest component.
     *
     * @tparam T the component type
     * @param v the vector
     * @return the axis of the largest component
     */
    template <typename T>
    vec<T,3> firstAxis(const vec<T,3>& v);

    /**
     * Returns the axis of the second largest component.
     *
     * @tparam T the component type
     * @param v the vector
     * @return the axis of the second largest component
     */
    template <typename T>
    vec<T,3> secondAxis(const vec<T,3>& v);

    /**
     * Returns the axis of the third largest component.
     *
     * @tparam T the component type
     * @param v the vector
     * @return the axis of the third largest component
     */
    template <typename T>
    vec<T,3> thirdAxis(const vec<T,3>& v);

    /* ========== arithmetic operators ========== */

    /**
     * Returns an inverted copy of this vector. The copy is inverted by negating every component.
     *
     * @return the inverted copy
     */
    template <typename T, size_t S>
    vec<T,S> operator-(const vec<T,S>& vector);

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
    vec<T,S> operator+(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> operator-(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> operator*(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> operator*(const vec<T,S>& lhs, T rhs);


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
    vec<T,S> operator*(T lhs, const vec<T,S>& rhs);

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
    vec<T,S> operator/(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> operator/(const vec<T,S>& lhs, T rhs);

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
    typename vec<T,S>::List operator+(const typename vec<T,S>::List& lhs, const vec<T,S>& rhs);

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
    typename vec<T,S>::List operator+(const vec<T,S>& lhs, const typename vec<T,S>::List& rhs);

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
    typename vec<T,S>::List operator*(const typename vec<T,S>::List& lhs, T rhs);

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
    typename vec<T,S>::List operator*(T lhs, const typename vec<T,S>::List& rhs);

    /* ========== stream operators ========== */

    template <typename T, size_t S>
    std::ostream& operator<< (std::ostream& stream, const vec<T,S>& vec);

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
    vec<T,S> abs(const vec<T,S>& v);

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
    vec<T,S> min(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> max(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> absMin(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S> absMax(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    T dot(const vec<T,S>& lhs, const vec<T,S>& rhs);

    /**
     * Returns the cross product (also called outer product) of the two given 3d vectors.
     *
     * @tparam T the component type
     * @param lhs the left hand vector
     * @param rhs the right hand vector
     * @return the cross product of the given vectors
     */
    template <typename T>
    vec<T,3> cross(const vec<T, 3>& lhs, const vec<T, 3>& rhs);

    /**
     * Returns the squared length of the given vector.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param vec the vector to normalize
     * @return the squared length of the given vector
     */
    template <typename T, size_t S>
    T squaredLength(const vec<T,S>& vec);

    /**
     * Returns the length of the given vector.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param vec the vector to return the length of
     * @return the length of the given vector
     */
    template <typename T, size_t S>
    T length(const vec<T,S>& vec);

    /**
     * Normalizes the given vector.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param vec the vector to return the squared length of
     * @return the normalized vector
     */
    template <typename T, size_t S>
    vec<T,S> normalize(const vec<T,S>& vec);

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
    vec<T,3> swizzle(const vec<T,3>& point, size_t axis);

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
    vec<T,3> unswizzle(const vec<T,3>& point, size_t axis);

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
    bool isUnit(const vec<T,S>& v, T epsilon = Constants<T>::almostZero());

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
    bool isZero(const vec<T,S>& v, T epsilon = Constants<T>::almostZero());

    /**
     * Checks whether the given vector NaN as any component.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param v the vector to check
     * @return true if the given vector has NaN as any component
     */
    template <typename T, size_t S>
    bool isNaN(const vec<T,S>& v);

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
    bool isIntegral(const vec<T,S>& v, T epsilon = static_cast<T>(0.0));

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
    vec<T,S> mix(const vec<T,S>& lhs, const vec<T,S>& rhs, const vec<T,S>& f);

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
    T distance(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    T squaredDistance(const vec<T,S>& lhs, const vec<T,S>& rhs);

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
    vec<T,S+1> toHomogeneousCoords(const vec<T,S>& point);

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
    vec<T,S-1> toCartesianCoords(const vec<T,S>& point);

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
    bool colinear(const vec<T,S>& a, const vec<T,S>& b, const vec<T,S>& c, T epsilon = Constants<T>::colinearEpsilon());

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
    bool parallel(const vec<T,S>& lhs, const vec<T,S>& rhs, T epsilon = Constants<T>::colinearEpsilon());

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
    vec<T,S> round(const vec<T,S>& v);

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
    vec<T,S> snapDown(const vec<T,S>& v, const vec<T,S>& m);

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
    vec<T,S> snapUp(const vec<T,S>& v, const vec<T,S>& m);

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
    vec<T,S> snap(const vec<T,S>& v, const vec<T,S>& m);

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
    vec<T,S> correct(const vec<T,S>& v, size_t decimals = 0, T epsilon = Constants<T>::correctEpsilon());

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
    bool between(const vec<T,S>& p, const vec<T,S>& start, const vec<T,S>& end);

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
     // TODO 2201: rename to avg
    template <typename I, typename G = Identity>
    auto average(I cur, I end, const G& get = G()) -> typename std::remove_reference<decltype(get(*cur))>::type;

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
    T angleBetween(const vec<T,3>& v, const vec<T,3>& axis, const vec<T,3>& up);

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
        EdgeDistance(const vec<T,S>& i_point, T i_distance);
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
    EdgeDistance<T,S> distanceOfPointAndSegment(const vec<T,S>& point, const vec<T,S>& start, const vec<T,S>& end);
}

#endif
