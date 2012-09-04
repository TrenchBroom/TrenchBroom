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

#ifndef TrenchBroom_Mat4f_h
#define TrenchBroom_Mat4f_h

#include "Utility/Mat2f.h"
#include "Utility/Mat3f.h"
#include "Utility/Quat.h"
#include "Utility/Vec3f.h"
#include "Utility/Vec4f.h"

namespace TrenchBroom {
    namespace Math {
        class Mat4f {
        public:
            static const Mat4f Identity;
            static const Mat4f Rot90XCW;
            static const Mat4f Rot90YCW;
            static const Mat4f Rot90ZCW;
            static const Mat4f Rot90XCCW;
            static const Mat4f Rot90YCCW;
            static const Mat4f Rot90ZCCW;
            static const Mat4f MirX;
            static const Mat4f MirY;
            static const Mat4f MirZ;
            
            float v[16];
            
            Mat4f() {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] = 0;
            }
            
            Mat4f(float v11, float v12, float v13, float v14, float v21, float v22, float v23, float v24, float v31, float v32, float v33, float v34, float v41, float v42, float v43, float v44) {
                v[ 0] = v11; v[ 4] = v12; v[ 8] = v13; v[12] = v14;
                v[ 1] = v21; v[ 5] = v22; v[ 9] = v23; v[13] = v24;
                v[ 2] = v31; v[ 6] = v32; v[10] = v33; v[14] = v34;
                v[ 3] = v41; v[ 7] = v42; v[11] = v43; v[15] = v44;
            }
            
            Mat4f& operator= (const Mat4f& right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] = right.v[i];
                return *this;
            }
            
            const Mat4f operator+ (const Mat4f& right) const {
                return Mat4f(v[ 0] + right.v[ 0], v[ 1] + right.v[ 1], v[ 2] + right.v[ 2], v[ 3] + right.v[ 3],
                             v[ 4] + right.v[ 4], v[ 5] + right.v[ 5], v[ 6] + right.v[ 6], v[ 7] + right.v[ 7],
                             v[ 8] + right.v[ 8], v[ 9] + right.v[ 9], v[10] + right.v[10], v[11] + right.v[11],
                             v[12] + right.v[12], v[13] + right.v[13], v[14] + right.v[14], v[15] + right.v[15]);
            }
            
            const Mat4f operator- (const Mat4f& right) const {
                return Mat4f(v[ 0] - right.v[ 0], v[ 1] - right.v[ 1], v[ 2] - right.v[ 2], v[ 3] - right.v[ 3],
                             v[ 4] - right.v[ 4], v[ 5] - right.v[ 5], v[ 6] - right.v[ 6], v[ 7] - right.v[ 7],
                             v[ 8] - right.v[ 8], v[ 9] - right.v[ 9], v[10] - right.v[10], v[11] - right.v[11],
                             v[12] - right.v[12], v[13] - right.v[13], v[14] - right.v[14], v[15] - right.v[15]);
            }
            
            const Mat4f operator* (const float right) const {
                return Mat4f(v[ 0] * right, v[ 1] * right, v[ 2] * right, v[ 3] * right,
                             v[ 4] * right, v[ 5] * right, v[ 6] * right, v[ 7] * right,
                             v[ 8] * right, v[ 9] * right, v[10] * right, v[11] * right,
                             v[12] * right, v[13] * right, v[14] * right, v[15] * right);
            }
            
            const Vec3f operator* (const Vec3f& right) const {
                float w = v[ 3] * right.x + v[ 7] * right.y + v[11] * right.z + v[15];
                return Vec3f((v[ 0] * right.x + v[ 4] * right.y + v[ 8] * right.z + v[12]) / w,
                             (v[ 1] * right.x + v[ 5] * right.y + v[ 9] * right.z + v[13]) / w,
                             (v[ 2] * right.x + v[ 6] * right.y + v[10] * right.z + v[14]) / w);
            }
            
