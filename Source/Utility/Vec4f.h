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

#include <cassert>
#include <cmath>
#include <ostream>
#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace Math {
        class Vec4f {
        public:
            float x, y, z, w;
            
            Vec4f() : x(0), y(0), z(0), w(0) {}
            
            Vec4f(const std::string& str) {
                const char* cstr = str.c_str();
                size_t pos = 0;
                std::string blank = " \t\n\r";
                
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
            
            Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

            bool operator== (const Vec4f& right) const {
                return x == right.x && y == right.y && z == right.z && w == right.w;
            }
            
            Vec4f& operator= (const Vec4f& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                    z = right.z;
                    w = right.w;
                }
                return *this;
            }
            
            const Vec4f operator+ (const Vec4f& right) const {
                return Vec4f(x + right.x,
                             y + right.y,
                             z + right.z,
                             w + right.w);
            }
            
            const Vec4f operator- (const Vec4f& right) const {
                return Vec4f(x - right.x,
                             y - right.y,
                             z - right.z,
                             w - right.w);
            }
            
            const Vec4f operator* (const float right) const {
                return Vec4f(x * right,
                             y * right,
                             z * right,
                             w * right);
            }
            
            const Vec4f operator/ (const float right) const {
                return Vec4f(x / right,
                             y / right,
                             z / right,
                             w / right);
            }
            
            const float operator| (const Vec4f right) const {
                return x * right.x + y * right.y + z * right.z + w * right.w;
            }
            
            Vec4f& operator+= (const Vec4f& right) {
                x += right.x;
                y += right.y;
                z += right.z;
                w += right.w;
                return *this;
            }
            
            Vec4f& operator-= (const Vec4f& right) {
                x -= right.x;
                y -= right.y;
                z -= right.z;
                w -= right.w;
                return *this;
            }
            
            Vec4f& operator*= (const float right) {
                x *= right;
                y *= right;
                z *= right;
                w *= right;
                return *this;
            }
            
            Vec4f& operator/= (const float right) {
                x /= right;
                y /= right;
                z /= right;
                w /= right;
                return *this;
            }
            
            float& operator[] (const unsigned int index) {
                assert(index >= 0 && index < 4);
                if (index == 0) return x;
                if (index == 1) return y;
                if (index == 2) return z;
                return w;
            }
            
            const float& operator[] (const unsigned int index) const {
                assert(index >= 0 && index < 4);
                if (index == 0) return x;
                if (index == 1) return y;
                if (index == 2) return z;
                return w;
            }
            
            float length() const {
                return sqrt(lengthSquared());
            }
            
            float lengthSquared() const {
                return *this | *this;
            }
            
            void normalize() {
                float l = length();
                x /= l;
                y /= l;
                z /= l;
                w /= l;
            }
            
            const Vec4f normalized() const {
                float l = length();
                return Vec4f(x / l,
                             y / l,
                             z / l,
                             w / l);
            }
            
            void correct() {
                x = Math::correct(x);
                y = Math::correct(y);
                z = Math::correct(z);
                w = Math::correct(w);
            }
            
            const Vec4f corrected() const {
                return Vec4f(Math::correct(x),
                             Math::correct(y),
                             Math::correct(z),
                             Math::correct(w));
            }
            
            bool equals(const Vec4f& other) const {
                return equals(other, Math::AlmostZero);
            }
            
            bool equals(const Vec4f& other, float delta) const {
                Vec4f diff = other - *this;
                return diff.lengthSquared() <= delta * delta;
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
            
            std::string asString() const {
                StringStream result;
                write(result);
                return result.str();
            }
        };
    }
}

#endif
