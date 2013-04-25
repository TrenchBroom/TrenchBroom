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

#ifndef TrenchBroom_Mat2_h
#define TrenchBroom_Mat2_h

#include "Utility/Vec.h"

#include <cassert>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Mat2 {
        public:
            T v[4];
            
            Mat2() {
                for (size_t i = 0; i < 4; i++)
                    v[i] = 0.0;
            }
            
            Mat2(const T v11, const T v12,
                 const T v21, const T v22) {
                v[0] = v11; v[2] = v12;
                v[1] = v21; v[3] = v22;
            }
            
            inline Mat2<T>& operator= (const Mat2<T>& right) {
                for (size_t i = 0; i < 4; i++)
                    v[i] = right.v[i];
                return *this;
            }
            
            inline const Mat2<T> operator- () const {
                return Mat2<T>(-v[0], -v[2],
                               -v[1], -v[3]);
            }
            
            inline const Mat2<T> operator+ (const Mat2<T>& right) const {
                return Mat2<T>(v[0] + right.v[0], v[2] + right.v[2],
                               v[1] + right.v[1], v[3] + right.v[3]);
            }
            
            inline const Mat2<T> operator- (const Mat2<T>& right) const {
                return Mat2<T>(v[0] - right.v[0], v[2] - right.v[2],
                               v[1] - right.v[1], v[3] - right.v[3]);
            }
            
            inline const Mat2<T> operator* (const T right) const {
                return Mat2<T>(v[0] * right, v[2] * right,
                               v[1] * right, v[3] * right);
            }
            
            inline const Vec2f operator* (const Vec2f& right) const {
                return Vec2f(v[0] * right[0] + v[2] * right[1],
                             v[1] * right[0] + v[3] * right[1]);
            }
            
            inline const Mat2<T> operator* (const Mat2<T>& right) const {
                return Mat2<T>(v[0] * right.v[0] + v[2] * right.v[1], v[0] * right.v[2] + v[1] * right.v[3],
                               v[1] * right.v[0] + v[3] * right.v[1], v[1] * right.v[2] + v[3] * right.v[3]);
            }
            
            inline const Mat2<T> operator/ (const T right) const {
                return Mat2<T>(v[0] / right, v[2] / right,
                               v[1] / right, v[3] / right);
            }
            
            inline Mat2<T>& operator+= (const Mat2<T>& right) {
                for (size_t i = 0; i < 4; i++)
                    v[i] += right.v[i];
                return *this;
            }
            
            inline Mat2<T>& operator-= (const Mat2<T>& right) {
                for (size_t i = 0; i < 4; i++)
                    v[i] -= right.v[i];
                return *this;
            }
            
            inline Mat2<T>& operator*= (const T right) {
                for (size_t i = 0; i < 4; i++)
                    v[i] *= right;
                return *this;
            }
            
            inline Mat2<T>& operator*= (const Mat2<T>& right) {
                *this = *this * right;
                return *this;
            }
            
            inline Mat2<T>& operator/= (const T right) {
                *this *= (1.0 / right);
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index >= 0 && index < 4);
                return v[index];
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index >= 0 && index < 4);
                return v[index];
            }
            
            inline Mat2<T>& setIdentity() {
                for (size_t c = 0; c < 2; c++)
                    for (size_t r = 0; r < 2; r++)
                        v[c * 2 + r] = c == r ? 1.0 : 0.0;
                return *this;
            }
            
            inline Mat2<T>& setValue(const size_t row, const size_t col, const T value) {
                assert(row >= 0 && row < 2);
                assert(col >= 0 && col < 2);
                v[2 * col + row] = value;
                return *this;
            }
            
            inline Mat2<T>& setColumn(const size_t col, const Vec2f& values) {
                assert(col >= 0 && col < 2);
                v[col + 0] = values[0];
                v[col + 1] = values[1];
                return *this;
            }
            
            inline Mat2<T>& invert(bool& invertible) {
                const T det = determinant();
                if (det == 0.0) {
                    invertible = false;
                } else {
                    invertible = true;
                    adjugate();
                    *this /= det;
                }
                return *this;
            }
            
            inline const Mat2<T> inverted(bool& invertible) const {
                Mat2<T> result = *this;
                result.invert(invertible);
                return result;
            }
            
            inline Mat2<T>& adjugate() {
                std::swap(v[0], v[3]);
                v[1] *= -1.0;
                v[2] *= -1.0;
                return *this;
            }
            
            inline const Mat2<T> adjugated() const {
                Mat2<T> result = *this;
                result.adjugate();
                return result;
            }
            
            inline Mat2<T>& negate() {
                for (size_t i = 0; i < 4; i++)
                    v[i] = -v[i];
                return *this;
            }
            
            inline const Mat2<T> negated() const {
                return Mat2<T>(v[0] * -1, v[1] * -1,
                               v[2] * -1, v[3] * -1);
            }
            
            inline Mat2<T>& transpose() {
                for (size_t c = 0; c < 2; c++)
                    for (size_t r = c + 1; r < 2; r++)
                        std::swap(v[c * 2 + r], v[r * 2 + c]);
                return *this;
            }
            
            inline const Mat2<T> transposed() const {
                Mat2<T> result = *this;
                result.transpose();
                return result;
            }
            
            inline T determinant() const {
                return v[0] * v[3] - v[2] * v[1];
            }
        };
        
        typedef Mat2<float> Mat2f;
        
        template <typename T>
        inline Mat2<T> operator*(const T left, const Mat2<T>& right) {
            return Mat2<T>(left * right.v[0], left * right.v[1],
                           left * right.v[2], left * right.v[3]);
        }
    }
}

#endif
