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

#ifndef TrenchBroom_Vec_h
#define TrenchBroom_Vec_h

#include "MathUtils.h"
#include "StringUtils.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <vector>

template <typename T, size_t S>
class Vec {
public:
    using FloatType = Vec<float, S>;
    using Type = T;
    static const size_t Size = S;

    static const Vec<T,S> PosX;
    static const Vec<T,S> PosY;
    static const Vec<T,S> PosZ;
    static const Vec<T,S> NegX;
    static const Vec<T,S> NegY;
    static const Vec<T,S> NegZ;
    static const Vec<T,S> Null;
    static const Vec<T,S> One;
    static const Vec<T,S> NaN;
    static const Vec<T,S> Min;
    static const Vec<T,S> Max;

    using List = std::vector<Vec<T,S>>;
    static const List EmptyList;
public:
    class GridCmp {
    private:
        const T m_precision;
    public:
        GridCmp(const T precision) : m_precision(precision) {
            assert(m_precision > 0.0);
        }

        bool operator()(const Vec<T, S> &lhs, const Vec<T, S> &rhs) const {
            return compareSnapped(lhs, rhs, m_precision) < 0;
        }
    };
private:
    class SelectionHeapCmp {
    private:
        const Vec<T,S>& m_vec;
        bool m_abs;
    public:
        SelectionHeapCmp(const Vec<T,S>& vec, const bool abs) :
        m_vec(vec),
        m_abs(abs) {}
        
        bool operator()(size_t lhs, size_t rhs) const {
            assert(lhs < S);
            assert(rhs < S);
            if (m_abs)
                return std::abs(m_vec.v[lhs]) < std::abs(m_vec.v[rhs]);
            return m_vec.v[lhs] < m_vec.v[rhs];
        }
    };
public:
    /**
     * Returns a vector with the component at the given index set to 1, and all others set to 0.
     *
     * @param index the index of the component to set to 1
     * @return the newly created vector
     */
    static const Vec<T,S> axis(const size_t index) {
        Vec<T,S> axis;
        axis[index] = static_cast<T>(1.0);
        return axis;
    }

