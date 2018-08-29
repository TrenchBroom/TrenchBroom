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

#ifndef TRENCHBROOM_VEC_TYPE_H
#define TRENCHBROOM_VEC_TYPE_H

#include "MathUtils.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <vector>

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
    static const vec<T,S> axis(const size_t index) {
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
    static vec<T,S> fill(const T value) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = value;
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
    static bool doParse(const std::string& str, size_t& pos, vec<T,S>& result) {
        static const std::string blank(" \t\n\r()");

        const char* cstr = str.c_str();
        for (size_t i = 0; i < S; ++i) {
            if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos)
                return false;
            result[i] = static_cast<T>(std::atof(cstr + pos));
            if ((pos = str.find_first_of(blank, pos)) == std::string::npos)
                return false;;
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
    vec(const vec<U,V>& other) {
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
    vec(const U1 i_x, const U2 i_y) {
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
    vec(const U1 i_x, const U2 i_y, const U3 i_z) {
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
    vec(const U1 i_x, const U2 i_y, const U3 i_z, const U4 i_w) {
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
    vec(const vec<U,O>& i_v, const U last) {
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
    vec(const vec<U,O>& i_v, const U lastButOne, const U last) {
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
    T& operator[](const size_t index) {
        assert(index < S);
        return v[index];
    }

    /**
     * Returns a const reference to the component at the given index. The index is not checked at runtime.
     *
     * @param index the index of the component
     * @return a const reference to the compononent at the given index
     */
    const T& operator[](const size_t index) const {
        assert(index < S);
        return v[index];
    }

    /**
     * Returns the value of the first compnent.
     *
     * @return the value of the first component
     */
    T x() const {
        assert(S > 0);
        return v[0];
    }

    /**
     * Returns the value of the component compnent.
     *
     * @return the value of the component component
     */
    T y() const {
        assert(S > 1);
        return v[1];
    }

    /**
     * Returns the value of the third compnent.
     *
     * @return the value of the third component
     */
    T z() const {
        assert(S > 2);
        return v[2];
    }

    /**
     * Returns the value of the fourth compnent.
     *
     * @return the value of the fourth component
     */
    T w() const {
        assert(S > 3);
        return v[3];
    }

    /**
     * Returns a vector with the values of the first and second component.
     *
     * @return a vector with the values of the first and second component
     */
    vec<T,2> xy() const {
        return vec<T,2>(x(), y());
    }

    /**
     * Returns a vector with the values of the first and third component.
     *
     * @return a vector with the values of the first and third component
     */
    vec<T,2> xz() const {
        return vec<T,2>(x(), z());
    }

    /**
     * Returns a vector with the values of the second and third component.
     *
     * @return a vector with the values of the second and third component
     */
    vec<T,2> yz() const {
        return vec<T,2>(y(), z());
    }

    /**
     * Returns a vector with the values of the first three components.
     *
     * @return a vector with the values of the first three components
     */
    vec<T,3> xyz() const {
        return vec<T,3>(x(), y(), z());
    }
            
    /**
     * Returns a vector with the values of the first four components.
     *
     * @return a vector with the values of the first four components
     */
    vec<T,4> xyzw() const {
        return vec<T,4>(x(), y(), z(), w());
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

template <typename T, size_t S>
const typename vec<T,S>::List vec<T,S>::EmptyList = vec<T,S>::List();

#endif
