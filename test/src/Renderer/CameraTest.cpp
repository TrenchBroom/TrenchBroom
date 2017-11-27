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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Renderer/Camera.h"
#include "Renderer/PerspectiveCamera.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(CameraTest, testInvalidUp) {
            PerspectiveCamera c;
            c.setDirection(Vec3f(0,0,1), Vec3f(0,0,1));
            
            ASSERT_FALSE(c.direction().nan());
            ASSERT_FALSE(c.right().nan());
            ASSERT_FALSE(c.up().nan());
        }
    }
}
