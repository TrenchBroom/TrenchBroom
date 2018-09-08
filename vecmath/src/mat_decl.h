/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_MAT_DECL_H
#define TRENCHBROOM_MAT_DECL_H

#include "vec_decl.h"
#include "quat_decl.h"

#include <cassert>
#include <vector>

namespace vm {
    template <typename T, size_t R, size_t C>
    class mat {
    public:
        using Type = T;
        static const size_t Rows = R;
        static const size_t Cols = C;

        static const mat<T,R,C> zero;
        static const mat<T,R,C> identity;
        static const mat<T,R,C> rot_90_x_cw;
        static const mat<T,R,C> rot_90_y_cw;
        static const mat<T,R,C> rot_90_z_cw;
        static const mat<T,R,C> rot_90_x_ccw;
        static const mat<T,R,C> rot_90_y_ccw;
        static const mat<T,R,C> rot_90_z_ccw;
        static const mat<T,R,C> rot_180_x;
        static const mat<T,R,C> rot_180_y;
        static const mat<T,R,C> rot_180_z;
        static const mat<T,R,C> mirror_x;
        static const mat<T,R,C> mirror_y;
        static const mat<T,R,C> mirror_z;
        static const mat<T,R,C> zero_x;
        static const mat<T,R,C> zero_y;
        static const mat<T,R,C> zero_z;
        static const mat<T,R,C> yiq_to_rgb;
        static const mat<T,R,C> rgb_to_yiq;

        using List = std::vector<mat<T,R,C>>;

        /**
         * The matrix components in column major format.
         */
        vec<T,R> v[C];
    public:
        /**
         * Returns a matrix where all components are set to the given value.
         *
         * @param value the value to set
         * @return the newly created matrix
         */
        static mat<T,R,C> fill(const T value);

        /**
         * Returns an identity matrix.
         *
         * @return a matrix with all values of the diagonal set to 1 and all other values set to 0
         */
        static mat<T,R,C> identityMatrix();
    public:
        /**
         * Creates a new matrix with its values initialized to an identity matrix.
         */
        mat();

        // Copy and move constructors
        mat(const mat<T,R,C>& other) = default;
        mat(mat<T,R,C>&& other) noexcept = default;

        // Assignment operators
        mat<T,R,C>& operator=(const mat<T,R,C>& other) = default;
        mat<T,R,C>& operator=(mat<T,R,C>&& other) noexcept = default;

        /**
         * Sets the values of the newly created matrix to the values of the given matrix and casts each value of the given
         * matrix to the component type of the newly created matrix.
         *
         * @tparam U the component type of the source matrix
         * @param other the source matrix
         */
        template <typename U>
        explicit mat(const mat<U,R,C>& other) {
            for (size_t c = 0; c < C; ++c) {
                for (size_t r = 0; r < R; ++r) {
                    v[c][r] = static_cast<T>(other[c][r]);
                }
            }
        }

        /**
         * Sets the values of the newly created matrix to the given values, and all other values to 0.
         *
         * @param v11 the value at column 1 and row 1
         * @param v12 the value at column 2 and row 1
         * @param v13 the value at column 3 and row 1
         * @param v21 the value at column 1 and row 2
         * @param v22 the value at column 2 and row 2
         * @param v23 the value at column 3 and row 2
         * @param v31 the value at column 1 and row 3
         * @param v32 the value at column 2 and row 3
         * @param v33 the value at column 3 and row 3
         */
        mat(const T v11, const T v12, const T v13,
            const T v21, const T v22, const T v23,
            const T v31, const T v32, const T v33);

        /**
         * Sets the values of the newly created matrix to the given values, and all other values to 0.
         *
         * @param v11 the value at column 1 and row 1
         * @param v12 the value at column 2 and row 1
         * @param v13 the value at column 3 and row 1
         * @param v14 the value at column 4 and row 1
         * @param v21 the value at column 1 and row 2
         * @param v22 the value at column 2 and row 2
         * @param v23 the value at column 3 and row 2
         * @param v24 the value at column 4 and row 2
         * @param v31 the value at column 1 and row 3
         * @param v32 the value at column 2 and row 3
         * @param v33 the value at column 3 and row 3
         * @param v34 the value at column 4 and row 3
         * @param v41 the value at column 1 and row 4
         * @param v42 the value at column 2 and row 4
         * @param v43 the value at column 3 and row 4
         * @param v44 the value at column 4 and row 4
         */
        mat(const T v11, const T v12, const T v13, const T v14,
            const T v21, const T v22, const T v23, const T v24,
            const T v31, const T v32, const T v33, const T v34,
            const T v41, const T v42, const T v43, const T v44);

