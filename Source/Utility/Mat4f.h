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
#include "Utility/Math.h"
#include "Utility/Quat.h"
#include "Utility/Vec3f.h"
#include "Utility/Vec4f.h"

#include <cassert>
#include <cmath>
#include <vector>

namespace TrenchBroom {
    namespace Math {
        class Mat4f {
        public:
            static const Mat4f Null;
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
            
            typedef std::vector<Mat4f> List;
            
            float v[16];
            
            Mat4f() {
                setIdentity();
            }
            
            Mat4f(float v11, float v12, float v13, float v14,
                  float v21, float v22, float v23, float v24,
                  float v31, float v32, float v33, float v34,
                  float v41, float v42, float v43, float v44) {
                v[ 0] = v11; v[ 4] = v12; v[ 8] = v13; v[12] = v14;
                v[ 1] = v21; v[ 5] = v22; v[ 9] = v23; v[13] = v24;
                v[ 2] = v31; v[ 6] = v32; v[10] = v33; v[14] = v34;
                v[ 3] = v41; v[ 7] = v42; v[11] = v43; v[15] = v44;
            }
            
            inline Mat4f& operator= (const Mat4f& right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] = right.v[i];
                return *this;
            }
            
            inline const Mat4f operator- () const {
                return Mat4f(-v[ 0], -v[ 4], -v[ 8], -v[12],
                             -v[ 1], -v[ 5], -v[ 9], -v[13],
                             -v[ 2], -v[ 6], -v[10], -v[14],
                             -v[ 3], -v[ 7], -v[11], -v[15]);
            }

            inline const Mat4f operator+ (const Mat4f& right) const {
                return Mat4f(v[ 0] + right.v[ 0], v[ 4] + right.v[ 4], v[ 8] + right.v[ 8], v[12] + right.v[12],
                             v[ 1] + right.v[ 1], v[ 5] + right.v[ 5], v[ 9] + right.v[ 9], v[13] + right.v[13],
                             v[ 2] + right.v[ 2], v[ 6] + right.v[ 6], v[10] + right.v[10], v[14] + right.v[14],
                             v[ 3] + right.v[ 3], v[ 7] + right.v[ 7], v[11] + right.v[11], v[15] + right.v[15]);
            }
            
            inline const Mat4f operator- (const Mat4f& right) const {
                return Mat4f(v[ 0] - right.v[ 0], v[ 4] - right.v[ 4], v[ 8] - right.v[ 8], v[12] - right.v[12],
                             v[ 1] - right.v[ 1], v[ 5] - right.v[ 5], v[ 9] - right.v[ 9], v[13] - right.v[13],
                             v[ 2] - right.v[ 2], v[ 6] - right.v[ 6], v[10] - right.v[10], v[14] - right.v[14],
                             v[ 3] - right.v[ 3], v[ 7] - right.v[ 7], v[11] - right.v[11], v[15] - right.v[15]);
            }
            
            inline const Mat4f operator* (const float right) const {
                return Mat4f(v[ 0] * right, v[ 4] * right, v[ 8] * right, v[12] * right,
                             v[ 1] * right, v[ 5] * right, v[ 9] * right, v[13] * right,
                             v[ 2] * right, v[ 6] * right, v[10] * right, v[14] * right,
                             v[ 3] * right, v[ 7] * right, v[11] * right, v[15] * right);
            }
            
            inline const Vec3f operator* (const Vec3f& right) const {
                float w =     v[ 3] * right.x + v[ 7] * right.y + v[11] * right.z + v[15];
                return Vec3f((v[ 0] * right.x + v[ 4] * right.y + v[ 8] * right.z + v[12]) / w,
                             (v[ 1] * right.x + v[ 5] * right.y + v[ 9] * right.z + v[13]) / w,
                             (v[ 2] * right.x + v[ 6] * right.y + v[10] * right.z + v[14]) / w);
            }
            
