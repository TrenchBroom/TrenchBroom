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

#ifndef TrenchBroom_Mat_h
#define TrenchBroom_Mat_h

#include "Utility/Quat.h"
#include "Utility/Vec.h"

namespace TrenchBroom {
    namespace VecMath {
        template <typename T, size_t R, size_t C>
        class Mat {
        public:
            static const Mat<T,R,C> Null;
            static const Mat<T,R,C> Identity;
            static const Mat<T,R,C> Rot90XCW;
            static const Mat<T,R,C> Rot90YCW;
            static const Mat<T,R,C> Rot90ZCW;
            static const Mat<T,R,C> Rot90XCCW;
            static const Mat<T,R,C> Rot90YCCW;
            static const Mat<T,R,C> Rot90ZCCW;
            static const Mat<T,R,C> Rot180X;
            static const Mat<T,R,C> Rot180Y;
            static const Mat<T,R,C> Rot180Z;
            static const Mat<T,R,C> MirX;
            static const Mat<T,R,C> MirY;
            static const Mat<T,R,C> MirZ;
            static const Mat<T,R,C> YIQToRGB;
            static const Mat<T,R,C> RGBToYIQ;
            
            typedef std::vector<Mat<T,R,C> > List;
            
            // we store in row-major format
            Vec<T,C> v[R];
            
            Mat<T>() {}
            
            Mat<T,3,3>(const T v11, const T v12, const T v13,
                       const T v21, const T v22, const T v23,
                       const T v31, const T v32, const T v33) {
                v[0][0] = v11; v[0][1] = v12; v[0][2] = v13;
                v[1][0] = v21; v[1][1] = v22; v[1][2] = v23;
                v[2][0] = v31; v[2][1] = v32; v[2][2] = v33;
                for (size_t r = 3; r < R; r++)
                    for (size_t c = 3; c < C; c++)
                        v[r][c] = static_cast<T>(0.0);
            }
            
            Mat<T,4,4>(const T v11, const T v12, const T v13, const T v14,
                       const T v21, const T v22, const T v23, const T v24,
                       const T v31, const T v32, const T v33, const T v34,
                       const T v41, const T v42, const T v43, const T v44) {
                v[0][0] = v11; v[0][1] = v12; v[0][2] = v13; v[0][3] = v14;
                v[1][0] = v21; v[1][1] = v22; v[1][2] = v23; v[1][3] = v24;
                v[2][0] = v31; v[2][1] = v32; v[2][2] = v33; v[2][3] = v34;
                v[3][0] = v41; v[3][1] = v42; v[3][2] = v43; v[3][3] = v44;
                for (size_t r = 4; r < R; r++)
                    for (size_t c = 4; c < C; c++)
                        v[r][c] = static_cast<T>(0.0);
            }
            
            inline Mat<T,R,C>& operator= (const Mat<T,R,C>& right) {
                for (size_t r = 0; r < R; r++)
                    for (size_t c = 0; c < C; c++)
                        v[r][c] = right[r][c];
                return *this;
            }
            
            inline const Mat<T,R,C> operator- () const {
                Mat<T,R,C> result;
                for (size_t r = 0; r < R; r++)
                    result[r] = -v[r];
                return result;
            }
            
            // Matrix addition and subtraction
            inline const Mat<T,R,C> operator+ (const Mat<T,R,C>& right) const {
                Mat<T,R,C> result(*this);
                return result += right;
            }
            
            inline Mat<T,R,C>& operator+= (const Mat<T,R,C>& right) {
                for (size_t r = 0; r < R; r++)
                    v[r] += right[r];
                return *this;
            }

            inline const Mat<T,R,C> operator- (const Mat<T,R,C>& right) const {
                Mat<T,R,C> result(*this);
                return result -= right;
            }
            
            inline Mat<T,R,C>& operator-= (const Mat<T,R,C>& right) {
                for (size_t r = 0; r < R; r++)
                    v[r] -= right[r];
                return *this;
            }
            
