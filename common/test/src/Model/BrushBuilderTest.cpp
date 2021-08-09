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

#include <kdl/result.h>

#include <string>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("BrushBuilderTest.createCube", "[BrushBuilderTest]") {
            const vm::bbox3 worldBounds(8192.0);

            BrushBuilder builder(MapFormat::Standard, worldBounds);
            const Brush cube = builder.createCube(128.0, "someName").value();
            CHECK(cube.fullySpecified());
            CHECK(cube.bounds() == vm::bbox3d(-64.0, +64.0));

            const auto faces = cube.faces();
            CHECK(faces.size() == 6u);

            for (size_t i = 0; i < faces.size(); ++i) {
                CHECK(faces[i].attributes().textureName() == "someName");
            }
        }

        TEST_CASE("BrushBuilderTest.createCubeDefaults", "[BrushBuilderTest]") {
            const vm::bbox3 worldBounds(8192.0);

            BrushFaceAttributes defaultAttribs("defaultTexture");
            defaultAttribs.setOffset(vm::vec2f(0.5f, 0.5f));
            defaultAttribs.setScale(vm::vec2f(0.5f, 0.5f));
            defaultAttribs.setRotation(45.0f);
            defaultAttribs.setSurfaceAttributes(Model::SurfaceAttributes::makeContentsFlagsValue(1, 2, 0.1f));
            defaultAttribs.setColor(Color(255, 255, 255, 255));

            BrushBuilder builder(MapFormat::Standard, worldBounds, defaultAttribs);
            const Brush cube = builder.createCube(128.0, "someName").value();
            CHECK(cube.fullySpecified());
            CHECK(cube.bounds() == vm::bbox3d(-64.0, +64.0));

            const auto faces = cube.faces();
            CHECK(faces.size() == 6u);

            for (size_t i = 0; i < faces.size(); ++i) {
                CHECK(faces[i].attributes().textureName() == "someName");
                CHECK(faces[i].attributes().offset() == vm::vec2f(0.5f, 0.5f));
                CHECK(faces[i].attributes().scale() == vm::vec2f(0.5f, 0.5f));
                CHECK(faces[i].attributes().rotation() == 45.0f);
                REQUIRE(faces[i].attributes().hasSurfaceAttributes());
                CHECK(faces[i].attributes().surfaceAttributes()->surfaceContents == 1);
                CHECK(faces[i].attributes().surfaceAttributes()->surfaceFlags == 2);
                CHECK(faces[i].attributes().surfaceAttributes()->surfaceValue == 0.1f);
                CHECK(faces[i].attributes().color() == Color(255, 255, 255, 255));
            }
        }
    }
}
