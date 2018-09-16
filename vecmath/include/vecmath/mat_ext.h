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

#ifndef TRENCHBROOM_MAT_EXT_H
#define TRENCHBROOM_MAT_EXT_H

#include "vec.h"
#include "mat.h"
#include "bbox.h"

#include <vector>
#include <tuple>

namespace vm {

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
    std::vector<vec<T,R>> operator*(const std::vector<vec<T,R>>& lhs, const mat<T,R,C>& rhs) {
        std::vector<vec<T,R>> result;
        result.reserve(lhs.size());
        for (const auto& v : lhs) {
            result.push_back(v * rhs);
        }
        return result;
    }

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
    std::vector<vec<T,R-1>> operator*(const std::vector<vec<T,R-1>>& lhs, const mat<T,R,C>& rhs) {
        std::vector<vec<T,R-1>> result;
        result.reserve(lhs.size());
        for (const auto& v : lhs) {
            result.push_back(v * rhs);
        }
        return result;
    }

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
    std::vector<vec<T,C>> operator*(const mat<T,R,C>& lhs, const std::vector<vec<T,C>>& rhs) {
        std::vector<vec<T,C>> result;
        result.reserve(rhs.size());
        for (const auto& v : rhs) {
            result.push_back(lhs * v);
        }
        return result;
    }

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
    std::vector<vec<T,C-1>> operator*(const mat<T,R,C>& lhs, const std::vector<vec<T,C-1>>& rhs) {
        std::vector<vec<T,C-1>> result;
        result.reserve(rhs.size());
        for (const auto& v : rhs) {
            result.push_back(lhs * v);
        }
        return result;
    }

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
    mat<T,4,4> perspectiveMatrix(const T fov, const T nearPlane, const T farPlane, const int width, const int height) {
        const auto vFrustum = std::tan(toRadians(fov) / static_cast<T>(2.0)) * static_cast<T>(0.75) * nearPlane;
        const auto hFrustum = vFrustum * static_cast<T>(width) / static_cast<T>(height);
        const auto depth = farPlane - nearPlane;

        static const auto zero = static_cast<T>(0.0);
        static const auto one  = static_cast<T>(1.0);
        static const auto two  = static_cast<T>(2.0);

        return mat<T,4,4>(nearPlane / hFrustum, zero,                    zero,                               zero,
                          zero,                 nearPlane / vFrustum,    zero,                               zero,
                          zero,                 zero,                   -(farPlane + nearPlane) / depth,    -two * farPlane * nearPlane / depth,
                          zero,                 zero,                   -one,                                zero);
    }

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
    mat<T,4,4> orthoMatrix(const T nearPlane, const T farPlane, const T left, const T top, const T right, const T bottom) {
        const auto width = right - left;
        const auto height = top - bottom;
        const auto depth = farPlane - nearPlane;

        static const auto zero = static_cast<T>(0.0);
        static const auto one  = static_cast<T>(1.0);
        static const auto two  = static_cast<T>(2.0);

        return mat<T,4,4>(two / width,  zero,            zero,          -(left + right) / width,
                          zero,         two / height,    zero,          -(top + bottom) / height,
                          zero,         zero,           -two / depth,   -(farPlane + nearPlane) / depth,
                          zero,         zero,            zero,           one);
    }

    /**
     * Returns a view transformation matrix which transforms normalized device coordinates to window coordinates.
     *
     * @tparam T the component type
     * @param direction the view direction
     * @param up the up vector
     * @return the view transformation matrix
     */
    template <typename T>
    mat<T,4,4> viewMatrix(const vec<T,3>& direction, const vec<T,3>& up) {
        const auto& f = direction;
        const auto  s = cross(f, up);
        const auto  u = cross(s, f);

        static const auto zero = static_cast<T>(0.0);
        static const auto one  = static_cast<T>(1.0);

        return mat<T,4,4>( s[0],  s[1],  s[2], zero,
                           u[0],  u[1],  u[2], zero,
                           -f[0], -f[1], -f[2], zero,
                           zero,  zero,  zero, one);
    }

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
    mat<T,4,4> rotationMatrix(const T roll, const T pitch, const T yaw) {
        static const auto I = static_cast<T>(1.0);
        static const auto O = static_cast<T>(0.0);

        const auto Cr = std::cos(roll);
        const auto Sr = std::sin(roll);
        const  mat<T,4,4> R( +I,  +O,  +O,  +O,
                             +O, +Cr, -Sr,  +O,
                             +O, +Sr, +Cr,  +O,
                             +O,  +O,  +O,  +I);

        const auto Cp = std::cos(pitch);
        const auto Sp = std::sin(pitch);
        const mat<T,4,4> P(+Cp,  +O, +Sp,  +O,
                           +O,  +I,  +O,  +O,
                           -Sp,  +O, +Cp,  +O,
                           +O,  +O,  +O,  +I);

        const auto Cy = std::cos(yaw);
        const auto Sy = std::sin(yaw);
        const mat<T,4,4> Y(+Cy, -Sy,  +O,  +O,
                           +Sy, +Cy,  +O,  +O,
                           +O,  +O,  +I,  +O,
                           +O,  +O,  +O,  +I);

        return Y * P * R;
    }

