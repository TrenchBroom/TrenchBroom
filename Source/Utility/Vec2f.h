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

#ifndef __TrenchBroom__Vec2f__
#define __TrenchBroom__Vec2f__

#include "Utility/Math.h"
#include "Utility/String.h"

#include <cassert>
#include <cmath>
#include <ostream>
#include <vector>

namespace TrenchBroom {
    namespace Math {
        
        class Vec2f {
        public:
            typedef std::vector<Vec2f> List;
            
            float x,y;
            
            Vec2f() : x(0), y(0) {}

            Vec2f(float i_x, float i_y) : x(i_x), y(i_y) {}
            
            Vec2f(int i_x, int i_y) : x(static_cast<float>(i_x)), y(static_cast<float>(i_y)) {}
            
            Vec2f(float f) : x(f), y(f) {}

            inline Vec2f& operator= (const Vec2f& right) {
                if (this != &right) {
                    x = right.x;
                    y = right.y;
                }
                return *this;
            }
            
            inline const Vec2f operator- () const {
                return Vec2f(-x, -y);
            }
            
            inline const Vec2f operator+ (const Vec2f& right) const {
                return Vec2f(x + right.x,
                             y + right.y);
            }
            
            inline const Vec2f operator- (const Vec2f& right) const {
                return Vec2f(x - right.x,
                             y - right.y);
            }
            
            inline const Vec2f operator* (const float right) const {
                return Vec2f(x * right,
                             y * right);
            }
            
            inline const Vec2f operator/ (const float right) const {
                return Vec2f(x / right,
                             y / right);
            }
            
            inline Vec2f& operator+= (const Vec2f& right) {
                x += right.x;
                y += right.y;
                return *this;
            }
            
            inline Vec2f& operator-= (const Vec2f& right) {
                x -= right.x;
                y -= right.y;
                return *this;
            }
            
            inline Vec2f& operator*= (const float right) {
                x *= right;
                y *= right;
                return *this;
            }
            
            inline Vec2f& operator/= (const float right) {
                x /= right;
                y /= right;
                return *this;
            }
            
            inline float& operator[] (const size_t index) {
                assert(index >= 0 && index < 2);
                if (index == 0) return x;
                return y;
            }
            
            inline const float& operator[] (const size_t index) const {
                assert(index >= 0 && index < 2);
                if (index == 0) return x;
                return y;
            }
            
            inline const float dot(const Vec2f& right) const {
                return x * right.x + y * right.y;
            }
            
            inline float length() const {
                return std::sqrt(lengthSquared());
            }
            
            inline float lengthSquared() const {
                return this->dot(*this);
            }
            
            inline Vec2f& normalize() {
                float l = length();
                x /= l;
                y /= l;
                return *this;
            }
            
            inline const Vec2f normalized() const {
                float l = length();
                return Vec2f(x / l,
                             y / l);
            }
            
            inline Vec2f& round() {
                x = Math::round(x);
                y = Math::round(y);
                return *this;
            }
            
            inline const Vec2f rounded() const {
                return Vec2f(Math::round(x), Math::round(y));
            }
            
            inline bool isInteger(float epsilon = Math::AlmostZero) const {
                return (std::abs(x - Math::round(x)) < epsilon &&
                        std::abs(y - Math::round(y)) < epsilon);
            }
            
            inline Vec2f& correct(float epsilon = Math::CorrectEpsilon) {
                x = Math::correct(x, epsilon);
                y = Math::correct(y, epsilon);
                return *this;
            }
            
            inline const Vec2f corrected(float epsilon = Math::CorrectEpsilon) const {
                return Vec2f(Math::correct(x, epsilon),
                             Math::correct(y, epsilon));
            }
            
            inline bool equals(const Vec2f& other, float delta = Math::AlmostZero) const {
                return std::abs(x - other.x) < delta &&
                       std::abs(y - other.y) < delta;     
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
        
        inline Vec2f operator*(float left, const Vec2f& right) {
            return Vec2f(left * right.x,
                         left * right.y);
        }
    }
}

#endif /* defined(__TrenchBroom__Vec2f__) */