            const Vec4f operator* (const Vec4f& right) const {
                return Vec4f(v[ 0] * right.x + v[ 4] * right.y + v[ 8] * right.z + v[12] * right.w,
                             v[ 1] * right.x + v[ 5] * right.y + v[ 9] * right.z + v[13] * right.w,
                             v[ 2] * right.x + v[ 6] * right.y + v[10] * right.z + v[14] * right.w,
                             v[ 3] * right.x + v[ 7] * right.y + v[11] * right.z + v[15] * right.w);
            }
            
            const Mat4f operator* (const Mat4f& right) const {
                Mat4f result;
                for (unsigned int c = 0; c < 4; c++) {
                    for (unsigned int r = 0; r < 4; r++) {
                        result[c * 4 + r] = 0;
                        for (unsigned int i = 0; i < 4; i++)
                            result[c * 4 + r] += v[i * 4 + r] * right.v[c * 4 + i];
                    }
                }
                return result;
            }
            
            const Mat4f operator/ (const float right) const {
                Mat4f result = *this;
                return result /= right;
            }
            
            Mat4f& operator+= (const Mat4f& right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] += right.v[i];
                return *this;
            }
            
            Mat4f& operator-= (const Mat4f& right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] -= right.v[i];
                return *this;
            }
            
            Mat4f& operator*= (const float right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] *= right;
                return *this;
            }
            
            Mat4f& operator*= (const Mat4f& right) {
                *this = *this * right;
                return *this;
            }
            
            Mat4f& operator/= (const float right) {
                *this *= (1.0f / right);
                return *this;
            }
            
            float& operator[] (const unsigned int index) {
                assert(index >= 0 && index < 16);
                return v[index];
            }
            
            const float& operator[] (const unsigned int index) const {
                assert(index >= 0 && index < 16);
                return v[index];
            }
            
            void setIdentity() {
                for (unsigned int r = 0; r < 4; r++)
                    for (unsigned int c = 0; c < 4; c++)
                        v[c * 4 + r] = r == c ? 1.0f : 0.0f;
            }
            
            void setValue(unsigned int row, unsigned int col, float value) {
                assert(row >= 0 && row < 4);
                assert(col >= 0 && col < 4);
                v[col * 4 + row] = value;
            }
            
            void setColumn(unsigned int col, const Vec3f& values) {
                assert(col >= 0 && col < 4);
                v[col * 4 + 0] = values.x;
                v[col * 4 + 1] = values.y;
                v[col * 4 + 2] = values.z;
                v[col * 4 + 3] = 0;
            }
            
            void setColumn(unsigned int col, const Vec4f& values) {
                assert(col >= 0 && col < 4);
                v[col * 4 + 0] = values.x;
                v[col * 4 + 1] = values.y;
                v[col * 4 + 2] = values.z;
                v[col * 4 + 3] = values.w;
            }
            
            void setSubMatrix(int index, const Mat2f& values) {
                switch (index) {
                    case 0:
                        v[ 0] = values.v[0];
                        v[ 1] = values.v[1];
                        v[ 4] = values.v[2];
                        v[ 5] = values.v[3];
                        break;
                    case 1:
                        v[ 2] = values.v[0];
                        v[ 3] = values.v[1];
                        v[ 6] = values.v[2];
                        v[ 7] = values.v[3];
                        break;
                    case 2:
                        v[ 8] = values.v[0];
                        v[ 9] = values.v[1];
                        v[12] = values.v[2];
                        v[13] = values.v[3];
                        break;
                    case 3:
                        v[10] = values.v[0];
                        v[11] = values.v[1];
                        v[14] = values.v[2];
                        v[15] = values.v[3];
                        break;
                    default:
                        break;
                }
            }
            
            const Mat2f subMatrix(int index) const {
                Mat2f result;
                switch (index) {
                    case 0:
                        result[0] = v[0];
                        result[1] = v[1];
                        result[2] = v[4];
                        result[3] = v[5];
                        break;
                    case 1:
                        result[0] = v[2];
                        result[1] = v[3];
                        result[2] = v[6];
                        result[3] = v[7];
                        break;
                    case 2:
                        result[0] = v[8];
                        result[1] = v[9];
                        result[2] = v[12];
                        result[3] = v[13];
                        break;
                    case 3:
                        result[0] = v[10];
                        result[1] = v[11];
                        result[2] = v[14];
                        result[3] = v[15];
                        break;
                    default:
                        break;
                }
                return result;
            }
            
            void invert(bool& invertible) {
                float det = determinant();
                invertible = det != 0.0f;
                if (invertible) {
                    Mat2f A, Ai;
                    bool invertibleA;
                    
                    A = subMatrix(0);
                    Ai = A.inverted(invertibleA);
                    if (invertibleA) { // use quick method
                        Mat4f result;
                        Mat2f B, C, D, CAi, CAiB, AiB;
                        bool invertibleD;
                        
                        B = subMatrix(2);
                        C = subMatrix(1);
                        D = subMatrix(3);
                        
                        CAi = C * Ai;
                        CAiB = CAi * B;
                        AiB = Ai * B;
                        
                        // calculate D
                        D = (D - CAiB).inverted(invertibleD);
                        
                        // calculate -C and -B
                        C = (D * CAi).negated();
                        B = AiB * D;
                        A = (B * CAi * Ai).negated();
                        
                        setSubMatrix(0, A);
                        setSubMatrix(1, C);
                        setSubMatrix(2, B);
                        setSubMatrix(3, D);
                    } else {
                        adjugate();
                        *this /= det;
                    }
                }
            }
            
            const Mat4f inverted(bool& invertible) const {
                Mat4f result = *this;
                result.invert(invertible);
                return result;
            }
            
            void adjugate() {
                *this = adjugated();
            }
            
            const Mat4f adjugated() const {
                Mat4f result = *this;
                for (unsigned int c = 0; c < 4; c++)
                    for (unsigned int r = 0; r < 4; r++)
                        result[c * 4 + r] = ((c + r) % 2 == 0 ? 1 : -1) * subMatrix(c, r).determinant();
                result.transpose();
                return result;
            }
            
            void negate() {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] = -v[i];
            }
            
            const Mat4f negated() const {
                return Mat4f(v[ 0] * -1, v[ 1] * -1, v[ 2] * -1, v[ 3] * -1,
                             v[ 4] * -1, v[ 5] * -1, v[ 6] * -1, v[ 7] * -1,
                             v[ 8] * -1, v[ 9] * -1, v[10] * -1, v[11] * -1,
                             v[12] * -1, v[13] * -1, v[14] * -1, v[15] * -1);
            }
            
            void transpose() {
                for (unsigned int c = 0; c < 4; c++)
                    for (unsigned int r = c + 1; r < 4; r++)
                        std::swap(v[c * 4 + r], v[r * 4 + c]);
            }
            
            const Mat4f transposed() const {
                Mat4f result = *this;
                result.transpose();
                return result;
            }
            
            float determinant() const {
                // Laplace after first col
                float det = 0;
                for (unsigned int r = 0; r < 4; r++)
                    det += (r % 2 == 0 ? 1 : -1) *v[r] * subMatrix(0, r).determinant();
                return det;
            }
            
            const Mat3f subMatrix(unsigned int row, unsigned int col) const {
                Mat3f result;
                unsigned int i = 0;
                for (unsigned int c = 0; c < 4; c++)
                    for (unsigned int r = 0; r < 4; r++)
                        if (c != col && r != row)
                            result[i++] = v[c * 4 + r];
                return result;
            }
            
            void rotate(float angle, const Vec3f& axis) {
                float s = sinf(angle);
                float c = cosf(angle);
                float i = 1 - c;
                
                float ix  = i  * axis.x;
                float ix2 = ix * axis.x;
                float ixy = ix * axis.y;
                float ixz = ix * axis.z;
                
                float iy  = i  * axis.y;
                float iy2 = iy * axis.y;
                float iyz = iy * axis.z;
                
                float iz2 = i  * axis.z * axis.z;
                
                float sx = s * axis.x;
                float sy = s * axis.y;
                float sz = s * axis.z;
                
                Mat4f temp;
                temp[ 0] = ix2 + c;
                temp[ 1] = ixy - sz;
                temp[ 2] = ixz + sy;
                temp[ 3] = 0;
                
                temp[ 4] = ixy + sz;
                temp[ 5] = iy2 + c;
                temp[ 6] = iyz - sx;
                temp[ 7] = 0;
                
                temp[ 8] = ixz - sy;
                temp[ 9] = iyz + sx;
                temp[10] = iz2 + c;
                temp[11] = 0;
                
                temp[12] = 0;
                temp[13] = 0;
                temp[14] = 0;
                temp[15] = 1;
                
                *this *= temp;
            }
            
            const Mat4f rotated(float angle, const Vec3f& axis) const {
                Mat4f result = *this;
                result.rotate(angle, axis);
                return result;
            }

            void rotate(const Quat& rotation) {
                float a = rotation.s;
                float b = rotation.v.x;
                float c = rotation.v.y;
                float d = rotation.v.z;
                
                float a2 = a * a;
                float b2 = b * b;
                float c2 = c * c;
                float d2 = d * d;
                
                Mat4f temp;
                temp[ 0] = a2 + b2 - c2 - d2;
                temp[ 1] = 2 * b * c + 2 * a * d;
                temp[ 2] = 2 * b * d - 2 * a * c;
                temp[ 3] = 0;
                
                temp[ 4] = 2 * b * c - 2 * a * d;
                temp[ 5] = a2 - b2 + c2 - d2;
                temp[ 6] = 2 * c * d + 2 * a * b;
                temp[ 7] = 0;
                
                temp[ 8] = 2 * b * d + 2 * a * c;
                temp[ 9] = 2 * c * d - 2 * a * b;
                temp[10] = a2 - b2 - c2 + d2;
                temp[11] = 0;
                
                temp[12] = 0;
                temp[13] = 0;
                temp[14] = 0;
                temp[15] = 1;
                
                *this *= temp;
            }
            
            const Mat4f rotated(const Quat& rotation) const {
                Mat4f result = *this;
                result.rotate(rotation);
                return result;
            }

            void translate(const Vec3f& delta) {
                Mat4f temp;
                temp.setIdentity();
                temp[12] += delta.x;
                temp[13] += delta.y;
                temp[14] += delta.z;
                *this *= temp;
            }
            
            const Mat4f translated(const Vec3f& delta) const {
                Mat4f result = *this;
                result.translate(delta);
                return result;
            }

            void scale(const Vec3f& factors) {
                for (unsigned int i = 0; i < 4; i++)
                    for (unsigned int j = 0; j < 3; j++)
                        v[i * 4 + j] *= factors[j];
            }
            
            const Mat4f scaled(const Vec3f& factors) const {
                return Mat4f(v[ 0] * factors.x, v[ 1] * factors.y, v[ 2] * factors.z, v[ 3],
                             v[ 4] * factors.x, v[ 5] * factors.y, v[ 6] * factors.z, v[ 7],
                             v[ 8] * factors.x, v[ 9] * factors.y, v[10] * factors.z, v[11],
                             v[12] * factors.x, v[13] * factors.y, v[14] * factors.z, v[15]);
            }
        };
        
        inline Mat4f operator*(float left, const Mat4f& right) {
            return Mat4f(left * right.v[ 0], left * right.v[ 1], left * right.v[ 2], left * right.v[ 3],
                         left * right.v[ 4], left * right.v[ 5], left * right.v[ 6], left * right.v[ 7],
                         left * right.v[ 8], left * right.v[ 9], left * right.v[10], left * right.v[11],
                         left * right.v[12], left * right.v[13], left * right.v[14], left * right.v[15]);
        }
    }
}

#endif