        /**
         * Returns the column at the given index.
         *
         * @param index the index of the column to return
         * @return the column at the given index
         */
        vec<T,R>& operator[] (const size_t index);

        /**
         * Returns the column at the given index.
         *
         * @param index the index of the column to return
         * @return the column at the given index
         */
        const vec<T,R>& operator[] (const size_t index) const;
    };

    /**
     * Compares the given two matrices column wise.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @param epsilon the epsilon value
     * @return a negative value if there is a column in the left matrix that compares less than its corresponding
     * column of the right matrix, a positive value in the opposite case, and 0 if all columns compare equal
     */
    template <typename T, size_t R, size_t C>
    int compare(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, T epsilon);

    /**
     * Checks whether the given matrices have identical components.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @return true if all components of the given matrices are equal, and false otherwise
     */
    template <typename T, size_t R, size_t C>
    bool operator==(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs);

    /**
     * Checks whether the given matrices have identical components.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @return false if all components of the given matrices are equal, and true otherwise
     */
    template <typename T, size_t R, size_t C>
    bool operator!=(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs);

    /**
     * Checks whether the given matrices have equal components.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @param epsilon the epsilon value
     * @return true if all components of the given matrices are equal, and false otherwise
     */
    template <typename T, size_t R, size_t C>
    bool isEqual(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, T epsilon);

    /**
     * Checks whether all columns of the given matrix are zero.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param m the matrix to check
     * @param epsilon the epsilon value
     * @return true if all columsn of the given matrix are zero
     */
    template <typename T, size_t R, size_t C>
    bool isZero(const mat<T,R,C>& m, T epsilon = constants<T>::almostZero());

    /**
     * Returns a matrix with the negated components of the given matrix.
     *
     * @param m the matrix to negate
     * @return the negated matrix
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator-(const mat<T,R,C>& m);

    /**
     * Computes the sum of two matrices by adding the corresponding components.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @return a matrix where each component is the sum of the two corresponding components of the given matrices
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator+(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs);

    /**
     * Computes the difference of two matrices by subtracting the corresponding components.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @return a matrix where each component is the difference of the two corresponding components of the given matrices
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator-(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs);

    /**
     * Computes the product of the given two matrices.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the first matrix
     * @param rhs the second matrix
     * @return the product of the given matrices
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator*(const mat<T,C,R>& lhs, const mat<T,C,R>& rhs);

    /**
     * Computes the scalar product of the given matrix and the given value.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the matrix
     * @param rhs the scalar
     * @return the product of the given matrix and the given value
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator*(const mat<T,R,C>& lhs, T rhs);

    /**
     * Computes the scalar product of the given matrix and the given value.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the scalar
     * @param rhs the matrix
     * @return the product of the given matrix and the given value
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator*(T lhs, const mat<T,R,C>& rhs);

    /**
     * Computes the scalar division of the given matrix and the given value.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the matrix
     * @param rhs the scalar
     * @return the division of the given matrix and the given value
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator/(const mat<T,R,C>& lhs, T rhs);

    /**
     * Multiplies the given vector by the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the vector
     * @param rhs the matrix
     * @return the product of the given vector and the given matrix
     */
    template <typename T, size_t R, size_t C>
    vec<T,R> operator*(const vec<T,R>& lhs, const mat<T,R,C>& rhs);

    /**
     * Multiplies the given vector by the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the matrix
     * @param rhs the vector
     * @return the product of the given vector and the given matrix
     */
    template <typename T, size_t R, size_t C>
    vec<T,R> operator*(const mat<T,R,C>& lhs, const vec<T,C>& rhs);

    /**
     * Multiplies the given vector by the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the matrix
     * @param rhs the vector
     * @return the product of the given vector and the given matrix
     */
    template <typename T, size_t R, size_t C>
    vec<T,C-1> operator*(const mat<T,R,C>& lhs, const vec<T,C-1>& rhs);

    /**
     * Multiplies the given vector by the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the vector
     * @param rhs the matrix
     * @return the product of the given vector and the given matrix
     */
    template <typename T, size_t R, size_t C>
    vec<T,R-1> operator*(const vec<T,R-1>& lhs, const mat<T,R,C>& rhs);

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    typename vec<T,R>::List operator*(const typename vec<T,R>::List& lhs, const mat<T,R,C>& rhs);

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    typename vec<T,R-1>::List operator*(const typename vec<T,R-1>::List& lhs, const mat<T,R,C>& rhs);

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    typename vec<T,C>::List operator*(const mat<T,R,C>& lhs, const typename vec<T,C>::List& rhs);

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    typename vec<T,C-1>::List operator*(const mat<T,R,C>& lhs, const typename vec<T,C-1>::List& rhs);