            // Matrix multiplication
            inline const Mat<T,R,C> operator* (const Mat<T,C,R>& right) const {
                Mat<T,R,C> result;
                for (size_t r = 0; r < R; r++)
                    for (size_t c = 0; c < C; c++)
                        result[r][c] += v[r][c] * right[c][r];
                return result;
            }
            
            inline Mat<T,R,C>& operator*= (const Mat<T,C,R>& right) {
                return *this = *this * right;
            }
            
            // Scalar multiplication
            inline const Mat<T,R,C> operator* (const T right) const {
                Mat<T,R,C> result(*this);
                return result *= right;
            }
            
            inline Mat<T,R,C>& operator*= (const T right) const {
                for (size_t r = 0; r < R; r++)
                    v[r] *= right;
            }
            
            inline const Mat<T,R,C> operator/ (const T right) const {
                Mat<T,R,C> result(*this);
                return result /= right;
            }
            
            inline Mat<T,R,C>& operator/= (const T right) const {
                for (size_t r = 0; r < R; r++)
                    v[r] /= right;
                return *this;
            }

            // Vector multiplication
            inline const Vec<T,C> operator* (const Vec<T,C>& right) const {
                Vec<T,C> result;
                for (size_t r = 0; r < R; r++)
                    result[r] = v[r].dot(right);
                return result;
            }
            
            inline const Vec<T,C-1> operator* (const Vec<T,C-1>& right) const {
                return Vec<T,C-1>(*this *Vec<T,C>(right, static_cast<T>(1.0)));
            }
            
            inline const Vec<T,C-2> operator* (const Vec<T,C-2>& right) const {
                return Vec<T,C-2>(*this *Vec<T,C>(right, static_cast<T>(1.0)));
            }
            
            inline const typename Vec<T,C>::List operator* (const typename Vec<T,C>::List& right) const {
                typename Vec<T,C>::List result;
                result.reserve(right.size());
            
                typename Vec<T,C>::List::const_iteartor it, end;
                for (it = right.begin(), end = right.end(); it != end; ++it)
                    result.push_back(*this * *it);
                return result;
            }
        
            inline const typename Vec<T,C-1>::List operator* (const typename Vec<T,C-1>::List& right) const {
                typename Vec<T,C-1>::List result;
                result.reserve(right.size());
                
                typename Vec<T,C-1>::List::const_iterator it, end;
                for (it = right.begin(), end = right.end(); it != end; ++it)
                    result.push_back(*this * *it);
                    return result;
            }
        
            inline const typename Vec<T,C-2>::List operator* (const typename Vec<T,C-2>::List& right) const {
                typename Vec<T,C-2>::List result;
                result.reserve(right.size());
                
                typename Vec<T,C-2>::List::const_iteartor it, end;
                for (it = right.begin(), end = right.end(); it != end; ++it)
                    result.push_back(*this * *it);
                    return result;
            }
        
            inline Vec<T,C>& operator[] (const size_t index) {
                assert(index < R);
                return v[index];
            }
            
            inline const Vec<T,C>& operator[] (const size_t index) const {
                assert(index < R);
                return v[index];
            }
            
            inline bool equals(const Mat<T,R,C>& other, const T epsilon = Math<T>::AlmostZero) const {
                for (size_t r = 0; r < R; r++)
                    if (!v[r].equals(other[r], epsilon))
                        return false;
                return true;
            }
            
            inline bool null() const {
                return equals(Null);
            }
            
            inline Mat<T,R,C>& setIdentity() {
                for (size_t r = 0; r < R; r++)
                    for (size_t c = 0; c < C; c++)
                        v[r][c] = c == r ? static_cast<T>(1.0) : static_cast<T>(0.0);
                return *this;
            }
            
            inline Mat<T,R,C>& setColumn(size_t c, const Vec<T,R>& values) {
                for (size_t r = 0; r < R; r++)
                    v[r][c] = values[r];
                return *this;
            }
            
            inline Mat<T,C,R>& transpose() {
                *this = transposed();
                return *this;
            }
            
