/*
Copyright (C) 2010-2014 Kristian Duske

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
#include <cassert>
#include <cstddef>
#include <map>
#include <ostream>
#include <set>
#include <vector>

template <typename T, size_t S>
class Vec {
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
    
    int weight(T c) const {
        if (std::abs(c - static_cast<T>(1.0)) < static_cast<T>(0.9))
            return 0;
        if (std::abs(c + static_cast<T>(1.0)) < static_cast<T>(0.9))
            return 1;
        return 2;
    }
public:
    typedef T Type;
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
    
    class LexicographicOrder {
    private:
        T m_epsilon;
    public:
        LexicographicOrder(const T epsilon = Math::Constants<T>::almostZero()) :
        m_epsilon(epsilon) {}
        
        bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
            for (size_t i = 0; i < S; ++i) {
                if (Math::lt(lhs[i], rhs[i], m_epsilon))
                    return true;
                if (Math::gt(lhs[i], rhs[i], m_epsilon))
                    return false;
            }
            return false;
        }
    };

    class ErrorOrder {
    public:
        bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
            const T lErr = (lhs - lhs.rounded()).lengthSquared();
            const T rErr = (rhs - rhs.rounded()).lengthSquared();
            return lErr < rErr;
        }
    };
    
    class DotOrder {
    private:
        const Vec<T,S>& m_dir;
    public:
        DotOrder(const Vec<T,S>& dir) :
        m_dir(dir) {
            assert(!m_dir.null());
        }
        
        bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
            return lhs.dot(m_dir) < rhs.dot(m_dir);
        }
    };
    
    class InverseDotOrder {
    private:
        const Vec<T,S>& m_dir;
    public:
        InverseDotOrder(const Vec<T,S>& dir) :
        m_dir(dir) {
            assert(!m_dir.null());
        }
        
        bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
            return lhs.dot(m_dir) > rhs.dot(m_dir);
        }
    };
    
    class LengthOrder {
    public:
        bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
            return lhs.squaredLength() < rhs.squaredLength();
        }
    };
    
    typedef std::vector<Vec<T,S> > List;
    typedef std::set<Vec<T,S>, LexicographicOrder> Set;
    typedef std::map<Vec<T,S>, Vec<T,S>, LexicographicOrder> Map;
    
    static const List EmptyList;
    static const Set EmptySet;
    static const Map EmptyMap;
    
    template <typename O>
    class SetOrder {
    private:
        O m_order;
    public:
        SetOrder(const O& order = O()) :
        m_order(order) {}

        bool operator()(const Set& lhs, const Set& rhs) const {
            return compare(lhs, rhs) < 0;
        }
    private:
        int compare(const Set& lhs, const Set& rhs) const {
            if (lhs.size() < rhs.size())
                return -1;
            if (lhs.size() > rhs.size())
                return 1;
            
            typename Set::const_iterator lIt = lhs.begin();
            typename Set::const_iterator rIt = rhs.begin();
            for (size_t i = 0; i < lhs.size(); ++i) {
                const Vec& lPos = *lIt++;
                const Vec& rPos = *rIt++;
                
                if (m_order(lPos, rPos))
                    return -1;
                if (m_order(rPos, lPos))
                    return 1;
            }
            return 0;
        }
    };
    
public:
    static const Vec<T,S> axis(const size_t index) {
        Vec<T,S> axis;
        axis[index] = static_cast<T>(1.0);
        return axis;
    }
    
    static Vec<T,S> fill(const T value) {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = value;
        return result;
    }
    
    static Vec<T,S> unit(const size_t index) {
        assert(index < S);
        
        Vec<T,S> result;
        result[index] = static_cast<T>(1.0);
        return result;
    }
    
    template <typename U1>
    static Vec<T,S> create(const U1 i_x) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
        }
        for (size_t i = 1; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }
    
    template <typename U1, typename U2>
    static Vec<T,S> create(const U1 i_x, const U2 i_y) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
            if (S > 1) {
                result[1] = static_cast<T>(i_y);
            }
        }
        for (size_t i = 2; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }

    template <typename U1, typename U2, typename U3>
    static Vec<T,S> create(const U1 i_x, const U2 i_y, const U3 i_z) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
            if (S > 1) {
                result[1] = static_cast<T>(i_y);
                if (S > 2) {
                    result[2] = static_cast<T>(i_z);
                }
            }
        }
        for (size_t i = 3; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }

    template <typename U1, typename U2, typename U3, typename U4>
    static Vec<T,S> create(const U1 i_x, const U2 i_y, const U3 i_z, const U4 i_w) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
            if (S > 1) {
                result[1] = static_cast<T>(i_y);
                if (S > 2) {
                    result[2] = static_cast<T>(i_z);
                    if (S > 3) {
                        result[3] = static_cast<T>(i_w);
                    }
                }
            }
        }
        for (size_t i = 4; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }
    
    static Vec<T,S> parse(const std::string& str) {
        size_t pos = 0;
        return doParse(str, pos);
    }
    
    static List parseList(const std::string& str) {
        static const std::string blank(" \t\n\r,;");
        
        size_t pos = 0;
        List result;

        while (pos != std::string::npos) {
            result.push_back(doParse(str, pos));
            pos = str.find_first_of(blank, pos);
        }
        
        return result;
    }

private:
    static Vec<T,S> doParse(const std::string& str, size_t& pos) {
        static const std::string blank(" \t\n\r()");

        Vec<T,S> result;
        const char* cstr = str.c_str();
        for (size_t i = 0; i < S; ++i) {
            if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos)
                break;
            result[i] = static_cast<T>(std::atof(cstr + pos));
            if ((pos = str.find_first_of(blank, pos)) == std::string::npos)
                break;
        }
        return result;
    }
public:
    T v[S];
    
    Vec() {
        setNull();
    }
            
    template <typename U1, typename U2>
    Vec(const U1 i_x, const U2 i_y) {
        if (S > 0) {
            v[0] = static_cast<T>(i_x);
            if (S > 1)
                v[1] = static_cast<T>(i_y);
        }
        for (size_t i = 2; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }
    
    template <typename U1, typename U2, typename U3>
    Vec(const U1 i_x, const U2 i_y, const U3 i_z) {
        if (S > 0) {
            v[0] = static_cast<T>(i_x);
            if (S > 1) {
                v[1] = static_cast<T>(i_y);
                if (S > 2)
                    v[2] = static_cast<T>(i_z);
            }
        }
        for (size_t i = 3; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }

    template <typename U1, typename U2, typename U3, typename U4>
    Vec(const U1 i_x, const U2 i_y, const U3 i_z, const U4 i_w) {
        if (S > 0) {
            v[0] = static_cast<T>(i_x);
            if (S > 1) {
                v[1] = static_cast<T>(i_y);
                if (S > 2) {
                    v[2] = static_cast<T>(i_z);
                    if (S > 3)
                        v[3] = static_cast<T>(i_w);
                }
            }
        }
        for (size_t i = 4; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }

    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec) {
        for (size_t i = 0; i < std::min(S,O); ++i)
            v[i] = static_cast<T>(vec[i]);
        for (size_t i = std::min(S,O); i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }

    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec, const U last) {
        for (size_t i = 0; i < std::min(S-1,O); ++i)
            v[i] = static_cast<T>(vec[i]);
        for (size_t i = std::min(S-1, O); i < S-1; ++i)
            v[i] = static_cast<T>(0.0);
        v[S-1] = static_cast<T>(last);
    }
    
    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec, const U oneButLast, const U last) {
        for (size_t i = 0; i < std::min(S-2,O); ++i)
            v[i] = static_cast<T>(vec[i]);
        for (size_t i = std::min(S-2, O); i < S-2; ++i)
            v[i] = static_cast<T>(0.0);
        v[S-2] = static_cast<T>(oneButLast);
        v[S-1] = static_cast<T>(last);
    }
    
    int compare(const Vec<T,S>& right, const T epsilon = static_cast<T>(0.0)) const {
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(v[i], right[i], epsilon))
                return -1;
            if (Math::gt(v[i], right[i], epsilon))
                return 1;
        }
        return 0;
    }
    
    bool operator==(const Vec<T,S>& right) const {
        return compare(right) == 0;
    }
    
    bool operator!= (const Vec<T,S>& right) const {
        return compare(right) != 0;
    }
    
    bool operator<(const Vec<T,S>& right) const {
        return compare(right) < 0;
    }
    
    bool operator<= (const Vec<T,S>& right) const {
        return compare(right) <= 0;
    }

    bool operator>(const Vec<T,S>& right) const {
        return compare(right) > 0;
    }
    
    bool operator>= (const Vec<T,S>& right) const {
        return compare(right) >= 0;
    }

    template <size_t O>
    Vec<T,S>& operator=(const Vec<T,O>& right) {
        for (size_t i = 0; i < std::min(S,O); ++i)
            v[i] = right[i];
        for (size_t i = std::min(S,O); i < S; ++i)
            v[i] = static_cast<T>(0.0);
        return *this;
    }
    
    const Vec<T,S> operator-() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = -v[i];
        return result;
    }

    const Vec<T,S> operator+(const Vec<T,S>& right) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = v[i] + right[i];
        return result;
    }
    
    Vec<T,S>& operator+= (const Vec<T,S>& right) {
        for (size_t i = 0; i < S; ++i)
            v[i] += right[i];
        return *this;
    }

    const Vec<T,S> operator-(const Vec<T,S>& right) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = v[i] - right[i];
        return result;
    }
    
    Vec<T,S>& operator-= (const Vec<T,S>& right) {
        for (size_t i = 0; i < S; ++i)
            v[i] -= right[i];
        return *this;
    }
    
    const Vec<T,S> operator*(const T right) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = v[i] * right;
        return result;
    }
    
    Vec<T,S>& operator*= (const T right) {
        for (size_t i = 0; i < S; ++i)
            v[i] *= right;
        return *this;
    }
    
    const Vec<T,S> operator*(const Vec<T,S>& right) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = v[i] * right[i];
        return result;
    }
    
    Vec<T,S>& operator*= (const Vec<T,S>& right) {
        for (size_t i = 0; i < S; ++i)
            v[i] *= right[i];
        return *this;
    }

    const Vec<T,S> operator/(const T right) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = v[i] / right;
        return result;
    }
    
    Vec<T,S>& operator/= (const T right) {
        for (size_t i = 0; i < S; ++i)
            v[i] /= right;
        return *this;
    }
    
    const Vec<T,S> operator/(const Vec<T,S>& right) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = v[i] / right[i];
        return result;
    }
    
    Vec<T,S>& operator/= (const Vec<T,S>& right) {
        for (size_t i = 0; i < S; ++i)
            v[i] /= right[i];
        return *this;
    }
    
    T& operator[] (const size_t index) {
        assert(index < S);
        return v[index];
    }
    
    const T& operator[] (const size_t index) const {
        assert(index < S);
        return v[index];
    }
    
    T x() const {
        assert(S > 0);
        return v[0];
    }
            
    T y() const {
        assert(S > 1);
        return v[1];
    }
    
    T z() const {
        assert(S > 2);
        return v[2];
    }

    T w() const {
        assert(S > 3);
        return v[3];
    }
            
    Vec<T,2> xy() const {
        return Vec<T,2>(x(), y());
    }

    Vec<T,2> xz() const {
        return Vec<T,2>(x(), z());
    }

    Vec<T,2> yz() const {
        return Vec<T,2>(y(), z());
    }
    
    Vec<T,3> xyz() const {
        return Vec<T,3>(x(), y(), z());
    }
            
    Vec<T,4> xyzw() const {
        return Vec<T,4>(x(), y(), z(), w());
    }
            
    Vec<T,S-1> overLast() const {
        Vec<T,S-1> result;
        for (size_t i = 0; i < S-1; ++i)
            result[i] = v[i] / v[S-1];
        return result;
    }

    Vec<T,S> roundDownToMultiple(const Vec<T,S>& m) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::roundDownToMultiple(v[i], m[i]);
        return result;
    }
    
    Vec<T,S> roundUpToMultiple(const Vec<T,S>& m) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::roundUpToMultiple(v[i], m[i]);
        return result;
    }
    
    Vec<T,S> roundToMultiple(const Vec<T,S>& m) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::roundToMultiple(v[i], m[i]);
        return result;
    }

    T dot(const Vec<T,S>& right) const {
        T result = static_cast<T>(0.0);
        for (size_t i = 0; i < S; ++i)
            result += (v[i] * right[i]);
        return result;
    }

    // projects the given distance along this (normalized) vector onto the given vector along the orthogonal of this vector
    // unlike the dot product which projects orthogonally to the other vector
    T inverseDot(const T l, const Vec<T,S>& cd) const {
        const T cos = dot(cd);
        return l / cos;
    }
    
    T length() const {
        return std::sqrt(squaredLength());
    }
    
    T squaredLength() const {
        return dot(*this);
    }
    
    T distanceTo(const Vec<T,S>& other) const {
        return (*this - other).length();
    }
    
    T squaredDistanceTo(const Vec<T,S>& other) const {
        return (*this - other).squaredLength();
    }
    
    Vec<T,S>& normalize() {
        *this /= length();
        return *this;
    }
    
    const Vec<T,S> normalized() const {
        return Vec<T,S>(*this).normalize();
    }
    
    bool isNormalized() const {
        return equals(normalized());
    }
    
    Vec<T,S> normalizeRadians() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::normalizeRadians(v[i]);
        return result;
    }
    
    Vec<T,S> normalizeDegrees() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::normalizeDegrees(v[i]);
        return result;
    }
    
    bool equals(const Vec<T,S>& other, const T epsilon = Math::Constants<T>::almostZero()) const {
        for (size_t i = 0; i < S; ++i)
            if (std::abs(v[i] - other[i]) > epsilon)
                return false;
        return true;
    }
    
    bool null() const {
        return equals(Null, Math::Constants<T>::almostZero());
    }

    void setNull() {
        for (size_t i = 0; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }
            
    void set(const T value) {
        for (size_t i = 0; i < S; ++i)
            v[i] = value;
    }
    
    bool nan() const {
        for (size_t i = 0; i < S; ++i)
            if (!Math::isnan(v[i]))
                return false;
        return true;
    }

    static bool colinear(const typename Vec<T,S>::List& points) {
        assert(points.size() == 3);
        return colinear(points[0], points[1], points[2]);
    }
    
    bool colinear(const Vec<T,S>& p2, const Vec<T,S>& p3, const T epsilon = Math::Constants<T>::colinearEpsilon()) const {
        return colinear(*this, p2, p3, epsilon);
    }

    static bool colinear(const Vec<T,S>& p1, const Vec<T,S>& p2, const Vec<T,S>& p3, const T epsilon = Math::Constants<T>::colinearEpsilon()) {
        const Vec<T,S> p1p2 = p2 - p1;
        const Vec<T,S> p2p3 = p3 - p2;
        const Vec<T,S> p1p3 = p3 - p1;
        
        return p1p3.equals(p1p2 + p2p3, epsilon);
        
        /*
        const Vec<T,S> v1 = p2 - p1;
        const Vec<T,S> v2 = p3 - p2;
        const Vec<T,S> v3 = p1 - p3;
        return v1.parallelTo(v2, epsilon) && v1.parallelTo(v3, epsilon) && v2.parallelTo(v3, epsilon);
        */
    }
    
    bool parallelTo(const Vec<T,S>& other, const T epsilon = Math::Constants<T>::colinearEpsilon()) const {
        const T d = normalized().dot(other.normalized());
        return Math::eq(std::abs(d), static_cast<T>(1.0), epsilon);
    }
    
    bool colinearTo(const Vec<T,3>& other, const T epsilon = Math::Constants<T>::colinearEpsilon()) const {
        return 1.0 - dot(other) < epsilon;
    }
    
    Vec<T,S> makePerpendicular() const {
        Vec<T,S> result;
        const T l = v[S-1];
        if (l == static_cast<T>(0.0)) {
            result[S-1] = static_cast<T>(1.0);
        } else {
            T lp = static_cast<T>(0.0);
            for (size_t i = 0; i < S-1; ++i) {
                result[i] = static_cast<T>(1.0);
                lp += v[i];
            }
            result[S-1] = lp / l;
            result.normalize();
        }
        return result;
    }
    
    int weight() const {
        return weight(v[0]) * 100 + weight(v[1]) * 10 + weight(v[2]);
    }
    
    bool hasMajorComponent(const T epsilon = Math::Constants<T>::almostZero()) const {
        if (S == 0)
            return false;
        if (S == 1)
            return true;
        
        Vec<T,S> copy(*this);
        const Math::Less<T, true> less;
        std::sort(&copy.v[0], &copy.v[S-1]+1, less);
        return less(copy[0], copy[1]);
    }
    
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
            std::push_heap(heap.begin(), heap.end(), cmp);
        }
        
        std::sort_heap(heap.begin(), heap.end(), cmp);
        return heap[S - k - 1];
    }

    const Vec<T,S> majorAxis(const size_t k) const {
        const size_t c = majorComponent(k);
        Vec<T,S> a = axis(c);
        if (v[c] < static_cast<T>(0.0))
            return -a;
        return a;
    }

    const Vec<T,S> absMajorAxis(const size_t k) const {
        const size_t c = majorComponent(k);
        return axis(c);
    }
    
    size_t firstComponent() const {
        return majorComponent(0);
    }
    
    size_t secondComponent() const {
        return majorComponent(1);
    }
    
    size_t thirdComponent() const {
        return majorComponent(2);
    }
    
    const Vec<T,3> firstAxis() const {
        return majorAxis(0);
    }
            
    const Vec<T,3> absFirstAxis() const {
        return absMajorAxis(0);
    }
    
    const Vec<T,3> secondAxis() const {
        return majorAxis(1);
    }
    
    const Vec<T,3> absSecondAxis() const {
        return absMajorAxis(1);
    }
    
    const Vec<T,3> thirdAxis() const {
        return majorAxis(2);
    }
    
    const Vec<T,3> absThirdAxis() const {
        return absMajorAxis(2);
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

    Vec<T,S>& makeAbsolute() {
        for (size_t i = 0; i < S; ++i)
            v[i] = std::abs(v[i]);
        return *this;
    }
            
    Vec<T,S> absolute() const {
        return Vec<T,S>(*this).makeAbsolute();
    }
    
    Vec<T,S> max(const Vec<T,S>& o) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = std::max(v[i], o[i]);
        return result;
    }
    
    Vec<T,S>& round() {
        for (size_t i = 0; i < S; ++i)
            v[i] = Math::round(v[i]);
        return *this;
    }
    
    Vec<T,S> rounded() const {
        return Vec<T,S>(*this).round();
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
        const Vec<T,S> edgeDir = edgeVec.normalized();
        const T dot = (*this - start).dot(edgeDir);
        
        // determine the closest point on the edge
        Vec<T,S> closestPoint;
        if (dot < 0.0)
            closestPoint = start;
        else if ((dot * dot) > edgeVec.squaredLength())
            closestPoint = end;
        else
            closestPoint = start + edgeDir * dot;

        const T distance = (*this - closestPoint).length();
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
        const Vec<T,S> dir = end - start;
        const T d = (*this - start).dot(dir);
        return Math::between(d, static_cast<T>(0.0), static_cast<T>(1.0));
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
const Vec<T,S> Vec<T,S>::PosX = Vec<T,S>::unit(0);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosY = Vec<T,S>::unit(1);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosZ = Vec<T,S>::unit(2);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegX = -Vec<T,S>::unit(0);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegY = -Vec<T,S>::unit(1);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegZ = -Vec<T,S>::unit(2);
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
template <typename T, size_t S>
const typename Vec<T,S>::Set Vec<T,S>::EmptySet = Vec<T,S>::Set();
template <typename T, size_t S>
const typename Vec<T,S>::Map Vec<T,S>::EmptyMap = Vec<T,S>::Map();

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

template <typename T, size_t S>
typename Vec<T,S>::List operator+(const typename Vec<T,S>::List& left, const Vec<T,S>& right) {
    typename Vec<T,S>::List result(left.size());
    for (size_t i = 0; i < left.size(); ++i)
        result[i] = left[i] + right;
    return result;
}

template <typename T, size_t S>
typename Vec<T,S>::List operator+(const Vec<T,S>& left, const typename Vec<T,S>::List& right) {
    return right + left;
}

template <typename T, size_t S>
Vec<T,S> operator*(const T left, const Vec<T,S>& right) {
    return Vec<T,S>(right) * left;
}

template <typename T, size_t S>
typename Vec<T,S>::List operator*(const typename Vec<T,S>::List& left, const T right) {
    typename Vec<T,S>::List result(left.size());
    for (size_t i = 0; i < left.size(); ++i)
        result[i] = left[i] * right;
    return result;
}

template <typename T, size_t S>
typename Vec<T,S>::List operator*(const T left, const typename Vec<T,S>::List& right) {
    return right * left;
}

template <typename T, size_t S>
std::ostream& operator<< (std::ostream& stream, const Vec<T,S>& vec) {
    stream << "(";
    if (S > 0) {
        stream << vec[0];
        for (size_t i = 1; i < S; ++i)
            stream << ", " << vec[i];
    }
    stream << ")";
    return stream;
}

template <typename T>
Vec<T,3>& cross(Vec<T,3>& left, const Vec<T,3>& right) {
    return left = crossed(left, right);
}

template <typename T>
const Vec<T,3> crossed(const Vec<T,3>& left, const Vec<T,3>& right) {
    return Vec<T,3>(left[1] * right[2] - left[2] * right[1],
                    left[2] * right[0] - left[0] * right[2],
                    left[0] * right[1] - left[1] * right[0]);
}

template <typename T>
T angleBetween(const Vec<T,3>& vec, const Vec<T,3>& axis, const Vec<T,3>& up) {
    // computes the CCW angle between axis and vector in relation to the given up vector
    // all vectors are expected to be normalized
    const T cos = vec.dot(axis);
    if (Math::one(+cos))
        return static_cast<T>(0.0);
    if (Math::one(-cos))
        return Math::Constants<T>::pi();
    const Vec<T,3> cross = crossed(axis, vec);
    if (!Math::neg(cross.dot(up)))
        return std::acos(cos);
    return Math::Constants<T>::twoPi() - std::acos(cos);
}

template <typename T>
bool commonPlane(const Vec<T,3>& p1, const Vec<T,3>& p2, const Vec<T,3>& p3, const Vec<T,3>& p4, const T epsilon = Math::Constants<T>::almostZero()) {
    assert(!p1.colinear(p2, p3, epsilon));
    const Vec<T,3> normal = crossed(p3 - p1, p2 - p1).normalized();
    const T offset = p1.dot(normal);
    const T dist = p4.dot(normal) - offset;
    return Math::abs(dist) < epsilon;
}

template <typename T, size_t S>
Vec<T,S> min(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::min(lhs[i], rhs[i]);
    return result;
}

template <typename T, size_t S>
Vec<T,S> max(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::max(lhs[i], rhs[i]);
    return result;
}

template <typename T, size_t S>
Vec<T,S> absMin(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::absMin(lhs[i], rhs[i]);
    return result;
}

template <typename T, size_t S>
Vec<T,S> absMax(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::absMax(lhs[i], rhs[i]);
    return result;
}

template <typename T>
Vec<T,3> crossed(const Vec<T,3>& point0, const Vec<T,3>& point1, const Vec<T,3>& point2) {
    const Vec<T,3> v1 = point2 - point0;
    const Vec<T,3> v2 = point1 - point0;
    return crossed(v1, v2);
}

template <typename T>
bool linearlyDependent(const Vec<T,3>& point0, const Vec<T,3>& point1, const Vec<T,3>& point2) {
    const Vec<T,3> normal = crossed(point0, point1, point2);
    return normal.null();
}

#endif
