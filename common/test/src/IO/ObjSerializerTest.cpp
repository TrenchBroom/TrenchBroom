/*
 Copyright (C) 2021 Kristian Duske

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

#include "Logger.h"
#include "IO/NodeWriter.h"
#include "IO/ObjSerializer.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>
#include <kdl/result_io.h>

#include <memory>
#include <sstream>

#include "Catch2.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("ObjSerializer.writeBrush") {
            const auto worldBounds = vm::bbox3{8192.0};

            auto map = Model::WorldNode{Model::Entity{}, Model::MapFormat::Quake3};

            auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
            auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "some_texture").value()};
            map.defaultLayer()->addChild(brushNode);

            auto objStream = std::ostringstream{};
            auto mtlStream = std::ostringstream{};
            const auto mtlFilename = "some_file_name.mtl";

            auto writer = NodeWriter{map, std::make_unique<ObjSerializer>(objStream, mtlStream, mtlFilename)};
            writer.writeMap();

            CHECK(objStream.str() == R"(mtllib some_file_name.mtl
# vertices
v -32 -32 -32
v -32 -32 32
v -32 32 32
v -32 32 -32
v 32 32 32
v 32 -32 32
v 32 -32 -32
v 32 32 -32

# texture coordinates
vt 32 -32
vt -32 -32
vt -32 32
vt 32 32

# normals
vn -1 0 -0
vn 0 0 1
vn 0 -1 -0
vn 0 1 -0
vn 0 0 -1
vn 1 0 -0

o entity0_brush0
usemtl some_texture
f  1/1/1  2/2/1  3/3/1  4/4/1
usemtl some_texture
f  5/4/2  3/3/2  2/2/2  6/1/2
usemtl some_texture
f  6/1/3  2/2/3  1/3/3  7/4/3
usemtl some_texture
f  8/4/4  4/3/4  3/2/4  5/1/4
usemtl some_texture
f  7/1/5  1/2/5  4/3/5  8/4/5
usemtl some_texture
f  8/4/6  5/3/6  6/2/6  7/1/6

)");
            
            CHECK(mtlStream.str() == R"(newmtl some_texture
)");
        }
    }
}