            inline const Vec4f operator* (const Vec4f& right) const {
                return Vec4f(v[ 0] * right.x + v[ 4] * right.y + v[ 8] * right.z + v[12] * right.w,
                             v[ 1] * right.x + v[ 5] * right.y + v[ 9] * right.z + v[13] * right.w,
                             v[ 2] * right.x + v[ 6] * right.y + v[10] * right.z + v[14] * right.w,
                             v[ 3] * right.x + v[ 7] * right.y + v[11] * right.z + v[15] * right.w);
            }
            
            inline const Mat4f operator* (const Mat4f& right) const {
                Mat4f result;
                for (unsigned int c = 0; c < 4; c++) {
                    for (unsigned int r = 0; r < 4; r++) {
                        result[c * 4 + r] = 0.0f;
                        for (unsigned int i = 0; i < 4; i++)
                            result[c * 4 + r] += v[i * 4 + r] * right.v[c * 4 + i];
                    }
                }
                return result;
            }
            
            inline const Mat4f operator/ (const float right) const {
                Mat4f result = *this;
                return result /= right;
            }
            
            inline Mat4f& operator+= (const Mat4f& right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] += right.v[i];
                return *this;
            }
            
            inline Mat4f& operator-= (const Mat4f& right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] -= right.v[i];
                return *this;
            }
            
            inline Mat4f& operator*= (const float right) {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] *= right;
                return *this;
            }
            
            inline Mat4f& operator*= (const Mat4f& right) {
                *this = *this * right;
                return *this;
            }
            
            inline Mat4f& operator/= (const float right) {
                *this *= (1.0f / right);
                return *this;
            }
            
            inline float& operator[] (const unsigned int index) {
                assert(index >= 0 && index < 16);
                return v[index];
            }
            
            inline const float& operator[] (const unsigned int index) const {
                assert(index >= 0 && index < 16);
                return v[index];
            }
            
            inline bool equals(const Mat4f& other) const {
                return equals(other, Math::AlmostZero);
            }
            
            inline bool equals(const Mat4f& other, float delta) const {
                for (unsigned int i = 0; i < 16; i++)
                    if (std::abs(v[i] - other.v[i]) > delta)
                        return false;
                return true;
            }

            inline bool identity() const {
                return equals(Identity);
            }
            
            inline bool null() const {
                return equals(Null);
            }
            
            inline Mat4f& setIdentity() {
                for (unsigned int r = 0; r < 4; r++)
                    for (unsigned int c = 0; c < 4; c++)
                        v[c * 4 + r] = r == c ? 1.0f : 0.0f;
                return *this;
            }
            
            inline Mat4f& setPerspective(float fov, float nearPlane, float farPlane, int width, int height) {
                float vFrustum = static_cast<float>(tan(Math::radians(fov) / 2.0f)) * 0.75f * nearPlane;
                float hFrustum = vFrustum * static_cast<float>(width) / static_cast<float>(height);
                float depth = farPlane - nearPlane;

                set(nearPlane / hFrustum,   0.0f,                    0.0f,                               0.0f,
                    0.0f,                   nearPlane / vFrustum,    0.0f,                               0.0f,
                    0.0f,                   0.0f,                   -(farPlane + nearPlane) / depth,    -2.0f * (farPlane * nearPlane) / depth,
                    0.0f,                   0.0f,                   -1.0f,                               0.0f);
                return *this;
            }

            inline Mat4f& setOrtho(float nearPlane, float farPlane, float left, float top, float right, float bottom) {
                float width = right - left;
                float height = top - bottom;
                float depth = farPlane - nearPlane;
                
                set(2.0f / width,   0.0f,            0.0f,           -(left + right) / width,
                    0.0f,           2.0f / height,   0.0f,           -(top + bottom) / height,
                    0.0f,           0.0f,           -2.0f / depth,   -(farPlane + nearPlane) / depth,
                    0.0f,           0.0f,            0.0f,            1.0f);
                return *this;
            }
            
            inline Mat4f& setView(const Vec3f& direction, const Vec3f& up) {
                const Vec3f& f = direction;
                Vec3f s = f.crossed(up);
                Vec3f u = s.crossed(f);
                
                set( s.x,  s.y,  s.z, 0.0f,
                     u.x,  u.y,  u.z, 0.0f,
                    -f.x, -f.y, -f.z, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f);
                return *this;
            }
            
            inline Mat4f& set(float v11, float v12, float v13, float v14,
                            float v21, float v22, float v23, float v24,
                            float v31, float v32, float v33, float v34,
                            float v41, float v42, float v43, float v44) {
                v[ 0] = v11; v[ 4] = v12; v[ 8] = v13; v[12] = v14;
                v[ 1] = v21; v[ 5] = v22; v[ 9] = v23; v[13] = v24;
                v[ 2] = v31; v[ 6] = v32; v[10] = v33; v[14] = v34;
                v[ 3] = v41; v[ 7] = v42; v[11] = v43; v[15] = v44;
                return *this;
            }

            inline Mat4f& setValue(unsigned int row, unsigned int col, float value) {
                assert(row >= 0 && row < 4);
                assert(col >= 0 && col < 4);
                v[col * 4 + row] = value;
                return *this;
            }
            
            inline Mat4f& setColumn(unsigned int col, const Vec3f& values) {
                assert(col >= 0 && col < 4);
                v[col * 4 + 0] = values.x;
                v[col * 4 + 1] = values.y;
                v[col * 4 + 2] = values.z;
                v[col * 4 + 3] = 0.0f;
                return *this;
            }
            
            inline Mat4f& setColumn(unsigned int col, const Vec4f& values) {
                assert(col >= 0 && col < 4);
                v[col * 4 + 0] = values.x;
                v[col * 4 + 1] = values.y;
                v[col * 4 + 2] = values.z;
                v[col * 4 + 3] = values.w;
                return *this;
            }
            
            Mat4f& setSubMatrix(unsigned int index, const Mat2f& values) {
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
                return *this;
            }
            
            const Mat2f subMatrix(unsigned int index) const {
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
            
            Mat4f& invert(bool& invertible) {
                float det = determinant();
                invertible = det != 0.0f;
                if (invertible) {
                    adjugate();
                    *this /= det;
                }
                return *this;
            }
            
            const Mat4f inverted(bool& invertible) const {
                Mat4f result = *this;
                result.invert(invertible);
                return result;
            }
            
            Mat4f& adjugate() {
                *this = adjugated();
                return *this;
            }
            
            const Mat4f adjugated() const {
                Mat4f result = *this;
                for (unsigned int c = 0; c < 4; c++)
                    for (unsigned int r = 0; r < 4; r++)
                        result[c * 4 + r] = ((c + r) % 2 == 0 ? 1 : -1) * subMatrix(c, r).determinant();
                // result.transpose();
                return result;
            }
            
            Mat4f& negate() {
                for (unsigned int i = 0; i < 16; i++)
                    v[i] = -v[i];
                return *this;
            }
            
            const Mat4f negated() const {
                return Mat4f(v[ 0] * -1, v[ 1] * -1, v[ 2] * -1, v[ 3] * -1,
                             v[ 4] * -1, v[ 5] * -1, v[ 6] * -1, v[ 7] * -1,
                             v[ 8] * -1, v[ 9] * -1, v[10] * -1, v[11] * -1,
                             v[12] * -1, v[13] * -1, v[14] * -1, v[15] * -1);
            }
            
            Mat4f& transpose() {
                for (unsigned int c = 0; c < 4; c++)
                    for (unsigned int r = c + 1; r < 4; r++)
                        std::swap(v[c * 4 + r], v[r * 4 + c]);
                return *this;
            }
            
            const Mat4f transposed() const {
                Mat4f result = *this;
                result.transpose();
                return result;
            }
            
            float determinant() const {
                // Laplace after first col
                float det = 0.0f;
                for (unsigned int r = 0; r < 4; r++)
                    det += (r % 2 == 0 ? 1.0f : -1.0f) * v[r] * subMatrix(r, 0).determinant();
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
            
            inline Mat4f& rotateCW(float angle, const Vec3f& axis) {
                return rotateCCW(-angle, axis);
            }
            
            Mat4f& rotateCCW(float angle, const Vec3f& axis) {
                float s = sinf(angle);
                float c = cosf(angle);
                float i = 1.0f - c;
                
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
                temp[ 3] = 0.0f;
                
                temp[ 4] = ixy + sz;
                temp[ 5] = iy2 + c;
                temp[ 6] = iyz - sx;
                temp[ 7] = 0.0f;
                
                temp[ 8] = ixz - sy;
                temp[ 9] = iyz + sx;
                temp[10] = iz2 + c;
                temp[11] = 0.0f;
                
                temp[12] = 0.0f;
                temp[13] = 0.0f;
                temp[14] = 0.0f;
                temp[15] = 1.0f;
                
                *this *= temp;
                return *this;
            }
            
            const Mat4f rotatedCW(float angle, const Vec3f& axis) const {
                Mat4f result = *this;
                result.rotateCW(angle, axis);
                return result;
            }
            
            const Mat4f rotatedCCW(float angle, const Vec3f& axis) const {
                Mat4f result = *this;
                result.rotateCCW(angle, axis);
                return result;
            }

            Mat4f& rotate(const Quat& rotation) {
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
                temp[ 1] = 2.0f * b * c + 2.0f * a * d;
                temp[ 2] = 2.0f * b * d - 2.0f * a * c;
                temp[ 3] = 0.0f;
                
                temp[ 4] = 2.0f * b * c - 2.0f * a * d;
                temp[ 5] = a2 - b2 + c2 - d2;
                temp[ 6] = 2.0f * c * d + 2.0f * a * b;
                temp[ 7] = 0.0f;
                
                temp[ 8] = 2.0f * b * d + 2.0f * a * c;
                temp[ 9] = 2.0f * c * d - 2.0f * a * b;
                temp[10] = a2 - b2 - c2 + d2;
                temp[11] = 0.0f;
                
                temp[12] = 0.0f;
                temp[13] = 0.0f;
                temp[14] = 0.0f;
                temp[15] = 1.0f;
                
                *this *= temp;
                return *this;
            }
            
            const Mat4f rotated(const Quat& rotation) const {
                Mat4f result = *this;
                result.rotate(rotation);
                return result;
            }

            inline Mat4f& translate(float x, float y, float z) {
                Mat4f translation = Mat4f::Identity;
                translation[12] += x;
                translation[13] += y;
                translation[14] += z;
                *this *= translation;
                return *this;
            }
            
            inline Mat4f translated(float x, float y, float z) const {
                Mat4f result = *this;
                result.translate(x, y, z);
                return result;
            }
            
            inline Mat4f& translate(const Vec3f& delta) {
                return translate(delta.x, delta.y, delta.z);
            }
            
            inline const Mat4f translated(const Vec3f& delta) const {
                return translated(delta.x, delta.y, delta.z);
            }

            Mat4f& scale(float x, float y, float z) {
                Mat4f scaling = Mat4f::Identity;
                scaling.v[ 0] *= x;
                scaling.v[ 5] *= y;
                scaling.v[10] *= z;
                *this *= scaling;
                return *this;
            }
            
            const Mat4f scaled(float x, float y, float z) const {
                Mat4f result = *this;
                result.scale(x, y, z);
                return result;
            }
            
            inline Mat4f& scale(float f) {
                return scale(f, f, f);
            }
            
            inline const Mat4f scaled(float f) const {
                return scaled(f, f, f);
            }
            
            inline Mat4f& scale(const Vec3f& factors) {
                scale(factors.x, factors.y, factors.z);
                return *this;
            }
            
            inline const Mat4f scaled(const Vec3f& factors) const {
                return scaled(factors.x, factors.y, factors.z);
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