            inline const Mat<T,C,R> transposed() const {
                Mat<T,C,R> result;
                for (size_t r = 0; r < R; r++)
                    for (size_t c = 0; c < C; c++)
                        result[c][r] = v[r][c];
                return result;
            }
        };
        
        template <typename T, size_t R, size_t C>
        inline Mat<T,R,C> operator*(const T left, const Mat<T,R,C>& right) {
            return right * left;
        }

        template <typename T, size_t S>
        inline Mat<T,S,S>& invert(Mat<T,S,S>& mat, bool& invertible) {
            mat = inverted(mat, invertible);
            return mat;
        }
        
        template <typename T, size_t S>
        inline const Mat<T,S,S> inverted(const Mat<T,S,S>& mat, bool& invertible) {
            const T det = determinant(mat);
            invertible = det != 0.0;
            if (!invertible)
                return mat;
            
            return adjoint(mat) /= det;
        }
        
        template <typename T, size_t S>
        const Mat<T,S,S> adjoint(const Mat<T,S,S>& mat) {
            Mat<T,S,S> result;
            for (size_t r = 0; r < S; r++) {
                for (size_t c = 0; c < S; c++) {
                    const T f = static_cast<T>((c + r) % 2 == 0 ? 1.0 : -1.0);
                    result[c][r] = f * determinant(subMatrix(mat, r,c));
                }
            }
            return result;
        }
        
        template <typename T, size_t S>
        inline Mat<T,S,S>& adjoin(Mat<T,S,S>& mat) {
            mat = adjoint(mat);
            return mat;
        }
        
        template <typename T, size_t S>
        const Mat<T,S-1,S-1> minor(const Mat<T,S,S>& mat, const size_t row, const size_t col) {
            Mat<T,S-1,S-1> min;
            size_t minR, minC;
            minR = 0;
            for (size_t r = 0; r < S; r++) {
                minC = 0;
                for (size_t c = 0; c < S; c++) {
                    if (c != col && r != row)
                        min[minR][minC++] = mat[r][c];
                }
                minR++;
            }
            return min;
        }

        template <typename T, size_t S>
        inline T determinant(const Mat<T,S,S>& mat) {
            if (S == 1)
                return mat[0][0];
            
            // Laplace after first col
            T det = 0.0;
            for (size_t r = 0; r < S; r++) {
                const T f = static_cast<T>(r % 2 == 0 ? 1.0 : -1.0);
                det += f * mat[r][0] * determinant(subMatrix(mat, r, 0));
            }
            return det;
        }
        
        template <typename T>
        inline Mat<T,4,4>& setPerspective(Mat<T,4,4>& mat, const T fov, const T nearPlane, const T farPlane, const int width, const int height) {
            const T vFrustum = std::tan(Math<T>::radians(fov) / static_cast<T>(2.0)) * static_cast<T>(0.75) * nearPlane;
            const T hFrustum = vFrustum * static_cast<T>(width) / static_cast<T>(height);
            const T depth = farPlane - nearPlane;
            
            static const T zero = static_cast<T>(0.0);
            static const T one  = static_cast<T>(1.0);
            static const T two  = static_cast<T>(2.0);

            mat[0] = Vec<T,4>(nearPlane / hFrustum, zero,                    zero,                               zero);
            mat[1] = Vec<T,4>(zero,                 nearPlane / vFrustum,    zero,                               zero);
            mat[2] = Vec<T,4>(zero,                 zero,                   -(farPlane / nearPlane) / depth,    -two * (farPlane * nearPlane) / depth);
            mat[3] = Vec<T,4>(zero,                 zero,                   -one,                                zero);
            
            return mat;
        }
        
