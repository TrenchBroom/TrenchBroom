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

#ifndef TrenchBroom_Mat3f_h
#define TrenchBroom_Mat3f_h

#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class Mat3f {
        public:
            float v[9];

            Mat3f() {
                for (unsigned int i = 0; i < 9; i++)
                    v[i] = 0;
            }
            
            Mat3f(float v11, float v12, float v13,
                  float v21, float v22, float v23,
                  float v31, float v32, float v33) {
                v[0] = v11; v[3] = v12; v[6] = v13;
                v[1] = v21; v[4] = v22; v[7] = v23;
                v[2] = v31; v[5] = v32; v[8] = v33;
            }
            
            inline Mat3f& operator= (const Mat3f& right) {
                for (unsigned int i = 0; i < 9; i++)
                    v[i] = right.v[i];
                return *this;
            }
            
            inline const Mat3f operator+ (const Mat3f& right) const {
                return Mat3f(v[0] + right.v[0], v[3] + right.v[3], v[6] + right.v[6],
                             v[1] + right.v[1], v[4] + right.v[4], v[7] + right.v[7],
                             v[2] + right.v[2], v[5] + right.v[5], v[8] + right.v[8]);
            }
            
            inline const Mat3f operator- (const Mat3f& right) const {
                return Mat3f(v[0] - right.v[0], v[3] - right.v[3], v[6] - right.v[6],
                             v[1] - right.v[1], v[4] - right.v[4], v[7] - right.v[7],
                             v[2] - right.v[2], v[5] - right.v[5], v[8] - right.v[8]);
            }
            
            inline const Mat3f operator* (const float right) const {
                return Mat3f(v[0] * right, v[3] * right, v[6] * right,
                             v[1] * right, v[4] * right, v[7] * right,
                             v[2] * right, v[5] * right, v[8] * right);
            }
            
            inline const Vec3f operator* (const Vec3f& right) const {
                return Vec3f(v[0] * right.x + v[3] * right.y + v[6] * right.z,
                             v[1] * right.x + v[4] * right.y + v[7] * right.z,
                             v[2] * right.x + v[5] * right.y + v[8] * right.z);
            }
            
            inline const Mat3f operator* (const Mat3f& right) const {
                Mat3f result;
                for (unsigned int c = 0; c < 3; c++)
                    for (unsigned int r = 0; r < 3; r++)
                        for (unsigned int i = 0; i < 3; i++)
                            result[c * 3 + r] += v[i * 3 + r] * right.v[c * 3 + i];
                return result;
            }
            
            inline const Mat3f operator/ (const float right) const {
                return Mat3f(v[0] / right, v[3] / right, v[6] / right,
                             v[1] / right, v[4] / right, v[7] / right,
                             v[2] / right, v[5] / right, v[8] / right);
            }
            
            inline Mat3f& operator+= (const Mat3f& right) {
                for (unsigned int i = 0; i < 9; i++)
                    v[i] += right.v[i];
                return *this;
            }
            
            inline Mat3f& operator-= (const Mat3f& right) {
                for (unsigned int i = 0; i < 9; i++)
                    v[i] -= right.v[i];
                return *this;
            }
            
            inline Mat3f& operator*= (const float right) {
                for (unsigned int i = 0; i < 9; i++)
                    v[i] *= right;
                return *this;
            }
            
            inline Mat3f& operator*= (const Mat3f& right) {
                *this = *this * right;
                return *this;
            }
            
            inline Mat3f& operator/= (const float right) {
                *this *= (1 / right);
                return *this;
            }
            
            inline float& operator[] (const unsigned int index) {
                assert(index >= 0 && index < 9);
                return v[index];
            }
            
            inline const float& operator[] (const unsigned int index) const {
                assert(index >= 0 && index < 9);
                return v[index];
            }
            
            inline void setIdentity() {
                for (unsigned int c = 0; c < 3; c++)
                    for (unsigned int r = 0; r < 3; r++)
                        v[c * 3 + r] = c == r ? 1.0f : 0.0f;
            }
            
            inline void setValue(unsigned int row, unsigned int col, float value) {
                assert(row >= 0 && row < 3);
                assert(col >= 0 && col < 3);
                v[col * 3 + row] = value;
            }
            
            inline void setColumn(unsigned int col, const Vec3f& values) {
                v[col + 0] = values.x;
                v[col + 1] = values.y;
                v[col + 2] = values.z;
            }
            
            void invert(bool& invertible) {
                float det = determinant();
                if (det == 0.0f) {
                    invertible = false;
                } else {
                    invertible = true;
                    adjugate();
                    *this /= det;
                }
            }

            const Mat3f inverted(bool& invertible) const {
                Mat3f result = *this;
                result.invert(invertible);
                return result;
            }
            
            void adjugate() {
                *this = adjugated();
            }

            const Mat3f adjugated() const {
                Mat3f result;
                for (unsigned int c = 0; c < 3; c++)
                    for (unsigned int r = 0; r < 3; r++)
                        result[c * 3 + r] = ((c + r) % 2 == 0 ? 1 : -1) * subMatrix(c, r).determinant();
                return result;
            }
            
            void negate() {
                for (unsigned int i = 0; i < 9; i++)
                    v[i] = -v[i];
            }
            
            const Mat3f negated() const {
                return Mat3f(v[0] * - 1, v[1] * - 1, v[2] * - 1,
                             v[3] * - 1, v[4] * - 1, v[5] * - 1,
                             v[6] * - 1, v[7] * - 1, v[8] * - 1);
            }
            
            void transpose() {
                for (unsigned int c = 0; c < 3; c++)
                    for (unsigned int r = c + 1; r < 3; r++)
                        std::swap(v[c * 3 + r], v[r * 3 + c]);
            }

            const Mat3f transposed() const {
                Mat3f result = *this;
                result.transpose();
                return result;
            }
            
            float determinant() const {
                return v[0] * v[4] * v[8]
                     + v[3] * v[7] * v[2]
                     + v[6] * v[1] * v[5]
                     - v[2] * v[4] * v[6]
                     - v[5] * v[7] * v[0]
                     - v[8] * v[1] * v[3];
            }
            
            const Mat2f subMatrix(unsigned int row, unsigned int col) const {
                Mat2f result;
                unsigned int i = 0;
                for (unsigned int c = 0; c < 3; c++)
                    for (unsigned int r = 0; r < 3; r++)
                        if (c != col && r != row)
                            result[i++] = v[c * 3 + r];
                return result;
            }
        };

        inline Mat3f operator*(float left, const Mat3f& right) {
            return Mat3f(left * right.v[0], left * right.v[1], left * right.v[2],
                         left * right.v[3], left * right.v[4], left * right.v[5],
                         left * right.v[6], left * right.v[7], left * right.v[8]);
        }
    }
}

#endif
