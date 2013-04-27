/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Vec_h
#define TrenchBroom_Vec_h

#include "Utility/Allocator.h"
#include "Utility/Math.h"
#include "Utility/String.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <map>
#include <ostream>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T, size_t S>
        class Vec : public Utility::Allocator<Vec<T,S> > {
        private:
            class SelectionHeapCmp {
            private:
                const Vec<T,S>& m_vec;
                bool m_abs;
            public:
                SelectionHeapCmp(const Vec<T,S>& vec, const bool abs) :
                m_vec(vec),
                m_abs(abs) {}
                
                inline bool operator()(size_t lhs, size_t rhs) const {
                    assert(lhs < S);
                    assert(rhs < S);
                    if (m_abs)
                        return std::abs(m_vec.v[lhs]) < std::abs(m_vec.v[rhs]);
                    return m_vec.v[lhs] < m_vec.v[rhs];
                }
            };
            
            inline int weight(T c) const {
                if (std::abs(c - static_cast<T>(1.0)) < static_cast<T>(0.9))
                    return 0;
                if (std::abs(c + static_cast<T>(1.0)) < static_cast<T>(0.9))
                    return 1;
                return 2;
            }
        public:
            static const Vec<T,S> PosX;
            static const Vec<T,S> PosY;
            static const Vec<T,S> PosZ;
            static const Vec<T,S> NegX;
            static const Vec<T,S> NegY;
            static const Vec<T,S> NegZ;
            static const Vec<T,S> Null;
            static const Vec<T,S> NaN;
            
            class LexicographicOrder {
            public:
                inline bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
                    for (size_t i = 0; i < S; i++) {
                        if (Math<T>::lt(lhs[i], rhs[i]))
                            return true;
                        if (Math<T>::gt(lhs[i], rhs[i]))
                            return false;
                    }
                    return false;
                }
            };

            class ErrorOrder {
            public:
                inline bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
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
                
                inline bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
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
                
                inline bool operator()(const Vec<T,S>& lhs, const Vec<T,S>& rhs) const {
                    return lhs.dot(m_dir) > rhs.dot(m_dir);
                }
            };
            
            typedef std::vector<Vec<T,S> > List;
            typedef std::set<Vec<T,S>, LexicographicOrder> Set;
            typedef std::map<Vec<T,S>, Vec<T,S>, LexicographicOrder> Map;
            
        public:
            inline static const Vec<T,S> axis(const size_t index) {
                Vec<T,S> axis;
                axis[index] = static_cast<T>(1.0);
                return axis;
            }
            
            T v[S];
            
            Vec() {
                setNull();
            }
            
            Vec(const std::string& str) {
                setNull();
                
                const char* cstr = str.c_str();
                size_t pos = 0;
                std::string blank = " \t\n\r";
                
                for (size_t i = 0; i < S; i++) {
                    if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos)
                        return;
                    v[i] = static_cast<T>(std::atof(cstr + pos));
                    if ((pos = str.find_first_of(blank, pos)) == std::string::npos)
                        return;
                }
            }
            
            Vec(const T i_x) {
                for (size_t i = 0; i < S; i++)
                    v[i] = i_x;
            }
                    
            Vec(const T i_x, const T i_y) {
                assert(S > 1);
                v[0] = i_x;
                v[1] = i_y;
                for (size_t i = 2; i < S; i++)
                    v[i] = static_cast<T>(0.0);
            }
            
            Vec(const T i_x, const T i_y, const T i_z) {
                assert(S > 2);
                v[0] = i_x;
                v[1] = i_y;
                v[2] = i_z;
                for (size_t i = 3; i < S; i++)
                    v[i] = static_cast<T>(0.0);
            }
            
            Vec(const T i_x, const T i_y, const T i_z, const T i_w) {
                assert(S > 3);
                v[0] = i_x;
                v[1] = i_y;
                v[2] = i_z;
                v[3] = i_w;
                for (size_t i = 4; i < S; i++)
                    v[i] = static_cast<T>(0.0);
            }
                    
            Vec(const Vec<T,S+1>& vec) {
                for (size_t i = 0; i < S; i++)
                    v[i] = vec[i] / vec[S];
            }

            Vec(const Vec<T,S-1>& vec, const T last) {
                for (size_t i = 0; i < S-1; i++)
                    v[i] = vec[i];
                v[S-1] = last;
            }
                    
            Vec(const Vec<T,S-2>& vec, const T oneButLast, const T last) {
                for (size_t i = 0; i < S-2; i++)
                    v[i] = vec[i];
                v[S-2] = oneButLast;
                v[S-1] = last;
            }
            
            Vec(const Vec<T,S>& right) {
                for (size_t i = 0; i < S; i++)
                    v[i] = right[i];
            }
                    
            inline bool operator== (const Vec<T,S>& right) const {
                for (size_t i = 0; i < S; i++)
                    if (v[i] != right[i])
                        return false;
                return true;
            }
            
            inline bool operator!= (const Vec<T,S>& right) const {
                return !(*this == right);
            }
            
            inline Vec<T,S>& operator= (const Vec<T,S>& right) {
                if (this != &right)
                    for (size_t i = 0; i < S; i++)
                        v[i] = right[i];
                return *this;
            }
            
            inline Vec<T,S>& operator= (const Vec<T,S-1>& right) {
                for (size_t i = 0; i < S-1; i++)
                    v[i] = right[i];
                v[S-1] = static_cast<T>(0.0);
                return *this;
            }
            
            inline const Vec<T,S> operator- () const {
                Vec<T,S> result;
                for (size_t i = 0; i < S; i++)
                    result[i] = -v[i];
                return result;
            }

            inline const Vec<T,S> operator+ (const Vec<T,S>& right) const {
                Vec<T,S> result;
                for (size_t i = 0; i < S; i++)
                    result[i] = v[i] + right[i];
                return result;
            }
            
            inline const Vec<T,S> operator- (const Vec<T,S>& right) const {
                Vec<T,S> result;
                for (size_t i = 0; i < S; i++)
                    result[i] = v[i] - right[i];
                return result;
            }
            
            inline const Vec<T,S> operator* (const T right) const {
                Vec<T,S> result;
                for (size_t i = 0; i < S; i++)
                    result[i] = v[i] * right;
                return result;
            }
            
            inline const Vec<T,S> operator/ (const T right) const {
                Vec<T,S> result;
                for (size_t i = 0; i < S; i++)
                    result[i] = v[i] / right;
                return result;
            }
            
            inline Vec<T,S>& operator+= (const Vec<T,S>& right) {
                for (size_t i = 0; i < S; i++)
                    v[i] += right[i];
                return *this;
            }
            
            inline Vec<T,S>& operator-= (const Vec<T,S>& right) {
                for (size_t i = 0; i < S; i++)
                    v[i] -= right[i];
                return *this;
            }
            
            inline Vec<T,S>& operator*= (const T right) {
                for (size_t i = 0; i < S; i++)
                    v[i] *= right;
                return *this;
            }
            
            inline Vec<T,S>& operator/= (const T right) {
                for (size_t i = 0; i < S; i++)
                    v[i] /= right;
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index < S);
                return v[index];
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index < S);
                return v[index];
            }
            
            inline T x() const {
                assert(S > 0);
                return v[0];
            }
                    
            inline T y() const {
                assert(S > 1);
                return v[1];
            }
            
            inline T z() const {
                assert(S > 2);
                return v[2];
            }

            inline T w() const {
                assert(S > 3);
                return v[3];
            }

            inline const T dot(const Vec<T,S>& right) const {
                T result = static_cast<T>(0.0);
                for (size_t i = 0; i < S; i++)
                    result += (v[i] * right[i]);
                return result;
            }
            
            inline T length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline T lengthSquared() const {
                return dot(*this);
            }
            
            inline T distanceTo(const Vec<T,S>& other) const {
                return (*this - other).length();
            }
            
            inline T squaredDistanceTo(const Vec<T,S>& other) const {
                return (*this - other).lengthSquared();
            }
            
            inline Vec<T,S>& normalize() {
                *this /= length();
                return *this;
            }
            
            inline const Vec<T,S> normalized() const {
                return Vec<T,S>(*this).normalize();
            }
            
            inline bool equals(const Vec<T,S>& other, const T epsilon = Math<T>::AlmostZero) const {
                for (size_t i = 0; i < S; i++)
                    if (std::abs(v[i] - other[i]) > epsilon)
                        return false;
                return true;
            }
            
            inline bool null() const {
                return equals(Null, Math<T>::AlmostZero);
            }
            
            inline void setNull() {
                for (size_t i = 0; i < S; i++)
                    v[i] = static_cast<T>(0.0);
            }
            
            inline bool nan() const {
                for (size_t i = 0; i < S; i++)
                    if (!Math<T>::isnan(v[i]))
                        return false;
                return true;
            }
            
            inline bool parallelTo(const Vec<T,S>& other, const T epsilon = Math<T>::ColinearEpsilon) const {
                return std::abs(dot(other)) <= epsilon;
            }
            
            inline int weight() const {
                return weight(v[0]) * 100 + weight(v[1]) * 10 + weight(v[2]);
            }
                    
            inline size_t majorComponent(const size_t k) const {
                assert(k < S);
                
                if (k == 0) {
                    size_t index = 0;
                    for (size_t i = 1; i < S; i++) {
                        if (std::abs(v[i]) > std::abs(v[index]))
                            index = i;
                    }
                    return index;
                }
                
                // simple selection algorithm
                // we store the indices of the values in heap
                SelectionHeapCmp cmp(*this, true);
                std::vector<size_t> heap;
                for (size_t i = 0; i < S; i++) {
                    heap.push_back(i);
                    std::push_heap(heap.begin(), heap.end(), cmp);
                }
                
                std::sort_heap(heap.begin(), heap.end(), cmp);
                return heap[S - k - 1];
            }

            inline const Vec<T,S> majorAxis(const bool abs, const size_t k) const {
                const size_t c = majorComponent(k);
                Vec<T,S> a = axis(c);
                if (!abs && v[c] < static_cast<T>(0.0))
                    return -a;
                return a;
            }

            inline size_t firstComponent() const {
                return majorComponent(0);
            }
            
            inline size_t secondComponent() const {
                return majorComponent(1);
            }
            
            inline size_t thirdComponent() const {
                return majorComponent(2);
            }
            
            inline const Vec<T,3> firstAxis(const bool abs = false) const {
                return majorAxis(abs, 0);
            }
            
            inline const Vec<T,3> secondAxis(const bool abs = false) const {
                return majorAxis(abs, 1);
            }
            
            inline const Vec<T,3> thirdAxis(const bool abs = false) const {
                return majorAxis(abs, 2);
            }
            
            void write(std::ostream& str) const {
                for (size_t i = 0; i < S; i++) {
                    str << v[i];
                    if (i < S - 1)
                        str << ' ';
                }
            }
            
            std::string asString() const {
                StringStream result;
                write(result);
                return result.str();
            }
                    
            inline Vec<T,S>& makeAbsolute() {
                for (size_t i = 0; i < S; i++)
                    v[i] = std::abs(v[i]);
                return *this;
            }
                    
            inline Vec<T,S> absolute() const {
                return Vec<T,S>(*this).makeAbsolute();
            }
            
            inline Vec<T,S>& round() {
                for (size_t i = 0; i < S; i++)
                    v[i] = Math<T>::round(v[i]);
                return *this;
            }
            
            inline const Vec<T,S> rounded() const {
                return Vec<T,S>(*this).round();
            }
            
            inline bool isInteger(const T epsilon = Math<T>::AlmostZero) const {
                for (size_t i = 0; i < S; i++)
                    if (std::abs(v[i] - Math<T>::round(v[i])) > epsilon)
                        return false;
                return true;
            }
            
            inline Vec<T,S>& correct(const T epsilon = Math<T>::CorrectEpsilon) {
                for (size_t i = 0; i < S; i++)
                    v[i] = Math<T>::correct(v[i], epsilon);
                return *this;
            }
            
            inline const Vec<T,S> corrected(const T epsilon = Math<T>::CorrectEpsilon) const {
                return Vec<T,S>(*this).correct();
            }
        };
                    
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::PosX = Vec<T,S>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::PosY = Vec<T,S>( static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::PosZ = Vec<T,S>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::NegX = Vec<T,S>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::NegY = Vec<T,S>( static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::NegZ = Vec<T,S>( static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::Null = Vec<T,S>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T, size_t S>
        const Vec<T,S> Vec<T,S>::NaN  = Vec<T,S>(std::numeric_limits<T>::quiet_NaN(),
                                                 std::numeric_limits<T>::quiet_NaN(),
                                                 std::numeric_limits<T>::quiet_NaN());
        
        typedef Vec<float,2> Vec2f;
        typedef Vec<float,3> Vec3f;
        typedef Vec<float,4> Vec4f;
        typedef Vec<double,2> Vec2d;
        typedef Vec<double,3> Vec3d;
        typedef Vec<double,4> Vec4d;
                    
        template <typename T, size_t S>
        inline Vec<T,S> operator*(const T left, const Vec<T,S>& right) {
            return Vec<T,S>(right) * left;
        }

        template <typename T>
        inline Vec<T,3>& cross(Vec<T,3>& left, const Vec<T,3>& right) {
            return left = crossed(left, right);
        }
        
        template <typename T>
        inline const Vec<T,3> crossed(const Vec<T,3>& left, const Vec<T,3>& right) {
            return Vec<T,3>(left[1] * right[2] - left[2] * right[1],
                            left[2] * right[0] - left[0] * right[2],
                            left[0] * right[1] - left[1] * right[0]);
        }
        
        template <typename T>
        inline T angleFrom(const Vec<T,3> vec, const Vec<T,3>& axis, const Vec<T,3>& up) {
            // all vectors are expected to be normalized
            const T cos = vec.dot(axis);
            if (Math<T>::eq(cos, static_cast<T>(1.0)))
                return static_cast<T>(0.0);
            if (Math<T>::eq(cos, static_cast<T>(-1.0)))
                return Math<T>::Pi;
            const Vec<T,3> cross = crossed(vec, axis);
            if (cross.dot(up) >= static_cast<T>(0.0))
                return std::acos(cos);
            return static_cast<T>(2.0) * Math<T>::Pi - std::acos(cos);
        }
    }
}

#endif