    /**
     * Transposes the given square matrix.
     *
     * @tparam T the component type
     * @tparam S the number of rows and columns
     * @param m the matrix to transpose
     * @return the transposed matrix
     */
    template <typename T, size_t S>
    mat<T,S,S> transpose(const mat<T,S,S>& m);

    /**
     * Computes a minor of the given square matrix. The minor of a matrix is obtained by erasing one column
     * and one row from that matrix. Thus, any minor matrix of an n*n matrix is a (n-1)*(n-1) matrix.
     *
     * @tparam T the component type
     * @tparam S the number of rows and columns of the given matrix
     * @param m the matrix to compute a minor of
     * @param row the row to strike
     * @param col the column to strike
     * @return the minor matrix
     */
    template <typename T, size_t S>
    mat<T,S-1,S-1> extractMinor(const mat<T,S,S>& m, size_t row, size_t col);

    /**
     * Computes the determinant of the given square matrix.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param m the matrix to compute the determinant of
     * @return the determinant of the given matrix
     */
    template <typename T, size_t S>
    T computeDeterminant(const mat<T,S,S>& m);

    /**
     * Computes the adjugate of the given square matrix.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param m the matrix to compute the adjugate of
     * @return the adjugate of the given matrix
     */
    template <typename T, size_t S>
    mat<T,S,S> computeAdjugate(const mat<T,S,S>& m);

    /**
     * Inverts the given square matrix if possible.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param m the matrix to invert
     * @return a pair of a boolean and a matrix such that the boolean indicates whether the
     * matrix is invertible, and if so, the matrix is the inverted given matrix
     */
    template <typename T, size_t S>
    std::tuple<bool, mat<T,S,S>> invert(const mat<T,S,S>& m);

    /**
     * Returns a perspective camera transformation with the given parameters. The returned matrix transforms from eye
     * coordinates to clip coordinates.
     *
     * @tparam T the component type
     * @param fov the field of view, in degrees
     * @param nearPlane the distance to the near plane
     * @param farPlane the distance to the far plane
     * @param width the viewport width
     * @param height the viewport height
     * @return the perspective transformation matrix
     */
    template <typename T>
    mat<T,4,4> perspectiveMatrix(T fov, T nearPlane, T farPlane, int width, int height);

    /**
     * Returns an orthographic camera transformation with the given parameters. The origin of the given screen coordinates
     * is at the center. The returned matrix transforms from eye coordinates to clip coordinates.
     *
     * @tparam T the component type
     * @param nearPlane the distance to the near plane
     * @param farPlane the distance to the far plane
     * @param left the screen coordinate of the left border of the viewport
     * @param top the screen coordinate of the top border of the viewport
     * @param right the screen coordinate of the right border of the viewport
     * @param bottom the screen coordinate of the bottom border of the viewport
     * @return the orthographic transformation matrix
     */
    template <typename T>
    mat<T,4,4> orthoMatrix(T nearPlane, T farPlane, T left, T top, T right, T bottom);

    /**
     * Returns a view transformation matrix which transforms normalized device coordinates to window coordinates.
     *
     * @tparam T the component type
     * @param direction the view direction
     * @param up the up vector
     * @return the view transformation matrix
     */
    template <typename T>
    mat<T,4,4> viewMatrix(const vec<T,3>& direction, const vec<T,3>& up);

    /**
     * Returns a matrix that will rotate a point counter clockwise by the given angles. The rotation is applied in the same
     * order the parameters are given: first roll, then pitch, then yaw.
     *
     * @tparam T the component type
     * @param roll the roll angle (in radians)
     * @param pitch the pitch angle (in radians)
     * @param yaw the yaw angle (in radians)
     * @return the rotation matrix
     */
    template <typename T>
    mat<T,4,4> rotationMatrix(T roll, T pitch, T yaw);

    /**
     * Returns a matrix that will rotate a point counter clockwise about the given axis by the given angle.
     *
     * @tparam T the component type
     * @param axis the axis to rotate about
     * @param angle the rotation angle (in radians)
     * @return the rotation matrix
     */
    template <typename T>
    mat<T,4,4> rotationMatrix(const vec<T,3>& axis, T angle);

