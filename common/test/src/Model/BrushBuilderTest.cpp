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

#include "Exceptions.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>

#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("BrushBuilderTest.createCube", "[BrushBuilderTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Model::Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            const Brush cube = builder.createCube(128.0, "someName").value();
            ASSERT_TRUE(cube.fullySpecified());
            ASSERT_EQ(vm::bbox3d(-64.0, +64.0), cube.bounds());

            const auto faces = cube.faces();
            ASSERT_EQ(6u, faces.size());

            for (size_t i = 0; i < faces.size(); ++i) {
                ASSERT_EQ(std::string("someName"), faces[i].attributes().textureName());
            }
        }

        TEST_CASE("BrushBuilderTest.createCubeDefaults", "[BrushBuilderTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Model::Entity(), MapFormat::Standard);

            BrushFaceAttributes defaultAttribs("defaultTexture");
            defaultAttribs.setOffset(vm::vec2f(0.5f, 0.5f));
            defaultAttribs.setScale(vm::vec2f(0.5f, 0.5f));
            defaultAttribs.setRotation(45.0f);
            defaultAttribs.setSurfaceContents(1);
            defaultAttribs.setSurfaceFlags(2);
            defaultAttribs.setSurfaceValue(0.1f);
            defaultAttribs.setColor(Color(255, 255, 255, 255));

            BrushBuilder builder(&world, worldBounds, defaultAttribs);
            const Brush cube = builder.createCube(128.0, "someName").value();
            ASSERT_TRUE(cube.fullySpecified());
            ASSERT_EQ(vm::bbox3d(-64.0, +64.0), cube.bounds());

            const auto faces = cube.faces();
            ASSERT_EQ(6u, faces.size());

            for (size_t i = 0; i < faces.size(); ++i) {
                ASSERT_EQ(std::string("someName"), faces[i].attributes().textureName());
                ASSERT_EQ(vm::vec2f(0.5f, 0.5f), faces[i].attributes().offset());
                ASSERT_EQ(vm::vec2f(0.5f, 0.5f), faces[i].attributes().scale());
                ASSERT_EQ(45.0f, faces[i].attributes().rotation());
                ASSERT_EQ(1, faces[i].attributes().surfaceContents());
                ASSERT_EQ(2, faces[i].attributes().surfaceFlags());
                ASSERT_EQ(0.1f, faces[i].attributes().surfaceValue());
                ASSERT_EQ(Color(255, 255, 255, 255), faces[i].attributes().color());
            }
        }
    }
}
