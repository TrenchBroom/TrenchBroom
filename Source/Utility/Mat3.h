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

#ifndef TrenchBroom_Mat3_h
#define TrenchBroom_Mat3_h

#include "Utility/Mat2.h"
#include "Utility/Vec.h"

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Mat3 {
        public:
            static const Mat3<T> Null;
            static const Mat3<T> Identity;
            static const Mat3<T> YIQToRGB;
            static const Mat3<T> RGBToYIQ;
            
            T v[9];
            
            Mat3() {
                for (size_t i = 0; i < 9; i++)
                    v[i] = 0;
            }
            
            Mat3(const T v11, const T v12, const T v13,
                 const T v21, const T v22, const T v23,
                 const T v31, const T v32, const T v33) {
                v[0] = v11; v[3] = v12; v[6] = v13;
                v[1] = v21; v[4] = v22; v[7] = v23;
                v[2] = v31; v[5] = v32; v[8] = v33;
            }
            
            inline Mat3<T>& operator= (const Mat3<T>& right) {
                for (size_t i = 0; i < 9; i++)
                    v[i] = right.v[i];
                return *this;
            }
            
            inline const Mat3<T> operator- () const {
                return Mat3<T>(-v[0], -v[3], -v[6],
                               -v[1], -v[4], -v[7],
                               -v[2], -v[5], -v[8]);
            }
            
            inline const Mat3<T> operator+ (const Mat3<T>& right) const {
                return Mat3<T>(v[0] + right.v[0], v[3] + right.v[3], v[6] + right.v[6],
                               v[1] + right.v[1], v[4] + right.v[4], v[7] + right.v[7],
                               v[2] + right.v[2], v[5] + right.v[5], v[8] + right.v[8]);
            }
            
            inline const Mat3<T> operator- (const Mat3<T>& right) const {
                return Mat3<T>(v[0] - right.v[0], v[3] - right.v[3], v[6] - right.v[6],
                               v[1] - right.v[1], v[4] - right.v[4], v[7] - right.v[7],
                               v[2] - right.v[2], v[5] - right.v[5], v[8] - right.v[8]);
            }
            
            inline const Mat3<T> operator* (const T right) const {
                return Mat3<T>(v[0] * right, v[3] * right, v[6] * right,
                               v[1] * right, v[4] * right, v[7] * right,
                               v[2] * right, v[5] * right, v[8] * right);
            }
            
            inline const Vec3f operator* (const Vec3f& right) const {
                return Vec3f(v[0] * right[0] + v[3] * right[1] + v[6] * right[2],
                             v[1] * right[0] + v[4] * right[1] + v[7] * right[2],
                             v[2] * right[0] + v[5] * right[1] + v[8] * right[2]);
            }
            
            inline const Mat3<T> operator* (const Mat3<T>& right) const {
                Mat3<T> result;
                for (size_t c = 0; c < 3; c++)
                    for (size_t r = 0; r < 3; r++)
                        for (size_t i = 0; i < 3; i++)
                            result[c * 3 + r] += v[i * 3 + r] * right.v[c * 3 + i];
                return result;
            }
            
            inline const Mat3<T> operator/ (const T right) const {
                return Mat3<T>(v[0] / right, v[3] / right, v[6] / right,
                               v[1] / right, v[4] / right, v[7] / right,
                               v[2] / right, v[5] / right, v[8] / right);
            }
            
            inline Mat3<T>& operator+= (const Mat3<T>& right) {
                for (size_t i = 0; i < 9; i++)
                    v[i] += right.v[i];
                return *this;
            }
            
            inline Mat3<T>& operator-= (const Mat3<T>& right) {
                for (size_t i = 0; i < 9; i++)
                    v[i] -= right.v[i];
                return *this;
            }
            
            inline Mat3<T>& operator*= (const T right) {
                for (size_t i = 0; i < 9; i++)
                    v[i] *= right;
                return *this;
            }
            
            inline Mat3<T>& operator*= (const Mat3<T>& right) {
                *this = *this * right;
                return *this;
            }
            
            inline Mat3<T>& operator/= (const T right) {
                *this *= (1 / right);
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index >= 0 && index < 9);
                return v[index];
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index >= 0 && index < 9);
                return v[index];
            }
            
            inline Mat3<T>& setIdentity() {
                for (size_t c = 0; c < 3; c++)
                    for (size_t r = 0; r < 3; r++)
                        v[c * 3 + r] = c == r ? 1.0f : 0.0f;
                return *this;
            }
            
            inline Mat3<T>& setValue(const size_t row, const size_t col, const T value) {
                assert(row >= 0 && row < 3);
                assert(col >= 0 && col < 3);
                v[col * 3 + row] = value;
                return *this;
            }
            
            inline Mat3<T>& setColumn(const size_t col, const Vec3f& values) {
                v[col + 0] = values[0];
                v[col + 1] = values[1];
                v[col + 2] = values[2];
                return *this;
            }
            
            Mat3<T>& invert(bool& invertible) {
                const T det = determinant();
                if (det == 0.0f) {
                    invertible = false;
                } else {
                    invertible = true;
                    adjugate();
                    *this /= det;
                }
                return *this;
            }
            
            const Mat3<T> inverted(bool& invertible) const {
                Mat3<T> result = *this;
                result.invert(invertible);
                return result;
            }
            
            Mat3<T>& adjugate() {
                *this = adjugated();
                return *this;
            }
            
            const Mat3<T> adjugated() const {
                Mat3<T> result;
                for (size_t c = 0; c < 3; c++)
                    for (size_t r = 0; r < 3; r++)
                        result[c * 3 + r] = ((c + r) % 2 == 0 ? 1 : -1) * subMatrix(c, r).determinant();
                return result;
            }
            
            Mat3<T>& negate() {
                for (size_t i = 0; i < 9; i++)
                    v[i] = -v[i];
                return *this;
            }
            
            const Mat3<T> negated() const {
                return Mat3<T>(v[0] * - 1, v[1] * - 1, v[2] * - 1,
                               v[3] * - 1, v[4] * - 1, v[5] * - 1,
                               v[6] * - 1, v[7] * - 1, v[8] * - 1);
            }
            
            Mat3<T>& transpose() {
                for (size_t c = 0; c < 3; c++)
                    for (size_t r = c + 1; r < 3; r++)
                        std::swap(v[c * 3 + r], v[r * 3 + c]);
                return *this;
            }
            
            const Mat3<T> transposed() const {
                Mat3<T> result = *this;
                result.transpose();
                return result;
            }
            
            T determinant() const {
                return (+ v[0] * v[4] * v[8]
                        + v[3] * v[7] * v[2]
                        + v[6] * v[1] * v[5]
                        - v[2] * v[4] * v[6]
                        - v[5] * v[7] * v[0]
                        - v[8] * v[1] * v[3]);
            }
            
            const Mat2f subMatrix(const size_t row, const size_t col) const {
                Mat2f result;
                size_t i = 0;
                for (size_t c = 0; c < 3; c++)
                    for (size_t r = 0; r < 3; r++)
                        if (c != col && r != row)
                            result[i++] = v[c * 3 + r];
                return result;
            }
        };
        
        template <typename T>
        const Mat3<T> Mat3<T>::Null         = Mat3<T>(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0),
                                                      static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0),
                                                      static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0));
        template <typename T>
        const Mat3<T> Mat3<T>::Identity     = Mat3<T>(static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0),
                                                      static_cast<T>(0.0), static_cast<T>(1.0), static_cast<T>(0.0),
                                                      static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0));
        template <typename T>
        const Mat3<T> Mat3<T>::YIQToRGB     = Mat3<T>(static_cast<T>(1.0), static_cast<T>( 0.9563), static_cast<T> (0.6210),
                                                      static_cast<T>(1.0), static_cast<T>(-0.2721), static_cast<T>(-0.6474),
                                                      static_cast<T>(1.0), static_cast<T>(-1.1070), static_cast<T>( 1.7046));
        template <typename T>
        const Mat3<T> Mat3<T>::RGBToYIQ     = Mat3<T>(static_cast<T>(0.299),    static_cast<T>( 0.587),    static_cast<T>( 0.114),
                                                      static_cast<T>(0.595716), static_cast<T>(-0.274453), static_cast<T>(-0.321263),
                                                      static_cast<T>(0.211456), static_cast<T>(-0.522591), static_cast<T>( 0.311135));

        typedef Mat3<float> Mat3f;
    
        template <typename T>
        inline Mat3<T> operator*(const T left, const Mat3<T>& right) {
            return Mat3<T>(left * right.v[0], left * right.v[1], left * right.v[2],
                           left * right.v[3], left * right.v[4], left * right.v[5],
                           left * right.v[6], left * right.v[7], left * right.v[8]);
        }
    }
}

#endif