    /**
     * Returns a vector where all components are set to the given value.
     *
     * @param value the value to set
     * @return the newly created vector
     */
    static Vec<T,S> fill(const T value) {
        Vec<T,S> result;
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
    static Vec<T,S> parse(const std::string& str) {
        size_t pos = 0;
        Vec<T,S> result;
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
     * vectors which conform to the vector syntax are added to the result list.
     *
     * @param str the string to parse
     * @return the list of vectors parsed from the string
     */
    static List parseList(const std::string& str) {
        static const std::string blank(" \t\n\r,;");
        
        size_t pos = 0;
        List result;

        while (pos != std::string::npos) {
            Vec<T,S> temp;
            if (doParse(str, pos, temp))
                result.push_back(temp);
            pos = str.find_first_of(blank, pos);
        }
        
        return result;
    }

private:
    static bool doParse(const std::string& str, size_t& pos, Vec<T,S>& result) {
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
protected:
    std::array<T, S> v;
public:
    /**
     * Creates a new vector with all components initialized to 0.
     */
    Vec() {
        for (size_t i = 0; i < S; ++i) {
            v[i] = static_cast<T>(0.0);
        }
    }

    // Copy and move constructors
    Vec(const Vec<T,S>& other) = default;
    Vec(Vec<T,S>&& other) = default;

    // Assignment operators
    Vec<T,S>& operator=(const Vec<T,S>& other) = default;
    Vec<T,S>& operator=(Vec<T,S>&& other) = default;

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
    Vec(const Vec<U,V>& other) {
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
    Vec(std::initializer_list<T> values) {
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
    Vec(const U1 i_x, const U2 i_y) {
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
    Vec(const U1 i_x, const U2 i_y, const U3 i_z) {
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
    Vec(const U1 i_x, const U2 i_y, const U3 i_z, const U4 i_w) {
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
     * @param vec the vector to initialize components 0...S-2 with
     * @param last the value to initialize component S-1 with
     */
    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec, const U last) {
        for (size_t i = 0; i < std::min(S-1,O); ++i) {
            v[i] = static_cast<T>(vec[i]);
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
     * @param vec the vector to initialize components 0...S-3 with
     * @param lastButOne the value to initialize component S-2 with
     * @param last the value to initialize component S-1 with
     */
    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec, const U lastButOne, const U last) {
        for (size_t i = 0; i < std::min(S-2,O); ++i) {
            v[i] = static_cast<T>(vec[i]);
        }
        for (size_t i = std::min(S-2, O); i < S-2; ++i) {
            v[i] = static_cast<T>(0.0);
        }
        v[S-2] = static_cast<T>(lastButOne);
        v[S-1] = static_cast<T>(last);
    }

    /**
     * Returns a new vector with all components set to the given value.
     *
     * @tparam U the type of the value to set
     * @param value the value to set
     * @return the newly created vector
     */
    template <typename U>
    static Vec<T,S> setAll(const U value) {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = static_cast<T>(value);
        }
        return result;
    }

    /**
     * Returns an inverted copy of this vector. The copy is inverted by negating every component.
     *
     * @return the inverted copy
     */
    const Vec<T,S> operator-() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = -v[i];
        return result;
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
    Vec<T,2> xy() const {
        return Vec<T,2>(x(), y());
    }

    /**
     * Returns a vector with the values of the first and third component.
     *
     * @return a vector with the values of the first and third component
     */
    Vec<T,2> xz() const {
        return Vec<T,2>(x(), z());
    }

    /**
     * Returns a vector with the values of the second and third component.
     *
     * @return a vector with the values of the second and third component
     */
    Vec<T,2> yz() const {
        return Vec<T,2>(y(), z());
    }

    /**
     * Returns a vector with the values of the first three components.
     *
     * @return a vector with the values of the first three components
     */
    Vec<T,3> xyz() const {
        return Vec<T,3>(x(), y(), z());
    }
            
    /**
     * Returns a vector with the values of the first four components.
     *
     * @return a vector with the values of the first four components
     */
    Vec<T,4> xyzw() const {
        return Vec<T,4>(x(), y(), z(), w());
    }

    /**
     * Returns the index of the component with the k-highest absolute value. The k-highest component is the
     * index of the component that receives index k if the components are sorted descendent by their absolute
     * value.
     *
     * @param k the value of k
     * @return the index of the k-highest component
     */
    size_t majorComponent(const size_t k) const {
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
        SelectionHeapCmp cmp(*this, true);
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
     * @param k the k value
     * @return the vector indicating the axis of the k-largest component.
     */
    const Vec<T,S> majorAxis(const size_t k) const {
        const size_t c = majorComponent(k);
        Vec<T,S> a = axis(c);
        if (v[c] < static_cast<T>(0.0))
            return -a;
        return a;
    }

    /**
     * Returns a vector indicating the axis of the k-largest component. The returning vector has all values
     * set to 0 except for the compnent that holds the h-largest value. The sign of the returned vector is
     * always positive.
     *
     * @param k the k value
     * @return the vector indicating the absolute axis of the k-largest component
     */
    const Vec<T,S> absMajorAxis(const size_t k) const {
        const size_t c = majorComponent(k);
        return axis(c);
    }

    /**
     * Returns the index of the largest component.
     *
     * @return the index of the largest component
     */
    size_t firstComponent() const {
        return majorComponent(0);
    }

    /**
     * Returns the index of the second largest component.
     *
     * @return the index of the second largest component
     */
    size_t secondComponent() const {
        return majorComponent(1);
    }

    /**
     * Returns the index of the third largest component.
     *
     * @return the index of the third largest component
     */
    size_t thirdComponent() const {
        return majorComponent(2);
    }

    /**
     * Returns the axis of the largest component.
     *
     * @return the axis of the largest component
     */
    const Vec<T,3> firstAxis() const {
        return majorAxis(0);
    }

    /**
     * Returns the axis of the second largest component.
     *
     * @return the axis of the second largest component
     */
    const Vec<T,3> secondAxis() const {
        return majorAxis(1);
    }

    /**
     * Returns the axis of the third largest component.
     *
     * @return the axis of the third largest component
     */
    const Vec<T,3> thirdAxis() const {
        return majorAxis(2);
    }

    /**
     * Returns a vector that is perpendicular to this vector and its weakest axis.
     *
     * @return the perpendicular vector
     */
    Vec<T,S> makePerpendicular() const {
        // get an axis that this vector has the least weight towards.
        const Vec<T,S> leastAxis = majorAxis(S-1);
        return normalize(cross(*this, leastAxis));
    }
    
    void write(std::ostream& str, const size_t components = S) const {
        for (size_t i = 0; i < components; ++i) {
            str << v[i];
            if (i < components - 1)
                str << ' ';
        }
    }
    
    std::string asString(const size_t components = S) const {
        StringStream result;
        write(result, components);
        return result.str();
    }

    Vec<T,S>& mix(const Vec<T,S>& vec, const Vec<T,S>& factor) {
        *this = *this * (Vec<T,S>::One - factor) + vec * factor;
        return *this;
    }
    
    Vec<T,S> mixed(const Vec<T,S>& vec, const Vec<T,S>& factor) const {
        return Vec<T,S>(*this).mix(vec, factor);
    }
    
    bool isInteger(const T epsilon = Math::Constants<T>::almostZero()) const {
        for (size_t i = 0; i < S; ++i)
            if (std::abs(v[i] - Math::round(v[i])) > epsilon)
                return false;
        return true;
    }
    
    Vec<T,S>& correct(const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon()) {
        for (size_t i = 0; i < S; ++i)
            v[i] = Math::correct(v[i], decimals, epsilon);
        return *this;
    }
    
    Vec<T,S> corrected(const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon()) const {
        return Vec<T,S>(*this).correct(decimals, epsilon);
    }
    
    Vec<T,S-1> at(const size_t j, const T a) const {
        assert(v[j] != 0.0f);
        
        const T f = a / v[j];
        Vec<T,S-1> result;
        size_t k = 0;
        for (size_t i = 0; i < S; ++i) {
            if (i != j)
                result[k++] = v[i] * f;
        }
        return result;
    }
    
    struct EdgeDistance {
        const Vec<T,S> point;
        const T distance;
        
        EdgeDistance(const Vec<T,S>& i_point, const T i_distance) :
        point(i_point),
        distance(i_distance) {}
    };
    
    EdgeDistance distanceToSegment(const Vec<T,S>& start, const Vec<T,S>& end) const {
        const Vec<T,S> edgeVec = end - start;
        const Vec<T,S> edgeDir = normalize(edgeVec);
        const T scale = dot(*this - start, edgeDir);
        
        // determine the closest point on the edge
        Vec<T,S> closestPoint;
        if (scale < 0.0) {
            closestPoint = start;
        } else if ((scale * scale) > squaredLength(edgeVec)) {
            closestPoint = end;
        } else {
            closestPoint = start + edgeDir * scale;
        }

        const T distance = length(*this - closestPoint);
        return EdgeDistance(closestPoint, distance);
    }

    static Vec<T,S> average(const typename Vec<T,S>::List& vecs) {
        assert(!vecs.empty());
        Vec<T,S> sum;
        for (size_t i = 0; i < vecs.size(); ++i)
            sum += vecs[i];
        return sum / static_cast<T>(vecs.size());
    }

    bool containedWithinSegment(const Vec<T,S>& start, const Vec<T,S>& end) const {
        assert(colinear(*this, start, end));
        const Vec<T,S> toStart = start - *this;
        const Vec<T,S> toEnd   =   end - *this;

        const T d = dot(toEnd, normalize(toStart));
        return !Math::pos(d);
    }

    template <typename I, typename G>
    static Vec<T,S> center(I cur, I end, const G& get) {
        assert(cur != end);
        Vec<T,S> result = get(*cur++);
        T count = 1.0;
        while (cur != end) {
            result += get(*cur++);
            count += 1.0;
        }
        return result / count;
    }
    
    template <typename I, typename G>
    static typename Vec<T,S>::List asList(I cur, I end, const G& get) {
        typename Vec<T,S>::List result;
        toList(cur, end, get, result);
        return result;
    }
    
    template <typename I, typename G>
    static void toList(I cur, I end, const G& get, typename Vec<T,S>::List& result) {
        addAll(cur, end, get, std::back_inserter(result));
    }
    
    template <typename I, typename G, typename O>
    static void addAll(I cur, I end, const G& get, O outp) {
        while (cur != end) {
            outp = get(*cur);
            ++cur;
        }
    }
};

template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosX = Vec<T,S>::axis(0);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosY = Vec<T,S>::axis(1);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosZ = Vec<T,S>::axis(2);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegX = -Vec<T,S>::axis(0);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegY = -Vec<T,S>::axis(1);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegZ = -Vec<T,S>::axis(2);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::Null = Vec<T,S>::fill(static_cast<T>(0.0));
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::One  = Vec<T,S>::fill(static_cast<T>(1.0));
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NaN  = Vec<T,S>::fill(std::numeric_limits<T>::quiet_NaN());
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::Min  = Vec<T,S>::fill(std::numeric_limits<T>::min());
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::Max  = Vec<T,S>::fill(std::numeric_limits<T>::max());

template <typename T, size_t S>
const typename Vec<T,S>::List Vec<T,S>::EmptyList = Vec<T,S>::List();

typedef Vec<float,1> Vec1f;
typedef Vec<double,1> Vec1d;
typedef Vec<int,1> Vec1i;
typedef Vec<long,1> Vec1l;
typedef Vec<size_t,1> Vec1s;
typedef Vec<float,2> Vec2f;
typedef Vec<double,2> Vec2d;
typedef Vec<int,2> Vec2i;
typedef Vec<long,2> Vec2l;
typedef Vec<size_t,2> Vec2s;
typedef Vec<bool,2> Vec2b;
typedef Vec<float,3> Vec3f;
typedef Vec<double,3> Vec3d;
typedef Vec<int,3> Vec3i;
typedef Vec<long,3> Vec3l;
typedef Vec<size_t,3> Vec3s;
typedef Vec<bool,3> Vec3b;
typedef Vec<float,4> Vec4f;
typedef Vec<double,4> Vec4d;
typedef Vec<int,4> Vec4i;
typedef Vec<long,4> Vec4l;
typedef Vec<size_t,4> Vec4s;
typedef Vec<bool,4> Vec4b;

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
int compare(const Vec<T,S>& lhs, const Vec<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
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
int compare(I lhsCur, I lhsEnd, I rhsCur, I rhsEnd, const typename I::value_type::Type epsilon = static_cast<typename I::value_type::Type>(0.0)) {
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
 * Lexicographically compares the given components of the vectors after snapping them to a grid of the given size.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @param precision the grid size for component wise comparison
 * @return -1 if the left hand size is less than the right hand size, +1 if the left hand size is greater than the right hand size, and 0 if both sides are equal
 */
template<typename T, size_t S>
int compareSnapped(const Vec<T,S>& lhs, const Vec<T,S>& rhs, const T precision) {
    for (size_t i = 0; i < S; ++i) {
        const T l = Math::snap(lhs[i], precision);
        const T r = Math::snap(rhs[i], precision);
        if (Math::lt(l, r, 0.0))
            return -1;
        if (Math::gt(l, r, 0.0))
            return 1;
    }
    return 0;
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
bool operator==(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
bool operator!=(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
bool operator<(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
bool operator<=(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
bool operator>(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
bool operator>=(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) >= 0;
}

/* ========== arithmetic operators ========== */

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
Vec<T,S> operator+(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
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
Vec<T,S>& operator+=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
Vec<T,S> operator-(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
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
Vec<T,S>& operator-=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
Vec<T,S> operator*(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
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
Vec<T,S> operator*(const Vec<T,S>& lhs, const T rhs) {
    Vec<T,S> result(lhs);
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
Vec<T,S> operator*(const T lhs, const Vec<T,S>& rhs) {
    return Vec<T,S>(rhs) * lhs;
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
Vec<T,S>& operator*=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
Vec<T,S>& operator*=(Vec<T,S>& lhs, const T rhs) {
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
Vec<T,S> operator/(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
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
Vec<T,S> operator/(const Vec<T,S>& lhs, const T rhs) {
    Vec<T,S> result(lhs);
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
Vec<T,S>& operator/=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
Vec<T,S>& operator/=(Vec<T,S>& lhs, const T rhs) {
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
typename Vec<T,S>::List operator+(const typename Vec<T,S>::List& lhs, const Vec<T,S>& rhs) {
    typename Vec<T,S>::List result;
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
typename Vec<T,S>::List operator+(const Vec<T,S>& lhs, const typename Vec<T,S>::List& rhs) {
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
typename Vec<T,S>::List operator*(const typename Vec<T,S>::List& lhs, const T rhs) {
    typename Vec<T,S>::List result;
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
typename Vec<T,S>::List operator*(const T lhs, const typename Vec<T,S>::List& rhs) {
    return rhs * lhs;
}

/* ========== stream operators ========== */

template <typename T, size_t S>
std::ostream& operator<< (std::ostream& stream, const Vec<T,S>& vec) {
    stream << "(";
    if (S > 0) {
        stream << vec[0];
        for (size_t i = 1; i < S; ++i) {
            stream << ", " << vec[i];
        }
    }
    stream << ")";
    return stream;
}


/* ========== arithmetic functions ========== */

/**
 * Returns a vector where each component is the absolute value of the corresponding component of the the
 * given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to make absolute
 * @return the absolute vector
 */
template <typename T, size_t S>
Vec<T,S> abs(const Vec<T,S>& vec) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::abs(vec[i]);
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
Vec<T,S> min(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
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
Vec<T,S> max(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
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
Vec<T,S> absMin(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
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
Vec<T,S> absMax(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
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
T dot(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
Vec<T,3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return Vec<T,3>(lhs[1] * rhs[2] - lhs[2] * rhs[1],
                    lhs[2] * rhs[0] - lhs[0] * rhs[2],
                    lhs[0] * rhs[1] - lhs[1] * rhs[0]);
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
T distance(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
T squaredDistance(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
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
Vec<T,S> normalize(const Vec<T,S>& vec) {
    return vec / length(vec);
}

/* ========== coordinate system conversions ========== */

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
Vec<T,S+1> toHomogeneousCoords(const Vec<T,S>& point) {
    return Vec<T,S+1>(point, static_cast<T>(1.0));
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
Vec<T,S-1> toCartesianCoords(const Vec<T,S>& point) {
    Vec<T,S-1> result;
    for (size_t i = 0; i < S-1; ++i) {
        result[i] = point[i] / point[S-1];
    }
    return result;
}

/* ========== geometric properties and comparison ========== */

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
bool equal(const Vec<T,S>& lhs, const Vec<T,S>& rhs, const T epsilon) {
    return compare(lhs, rhs, epsilon) == 0;
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
bool colinear(const Vec<T,S>& a, const Vec<T,S>& b, const Vec<T,S>& c, const T epsilon = Math::Constants<T>::colinearEpsilon()) {
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
bool parallel(const Vec<T,S>& lhs, const Vec<T,S>& rhs, const T epsilon = Math::Constants<T>::colinearEpsilon()) {
    const T cos = dot(normalize(lhs), normalize(rhs));
    return Math::one(Math::abs(cos), epsilon);
}

/* ========== computing properties of single vectors ========== */

/**
 * Returns the length of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the length of
 * @return the length of the given vector
 */
template <typename T, size_t S>
T length(const Vec<T,S>& vec) {
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
T squaredLength(const Vec<T,S>& vec) {
    return dot(vec, vec);
}

/**
 * Checks whether the given vector has unit length (1).
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 1 and false otherwise
 */
template <typename T, size_t S>
bool isUnit(const Vec<T,S>& vec, const T epsilon = Math::Constants<T>::almostZero()) {
    return Math::one(length(vec), epsilon);
}

/**
 * Checks whether the given vector has a length of 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 0 and false otherwise
 */
template <typename T, size_t S>
bool isNull(const Vec<T,S>& vec, const T epsilon = Math::Constants<T>::almostZero()) {
    return Math::zero(length(vec), epsilon);
}

/**
 * Checks whether the given vector NaN as any component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to check
 * @return true if the given vector has NaN as any component
 */
template <typename T, size_t S>
bool isNaN(const Vec<T,S>& vec) {
    for (size_t i = 0; i < S; ++i) {
        if (Math::isnan(vec[i])) {
            return true;
        }
    }
    return false;
}

/* ========== rounding and error correction ========== */

/**
 * Returns a vector where each component is the rounded value of the corresponding component of the given
 * vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> round(const Vec<T,S>& vec) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::round(vec[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector down to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round down
 * @param m the multiples to round down to
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> roundDownToMultiple(const Vec<T,S>& vec, const Vec<T,S>& m) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundDownToMultiple(vec[i], m[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector up to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round down
 * @param m the multiples to round up to
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> roundUpToMultiple(const Vec<T,S>& vec, const Vec<T,S>& m) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundUpToMultiple(vec[i], m[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round down
 * @param m the multiples to round to
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> roundToMultiple(const Vec<T,S>& vec, const Vec<T,S>& m) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundToMultiple(vec[i], m[i]);
    }
    return result;
}



/*
 * The normal will be pointing towards the reader when the points are oriented like this:
 *
 * 1
 * |
 * v2
 * |
 * |
 * 0------v1----2
 */
template <typename T>
bool planeNormal(Vec<T,3>& normal, const Vec<T,3>& point0, const Vec<T,3>& point1, const Vec<T,3>& point2, const T epsilon = Math::Constants<T>::angleEpsilon()) {
    const Vec<T,3> v1 = point2 - point0;
    const Vec<T,3> v2 = point1 - point0;
    normal = cross(v1, v2);
    
    // Fail if v1 and v2 are parallel, opposite, or either is zero-length.
    // Rearranging "A cross B = ||A|| * ||B|| * sin(theta) * n" (n is a unit vector perpendicular to A and B) gives sin_theta below
    const T sin_theta = Math::abs(length(normal) / (length(v1) * length(v2)));
    if (Math::isnan(sin_theta) ||
        Math::isinf(sin_theta) ||
        sin_theta < epsilon)
        return false;
    
    normal = normalize(normal);
    return true;
}

/**
 Computes the CCW angle between axis and vector in relation to the given up vector.
 All vectors are expected to be normalized.
 */
template <typename T>
T angleBetween(const Vec<T,3>& vec, const Vec<T,3>& axis, const Vec<T,3>& up) {
    const T cos = dot(vec, axis);
    if (Math::one(+cos))
        return static_cast<T>(0.0);
    if (Math::one(-cos))
        return Math::Constants<T>::pi();
    const Vec<T,3> perp = cross(axis, vec);
    if (!Math::neg(dot(perp, up)))
        return std::acos(cos);
    return Math::Constants<T>::twoPi() - std::acos(cos);
}

template <typename T>
bool commonPlane(const Vec<T,3>& p1, const Vec<T,3>& p2, const Vec<T,3>& p3, const Vec<T,3>& p4, const T epsilon = Math::Constants<T>::almostZero()) {
    assert(!colinear(p1, p2, p3, epsilon));
    const Vec<T,3> normal = normalize(cross(p3 - p1, p2 - p1));
    const T offset = dot(p1, normal);
    const T dist = dot(p4, normal) - offset;
    return Math::abs(dist) < epsilon;
}

#endif