    /**
     * Returns a matrix that will rotate a point counter clockwise about the given axis by the given angle.
     *
     * @tparam T the component type
     * @param axis the axis to rotate about
     * @param angle the rotation angle (in radians)
     * @return the rotation matrix
     */
    template <typename T>
    mat<T,4,4> rotationMatrix(const vec<T,3>& axis, const T angle) {
        const auto s = std::sin(-angle);
        const auto c = std::cos(-angle);
        const auto i = static_cast<T>(1.0 - c);

        const auto ix  = i  * axis[0];
        const auto ix2 = ix * axis[0];
        const auto ixy = ix * axis[1];
        const auto ixz = ix * axis[2];

        const auto iy  = i  * axis[1];
        const auto iy2 = iy * axis[1];
        const auto iyz = iy * axis[2];

        const auto iz2 = i  * axis[2] * axis[2];

        const auto sx = s * axis[0];
        const auto sy = s * axis[1];
        const auto sz = s * axis[2];

        mat<T,4,4> rotation;
        rotation[0][0] = ix2 + c;
        rotation[0][1] = ixy - sz;
        rotation[0][2] = ixz + sy;

        rotation[1][0] = ixy + sz;
        rotation[1][1] = iy2 + c;
        rotation[1][2] = iyz - sx;

        rotation[2][0] = ixz - sy;
        rotation[2][1] = iyz + sx;
        rotation[2][2] = iz2 + c;

        return rotation;
    }

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
    mat<T,4,4> rotationMatrix(const quat<T>& quat) {
        const auto x = quat.v[0];
        const auto y = quat.v[1];
        const auto z = quat.v[2];
        const auto w = quat.r;

        const auto x2 = x*x;
        const auto y2 = y*y;
        const auto z2 = z*z;

        mat<T,4,4> rotation;
        rotation[0][0] = static_cast<T>(1.0 - 2.0*(y2 + z2));// a2 + b2 - c2 - d2;
        rotation[0][1] = static_cast<T>(2.0*(x*y + z*w));
        rotation[0][2] = static_cast<T>(2.0*(x*z - y*w));

        rotation[1][0] = static_cast<T>(2.0*(x*y - z*w));
        rotation[1][1] = static_cast<T>(1.0 - 2.0*(x2 + z2));//a2 - b2 + c2 - d2;
        rotation[1][2] = static_cast<T>(2.0*(y*z + x*w));

        rotation[2][0] = static_cast<T>(2.0*(x*z + y*w));
        rotation[2][1] = static_cast<T>(2.0*(y*z - x*w));
        rotation[2][2] = static_cast<T>(1.0 - 2.0*(x2 + y2));// a2 - b2 - c2 + d2;

        return rotation;
    }

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
    mat<T,4,4> rotationMatrix(const vec<T,3>& from, const vec<T,3>& to) {
        return rotationMatrix(quat<T>(from, to));
    }

    /**
     * Returns a matrix that translates by the given delta.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param delta the deltas by which to translate
     * @return the translation matrix
     */
    template <typename T, size_t S>
    mat<T,S+1,S+1> translationMatrix(const vec<T,S>& delta) {
        mat<T,S+1,S+1> translation;
        for (size_t i = 0; i < S; ++i) {
            translation[S][i] = delta[i];
        }
        return translation;
    }

    /**
     * Returns a matrix that contains only the translation part of the given transformation matrix.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param m the transformation matrix
     * @return the translation matrix
     */
    template <typename T, size_t S>
    mat<T,S,S> translationMatrix(const mat<T,S,S>& m) {
        mat<T,S,S> result;
        for (size_t i = 0; i < S-1; ++i) {
            result[S-1][i] = m[S-1][i];
        }
        return result;
    }

    /**
     * Returns a scaling matrix with the given scaling factors.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param factors the scaling factors
     * @return the scaling matrix
     */
    template <typename T, size_t S>
    mat<T,S+1,S+1> scalingMatrix(const vec<T,S>& factors) {
        mat<T,S+1,S+1> scaling;
        for (size_t i = 0; i < S; ++i) {
            scaling[i][i] = factors[i];
        }
        return scaling;
    }