        template <typename T>
        inline Mat<T,4,4>& setOrtho(Mat<T,4,4>& mat, const T nearPlane, const T farPlane, const T left, const T top, const T right, const T bottom) {
            const T width = right - left;
            const T height = top - bottom;
            const T depth = farPlane - nearPlane;
            
            static const T zero = static_cast<T>(0.0);
            static const T one  = static_cast<T>(1.0);
            static const T two  = static_cast<T>(2.0);
            
            mat[0] = Vec<T,4>(two / width,  zero,            zero,          -(left + right) / width);
            mat[1] = Vec<T,4>(zero,         two / height,    zero,          -(top + bottom) / height);
            mat[2] = Vec<T,4>(zero,         zero,           -two / depth,   -(farPlane + nearPlane) / depth);
            mat[3] = Vec<T,4>(zero,         zero,            zero,           one);
            
            return mat;
        }
        
        template <typename T>
        inline Mat<T,4,4>& setView(Mat<T,4,4>&mat, const Vec<T,3>& direction, const Vec<T,3>& up) {
            const Vec<T,3>& f = direction;
            const Vec<T,3> s = crossed(f, up);
            const Vec<T,3> u = crossed(s, f);

            static const T zero = static_cast<T>(0.0);
            static const T one  = static_cast<T>(1.0);

            mat[0] = Vec<T,4>( s, zero);
            mat[1] = Vec<T,4>( u, zero);
            mat[2] = Vec<T,4>(-f, zero);
            mat[3] = Vec<T,4>(zero, zero, zero, one);
            
            return mat;
        }
        
        template <typename T>
        inline Mat<T,4,4>& rotateCW(Mat<T,4,4>&mat, const T angle, const Vec<T,3>& axis) {
            return rotateCCW(-angle, axis);
        }
        
        template <typename T>
        inline Mat<T,4,4>& rotateCCW(Mat<T,4,4>& mat, const T angle, const Vec<T,3>& axis) {
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
            
            Mat<T,4,4> temp;
            temp[0][0] = ix2 + c;
            temp[0][1] = ixy - sz;
            temp[0][2] = ixz + sy;
            temp[0][3] = 0.0;
            
            temp[1][0] = ixy + sz;
            temp[1][1] = iy2 + c;
            temp[1][2] = iyz - sx;
            temp[1][3] = 0.0;
            
            temp[2][0] = ixz - sy;
            temp[2][1] = iyz + sx;
            temp[2][2] = iz2 + c;
            temp[2][3] = 0.0;
            
            temp[3][0] = 0.0;
            temp[3][1] = 0.0;
            temp[3][2] = 0.0;
            temp[3][3] = 1.0;
            
            return mat *= temp;
        }
        
        template <typename T>
        const Mat<T,4,4> rotatedCW(const Mat<T,4,4>& mat, const T angle, const Vec<T,3>& axis) {
            return rotateCW(Mat<T,4,4>(mat), angle, axis);
        }
        
        template <typename T>
        const Mat<T,4,4> rotatedCCW(const Mat<T,4,4>& mat, const T angle, const Vec<T,3>& axis) {
            return rotateCCW(Mat<T,4,4>(mat), angle, axis);
        }
        
        template <typename T>
        inline Mat<T,4,4>& rotate(Mat<T,4,4>& mat, const Quat<T>& rotation) {
            const T a = rotation.s;
            const T b = rotation.v[0];
            const T c = rotation.v[1];
            const T d = rotation.v[2];
            
            const T a2 = a * a;
            const T b2 = b * b;
            const T c2 = c * c;
            const T d2 = d * d;
            
            Mat<T,4,4> temp;
            temp[0][0] = a2 + b2 - c2 - d2;
            temp[0][1] = static_cast<T>(2.0 * b * c + 2.0 * a * d);
            temp[0][2] = static_cast<T>(2.0 * b * d - 2.0 * a * c);
            temp[0][3] = static_cast<T>(0.0);
            
            temp[1][0] = static_cast<T>(2.0 * b * c - 2.0 * a * d);
            temp[1][1] = a2 - b2 + c2 - d2;
            temp[1][2] = static_cast<T>(2.0 * c * d + 2.0 * a * b);
            temp[1][3] = static_cast<T>(0.0);
            
            temp[2][0] = static_cast<T>(2.0 * b * d + 2.0 * a * c);
            temp[2][1] = static_cast<T>(2.0 * c * d - 2.0 * a * b);
            temp[2][2] = a2 - b2 - c2 + d2;
            temp[2][3] = static_cast<T>(0.0);
            
            temp[3][0] = static_cast<T>(0.0);
            temp[3][1] = static_cast<T>(0.0);
            temp[3][2] = static_cast<T>(0.0);
            temp[3][3] = static_cast<T>(1.0);
            
            return mat *= temp;
        }
        
