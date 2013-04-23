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

#ifndef TrenchBroom_Vec3_h
#define TrenchBroom_Vec3_h

#include "Utility/Allocator.h"
#include "Utility/Math.h"
#include "Utility/String.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <map>
#include <ostream>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Vec3 : public Utility::Allocator<Vec3<T> > {
        private:
            inline int weight(T v) const {
                if (std::abs(v - 1.0) < static_cast<T>(0.9f))
                    return 0;
                if (std::abs(v + 1.0) < static_cast<T>(0.9f))
                    return 1;
                return 2;
            }
        public:
            static const Vec3<T> PosX;
            static const Vec3<T> PosY;
            static const Vec3<T> PosZ;
            static const Vec3<T> NegX;
            static const Vec3<T> NegY;
            static const Vec3<T> NegZ;
            static const Vec3<T> Null;
            static const Vec3<T> NaN;

            class LexicographicOrder {
            public:
                inline bool operator()(const Vec3<T>& lhs, const Vec3<T>& rhs) const {
                    if (Math<T>::lt(lhs.x, rhs.x))
                        return true;
                    if (Math<T>::gt(lhs.x, rhs.x))
                        return false;
                    if (Math<T>::lt(lhs.y, rhs.y))
                        return true;
                    if (Math<T>::gt(lhs.y, rhs.y))
                        return false;
                    if (Math<T>::lt(lhs.z, rhs.z))
                        return true;
                    return false;
                }
            };

            class ErrorOrder {
            public:
                inline bool operator()(const Vec3<T>& lhs, const Vec3<T>& rhs) const {
                    const T lErr = (lhs - lhs.rounded()).lengthSquared();
                    const T rErr = (rhs - rhs.rounded()).lengthSquared();
                    return lErr < rErr;
                }
            };
            
            class DotOrder {
            private:
                const Vec3<T>& m_dir;
            public:
                DotOrder(const Vec3<T>& dir) :
                m_dir(dir) {
                    assert(!m_dir.null());
                }
                
                inline bool operator()(const Vec3<T>& lhs, const Vec3<T>& rhs) const {
                    return lhs.dot(m_dir) < rhs.dot(m_dir);
                }
            };
            
            class InverseDotOrder {
            private:
                const Vec3<T>& m_dir;
            public:
                InverseDotOrder(const Vec3<T>& dir) :
                m_dir(dir) {
                    assert(!m_dir.null());
                }
                
                inline bool operator()(const Vec3<T>& lhs, const Vec3<T>& rhs) const {
                    return lhs.dot(m_dir) > rhs.dot(m_dir);
                }
            };
            
            typedef std::vector<Vec3<T> > List;
            typedef std::set<Vec3<T>, LexicographicOrder> Set;
            typedef std::map<Vec3<T>, Vec3<T>, LexicographicOrder> Map;
            
            T x, y, z;
            
            Vec3() : x(0.0), y(0.0), z(0.0) {}
            
            Vec3(const std::string& str) : x(0.0), y(0.0), z(0.0) {
                const char* cstr = str.c_str();
                size_t pos = 0;
                std::string blank = " \t\n\r";
                
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                x = static_cast<T>(atof(cstr + pos));
                pos = str.find_first_of(blank, pos);
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                y = static_cast<T>(atof(cstr + pos));
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                z = static_cast<T>(atof(cstr + pos));
            }
            
            Vec3(const T i_x, const T i_y, const T i_z) : x(i_x), y(i_y), z(i_z) {}
            
            Vec3(const Vec3<T>& v, const T i_z) : x(v.x), y(v.y), z(i_z) {}
            
            Vec3(const T xyz) : x(xyz), y(xyz), z(xyz) {}
            
            inline bool operator== (const Vec3<T>& right) const {
                return x == right.x && y == right.y && z == right.z;
                // return Math<T>::relEq(x, right.x, 0.0001f) && Math<T>::relEq(y, right.y, 0.0001f) && Math<T>::relEq(z, right.z, 0.0001f);
            }
            
            inline bool operator!= (const Vec3<T>& right) const {
                return !(*this == right);
            }
            
            inline Vec3<T>& operator= (const Vec3<T>& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                    z = right.z;
                }
                return *this;
            }
            
            inline const Vec3<T> operator- () const {
                return Vec3<T>(-x, -y, -z);
            }

            inline const Vec3<T> operator+ (const Vec3<T>& right) const {
                return Vec3<T>(x + right.x,
                               y + right.y,
                               z + right.z);
            }
            
            inline const Vec3<T> operator- (const Vec3<T>& right) const {
                return Vec3<T>(x - right.x,
                               y - right.y,
                               z - right.z);
            }
            
            inline const Vec3<T> operator* (const T right) const {
                return Vec3<T>(x * right,
                               y * right,
                               z * right);
            }
            
            inline const Vec3<T> operator/ (const T right) const {
                return Vec3<T>(x / right,
                               y / right,
                               z / right);
            }
            
            inline Vec3<T>& operator+= (const Vec3<T>& right) {
                x += right.x;
                y += right.y;
                z += right.z;
                return *this;
            }
            
            inline Vec3<T>& operator-= (const Vec3<T>& right) {
                x -= right.x;
                y -= right.y;
                z -= right.z;
                return *this;
            }
            
            inline Vec3<T>& operator*= (const T right) {
                x *= right;
                y *= right;
                z *= right;
                return *this;
            }
            
            inline Vec3<T>& operator/= (const T right) {
                x /= right;
                y /= right;
                z /= right;
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index >= 0 && index < 3);
                if (index == 0) return x;
                if (index == 1) return y;
                return z;
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index >= 0 && index < 3);
                if (index == 0) return x;
                if (index == 1) return y;
                return z;
            }
            
            inline const T dot(const Vec3<T>& right) const {
                return x * right.x + y * right.y + z * right.z;
            }
            
            inline Vec3<T>& cross(const Vec3<T>& right) {
                T xt = y * right.z - z * right.y;
                T yt = z * right.x - x * right.z;
                z = x * right.y - y * right.x;
                x = xt;
                y = yt;
                return *this;
            }
            
            inline const Vec3<T> crossed(const Vec3<T>& right) const {
                return Vec3<T>(y * right.z - z * right.y,
                               z * right.x - x * right.z,
                               x * right.y - y * right.x);
            }
            
            inline T length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline T lengthSquared() const {
                return this->dot(*this);
            }
            
            inline T distanceTo(const Vec3<T>& other) const {
                return (*this - other).length();
            }
            
            inline T squaredDistanceTo(const Vec3<T>& other) const {
                return (*this - other).lengthSquared();
            }
            
            inline Vec3<T>& normalize() {
                const T l = length();
                x /= l;
                y /= l;
                z /= l;
                return *this;
            }
            
            inline const Vec3<T> normalized() const {
                const T l = length();
                return Vec3<T>(x / l,
                               y / l,
                               z / l);
            }
            
            inline bool equals(const Vec3<T>& other, const T delta = Math<T>::AlmostZero) const {
                const T xd = std::abs(x - other.x);
                const T yd = std::abs(y - other.y);
                const T zd = std::abs(z - other.z);
                return xd <= delta &&
                       yd <= delta &&
                       zd <= delta;
            }
            
            inline bool null() const {
                return equals(Null, Math<T>::AlmostZero);
            }
            
            inline bool nan() const {
                return Math<T>::isnan(x) || Math<T>::isnan(y) || Math<T>::isnan(z);
            }
            
            inline bool parallelTo(const Vec3<T>& other, const T delta = Math<T>::ColinearEpsilon) const {
                const Vec3<T> cross = this->crossed(other);
                return cross.equals(Null, delta);
            }
            
            inline T angleFrom(const Vec3<T>& axis, const Vec3<T>& up) const {
                // all vectors are expected to be normalized
                const T cos = dot(axis);
                if (Math<T>::eq(cos, 1.0))
                    return 0.0;
                if (Math<T>::eq(cos, -1.0))
                    return Math<T>::Pi;
                const Vec3<T> cross = crossed(axis);
                if (cross.dot(up) >= 0.0)
                    return std::acos(cos);
                return static_cast<T>(2.0) * Math<T>::Pi - std::acos(cos);
            }
            
            inline int weight() const {
                return weight(x) * 100 + weight(y) * 10 + weight(z);
            }
            
            inline Axis::Type firstComponent() const {
                const T ax = std::abs(x);
                const T ay = std::abs(y);
                const T az = std::abs(z);
                if (ax >= ay && ax >= az)
                    return Axis::AX;
                if (ay >= ax && ay >= az)
                    return Axis::AY;
                return Axis::AZ;
            }
            
            inline Axis::Type secondComponent() const {
                const T ax = std::abs(x);
                const T ay = std::abs(y);
                const T az = std::abs(z);
                if (ax >= ay && ax <= az)
                    return Axis::AX;
                if (ay >= ax && ay <= az)
                    return Axis::AY;
                return Axis::AZ;
            }
            
            inline Axis::Type thirdComponent() const {
                const T ax = std::abs(x);
                const T ay = std::abs(y);
                const T az = std::abs(z);
                if (ax <= ay && ax <= az)
                    return Axis::AX;
                if (ay <= ax && ay <= az)
                    return Axis::AY;
                return Axis::AZ;
            }
            
            inline const Vec3<T>& firstAxis(const bool abs = false) const {
                if (equals(Null)) {
                    return Null;
                } else {
                    const T xa = std::abs(x);
                    const T ya = std::abs(y);
                    const T za = std::abs(z);
                    
                    if (xa >= ya && xa >= za) {
                        if (x > 0.0 || abs)
                            return PosX;
                        else
                            return NegX;
                    } else if (ya >= xa && ya >= za) {
                        if (y > 0.0 || abs)
                            return PosY;
                        else
                            return NegY;
                    } else {
                        if (z > 0.0 || abs)
                            return PosZ;
                        else
                            return NegZ;
                    }
                }
            }
            
            inline const Vec3<T>& secondAxis(const bool abs = false) const {
                if (equals(Null)) {
                    return Null;
                } else {
                    const T xa = std::abs(x);
                    const T ya = std::abs(y);
                    const T za = std::abs(z);
                    
                    if ((xa <= ya && xa >= za) ||
                        (xa >= ya && xa <= za)) {
                        if (x > 0.0 || abs)
                            return PosX;
                        else
                            return NegX;
                    } else if ((ya <= xa && ya >= za) || 
                               (ya >= xa && ya <= za)) {
                        if (y > 0.0 || abs)
                            return PosY;
                        else
                            return NegY;
                    } else {
                        if (z > 0.0 || abs)
                            return PosZ;
                        else
                            return NegZ;
                    }
                }
            }
            
            inline const Vec3<T>& thirdAxis(const bool abs = false) const {
                if (equals(Null)) {
                    return Null;
                } else {
                    const T xa = std::abs(x);
                    const T ya = std::abs(y);
                    const T za = std::abs(z);
                    
                    if (xa <= ya && xa <= za) {
                        if (x > 0.0 || abs)
                            return PosX;
                        else
                            return NegX;
                    } else if (ya <= xa && ya <= za) {
                        if (y > 0.0 || abs)
                            return PosY;
                        else
                            return NegY;
                    } else {
                        if (z > 0.0 || abs)
                            return PosZ;
                        else
                            return NegZ;
                    }
                }
            }
            
            void write(std::ostream& str) const {
                str << x << ' ' << y << ' ' << z;
            }
            
            std::string asString() const {
                StringStream result;
                write(result);
                return result.str();
            }
            
            inline Vec3<T>& round() {
                x = Math<T>::round(x);
                y = Math<T>::round(y);
                y = Math<T>::round(z);
                return *this;
            }
            
            inline const Vec3<T> rounded() const {
                return Vec3<T>(Math<T>::round(x), Math<T>::round(y), Math<T>::round(z));
            }
            
            inline bool isInteger(const T epsilon = Math<T>::AlmostZero) const {
                return (std::abs(x - Math<T>::round(x)) < epsilon &&
                        std::abs(y - Math<T>::round(y)) < epsilon &&
                        std::abs(z - Math<T>::round(z)) < epsilon);
            }
            
            inline Vec3<T>& correct(const T epsilon = Math<T>::CorrectEpsilon) {
                x = Math<T>::correct(x, epsilon);
                y = Math<T>::correct(y, epsilon);
                z = Math<T>::correct(z, epsilon);
                return *this;
            }
            
            inline const Vec3<T> corrected(const T epsilon = Math<T>::CorrectEpsilon) const {
                return Vec3<T>(Math<T>::correct(x, epsilon),
                               Math<T>::correct(y, epsilon),
                               Math<T>::correct(z, epsilon));
            }
            
            inline Vec3<T>& rotate90(const Axis::Type axis, const bool clockwise) {
                switch (axis) {
                    case Axis::AX:
                        if (clockwise) {
                            float t = y;
                            y = z;
                            z = -t;
                        } else {
                            float t = y;
                            y = -z;
                            z = t;
                        }
                        break;
                    case Axis::AY:
                        if (clockwise) {
                            float t = x;
                            x = -z;
                            z = t;
                        } else {
                            float t = x;
                            x = z;
                            z = -t;
                        }
                        break;
                    default:
                        if (clockwise) {
                            float t = x;
                            x = y;
                            y = -t;
                        } else {
                            float t = x;
                            x = -y;
                            y = t;
                        }
                        break;
                }
                return *this;
            }
            
            inline Vec3<T>& rotate90(const Axis::Type axis, const Vec3<T>& center, const bool clockwise) {
                *this -= center;
                rotate90(axis, clockwise);
                *this += center;
                return *this;
            }
            
            inline const Vec3<T> rotated90(const Axis::Type axis, const bool clockwise) const {
                switch (axis) {
                    case Axis::AX:
                        if (clockwise)
                            return Vec3<T>(x, z, -y);
                        return Vec3<T>(x, -z, y);
                    case Axis::AY:
                        if (clockwise)
                            return Vec3<T>(-z, y, x);
                        return Vec3<T>(z, y, -x);
                    default:
                        if (clockwise)
                            return Vec3<T>(y, -x, z);
                        return Vec3<T>(-y, x, z);
                }
            }
            
            inline const Vec3<T> rotated90(const Axis::Type axis, const Vec3<T>& center, const bool clockwise) const {
                Vec3<T> result = *this - center;
                result.rotate90(axis, clockwise);
                result += center;
                return result;
            }

            inline Vec3<T>& flip(const Axis::Type axis) {
                switch (axis) {
                    case Axis::AX:
                        x = -x;
                        break;
                    case Axis::AY:
                        y = -y;
                        break;
                    default:
                        z = -z;
                        break;
                }
                return *this;
            }
            
            inline Vec3<T>& flip(const Axis::Type axis, const Vec3<T>& center) {
                *this -= center;
                flip(axis);
                *this += center;
                return *this;
            }
            
            inline const Vec3<T> flipped(const Axis::Type axis) const {
                switch (axis) {
                    case Axis::AX:
                        return Vec3<T>(-x, y, z);
                    case Axis::AY:
                        return Vec3<T>(x, -y, z);
                    default:
                        return Vec3<T>(x, y, -z);
                }
            }
            
            inline const Vec3<T> flipped(const Axis::Type axis, const Vec3<T>& center) const {
                Vec3<T> result = *this - center;
                result.flip(axis);
                result += center;
                return result;
            }
        };

        template <typename T>
        const Vec3<T> Vec3<T>::PosX = Vec3<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T>
        const Vec3<T> Vec3<T>::PosY = Vec3<T>( static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0));
        template <typename T>
        const Vec3<T> Vec3<T>::PosZ = Vec3<T>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Vec3<T> Vec3<T>::NegX = Vec3<T>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T>
        const Vec3<T> Vec3<T>::NegY = Vec3<T>( static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0));
        template <typename T>
        const Vec3<T> Vec3<T>::NegZ = Vec3<T>( static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0));
        template <typename T>
        const Vec3<T> Vec3<T>::Null = Vec3<T>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T>
        const Vec3<T> Vec3<T>::NaN  = Vec3<T>(std::numeric_limits<T>::quiet_NaN(),
                                              std::numeric_limits<T>::quiet_NaN(),
                                              std::numeric_limits<T>::quiet_NaN());

        typedef Vec3<float> Vec3f;
                    
        template <typename T>
        inline Vec3<T> operator*(const T left, const Vec3<T>& right) {
            return Vec3<T>(left * right.x,
                           left * right.y,
                           left * right.z);
        }
    }
}

#endif