    /**
     * Returns a matrix that mirrors along the given axis.
     *
     * @tparam T the component type
     * @param axis the axis along which to mirror
     * @return the mirroring axis
     */
    template <typename T>
    mat<T,4,4> mirrorMatrix(const axis::type axis) {
        switch (axis) {
            case axis::x:
                return mat<T,4,4>::mirror_x;
            case axis::y:
                return mat<T,4,4>::mirror_y;
            case axis::z:
                return mat<T,4,4>::mirror_z;
            default:
                return mat<T,4,4>::identity;
        }
    }

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
    mat<T,4,4> coordinateSystemMatrix(const vec<T,3>& x, const vec<T,3>& y, const vec<T,3>& z, const vec<T,3>& o) {
        [[maybe_unused]] bool invertible;
        mat<T,4,4> result;
        std::tie(invertible, result) = invert(mat<T,4,4>(x[0], y[0], z[0], o[0],
                                                         x[1], y[1], z[1], o[1],
                                                         x[2], y[2], z[2], o[2],
                                                         0.0,  0.0,  0.0,  1.0));
        assert(invertible);
        return result;
    }

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
    mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal, const vec<T,3>& direction) {
        // create some coordinate system where the X and Y axes are contained within the plane
        // and the Z axis is the projection direction
        vec<T,3> xAxis;

        switch (firstComponent(normal)) {
            case axis::x:
                xAxis = normalize(cross(normal, vec<T, 3>::pos_z));
                break;
            default:
                xAxis = normalize(cross(normal, vec<T, 3>::pos_x));
                break;
        }
        const auto  yAxis = normalize(cross(normal, xAxis));
        const auto& zAxis = direction;

        assert(isUnit(xAxis));
        assert(isUnit(yAxis));
        assert(isUnit(zAxis));

        return coordinateSystemMatrix(xAxis, yAxis, zAxis, distance * normal);
    }

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
    mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal) {
        return planeProjectionMatrix(distance, normal, normal);
    }

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
    mat<T,4,4> shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
        return mat<T,4,4>(1.0, Syx, Szx, 0.0,
                          Sxy, 1.0, Szy, 0.0,
                          Sxz, Syz, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
    }

    // TODO: add documentation and tests
    template <typename T>
    mat<T,4,4> scaleBBoxMatrix(const bbox<T,3>& oldBBox, const bbox<T,3>& newBBox) {
        const auto scaleFactors = newBBox.size() / oldBBox.size();
        return translationMatrix(newBBox.min) * scalingMatrix(scaleFactors) * translationMatrix(-oldBBox.min);
    }

    // TODO: add documentation and tests
    template <typename T>
    mat<T,4,4> scaleBBoxMatrixWithAnchor(const bbox<T,3>& oldBBox, const vec<T,3>& newSize, const vec<T,3>& anchorPoint) {
        const auto scaleFactors = newSize / oldBBox.size();
        return translationMatrix(anchorPoint) * scalingMatrix(scaleFactors) * translationMatrix(-anchorPoint);
    }

    // TODO: add documentation and tests
    template <typename T>
    mat<T,4,4> shearBBoxMatrix(const bbox<T,3>& box, const vec<T,3>& sideToShear, const vec<T,3>& delta) {
        const auto oldSize = box.size();

        // shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
        mat<T,4,4> shearMat;
        if (sideToShear == vec<T,3>::pos_x) {
            const auto relativeDelta = delta / oldSize.x();
            shearMat = shearMatrix(relativeDelta.y(), relativeDelta.z(), 0., 0., 0., 0.);
        } else if (sideToShear == vec<T,3>::neg_x) {
            const auto relativeDelta = delta / oldSize.x();
            shearMat = shearMatrix(-relativeDelta.y(), -relativeDelta.z(), 0., 0., 0., 0.);
        } else if (sideToShear == vec<T,3>::pos_y) {
            const auto relativeDelta = delta / oldSize.y();
            shearMat = shearMatrix(0., 0., relativeDelta.x(), relativeDelta.z(), 0., 0.);
        } else if (sideToShear == vec<T,3>::neg_y) {
            const auto relativeDelta = delta / oldSize.y();
            shearMat = shearMatrix(0., 0., -relativeDelta.x(), -relativeDelta.z(), 0., 0.);
        } else if (sideToShear == vec<T,3>::pos_z) {
            const auto relativeDelta = delta / oldSize.z();
            shearMat = shearMatrix(0., 0., 0., 0., relativeDelta.x(), relativeDelta.y());
        } else if (sideToShear == vec<T,3>::neg_z) {
            const auto relativeDelta = delta / oldSize.z();
            shearMat = shearMatrix(0., 0., 0., 0., -relativeDelta.x(), -relativeDelta.y());
        }

        // grab any vertex on side that is opposite the one being sheared.
        const auto sideOppositeToShearSide = -sideToShear;
        vec<T,3> vertOnOppositeSide;
        bool didGrab = false;
        auto visitor = [&](const vec<T,3>& p0, const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3, const vec<T,3>& n){
            if (n == sideOppositeToShearSide) {
                vertOnOppositeSide = p0;
                didGrab = true;
            }
        };
        box.forEachFace(visitor);
        assert(didGrab);

        return translationMatrix(vertOnOppositeSide) * shearMat * translationMatrix(-vertOnOppositeSide);
    }
}

#endif //TRENCHBROOM_MAT_EXT_H
