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

#include "Utility/Mat2.h"
#include "Utility/Mat3.h"
#include "Utility/Math.h"
#include "Utility/Quat.h"
#include "Utility/Vec.h"
#include "Utility/Vec.h"
#include "Utility/Vec.h"

#include <cassert>
#include <cmath>
#include <vector>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Mat4 {
        public:
            static const Mat4<T> Null;
            static const Mat4<T> Identity;
            static const Mat4<T> Rot90XCW;
            static const Mat4<T> Rot90YCW;
            static const Mat4<T> Rot90ZCW;
            static const Mat4<T> Rot90XCCW;
            static const Mat4<T> Rot90YCCW;
            static const Mat4<T> Rot90ZCCW;
            static const Mat4<T> Rot180X;
            static const Mat4<T> Rot180Y;
            static const Mat4<T> Rot180Z;
            static const Mat4<T> MirX;
            static const Mat4<T> MirY;
            static const Mat4<T> MirZ;
            
            typedef std::vector<Mat4<T> > List;
            
            T v[16];
            
            Mat4<T>() {
                setIdentity();
            }
            
            Mat4<T>(const T v11, const T v12, const T v13, const T v14,
                    const T v21, const T v22, const T v23, const T v24,
                    const T v31, const T v32, const T v33, const T v34,
                    const T v41, const T v42, const T v43, const T v44) {
                v[ 0] = v11; v[ 4] = v12; v[ 8] = v13; v[12] = v14;
                v[ 1] = v21; v[ 5] = v22; v[ 9] = v23; v[13] = v24;
                v[ 2] = v31; v[ 6] = v32; v[10] = v33; v[14] = v34;
                v[ 3] = v41; v[ 7] = v42; v[11] = v43; v[15] = v44;
            }
            
            inline Mat4<T>& operator= (const Mat4<T>& right) {
                for (size_t i = 0; i < 16; i++)
                    v[i] = right.v[i];
                return *this;
            }
            
            inline const Mat4<T> operator- () const {
                return Mat4<T>(-v[ 0], -v[ 4], -v[ 8], -v[12],
                               -v[ 1], -v[ 5], -v[ 9], -v[13],
                               -v[ 2], -v[ 6], -v[10], -v[14],
                               -v[ 3], -v[ 7], -v[11], -v[15]);
            }
            
            inline const Mat4<T> operator+ (const Mat4<T>& right) const {
                return Mat4<T>(v[ 0] + right.v[ 0], v[ 4] + right.v[ 4], v[ 8] + right.v[ 8], v[12] + right.v[12],
                               v[ 1] + right.v[ 1], v[ 5] + right.v[ 5], v[ 9] + right.v[ 9], v[13] + right.v[13],
                               v[ 2] + right.v[ 2], v[ 6] + right.v[ 6], v[10] + right.v[10], v[14] + right.v[14],
                               v[ 3] + right.v[ 3], v[ 7] + right.v[ 7], v[11] + right.v[11], v[15] + right.v[15]);
            }
            
            inline const Mat4<T> operator- (const Mat4<T>& right) const {
                return Mat4<T>(v[ 0] - right.v[ 0], v[ 4] - right.v[ 4], v[ 8] - right.v[ 8], v[12] - right.v[12],
                               v[ 1] - right.v[ 1], v[ 5] - right.v[ 5], v[ 9] - right.v[ 9], v[13] - right.v[13],
                               v[ 2] - right.v[ 2], v[ 6] - right.v[ 6], v[10] - right.v[10], v[14] - right.v[14],
                               v[ 3] - right.v[ 3], v[ 7] - right.v[ 7], v[11] - right.v[11], v[15] - right.v[15]);
            }
            
            inline const Mat4<T> operator* (const T right) const {
                return Mat4<T>(v[ 0] * right, v[ 4] * right, v[ 8] * right, v[12] * right,
                               v[ 1] * right, v[ 5] * right, v[ 9] * right, v[13] * right,
                               v[ 2] * right, v[ 6] * right, v[10] * right, v[14] * right,
                               v[ 3] * right, v[ 7] * right, v[11] * right, v[15] * right);
            }
            
            inline const Vec<T,3> operator* (const Vec<T,2>& right) const {
                const T w =     v[ 3] * right[0] + v[ 7] * right[1] + v[15];
                return Vec<T,3>((v[ 0] * right[0] + v[ 4] * right[1] + v[12]) / w,
                               (v[ 1] * right[0] + v[ 5] * right[1] + v[13]) / w,
                               (v[ 2] * right[0] + v[ 6] * right[1] + v[14]) / w);
            }
            
            inline const typename Vec<T,3>::List operator* (const typename Vec<T,2>::List& right) const {
                typename Vec<T,3>::List result;
                typename Vec<T,2>::List::const_iterator it, end;
                for (it = right.begin(), end = right.end(); it != end; ++it)
                    result.push_back(*this * *it);
                    return result;
            }
            
            inline const Vec<T,3> operator* (const Vec<T,3>& right) const {
                const T w =     v[ 3] * right[0] + v[ 7] * right[1] + v[11] * right[2] + v[15];
                return Vec<T,3>((v[ 0] * right[0] + v[ 4] * right[1] + v[ 8] * right[2] + v[12]) / w,
                               (v[ 1] * right[0] + v[ 5] * right[1] + v[ 9] * right[2] + v[13]) / w,
                               (v[ 2] * right[0] + v[ 6] * right[1] + v[10] * right[2] + v[14]) / w);
            }
            
            inline const typename Vec<T,3>::List operator* (const typename Vec<T,3>::List& right) const {
                typename Vec<T,3>::List result;
                typename Vec<T,3>::List::const_iterator it, end;
                for (it = right.begin(), end = right.end(); it != end; ++it)
                    result.push_back(*this * *it);
                    return result;
            }
            
            inline const Vec<T,4> operator* (const Vec<T,4>& right) const {
                return Vec<T,4>(v[ 0] * right[0] + v[ 4] * right[1] + v[ 8] * right[2] + v[12] * right.w,
                               v[ 1] * right[0] + v[ 5] * right[1] + v[ 9] * right[2] + v[13] * right.w,
                               v[ 2] * right[0] + v[ 6] * right[1] + v[10] * right[2] + v[14] * right.w,
                               v[ 3] * right[0] + v[ 7] * right[1] + v[11] * right[2] + v[15] * right.w);
            }
            
            inline const typename Vec<T,4>::List operator* (const typename Vec<T,4>::List& right) const {
                typename Vec<T,4>::List result;
                typename Vec<T,4>::List::const_iterator it, end;
                for (it = right.begin(), end = right.end(); it != end; ++it)
                    result.push_back(*this * *it);
                    return result;
            }
            
            inline const Mat4<T> operator* (const Mat4<T>& right) const {
                Mat4<T> result;
                for (size_t c = 0; c < 4; c++) {
                    for (size_t r = 0; r < 4; r++) {
                        result[c * 4 + r] = 0.0;
                        for (size_t i = 0; i < 4; i++)
                            result[c * 4 + r] += v[i * 4 + r] * right.v[c * 4 + i];
                    }
                }
                return result;
            }
            
            inline const Mat4<T> operator/ (const T right) const {
                Mat4<T> result = *this;
                return result /= right;
            }
            
            inline Mat4<T>& operator+= (const Mat4<T>& right) {
                for (size_t i = 0; i < 16; i++)
                    v[i] += right.v[i];
                return *this;
            }
            
            inline Mat4<T>& operator-= (const Mat4<T>& right) {
                for (size_t i = 0; i < 16; i++)
                    v[i] -= right.v[i];
                return *this;
            }
            
            inline Mat4<T>& operator*= (const T right) {
                for (size_t i = 0; i < 16; i++)
                    v[i] *= right;
                return *this;
            }
            
            inline Mat4<T>& operator*= (const Mat4<T>& right) {
                *this = *this * right;
                return *this;
            }
            
            inline Mat4<T>& operator/= (const T right) {
                *this *= (static_cast<T>(1.0) / right);
                return *this;
            }
            
            inline T& operator[] (const size_t index) {
                assert(index >= 0 && index < 16);
                return v[index];
            }
            
            inline const T& operator[] (const size_t index) const {
                assert(index >= 0 && index < 16);
                return v[index];
            }
            
            inline bool equals(const Mat4<T>& other) const {
                return equals(other, Math<T>::AlmostZero);
            }
            
            inline bool equals(const Mat4<T>& other, const T epsilon) const {
                for (size_t i = 0; i < 16; i++)
                    if (std::abs(v[i] - other.v[i]) > epsilon)
                        return false;
                return true;
            }
            
            inline bool identity() const {
                return equals(Identity);
            }
            
            inline bool null() const {
                return equals(Null);
            }
            
            inline Mat4<T>& setIdentity() {
                for (size_t r = 0; r < 4; r++)
                    for (size_t c = 0; c < 4; c++)
                        v[c * 4 + r] = r == c ? static_cast<T>(1.0) : static_cast<T>(0.0);
                return *this;
            }
            
            inline Mat4<T>& setPerspective(const T fov, const T nearPlane, const T farPlane, const int width, const int height) {
                const T vFrustum = std::tan(Math<T>::radians(fov) / static_cast<T>(2.0)) * static_cast<T>(0.75) * nearPlane;
                const T hFrustum = vFrustum * static_cast<T>(width) / static_cast<T>(height);
                const T depth = farPlane - nearPlane;
                
                set(static_cast<T>(nearPlane / hFrustum),   static_cast<T>(0.0),                    static_cast<T>(0.0),                                static_cast<T>(0.0),
                    static_cast<T>(0.0),                    static_cast<T>(nearPlane / vFrustum),   static_cast<T>(0.0),                                static_cast<T>(0.0),
                    static_cast<T>(0.0),                    static_cast<T>(0.0),                    static_cast<T>(-(farPlane + nearPlane) / depth),    static_cast<T>(-2.0 * (farPlane * nearPlane) / depth),
                    static_cast<T>(0.0),                    static_cast<T>(0.0),                    static_cast<T>(-1.0),                               static_cast<T>(0.0));
                return *this;
            }
            
            inline Mat4<T>& setOrtho(const T nearPlane, const T farPlane, const T left, const T top, const T right, const T bottom) {
                const T width = right - left;
                const T height = top - bottom;
                const T depth = farPlane - nearPlane;
                
                set(static_cast<T>(2.0 / width),    static_cast<T>(0.0),            static_cast<T>( 0.0),           static_cast<T>(-(left + right) / width),
                    static_cast<T>(0.0),            static_cast<T>(2.0 / height),   static_cast<T>( 0.0),           static_cast<T>(-(top + bottom) / height),
                    static_cast<T>(0.0),            static_cast<T>(0.0),            static_cast<T>(-2.0 / depth),   static_cast<T>(-(farPlane + nearPlane) / depth),
                    static_cast<T>(0.0),            static_cast<T>(0.0),            static_cast<T>( 0.0),           static_cast<T>( 1.0));
                return *this;
            }
            
            inline Mat4<T>& setView(const Vec<T,3>& direction, const Vec<T,3>& up) {
                const Vec<T,3>& f = direction;
                const Vec<T,3> s = f.crossed(up);
                const Vec<T,3> u = s.crossed(f);
                
                set( s[0],  s[1],  s[2], 0.0,
                     u[0],  u[1],  u[2], 0.0,
                    -f[0], -f[1], -f[2], 0.0,
                     0.0,  0.0,  0.0, 1.0);
                return *this;
            }
            
            inline Mat4<T>& set(const T v11, const T v12, const T v13, const T v14,
                                const T v21, const T v22, const T v23, const T v24,
                                const T v31, const T v32, const T v33, const T v34,
                                const T v41, const T v42, const T v43, const T v44) {
                v[ 0] = v11; v[ 4] = v12; v[ 8] = v13; v[12] = v14;
                v[ 1] = v21; v[ 5] = v22; v[ 9] = v23; v[13] = v24;
                v[ 2] = v31; v[ 6] = v32; v[10] = v33; v[14] = v34;
                v[ 3] = v41; v[ 7] = v42; v[11] = v43; v[15] = v44;
                return *this;
            }
            
            inline Mat4<T>& setValue(size_t row, size_t col, const T value) {
                assert(row >= 0 && row < 4);
                assert(col >= 0 && col < 4);
                v[col * 4 + row] = value;
                return *this;
            }
            
            inline Mat4<T>& setColumn(size_t col, const Vec<T,3>& values) {
                assert(col >= 0 && col < 4);
                v[col * 4 + 0] = values[0];
                v[col * 4 + 1] = values[1];
                v[col * 4 + 2] = values[2];
                v[col * 4 + 3] = 0.0;
                return *this;
            }
            
            inline Mat4<T>& setColumn(size_t col, const Vec<T,4>& values) {
                assert(col >= 0 && col < 4);
                v[col * 4 + 0] = values[0];
                v[col * 4 + 1] = values[1];
                v[col * 4 + 2] = values[2];
                v[col * 4 + 3] = values.w;
                return *this;
            }
            
            Mat4<T>& setSubMatrix(size_t index, const Mat2f& values) {
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
            
            const Mat2f subMatrix(size_t index) const {
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
            
            Mat4<T>& invert(bool& invertible) {
                const T det = determinant();
                invertible = det != 0.0;
                if (invertible) {
                    adjugate();
                    *this /= det;
                }
                return *this;
            }
            
            const Mat4<T> inverted(bool& invertible) const {
                Mat4<T> result = *this;
                result.invert(invertible);
                return result;
            }
            
            Mat4<T>& adjugate() {
                *this = adjugated();
                return *this;
            }
            
            const Mat4<T> adjugated() const {
                Mat4<T> result = *this;
                for (size_t c = 0; c < 4; c++)
                    for (size_t r = 0; r < 4; r++)
                        result[c * 4 + r] = ((c + r) % 2 == 0 ? 1 : -1) * subMatrix(c, r).determinant();
                // result.transpose();
                return result;
            }
            
            Mat4<T>& negate() {
                for (size_t i = 0; i < 16; i++)
                    v[i] = -v[i];
                return *this;
            }
            
            const Mat4<T> negated() const {
                return Mat4<T>(v[ 0] * -1, v[ 1] * -1, v[ 2] * -1, v[ 3] * -1,
                               v[ 4] * -1, v[ 5] * -1, v[ 6] * -1, v[ 7] * -1,
                               v[ 8] * -1, v[ 9] * -1, v[10] * -1, v[11] * -1,
                               v[12] * -1, v[13] * -1, v[14] * -1, v[15] * -1);
            }
            
            Mat4<T>& transpose() {
                for (size_t c = 0; c < 4; c++)
                    for (size_t r = c + 1; r < 4; r++)
                        std::swap(v[c * 4 + r], v[r * 4 + c]);
                return *this;
            }
            
            const Mat4<T> transposed() const {
                Mat4<T> result = *this;
                result.transpose();
                return result;
            }
            
            T determinant() const {
                // Laplace after first col
                T det = 0.0;
                for (size_t r = 0; r < 4; r++)
                    det += (r % 2 == 0 ? static_cast<T>(1.0) : static_cast<T>(-1.0)) * v[r] * subMatrix(r, 0).determinant();
                return det;
            }
            
            const Mat3f subMatrix(const size_t row, const size_t col) const {
                Mat3f result;
                size_t i = 0;
                for (size_t c = 0; c < 4; c++)
                    for (size_t r = 0; r < 4; r++)
                        if (c != col && r != row)
                            result[i++] = v[c * 4 + r];
                return result;
            }
            
            inline Mat4<T>& rotateCW(const T angle, const Vec<T,3>& axis) {
                return rotateCCW(-angle, axis);
            }
            
            Mat4<T>& rotateCCW(const T angle, const Vec<T,3>& axis) {
                const T s = sinf(angle);
                const T c = cosf(angle);
                const T i = static_cast<T>(1.0 - c);
                
                const T ix  = i  * axis[0];
                const T ix2 = ix * axis[0];
                const T ixy = ix * axis[1];
                const T ixz = ix * axis[2];
                
                const T iy  = i  * axis[1];
                const T iy2 = iy * axis[1];
                const T iyz = iy * axis[2];
                
                const T iz2 = i  * axis[2] * axis[2];
                
                const T sx = s * axis[0];
                const T sy = s * axis[1];
                const T sz = s * axis[2];
                
                Mat4<T> temp;
                temp[ 0] = ix2 + c;
                temp[ 1] = ixy - sz;
                temp[ 2] = ixz + sy;
                temp[ 3] = 0.0;
                
                temp[ 4] = ixy + sz;
                temp[ 5] = iy2 + c;
                temp[ 6] = iyz - sx;
                temp[ 7] = 0.0;
                
                temp[ 8] = ixz - sy;
                temp[ 9] = iyz + sx;
                temp[10] = iz2 + c;
                temp[11] = 0.0;
                
                temp[12] = 0.0;
                temp[13] = 0.0;
                temp[14] = 0.0;
                temp[15] = 1.0;
                
                *this *= temp;
                return *this;
            }
            
            const Mat4<T> rotatedCW(const T angle, const Vec<T,3>& axis) const {
                Mat4<T> result = *this;
                result.rotateCW(angle, axis);
                return result;
            }
            
            const Mat4<T> rotatedCCW(const T angle, const Vec<T,3>& axis) const {
                Mat4<T> result = *this;
                result.rotateCCW(angle, axis);
                return result;
            }
            
            Mat4<T>& rotate(const Quat<T>& rotation) {
                const T a = rotation.s;
                const T b = rotation.v[0];
                const T c = rotation.v[1];
                const T d = rotation.v[2];
                
                const T a2 = a * a;
                const T b2 = b * b;
                const T c2 = c * c;
                const T d2 = d * d;
                
                Mat4<T> temp;
                temp[ 0] = a2 + b2 - c2 - d2;
                temp[ 1] = static_cast<T>(2.0 * b * c + 2.0 * a * d);
                temp[ 2] = static_cast<T>(2.0 * b * d - 2.0 * a * c);
                temp[ 3] = static_cast<T>(0.0);
                
                temp[ 4] = static_cast<T>(2.0 * b * c - 2.0 * a * d);
                temp[ 5] = a2 - b2 + c2 - d2;
                temp[ 6] = static_cast<T>(2.0 * c * d + 2.0 * a * b);
                temp[ 7] = static_cast<T>(0.0);
                
                temp[ 8] = static_cast<T>(2.0 * b * d + 2.0 * a * c);
                temp[ 9] = static_cast<T>(2.0 * c * d - 2.0 * a * b);
                temp[10] = a2 - b2 - c2 + d2;
                temp[11] = static_cast<T>(0.0);
                
                temp[12] = static_cast<T>(0.0);
                temp[13] = static_cast<T>(0.0);
                temp[14] = static_cast<T>(0.0);
                temp[15] = static_cast<T>(1.0);
                
                *this *= temp;
                return *this;
            }
            
            const Mat4<T> rotated(const Quat<T>& rotation) const {
                Mat4<T> result = *this;
                result.rotate(rotation);
                return result;
            }
            
            inline Mat4<T>& translate(const T x, const T y, const T z) {
                Mat4<T> translation = Mat4<T>::Identity;
                translation[12] += x;
                translation[13] += y;
                translation[14] += z;
                *this *= translation;
                return *this;
            }
            
            inline Mat4<T> translated(const T x, const T y, const T z) const {
                Mat4<T> result = *this;
                result.translate(x, y, z);
                return result;
            }
            
            inline Mat4<T>& translate(const Vec<T,3>& delta) {
                return translate(delta[0], delta[1], delta[2]);
            }
            
            inline const Mat4<T> translated(const Vec<T,3>& delta) const {
                return translated(delta[0], delta[1], delta[2]);
            }
            
            Mat4<T>& scale(const T x, const T y, const T z) {
                Mat4<T> scaling = Mat4<T>::Identity;
                scaling.v[ 0] *= x;
                scaling.v[ 5] *= y;
                scaling.v[10] *= z;
                *this *= scaling;
                return *this;
            }
            
            const Mat4<T> scaled(const T x, const T y, const T z) const {
                Mat4<T> result = *this;
                result.scale(x, y, z);
                return result;
            }
            
            inline Mat4<T>& scale(const T f) {
                return scale(f, f, f);
            }
            
            inline const Mat4<T> scaled(const T f) const {
                return scaled(f, f, f);
            }
            
            inline Mat4<T>& scale(const Vec<T,3>& factors) {
                scale(factors[0], factors[1], factors[2]);
                return *this;
            }
            
            inline const Mat4<T> scaled(const Vec<T,3>& factors) const {
                return scaled(factors[0], factors[1], factors[2]);
            }
        };
        
        template <typename T>
        const Mat4<T> Mat4<T>::Null         = Mat4<T>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Identity     = Mat4<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot90XCW     = Mat4<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot90YCW     = Mat4<T>( static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot90ZCW     = Mat4<T>( static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                      -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot90XCCW    = Mat4<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot90YCCW    = Mat4<T>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                      -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot90ZCCW    = Mat4<T>( static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot180X      = Mat4<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot180Y      = Mat4<T>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::Rot180Z      = Mat4<T>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::MirX         = Mat4<T>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::MirY         = Mat4<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T>
        const Mat4<T> Mat4<T>::MirZ         = Mat4<T>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                       static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));

        typedef Mat4<float> Mat4f;
        
        template <typename T>
        inline Mat4<T> operator*(const T left, const Mat4<T>& right) {
            return Mat4<T>(left * right.v[ 0], left * right.v[ 1], left * right.v[ 2], left * right.v[ 3],
                           left * right.v[ 4], left * right.v[ 5], left * right.v[ 6], left * right.v[ 7],
                           left * right.v[ 8], left * right.v[ 9], left * right.v[10], left * right.v[11],
                           left * right.v[12], left * right.v[13], left * right.v[14], left * right.v[15]);
        }
    }
}

#endif
