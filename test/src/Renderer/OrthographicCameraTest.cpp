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
#include "Renderer/OrthographicCamera.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(OrthographicCameraTest, testPickRay) {
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            OrthographicCamera camera(near, far, viewport, position, direction, up);
            
            Ray3f pickRay = camera.pickRay(512, 384);
            ASSERT_VEC_EQ(camera.position(), pickRay.origin);
            ASSERT_VEC_EQ(camera.direction(), pickRay.direction);
            
            pickRay = camera.pickRay(256, 384);
            ASSERT_VEC_EQ(Vec3f(0.0f, 256.0f, 0.0f), pickRay.origin);
            ASSERT_VEC_EQ(camera.direction(), pickRay.direction);
            
            pickRay = camera.pickRay(13, 778);
            ASSERT_VEC_EQ(Vec3f(0.0f, 499.0f, -393.99994f), pickRay.origin);
            ASSERT_VEC_EQ(camera.direction(), pickRay.direction);
        }
        
        TEST(OrthographicCameraTest, testDistanceTo) {
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            OrthographicCamera camera(near, far, viewport, position, direction, up);
            
            ASSERT_FLOAT_EQ(13.0f, camera.distanceTo(Vec3f(13.0f,  0.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f, camera.distanceTo(Vec3f( 0.0f, 13.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f, camera.distanceTo(Vec3f( 0.0f,  0.0f, 13.0f)));
            
            const Vec3f point(13.0f, 13.0f, 13.0f);
            ASSERT_FLOAT_EQ(point.length(), camera.distanceTo(point));
        }
        
        TEST(OrthographicCameraTest, testSquaredDistanceTo) {
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            OrthographicCamera camera(near, far, viewport, position, direction, up);
            
            ASSERT_FLOAT_EQ(13.0f * 13.0f, camera.squaredDistanceTo(Vec3f(13.0f,  0.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f * 13.0f, camera.squaredDistanceTo(Vec3f( 0.0f, 13.0f,  0.0f)));
            ASSERT_FLOAT_EQ(13.0f * 13.0f, camera.squaredDistanceTo(Vec3f( 0.0f,  0.0f, 13.0f)));
            
            const Vec3f point(13.0f, 13.0f, 13.0f);
            ASSERT_FLOAT_EQ(point.squaredLength(), camera.squaredDistanceTo(point));
        }
        
        TEST(OrthographicCameraTest, testDefaultPoint) {
            const float near = 1.0f;
            const float far = 100.0f;
            const Camera::Viewport viewport(0, 0, 1024, 768);
            
            const Vec3f position(Vec3f::Null);
            const Vec3f direction(Vec3f::PosX);
            const Vec3f up(Vec3f::PosZ);
            OrthographicCamera camera(near, far, viewport, position, direction, up);
            
            ASSERT_VEC_EQ(Vec3f(256.0f, 0.0f, 0.0f), camera.defaultPoint());
            ASSERT_VEC_EQ(Vec3f(1.0f, 1.0f, 1.0f).normalized() * 256.0f, camera.defaultPoint(Ray3f(camera.position(), Vec3f(1.0f, 1.0f, 1.0f).normalized())));
            
            const Vec3f point = camera.defaultPoint(7, 223);
            ASSERT_VEC_EQ(Vec3f(256.0f, 505.0f, 161.0f), point);
        }
    }
}
