/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include <gtest/gtest.h>

#include "VecMath.h"
#include "TestUtils.h"
#include "Renderer/PerspectiveCamera.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(PerspectiveCameraTest, moveTo) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            const Vec3f right(Vec3f::NegY);
            const Vec3f newPosition(10.0f, 23.0f, -132.0f);
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            camera.moveTo(newPosition);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(newPosition, camera.position());
            ASSERT_VEC_EQ(direction, camera.direction());
            ASSERT_VEC_EQ(up, camera.up());
            ASSERT_VEC_EQ(right, camera.right());
        }
        
        TEST(PerspectiveCameraTest, moveBy) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            const Vec3f right(Vec3f::NegY);
            const Vec3f delta(20.0f, -22.0f, 48.0f);
            const Vec3f newPosition = position + delta;
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            camera.moveBy(delta);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(newPosition, camera.position());
            ASSERT_VEC_EQ(direction, camera.direction());
            ASSERT_VEC_EQ(up, camera.up());
            ASSERT_VEC_EQ(right, camera.right());
        }
        
        TEST(PerspectiveCameraTest, lookAt) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);

            const Vec3f focus(1.0f, 1.0f, 1.0f);
            const Vec3f newDirection = focus.normalized();
            const Vec3f newRight = crossed(newDirection, Vec3f::PosZ).normalized();
            const Vec3f newUp = crossed(newRight, newDirection).normalized();
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            camera.lookAt(focus, Vec3f::PosZ);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(position, camera.position());
            ASSERT_VEC_EQ(newDirection, camera.direction());
            ASSERT_VEC_EQ(newUp, camera.up());
            ASSERT_VEC_EQ(newRight, camera.right());
        }
        
        TEST(PerspectiveCameraTest, setDirection) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            
            const Vec3f focus(1.0f, 1.0f, 1.0f);
            const Vec3f newDirection = focus.normalized();
            const Vec3f newRight = crossed(newDirection, Vec3f::PosZ).normalized();
            const Vec3f newUp = crossed(newRight, newDirection).normalized();
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            camera.setDirection(newDirection, Vec3f::PosZ);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(position, camera.position());
            ASSERT_VEC_EQ(newDirection, camera.direction());
            ASSERT_VEC_EQ(newUp, camera.up());
            ASSERT_VEC_EQ(newRight, camera.right());
        }
        
        TEST(PerspectiveCameraTest, rotateNoLock) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);

            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);

            const float yaw = Math::radians(15.0f);
            const float pitch = Math::radians(20.0f);
            const Quatf rotation = Quatf(Vec3f::PosZ, yaw) * Quatf(camera.right(), pitch);
            
            const Vec3f newDirection = rotation * direction;
            const Vec3f newUp = rotation * up;
            const Vec3f newRight = crossed(newDirection, newUp);
            
            camera.rotate(yaw, pitch);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(position, camera.position());
            ASSERT_VEC_EQ(newDirection, camera.direction());
            ASSERT_VEC_EQ(newUp, camera.up());
            ASSERT_VEC_EQ(newRight, camera.right());
        }

        TEST(PerspectiveCameraTest, rotateLockUp) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            
            const float yaw = Math::radians(15.0f);
            const float pitch = Math::radians(92.0f);
            const Quatf yawRotation = Quatf(Vec3f::PosZ, yaw);
            
            const Vec3f newDirection = Vec3f::PosZ;
            const Vec3f newUp = -(yawRotation * direction);
            const Vec3f newRight = crossed(newDirection, newUp);
            
            camera.rotate(yaw, pitch);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(position, camera.position());
            ASSERT_VEC_EQ(newDirection, camera.direction());
            ASSERT_VEC_EQ(newUp, camera.up());
            ASSERT_VEC_EQ(newRight, camera.right());
        }
        
        TEST(PerspectiveCameraTest, rotateLockDown) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            
            const float yaw = Math::radians(15.0f);
            const float pitch = Math::radians(-107.0f);
            const Quatf yawRotation = Quatf(Vec3f::PosZ, yaw);
            
            const Vec3f newDirection = Vec3f::NegZ;
            const Vec3f newUp = yawRotation * direction;
            const Vec3f newRight = crossed(newDirection, newUp);
            
            camera.rotate(yaw, pitch);
            
            ASSERT_FLOAT_EQ(fov, camera.fov());
            ASSERT_FLOAT_EQ(near, camera.nearPlane());
            ASSERT_FLOAT_EQ(far, camera.farPlane());
            ASSERT_VEC_EQ(position, camera.position());
            ASSERT_VEC_EQ(newDirection, camera.direction());
            ASSERT_VEC_EQ(newUp, camera.up());
            ASSERT_VEC_EQ(newRight, camera.right());
        }
        
        TEST(PerspectiveCameraTest, testViewRay) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            const Ray3f viewRay = camera.viewRay();
            ASSERT_VEC_EQ(camera.position(), viewRay.origin);
            ASSERT_VEC_EQ(camera.direction(), viewRay.direction);
        }
        
        TEST(PerspectiveCameraTest, testPickRay) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);

            Ray3f pickRay = camera.pickRay(512, 384);
            ASSERT_VEC_EQ(camera.position(), pickRay.origin);
            ASSERT_VEC_EQ(camera.direction(), pickRay.direction);
            
            pickRay = camera.pickRay(256, 384);
            ASSERT_VEC_EQ(camera.position(), pickRay.origin);
            ASSERT_VEC_EQ(Vec3f(0.89442718f, 0.44721359f, 0.0f), pickRay.direction);
            
            pickRay = camera.pickRay(13, 778);
            ASSERT_VEC_EQ(camera.position(), pickRay.origin);
            ASSERT_VEC_EQ(Vec3f(0.62720376f, 0.61127871f, -0.4826529f), pickRay.direction);
        }
        
        TEST(PerspectiveCameraTest, testDistanceTo) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            
            ASSERT_FLOAT_EQ(13.0f, camera.distanceTo(Vec3f(13.0f,  0.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f, camera.distanceTo(Vec3f( 0.0f, 13.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f, camera.distanceTo(Vec3f( 0.0f,  0.0f, 13.0f)));
            
            const Vec3f point(13.0f, 13.0f, 13.0f);
            ASSERT_FLOAT_EQ(point.length(), camera.distanceTo(point));
        }
        
        TEST(PerspectiveCameraTest, testSquaredDistanceTo) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            
            ASSERT_FLOAT_EQ(13.0f * 13.0f, camera.squaredDistanceTo(Vec3f(13.0f,  0.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f * 13.0f, camera.squaredDistanceTo(Vec3f( 0.0f, 13.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f * 13.0f, camera.squaredDistanceTo(Vec3f( 0.0f,  0.0f, 13.0f)));
            
            const Vec3f point(13.0f, 13.0f, 13.0f);
            ASSERT_FLOAT_EQ(point.squaredLength(), camera.squaredDistanceTo(point));
        }
        
        TEST(PerspectiveCameraTest, testDefaultPoint) {
            const float fov = 90.0f;
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            PerspectiveCamera camera(fov, near, far, viewport, position, direction, up);
            
            ASSERT_VEC_EQ(Vec3f(256.0f, 0.0f, 0.0f), camera.defaultPoint());
            ASSERT_VEC_EQ(Vec3f(1.0f, 1.0f, 1.0f).normalized() * 256.0f, camera.defaultPoint(Ray3f(camera.position(), Vec3f(1.0f, 1.0f, 1.0f).normalized())));
            
            const Vec3f point = camera.defaultPoint(7, 223);
            ASSERT_EQ(Vec3f(177.85825f, 175.42659f, 55.928089f), point);
        }
    }
}
