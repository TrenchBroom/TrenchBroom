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

#ifndef TrenchBroom_Vec4f_h
#define TrenchBroom_Vec4f_h

#include "Utility/Math.h"
#include "Utility/String.h"
#include "Utility/Vec3f.h"

#include <cassert>
#include <cmath>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Math {
        class Vec4f {
        public:
            static const Vec4f Null;

            float x, y, z, w;
            
            typedef std::vector<Vec4f> List;
            static const List EmptyList;
            
            Vec4f() : x(0), y(0), z(0), w(0) {}
            
            Vec4f(const String& str) {
                const char* cstr = str.c_str();
                size_t pos = 0;
                String blank = " \t\n\r";
                
                pos = str.find_first_not_of(blank, pos);
                x = static_cast<float>(atof(cstr + pos));
                pos = str.find_first_of(blank, pos);
                pos = str.find_first_not_of(blank, pos);
                y = static_cast<float>(atof(cstr + pos));
                pos = str.find_first_of(blank, pos);
                pos = str.find_first_not_of(blank, pos);
                z = static_cast<float>(atof(cstr + pos));
                pos = str.find_first_of(blank, pos);
                pos = str.find_first_not_of(blank, pos);
                w = static_cast<float>(atof(cstr + pos));
            }
            
            Vec4f(float i_x, float i_y, float i_z, float i_w) : x(i_x), y(i_y), z(i_z), w(i_w) {}

            Vec4f(Vec3f xyz, float i_w) : x(xyz.x), y(xyz.y), z(xyz.z), w(i_w) {}

            Vec4f(Vec4f xyz, float i_w) : x(xyz.x), y(xyz.y), z(xyz.z), w(i_w) {}

            Vec4f(float f) : x(f), y(f), z(f), w(f) {}

            inline bool operator== (const Vec4f& right) const {
                return x == right.x && y == right.y && z == right.z && w == right.w;
            }
            
            inline Vec4f& operator= (const Vec4f& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                    z = right.z;
                    w = right.w;
                }
                return *this;
            }
            
            inline const Vec4f operator- () const {
                return Vec4f(-x, -y, -z, -w);
            }
            
            inline const Vec4f operator+ (const Vec4f& right) const {
                return Vec4f(x + right.x,
                             y + right.y,
                             z + right.z,
                             w + right.w);
            }
            
            inline const Vec4f operator- (const Vec4f& right) const {
                return Vec4f(x - right.x,
                             y - right.y,
                             z - right.z,
                             w - right.w);
            }
            
            inline const Vec4f operator* (const float right) const {
                return Vec4f(x * right,
                             y * right,
                             z * right,
                             w * right);
            }
            
            inline const Vec4f operator/ (const float right) const {
                return Vec4f(x / right,
                             y / right,
                             z / right,
                             w / right);
            }
            
            inline Vec4f& operator+= (const Vec4f& right) {
                x += right.x;
                y += right.y;
                z += right.z;
                w += right.w;
                return *this;
            }
            
            inline Vec4f& operator-= (const Vec4f& right) {
                x -= right.x;
                y -= right.y;
                z -= right.z;
                w -= right.w;
                return *this;
            }
            
            inline Vec4f& operator*= (const float right) {
                x *= right;
                y *= right;
                z *= right;
                w *= right;
                return *this;
            }
            
            inline Vec4f& operator/= (const float right) {
                x /= right;
                y /= right;
                z /= right;
                w /= right;
                return *this;
            }
            
            inline float& operator[] (const unsigned int index) {
                assert(index >= 0 && index < 4);
                if (index == 0) return x;
                if (index == 1) return y;
                if (index == 2) return z;
                return w;
            }
            
            inline const float& operator[] (const unsigned int index) const {
                assert(index >= 0 && index < 4);
                if (index == 0) return x;
                if (index == 1) return y;
                if (index == 2) return z;
                return w;
            }
            
            inline const float dot(const Vec4f right) const {
                return x * right.x + y * right.y + z * right.z + w * right.w;
            }
            
            inline float length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline float lengthSquared() const {
                return this->dot(*this);
            }
            
            inline Vec4f& normalize() {
                float l = length();
                x /= l;
                y /= l;
                z /= l;
                w /= l;
                return *this;
            }
            
            inline const Vec4f normalized() const {
                float l = length();
                return Vec4f(x / l,
                             y / l,
                             z / l,
                             w / l);
            }
            
            inline Vec4f& correct() {
                x = Math::correct(x);
                y = Math::correct(y);
                z = Math::correct(z);
                w = Math::correct(w);
                return *this;
            }
            
            inline const Vec4f corrected() const {
                return Vec4f(Math::correct(x),
                             Math::correct(y),
                             Math::correct(z),
                             Math::correct(w));
            }
            
            inline bool equals(const Vec4f& other) const {
                return equals(other, Math::AlmostZero);
            }
            
            inline bool equals(const Vec4f& other, float delta) const {
                return std::abs(x - other.x) < delta &&
                       std::abs(y - other.y) < delta &&
                       std::abs(z - other.z) < delta &&
                       std::abs(w - other.w) < delta;
            }
            
            void write(std::ostream& str) const {
                str << x;
                str << ' ';
                str << y;
                str << ' ';
                str << z;
                str << ' ';
                str << w;
            }
            
            String asString() const {
                StringStream result;
                write(result);
                return result.str();
            }
        };
        
        inline Vec4f operator*(float left, const Vec4f& right) {
            return Vec4f(left * right.x,
                         left * right.y,
                         left * right.z,
                         left * right.w);
        }
    }
}

#endif
