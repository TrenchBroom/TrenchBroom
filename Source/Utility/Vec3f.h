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

#ifndef TrenchBroom_Vec3f_h
#define TrenchBroom_Vec3f_h

#include "Utility/Allocator.h"
#include "Utility/Math.h"
#include "Utility/String.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <ostream>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Math {
        class Vec3f : public Utility::Allocator<Vec3f> {
        public:
            static const Vec3f PosX;
            static const Vec3f PosY;
            static const Vec3f PosZ;
            static const Vec3f NegX;
            static const Vec3f NegY;
            static const Vec3f NegZ;
            static const Vec3f Null;
            static const Vec3f NaN;

            class LexicographicOrder {
            public:
                inline bool operator()(const Vec3f& lhs, const Vec3f& rhs) const {
                    if (lhs.x < rhs.x)
                        return true;
                    if (lhs.x > rhs.x)
                        return false;
                    if (lhs.y < rhs.y)
                        return true;
                    if (lhs.y > rhs.y)
                        return false;
                    if (lhs.z < rhs.z)
                        return true;
                    return false;
                }
            };

            class DotOrder {
            private:
                const Vec3f& m_dir;
            public:
                DotOrder(const Vec3f& dir) :
                m_dir(dir) {
                    assert(!m_dir.null());
                }
                
                inline bool operator()(const Vec3f& lhs, const Vec3f& rhs) const {
                    return lhs.dot(m_dir) < rhs.dot(m_dir);
                }
            };
            
            class InverseDotOrder {
            private:
                const Vec3f& m_dir;
            public:
                InverseDotOrder(const Vec3f& dir) :
                m_dir(dir) {
                    assert(!m_dir.null());
                }
                
                inline bool operator()(const Vec3f& lhs, const Vec3f& rhs) const {
                    return lhs.dot(m_dir) > rhs.dot(m_dir);
                }
            };
            
            typedef std::vector<Vec3f> List;
            static const List EmptyList;
            typedef std::set<Vec3f, LexicographicOrder> Set;
            static const Set EmptySet;
            
            float x, y, z;
            
            Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
            
            Vec3f(const std::string& str) : x(0.0f), y(0.0f), z(0.0f) {
                const char* cstr = str.c_str();
                size_t pos = 0;
                std::string blank = " \t\n\r";
                
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                x = static_cast<float>(atof(cstr + pos));
                pos = str.find_first_of(blank, pos);
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                y = static_cast<float>(atof(cstr + pos));
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                z = static_cast<float>(atof(cstr + pos));
            }
            
            Vec3f(float i_x, float i_y, float i_z) : x(i_x), y(i_y), z(i_z) {}
            
            Vec3f(const Vec3f v, float i_z) : x(v.x), y(v.y), z(i_z) {}
            
            Vec3f(float f) : x(f), y(f), z(f) {}
            
            inline bool operator== (const Vec3f& right) const {
                return x == right.x && y == right.y && z == right.z;
            }
            
            inline bool operator!= (const Vec3f& right) const {
                return !(*this == right);
            }
            
            inline Vec3f& operator= (const Vec3f& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                    z = right.z;
                }
                return *this;
            }
            
            inline const Vec3f operator- () const {
                return Vec3f(-x, -y, -z);
            }

            inline const Vec3f operator+ (const Vec3f& right) const {
                return Vec3f(x + right.x,
                             y + right.y,
                             z + right.z);
            }
            
            inline const Vec3f operator- (const Vec3f& right) const {
                return Vec3f(x - right.x,
                             y - right.y,
                             z - right.z);
            }
            
            inline const Vec3f operator* (const float right) const {
                return Vec3f(x * right,
                             y * right,
                             z * right);
            }
            
            inline const Vec3f operator/ (const float right) const {
                return Vec3f(x / right,
                             y / right,
                             z / right);
            }
            
            inline Vec3f& operator+= (const Vec3f& right) {
                x += right.x;
                y += right.y;
                z += right.z;
                return *this;
            }
            
            inline Vec3f& operator-= (const Vec3f& right) {
                x -= right.x;
                y -= right.y;
                z -= right.z;
                return *this;
            }
            
            inline Vec3f& operator*= (const float right) {
                x *= right;
                y *= right;
                z *= right;
                return *this;
            }
            
            inline Vec3f& operator/= (const float right) {
                x /= right;
                y /= right;
                z /= right;
                return *this;
            }
            
            inline float& operator[] (const unsigned int index) {
                assert(index >= 0 && index < 3);
                if (index == 0) return x;
                if (index == 1) return y;
                return z;
            }
            
            inline const float& operator[] (const unsigned int index) const {
                assert(index >= 0 && index < 3);
                if (index == 0) return x;
                if (index == 1) return y;
                return z;
            }
            
            inline const float dot(const Vec3f& right) const {
                return x * right.x + y * right.y + z * right.z;
            }
            
            inline Vec3f& cross(const Vec3f& right) {
                float xt = y * right.z - z * right.y;
                float yt = z * right.x - x * right.z;
                z = x * right.y - y * right.x;
                x = xt;
                y = yt;
                return *this;
            }
            
            inline const Vec3f crossed(const Vec3f& right) const {
                return Vec3f(y * right.z - z * right.y,
                             z * right.x - x * right.z,
                             x * right.y - y * right.x);
            }
            
            inline float length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline float lengthSquared() const {
                return this->dot(*this);
            }
            
            inline Vec3f& normalize() {
                float l = length();
                x /= l;
                y /= l;
                z /= l;
                return *this;
            }
            
            inline const Vec3f normalized() const {
                float l = length();
                return Vec3f(x / l,
                             y / l,
                             z / l);
            }
            
            inline Vec3f& correct() {
                x = Math::correct(x);
                y = Math::correct(y);
                z = Math::correct(z);
                return *this;
            }
            
            inline const Vec3f corrected() const {
                return Vec3f(Math::correct(x),
                             Math::correct(y),
                             Math::correct(z));
            }
            
            inline bool equals(const Vec3f& other) const {
                return equals(other, Math::AlmostZero);
            }
            
            inline bool equals(const Vec3f& other, float delta) const {
                return std::abs(x - other.x) < delta &&
                       std::abs(y - other.y) < delta &&
                       std::abs(z - other.z) < delta;
            }
            
            inline bool null() const {
                return equals(Null, Math::AlmostZero);
            }
            
            inline bool nan() const {
                return Math::isnan(x) || Math::isnan(y) || Math::isnan(z);
            }
            
            inline bool parallelTo(const Vec3f& other, float delta = Math::AlmostZero) const {
                Vec3f cross = this->crossed(other);
                return cross.equals(Null, delta);
            }
            
            inline float angleFrom(const Vec3f& axis, const Vec3f& up) const {
                // all vectors are expected to be normalized
                float cos = dot(axis);
                if (eq(cos, 1.0f))
                    return 0.0f;
                if (eq(cos, -1.0f))
                    return Math::Pi;
                Vec3f cross = crossed(axis);
                if (cross.dot(up) >= 0.0f)
                    return std::acos(cos);
                return 2.0f * Math::Pi - std::acos(cos);
            }
            
            inline Axis::Type firstComponent() const {
                float ax = std::abs(x);
                float ay = std::abs(y);
                float az = std::abs(z);
                if (ax >= ay && ax >= az) return Axis::AX;
                if (ay >= ax && ay >= az) return Axis::AY;
                return Axis::AZ;
            }
            
            inline Axis::Type secondComponent() const {
                float ax = std::abs(x);
                float ay = std::abs(y);
                float az = std::abs(z);
                if (ax >= ay && ax <= az) return Axis::AX;
                if (ay >= ax && ay <= az) return Axis::AY;
                return Axis::AZ;
            }
            
            inline Axis::Type thirdComponent() const {
                float ax = std::abs(x);
                float ay = std::abs(y);
                float az = std::abs(z);
                if (ax <= ay && ax <= az) return Axis::AX;
                if (ay <= ax && ay <= az) return Axis::AY;
                return Axis::AZ;
            }
            
            inline const Vec3f& firstAxis(bool pos = true) const {
                if (equals(Null)) {
                    return Null;
                } else {
                    float xa = std::abs(x);
                    float ya = std::abs(y);
                    float za = std::abs(z);
                    
                    if (xa >= ya && xa >= za) {
                        if (x > 0.0f && pos)
                            return PosX;
                        else
                            return NegX;
                    } else if (ya >= xa && ya >= za) {
                        if (y > 0.0f && pos)
                            return PosY;
                        else
                            return NegY;
                    } else {
                        if (z > 0.0f && pos)
                            return PosZ;
                        else
                            return NegZ;
                    }
                }
            }
            
            inline const Vec3f& secondAxis(bool pos = true) const {
                if (equals(Null)) {
                    return Null;
                } else {
                    float xa = std::abs(x);
                    float ya = std::abs(y);
                    float za = std::abs(z);
                    
                    if ((xa <= ya && xa >= za) ||
                        (xa >= ya && xa <= za)) {
                        if (x > 0.0f && pos)
                            return PosX;
                        else
                            return NegX;
                    } else if ((ya <= xa && ya >= za) || 
                               (ya >= xa && ya <= za)) {
                        if (y > 0.0f && pos)
                            return PosY;
                        else
                            return NegY;
                    } else {
                        if (z > 0.0f && pos)
                            return PosZ;
                        else
                            return NegZ;
                    }
                }
            }
            
            inline const Vec3f& thirdAxis(bool pos = true) const {
                if (equals(Null)) {
                    return Null;
                } else {
                    float xa = std::abs(x);
                    float ya = std::abs(y);
                    float za = std::abs(z);
                    
                    if (xa <= ya && xa <= za) {
                        if (x > 0.0f && pos)
                            return PosX;
                        else
                            return NegX;
                    } else if (ya <= xa && ya <= za) {
                        if (y > 0.0f && pos)
                            return PosY;
                        else
                            return NegY;
                    } else {
                        if (z > 0.0f && pos)
                            return PosZ;
                        else
                            return NegZ;
                    }
                }
            }
            
            void write(std::ostream& str) const {
                str << x;
                str << ' ';
                str << y;
                str << ' ';
                str << z;
            }
            
            std::string asString() const {
                StringStream result;
                write(result);
                return result.str();
            }
            
            inline Vec3f& snap() {
                snap(Math::AlmostZero);
                return *this;
            }
            
            inline Vec3f& snap(float epsilon) {
                x = Math::round(x);
                y = Math::round(y);
                z = Math::round(z);
                return *this;
            }
            
            inline const Vec3f snapped() const {
                return snapped(Math::AlmostZero);
            }
            
            inline const Vec3f snapped(float epsilon) const {
                float xr = Math::round(x);
                float yr = Math::round(y);
                float zr = Math::round(z);
                return Vec3f(Math::eq(x, xr) ? xr : x,
                             Math::eq(y, yr) ? yr : y,
                             Math::eq(z, zr) ? zr : z);
            }
            
            inline Vec3f& round() {
                x = Math::round(x);
                y = Math::round(y);
                y = Math::round(z);
                return *this;
            }
            
            inline const Vec3f rounded() const {
                return Vec3f(Math::round(x), Math::round(y), Math::round(z));
            }
            
            inline Vec3f& rotate90(Axis::Type axis, bool clockwise) {
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
            
            inline Vec3f& rotate90(Axis::Type axis, const Vec3f& center, bool clockwise) {
                *this -= center;
                rotate90(axis, clockwise);
                *this += center;
                return *this;
            }
            
            inline const Vec3f rotated90(Axis::Type axis, bool clockwise) const {
                switch (axis) {
                    case Axis::AX:
                        if (clockwise)
                            return Vec3f(x, z, -y);
                        return Vec3f(x, -z, y);
                    case Axis::AY:
                        if (clockwise)
                            return Vec3f(-z, y, x);
                        return Vec3f(z, y, -x);
                    default:
                        if (clockwise)
                            return Vec3f(y, -x, z);
                        return Vec3f(-y, x, z);
                }
            }
            
            inline const Vec3f rotated90(Axis::Type axis, const Vec3f& center, bool clockwise) const {
                Vec3f result = *this - center;
                result.rotate90(axis, clockwise);
                result += center;
                return result;
            }
            
            inline Vec3f& flip(Axis::Type axis) {
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
            
            inline Vec3f& flip(Axis::Type axis, const Vec3f& center) {
                *this -= center;
                flip(axis);
                *this += center;
                return *this;
            }
            
            inline const Vec3f flipped(Axis::Type axis) const {
                switch (axis) {
                    case Axis::AX:
                        return Vec3f(-x, y, z);
                    case Axis::AY:
                        return Vec3f(x, -y, z);
                    default:
                        return Vec3f(x, y, -z);
                }
            }
            
            inline const Vec3f flipped(Axis::Type axis, const Vec3f& center) const {
                Vec3f result = *this - center;
                result.flip(axis);
                result += center;
                return result;
            }
        };
        
        inline Vec3f operator*(float left, const Vec3f& right) {
            return Vec3f(left * right.x,
                         left * right.y,
                         left * right.z);
        }
    }
}

#endif
