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

#ifndef TRENCHBROOM_VEC_IMPL_H
#define TRENCHBROOM_VEC_IMPL_H

#include "vec_decl.h"

namespace vm {
    template <typename T, size_t S>
    vec<T,S> vec<T,S>::axis(const size_t index) {
        vec<T,S> axis;
        axis[index] = static_cast<T>(1.0);
        return axis;
    }

    template <typename T, size_t S>
    vec<T,S> vec<T,S>::fill(const T value) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = value;
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> vec<T,S>::parse(const std::string& str) {
        size_t pos = 0;
        vec<T,S> result;
        doParse(str, pos, result);
        return result;
    }

    template <typename T, size_t S>
    bool vec<T,S>::doParse(const std::string& str, size_t& pos, vec<T,S>& result) {
        static const std::string blank(" \t\n\r()");

        const char* cstr = str.c_str();
        for (size_t i = 0; i < S; ++i) {
            if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) {
                return false;
            }
            result[i] = static_cast<T>(std::atof(cstr + pos));
            if ((pos = str.find_first_of(blank, pos)) == std::string::npos) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    vec<T,S>::vec() {
        for (size_t i = 0; i < S; ++i) {
            v[i] = static_cast<T>(0.0);
        }
    }

    template <typename T, size_t S>
    vec<T,S>::vec(std::initializer_list<T> values) {
        auto it = std::begin(values);
        for (size_t i = 0; i < std::min(S, values.size()); ++i) {
            v[i] = *it++;
        }
        for (size_t i = values.size(); i < S; ++i) {
            v[i] = static_cast<T>(0.0);
        }
    }

    template <typename T, size_t S>
    T& vec<T,S>::operator[](const size_t index) {
        assert(index < S);
        return v[index];
    }

    template <typename T, size_t S>
    const T& vec<T,S>::operator[](const size_t index) const {
        assert(index < S);
        return v[index];
    }

    template <typename T, size_t S>
    T vec<T,S>::x() const {
        static_assert(S > 0);
        return v[0];
    }

    template <typename T, size_t S>
    T vec<T,S>::y() const {
        static_assert(S > 1);
        return v[1];
    }

    template <typename T, size_t S>
    T vec<T,S>::z() const {
        static_assert(S > 2);
        return v[2];
    }

    template <typename T, size_t S>
    T vec<T,S>::w() const {
        static_assert(S > 3);
        return v[3];
    }

    template <typename T, size_t S>
    vec<T,2> vec<T,S>::xy() const {
        static_assert(S > 1);
        return vec<T,2>(x(), y());
    }

    template <typename T, size_t S>
    vec<T,2> vec<T,S>::xz() const {
        static_assert(S > 1);
        return vec<T,2>(x(), z());
    }

    template <typename T, size_t S>
    vec<T,2> vec<T,S>::yz() const {
        static_assert(S > 1);
        return vec<T,2>(y(), z());
    }

    template <typename T, size_t S>
    vec<T,3> vec<T,S>::xyz() const {
        static_assert(S > 2);
        return vec<T,3>(x(), y(), z());
    }

    template <typename T, size_t S>
    vec<T,4> vec<T,S>::xyzw() const {
        static_assert(S > 3);
        return vec<T,4>(x(), y(), z(), w());
    }

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

    template <typename T, size_t S>
    int compare(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon) {
        for (size_t i = 0; i < S; ++i) {
            if (lt(lhs[i], rhs[i], epsilon))
                return -1;
            if (gt(lhs[i], rhs[i], epsilon))
                return 1;
        }
        return 0;
    }

    template <typename I>
    int compare(I lhsCur, I lhsEnd, I rhsCur, I rhsEnd, const typename I::value_type::type epsilon) {
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

    template <typename T, size_t S>
    bool isEqual(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon) {
        return compare(lhs, rhs, epsilon) == 0;
    }

    template <typename T, size_t S>
    bool operator==(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return compare(lhs, rhs) == 0;
    }

    template <typename T, size_t S>
    bool operator!=(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return compare(lhs, rhs) != 0;
    }

    template <typename T, size_t S>
    bool operator<(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return compare(lhs, rhs) < 0;
    }

    template <typename T, size_t S>
    bool operator<=(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return compare(lhs, rhs) <= 0;
    }

    template <typename T, size_t S>
    bool operator>(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return compare(lhs, rhs) > 0;
    }

    template <typename T, size_t S>
    bool operator>=(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return compare(lhs, rhs) >= 0;
    }

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

    template <typename T, size_t S>
    vec<T,S> majorAxis(const vec<T,S>& v, const size_t k) {
        const auto c = majorComponent(v, k);
        auto a = vec<T,S>::axis(c);
        if (v[c] < static_cast<T>(0.0)) {
            return -a;
        }
        return a;
    }

    template <typename T, size_t S>
    vec<T,S> absMajorAxis(const vec<T,S>& v, const size_t k) {
        const auto c = majorComponent(v, k);
        return vec<T,S>::axis(c);
    }

    template <typename T, size_t S>
    size_t firstComponent(const vec<T,S>& v) {
        return majorComponent(v, 0);
    }

    template <typename T, size_t S>
    size_t secondComponent(const vec<T,S>& v) {
        return majorComponent(v, 1);
    }

    template <typename T, size_t S>
    size_t thirdComponent(const vec<T,S>& v) {
        return majorComponent(v, 2);
    }

    template <typename T>
    vec<T,3> firstAxis(const vec<T,3>& v) {
        return majorAxis(v, 0);
    }

    template <typename T>
    vec<T,3> secondAxis(const vec<T,3>& v) {
        return majorAxis(v, 1);
    }

    template <typename T>
    vec<T,3> thirdAxis(const vec<T,3>& v) {
        return majorAxis(v, 2);
    }

    template <typename T, size_t S>
    vec<T,S> operator-(const vec<T,S>& vector) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = -vector[i];
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> operator+(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] + rhs[i];
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> operator-(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] - rhs[i];
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> operator*(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] * rhs[i];
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> operator*(const vec<T,S>& lhs, const T rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] * rhs;
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> operator*(const T lhs, const vec<T,S>& rhs) {
        return vec<T,S>(rhs) * lhs;
    }

    template <typename T, size_t S>
    vec<T,S> operator/(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] / rhs[i];
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> operator/(const vec<T,S>& lhs, const T rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = lhs[i] / rhs;
        }
        return result;
    }

    template <typename T, size_t S>
    typename vec<T,S>::List operator+(const typename vec<T,S>::List& lhs, const vec<T,S>& rhs) {
        typename vec<T,S>::List result;
        result.reserve(lhs.size());
        for (const auto& vec : lhs) {
            result.push_back(vec + rhs);
        }
        return result;
    }

    template <typename T, size_t S>
    typename vec<T,S>::List operator+(const vec<T,S>& lhs, const typename vec<T,S>::List& rhs) {
        return rhs + lhs;
    }

    template <typename T, size_t S>
    typename vec<T,S>::List operator*(const typename vec<T,S>::List& lhs, const T rhs) {
        typename vec<T,S>::List result;
        result.reserve(lhs.size());
        for (const auto& vec : lhs) {
            result.push_back(vec + rhs);
        }
        return result;
    }

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

    template <typename T, size_t S>
    vec<T,S> abs(const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = abs(v[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> min(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = min(lhs[i], rhs[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> max(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = max(lhs[i], rhs[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> absMin(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = absMin(lhs[i], rhs[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> absMax(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = absMax(lhs[i], rhs[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    T dot(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        auto result = static_cast<T>(0.0);
        for (size_t i = 0; i < S; ++i) {
            result += (lhs[i] * rhs[i]);
        }
        return result;
    }

    template <typename T>
    vec<T,3> cross(const vec<T, 3>& lhs, const vec<T, 3>& rhs) {
        return vec<T,3>(lhs[1] * rhs[2] - lhs[2] * rhs[1],
                        lhs[2] * rhs[0] - lhs[0] * rhs[2],
                        lhs[0] * rhs[1] - lhs[1] * rhs[0]);
    }

    template <typename T, size_t S>
    T squaredLength(const vec<T,S>& vec) {
        return dot(vec, vec);
    }

    template <typename T, size_t S>
    T length(const vec<T,S>& vec) {
        return std::sqrt(squaredLength(vec));
    }

    template <typename T, size_t S>
    vec<T,S> normalize(const vec<T,S>& vec) {
        return vec / length(vec);
    }

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

    template <typename T, size_t S>
    bool isUnit(const vec<T,S>& v, const T epsilon) {
        return isEqual(length(v), T(1.0), epsilon);
    }

    template <typename T, size_t S>
    bool isZero(const vec<T,S>& v, const T epsilon) {
        for (size_t i = 0; i < S; ++i) {
            if (!isZero(v[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    bool isNaN(const vec<T,S>& v) {
        for (size_t i = 0; i < S; ++i) {
            if (isNan(v[i])) {
                return true;
            }
        }
        return false;
    }

    template <typename T, size_t S>
    bool isIntegral(const vec<T,S>& v, const T epsilon) {
        for (size_t i = 0; i < S; ++i) {
            if (std::abs(v[i] - round(v[i])) >= epsilon) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    vec<T,S> mix(const vec<T,S>& lhs, const vec<T,S>& rhs, const vec<T,S>& f) {
        return (vec<T,S>::one - f) * lhs + f * rhs;
    }

    template <typename T, size_t S>
    T distance(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return length(lhs - rhs);
    }

    template <typename T, size_t S>
    T squaredDistance(const vec<T,S>& lhs, const vec<T,S>& rhs) {
        return squaredLength(lhs - rhs);
    }

    template <typename T, size_t S>
    vec<T,S+1> toHomogeneousCoords(const vec<T,S>& point) {
        return vec<T,S+1>(point, static_cast<T>(1.0));
    }

    template <typename T, size_t S>
    vec<T,S-1> toCartesianCoords(const vec<T,S>& point) {
        vec<T,S-1> result;
        for (size_t i = 0; i < S-1; ++i) {
            result[i] = point[i] / point[S-1];
        }
        return result;
    }

    template <typename T, size_t S>
    bool colinear(const vec<T,S>& a, const vec<T,S>& b, const vec<T,S>& c, const T epsilon) {
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

    template <typename T, size_t S>
    bool parallel(const vec<T,S>& lhs, const vec<T,S>& rhs, const T epsilon) {
        const T cos = dot(normalize(lhs), normalize(rhs));
        return isEqual(abs(cos), T(1.0), epsilon);
    }

    /* ========== rounding and error correction ========== */

    template <typename T, size_t S>
    vec<T,S> round(const vec<T,S>& v) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = round(v[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> snapDown(const vec<T,S>& v, const vec<T,S>& m) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = snapDown(v[i], m[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> snapUp(const vec<T,S>& v, const vec<T,S>& m) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = snapUp(v[i], m[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> snap(const vec<T,S>& v, const vec<T,S>& m) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = snap(v[i], m[i]);
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,S> correct(const vec<T,S>& v, const size_t decimals, const T epsilon) {
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = correct(v[i], decimals, epsilon);
        }
        return result;
    }

    template <typename T, size_t S>
    bool between(const vec<T,S>& p, const vec<T,S>& start, const vec<T,S>& end) {
        assert(colinear(p, start, end));
        const vec<T,S> toStart = start - p;
        const vec<T,S> toEnd   =   end - p;

        const T d = dot(toEnd, normalize(toStart));
        return !isPositive(d);
    }

    template <typename I, typename G>
    auto average(I cur, I end, const G& get) -> typename std::remove_reference<decltype(get(*cur))>::type {
        assert(cur != end);

        auto result = get(*cur++);
        auto count = 1.0;
        while (cur != end) {
            result = result + get(*cur++);
            count = count + 1.0;
        }
        return result / count;
    }

    template <typename T>
    T angleBetween(const vec<T,3>& v, const vec<T,3>& axis, const vec<T,3>& up) {
        const auto cos = dot(v, axis);
        if (isEqual(+cos, T(1.0))) {
            return static_cast<T>(0.0);
        } else if (isEqual(-cos, T(1.0))) {
            return Constants<T>::pi();
        } else {
            const auto perp = cross(axis, v);
            if (!isNegative(dot(perp, up))) {
                return std::acos(cos);
            } else {
                return Constants<T>::twoPi() - std::acos(cos);
            }
        }
    }

    template <typename T, size_t S>
    EdgeDistance<T,S>::EdgeDistance(const vec<T,S>& i_point, const T i_distance) :
    point(i_point),
    distance(i_distance) {}

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
}

#endif //TRENCHBROOM_VEC_IMPL_H
