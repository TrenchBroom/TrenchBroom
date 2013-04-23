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

#ifndef TrenchBroom_Vec4_h
#define TrenchBroom_Vec4_h

#include "Utility/Math.h"
#include "Utility/String.h"
#include "Utility/Vec3.h"

#include <cassert>
#include <cmath>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Vec4 {
        public:
            static const Vec4<T> Null;
            
            T x, y, z, w;
            
            typedef std::vector<Vec4<T> > List;
            
            Vec4<T>() : x(0.0), y(0.0), z(0.0), w(0.0) {}
            
            Vec4<T>(const String& str) {
                const char* cstr = str.c_str();
                size_t pos = 0;
                String blank = " \t\n\r";
                
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                x = static_cast<T>(atof(cstr + pos));
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                y = static_cast<T>(atof(cstr + pos));
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                z = static_cast<T>(atof(cstr + pos));
                if ((pos = str.find_first_of(blank, pos)) == std::string::npos) return;
                if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) return;
                w = static_cast<T>(atof(cstr + pos));
            }
            
            Vec4<T>(const T i_x, const T i_y, const T i_z, const T i_w) : x(i_x), y(i_y), z(i_z), w(i_w) {}
            
            Vec4<T>(const Vec3<T>& xyz, const T i_w) : x(xyz.x), y(xyz.y), z(xyz.z), w(i_w) {}
            
            Vec4<T>(const Vec4<T>& xyz, const T i_w) : x(xyz.x), y(xyz.y), z(xyz.z), w(i_w) {}
            
            Vec4<T>(const T xyzw) : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
            
            inline bool operator== (const Vec4<T>& right) const {
                return x == right.x && y == right.y && z == right.z && w == right.w;
            }
            
            inline Vec4<T>& operator= (const Vec4<T>& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                    z = right.z;
                    w = right.w;
                }
                return *this;
            }
            
            inline const Vec4<T> operator- () const {
                return Vec4<T>(-x, -y, -z, -w);
            }
            
            inline const Vec4<T> operator+ (const Vec4<T>& right) const {
                return Vec4<T>(x + right.x,
                               y + right.y,
                               z + right.z,
                               w + right.w);
            }
            
            inline const Vec4<T> operator- (const Vec4<T>& right) const {
                return Vec4<T>(x - right.x,
                               y - right.y,
                               z - right.z,
                               w - right.w);
            }
            
            inline const Vec4<T> operator* (const T right) const {
                return Vec4<T>(x * right,
                               y * right,
                               z * right,
                               w * right);
            }
            
            inline const Vec4<T> operator/ (const T right) const {
                return Vec4<T>(x / right,
                               y / right,
                               z / right,
                               w / right);
            }
            
            inline Vec4<T>& operator+= (const Vec4<T>& right) {
                x += right.x;
                y += right.y;
                z += right.z;
                w += right.w;
                return *this;
            }
            
            inline Vec4<T>& operator-= (const Vec4<T>& right) {
                x -= right.x;
                y -= right.y;
                z -= right.z;
                w -= right.w;
                return *this;
            }
            
            inline Vec4<T>& operator*= (const T right) {
                x *= right;
                y *= right;
                z *= right;
                w *= right;
                return *this;
            }
            
            inline Vec4<T>& operator/= (const T right) {
                x /= right;
                y /= right;
                z /= right;
                w /= right;
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index >= 0 && index < 4);
                if (index == 0) return x;
                if (index == 1) return y;
                if (index == 2) return z;
                return w;
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index >= 0 && index < 4);
                if (index == 0) return x;
                if (index == 1) return y;
                if (index == 2) return z;
                return w;
            }
            
            inline const T dot(const Vec4<T> right) const {
                return x * right.x + y * right.y + z * right.z + w * right.w;
            }
            
            inline T length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline T lengthSquared() const {
                return this->dot(*this);
            }
            
            inline Vec4<T>& normalize() {
                const T l = length();
                x /= l;
                y /= l;
                z /= l;
                w /= l;
                return *this;
            }
            
            inline const Vec4<T> normalized() const {
                const T l = length();
                return Vec4<T>(x / l,
                               y / l,
                               z / l,
                               w / l);
            }
            
            inline Vec4<T>& correct(const T epsilon = Math<T>::CorrectEpsilon) {
                x = Math<T>::correct(x, epsilon);
                y = Math<T>::correct(y, epsilon);
                z = Math<T>::correct(z, epsilon);
                w = Math<T>::correct(w, epsilon);
                return *this;
            }
            
            inline const Vec4<T> corrected(const T epsilon = Math<T>::CorrectEpsilon) const {
                return Vec4<T>(Math<T>::correct(x, epsilon),
                               Math<T>::correct(y, epsilon),
                               Math<T>::correct(z, epsilon),
                               Math<T>::correct(w, epsilon));
            }
            
            inline bool equals(const Vec4<T>& other, const T epsilon = Math<T>::AlmostZero) const {
                return std::abs(x - other.x) < epsilon &&
                std::abs(y - other.y) < epsilon &&
                std::abs(z - other.z) < epsilon &&
                std::abs(w - other.w) < epsilon;
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
        
        template <typename T>
        const Vec4<T> Vec4<T>::Null = Vec4<T>(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0));

        typedef Vec4<float> Vec4f;
        
        template <typename T>
        inline Vec4<T> operator*(const T left, const Vec4<T>& right) {
            return Vec4<T>(left * right.x,
                           left * right.y,
                           left * right.z,
                           left * right.w);
        }
    }
}

#endif