    /**
     * Returns a rotation matrix that performs the same rotation as the given quaternion.
     *
     * @see http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
     *
     * @tparam T the component type
     * @param quat the quaternion
     * @return the rotation matrix
     */
    template <typename T>
    mat<T,4,4> rotationMatrix(const quat<T>& quat);

    /**
     * Returns a matrix that will rotate the first given vector onto the second given vector about their perpendicular
     * axis. The vectors are expected to be normalized.
     *
     * @tparam T the component type
     * @param from the vector to rotate
     * @param to the vector to rotate onto
     * @return the rotation matrix
     */
    template <typename T>
    mat<T,4,4> rotationMatrix(const vec<T,3>& from, const vec<T,3>& to);

    /**
     * Returns a matrix that translates by the given delta.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param delta the deltas by which to translate
     * @return the translation matrix
     */
    template <typename T, size_t S>
    mat<T,S+1,S+1> translationMatrix(const vec<T,S>& delta);

    /**
     * Returns a matrix that contains only the translation part of the given transformation matrix.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param m the transformation matrix
     * @return the translation matrix
     */
    template <typename T, size_t S>
    mat<T,S,S> translationMatrix(const mat<T,S,S>& m);

    /**
     * Strips the translation part from the given transformation matrix.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param m the transformation matrix
     * @return the transformation matrix without its translation part
     */
    template <typename T, size_t S>
    mat<T,S,S> stripTranslation(const mat<T,S,S>& m);

    /**
     * Returns a scaling matrix with the given scaling factors.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param factors the scaling factors
     * @return the scaling matrix
     */
    template <typename T, size_t S>
    mat<T,S+1,S+1> scalingMatrix(const vec<T,S>& factors);

    /**
     * Returns a matrix that mirrors along the given axis.
     *
     * @tparam T the component type
     * @param axis the axis along which to mirror
     * @return the mirroring axis
     */
    template <typename T>
    mat<T,4,4> mirrorMatrix(axis::type axis);

    /**
     * Returns a matrix that transforms to a coordinate system specified by the given axes and offset.
     *
     * @tparam T the component type
     * @param x the X axis of the target coordinate system, expressed relative to the source coordinate system
     * @param y the Y axis of the target coordinate system, expressed relative to the source coordinate system
     * @param z the Z axis of the target coordinate system, expressed relative to the source coordinate system
     * @param o the offset of the target coordinate system, expressed relative to the source coordinate system
     * @return the transformation matrix
     */
    template <typename T>
    mat<T,4,4> coordinateSystemMatrix(const vec<T,3>& x, const vec<T,3>& y, const vec<T,3>& z, const vec<T,3>& o);

    /**
     * Returns a matrix that will transform a point to a coordinate system where the X and
     * Y axes are in the given plane and the Z axis is parallel to the given direction. This is useful for
     * projecting points onto a plane along a particular direction.
     *
     * @tparam T the component type
     * @param distance the distance of the plane
     * @param normal the normal of the plane
     * @param direction the projection direction
     * @return the transformation matrix
     */
    template <typename T>
    mat<T,4,4> planeProjectionMatrix(T distance, const vec<T,3>& normal, const vec<T,3>& direction);

    /**
     * Returns a matrix that will transform a point to a coordinate system where the X and
     * Y axes are in the given plane and the Z axis is the plane normal. This is useful for vertically
     * projecting points onto a plane.
     *
     * @tparam T the component type
     * @param distance the distance of the plane
     * @param normal the normal of the plane
     * @return the transformation matrix
     */
    template <typename T>
    mat<T,4,4> planeProjectionMatrix(T distance, const vec<T,3>& normal);

    /**
     * Returns a matrix that performs a shearing transformation. In 3D, six shearing directions are possible:
     *
     * - X in direction of Y
     * - X in direction of Z
     * - Y in direction of X
     * - Y in direction of Z
     * - Z in direction of X
     * - Z in direction of Y
     *
     * @tparam T the component type
     * @param Sxy amount by which to share the X axis in direction of the Y axis
     * @param Sxz amount by which to share the X axis in direction of the Z axis
     * @param Syx amount by which to share the Y axis in direction of the X axis
     * @param Syz amount by which to share the Y axis in direction of the Z axis
     * @param Szx amount by which to share the Z axis in direction of the X axis
     * @param Szy amount by which to share the Z axis in direction of the Y axis
     * @return the shearing matrix
     */
    template <typename T>
    mat<T,4,4> shearMatrix(T Sxy, T Sxz, T Syx, T Syz, T Szx, T Szy);
}

#endif
