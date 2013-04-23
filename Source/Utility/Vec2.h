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

#ifndef __TrenchBroom__Vec2__
#define __TrenchBroom__Vec2__

#include "Utility/Math.h"
#include "Utility/String.h"

#include <cassert>
#include <cmath>
#include <ostream>
#include <vector>

namespace TrenchBroom {
    namespace VecMath {
        
        template <typename T>
        class Vec2 {
        public:
            typedef std::vector<Vec2<T> > List;
            
            T x,y;
            
            Vec2() : x(0.0), y(0.0) {}

            Vec2(const T i_x, const T i_y) : x(i_x), y(i_y) {}
            
            Vec2(const int i_x, const int i_y) : x(static_cast<T>(i_x)), y(static_cast<T>(i_y)) {}
            
            Vec2(const T f) : x(f), y(f) {}

            inline Vec2<T>& operator= (const Vec2<T>& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                }
                return *this;
            }
            
            inline const Vec2<T> operator- () const {
                return Vec2<T>(-x, -y);
            }
            
            inline const Vec2<T> operator+ (const Vec2<T>& right) const {
                return Vec2<T>(x + right.x,
                               y + right.y);
            }
            
            inline const Vec2<T> operator- (const Vec2<T>& right) const {
                return Vec2<T>(x - right.x,
                               y - right.y);
            }
            
            inline const Vec2<T> operator* (const T right) const {
                return Vec2<T>(x * right,
                               y * right);
            }
            
            inline const Vec2<T> operator/ (const T right) const {
                return Vec2<T>(x / right,
                               y / right);
            }
            
            inline Vec2<T>& operator+= (const Vec2<T>& right) {
                x += right.x;
                y += right.y;
                return *this;
            }
            
            inline Vec2<T>& operator-= (const Vec2<T>& right) {
                x -= right.x;
                y -= right.y;
                return *this;
            }
            
            inline Vec2<T>& operator*= (const T right) {
                x *= right;
                y *= right;
                return *this;
            }
            
            inline Vec2<T>& operator/= (const T right) {
                x /= right;
                y /= right;
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index >= 0 && index < 2);
                if (index == 0) return x;
                return y;
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index >= 0 && index < 2);
                if (index == 0) return x;
                return y;
            }
            
            inline const T dot(const Vec2<T>& right) const {
                return x * right.x + y * right.y;
            }
            
            inline T length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline T lengthSquared() const {
                return this->dot(*this);
            }
            
            inline Vec2<T>& normalize() {
                const T l = length();
                x /= l;
                y /= l;
                return *this;
            }
            
            inline const Vec2<T> normalized() const {
                const T l = length();
                return Vec2<T>(x / l,
                               y / l);
            }
            
            inline Vec2<T>& round() {
                x = Math<T>::round(x);
                y = Math<T>::round(y);
                return *this;
            }
            
            inline const Vec2<T> rounded() const {
                return Vec2<T>(Math<T>::round(x), Math<T>::round(y));
            }
            
            inline bool isInteger(const T epsilon = Math<T>::AlmostZero) const {
                return (std::abs(x - Math<T>::round(x)) < epsilon &&
                        std::abs(y - Math<T>::round(y)) < epsilon);
            }
            
            inline Vec2<T>& correct(const T epsilon = Math<T>::CorrectEpsilon) {
                x = Math<T>::correct(x, epsilon);
                y = Math<T>::correct(y, epsilon);
                return *this;
            }
            
            inline const Vec2<T> corrected(const T epsilon = Math<T>::CorrectEpsilon) const {
                return Vec2<T>(Math<T>::correct(x, epsilon),
                               Math<T>::correct(y, epsilon));
            }
            
            inline bool equals(const Vec2<T>& other, const T epsilon = Math<T>::AlmostZero) const {
                return std::abs(x - other.x) < epsilon &&
                       std::abs(y - other.y) < epsilon;     
            }
            
            void write(std::ostream& str) const {
                str << x;
                str << ' ';
                str << y;
            }
            
            std::string asString() const {
                StringStream result;
                write(result);
                return result.str();
            }
        };
        
        typedef Vec2<float> Vec2f;
        
        template <typename T>
        inline Vec2<T> operator*(const T left, const Vec2<T>& right) {
            return Vec2<T>(left * right.x,
                           left * right.y);
        }
    }
}

#endif /* defined(__TrenchBroom__Vec2f__) */
