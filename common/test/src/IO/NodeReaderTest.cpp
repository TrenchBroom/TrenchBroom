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

#include "Catch2.h"

#include "FloatType.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/BrushNode.h"
#include "Model/EntityProperties.h"
#include "Model/GroupNode.h"
#include "Model/ParaxialTexCoordSystem.h"

#include <vecmath/bbox.h>

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("NodeReaderTest.parseFaceAsNode", "[NodeReaderTest]") {
            const std::string data(R"(
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
)");

            const vm::bbox3 worldBounds(4096.0);

            IO::TestParserStatus status;

            CHECK(IO::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status).empty());
        }


        TEST_CASE("NodeReaderTest.convertValveToStandardMapFormat", "[NodeReaderTest]") {
            const std::string data(R"(
// entity 0
{
"classname" "worldspawn"
"mapversion" "220"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)");

            const vm::bbox3 worldBounds(4096.0);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, MapFormat::Standard, worldBounds, {}, status);
            auto* brushNode = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
            REQUIRE(brushNode != nullptr);

            Brush brush = brushNode->brush();
            CHECK(dynamic_cast<const ParaxialTexCoordSystem*>(&brush.face(0).texCoordSystem()) != nullptr);
        }

        TEST_CASE("NodeReaderTest.convertValveToStandardMapFormatInGroups", "[NodeReaderTest]") {
            // Data comes from copying a Group in 2020.2
            const std::string data(R"(// entity 0
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Unnamed"
"_tb_id" "3"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)");

            const vm::bbox3 worldBounds(4096.0);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, MapFormat::Standard, worldBounds, {}, status);

            auto* groupNode = dynamic_cast<GroupNode*>(nodes.at(0));
            REQUIRE(groupNode != nullptr);

            auto* brushNode = dynamic_cast<BrushNode*>(groupNode->children().at(0));
            REQUIRE(brushNode != nullptr);

            const Brush brush = brushNode->brush();
            CHECK(dynamic_cast<const ParaxialTexCoordSystem*>(&brush.face(0).texCoordSystem()) != nullptr);
        }
    }
}