        template <typename T>
        inline const Mat<T,4,4> rotated(const Mat<T,4,4>& mat, const Quat<T>& rotation) {
            return rotate(Mat<T,4,4>(mat), rotation);
        }
        
        template <typename T>
        inline Mat<T,4,4>& translate(Mat<T,4,4>& mat, const Vec<T,3>& delta) {
            Mat<T,4,4> translation;
            for (size_t i = 0; i < 3; i++)
                translation[3][i] = delta[i];
            return mat *= translation;
        }
        
        template <typename T>
        inline const Mat<T,4,4> translated(const Mat<T,4,4>& mat, const Vec<T,3>& delta) {
            return translate(Mat<T,4,4>(mat), delta);
        }
        
        template <typename T>
        inline Mat<T,4,4>& scale(Mat<T,4,4>& mat, const Vec<T,3>& factors) {
            Mat<T,4,4> scaling;
            for (size_t i = 0; i < 3; i++)
                scaling[i][i] = factors[i];
            return mat *= scaling;
        }
        
        template <typename T>
        inline const Mat<T,4,4> scaled(const Mat<T,4,4>& mat, const Vec<T,3>& factors) {
            return scale(Mat<T,4,4>(mat), factors);
        }

        template <typename T, size_t R, size_t C>
        inline Mat<T,4,4>& scale(Mat<T,4,4>& mat, const T f) {
            return scale(Vec<T,3>(f, f, f));
        }
        
        template <typename T, size_t R, size_t C>
        inline const Mat<T,4,4> scaled(const Mat<T,4,4>& mat, const T f) {
            return scale(Mat<T,4,4>(mat), f);
        }

        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Identity = Mat<T,R,C>().setIdentity();

        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot90XCW    = Mat<T,R,C>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot90YCW    = Mat<T,R,C>( static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot90ZCW    = Mat<T,R,C>( static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                              -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot90XCCW   = Mat<T,R,C>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot90YCCW   = Mat<T,R,C>( static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                              -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot90ZCCW   = Mat<T,R,C>( static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot180X     = Mat<T,R,C>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot180Y     = Mat<T,R,C>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Rot180Z     = Mat<T,R,C>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::MirX        = Mat<T,R,C>(-static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::MirY        = Mat<T,R,C>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::MirZ        = Mat<T,R,C>( static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(1.0),  static_cast<T>(0.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0), -static_cast<T>(1.0),  static_cast<T>(0.0),
                                                               static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(0.0),  static_cast<T>(1.0));
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::YIQToRGB     = Mat<T,R,C>( static_cast<T>(1.0), static_cast<T>( 0.9563), static_cast<T> (0.6210),
                                                                static_cast<T>(1.0), static_cast<T>(-0.2721), static_cast<T>(-0.6474),
                                                                static_cast<T>(1.0), static_cast<T>(-1.1070), static_cast<T>( 1.7046));
        
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::RGBToYIQ     = Mat<T,R,C>( static_cast<T>(0.299),    static_cast<T>( 0.587),    static_cast<T>( 0.114),
                                                                static_cast<T>(0.595716), static_cast<T>(-0.274453), static_cast<T>(-0.321263),
                                                                static_cast<T>(0.211456), static_cast<T>(-0.522591), static_cast<T>( 0.311135));
        
        
        template <typename T, size_t R, size_t C>
        const Mat<T,R,C> Mat<T,R,C>::Null = Mat<T,R,C>();
        
        typedef Mat<float,2,2> Mat2f;
        typedef Mat<float,3,3> Mat3f;
        typedef Mat<float,4,4> Mat4f;
    }
}

#endif
