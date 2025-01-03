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

#include "io/NodeReader.h"
#include "io/TestParserStatus.h"
#include "mdl/BrushNode.h"
#include "mdl/GroupNode.h"
#include "mdl/ParaxialUVCoordSystem.h"

#include "kdl/task_manager.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("NodeReader")
{
  auto taskManager = kdl::task_manager{};
  const auto worldBounds = vm::bbox3d{4096.0};
  auto status = io::TestParserStatus{};

  SECTION("parseFaceAsNode")
  {
    const auto data = R"(
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
)";

    CHECK(
      io::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status, taskManager)
        .empty());
  }

  SECTION("convertValveToStandardMapFormat")
  {
    const auto data = R"(
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
)";

    auto nodes = io::NodeReader::read(
      data, MapFormat::Standard, worldBounds, {}, status, taskManager);
    auto* brushNode = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
    REQUIRE(brushNode != nullptr);

    auto brush = brushNode->brush();
    CHECK(
      dynamic_cast<const ParaxialUVCoordSystem*>(&brush.face(0).uvCoordSystem())
      != nullptr);
  }

  SECTION("convertValveToStandardMapFormatInGroups")
  {
    // Data comes from copying a Group in 2020.2
    const auto data = R"(// entity 0
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
)";

    auto nodes = io::NodeReader::read(
      data, MapFormat::Standard, worldBounds, {}, status, taskManager);

    auto* groupNode = dynamic_cast<GroupNode*>(nodes.at(0));
    REQUIRE(groupNode != nullptr);

    auto* brushNode = dynamic_cast<BrushNode*>(groupNode->children().at(0));
    REQUIRE(brushNode != nullptr);

    const auto brush = brushNode->brush();
    CHECK(
      dynamic_cast<const ParaxialUVCoordSystem*>(&brush.face(0).uvCoordSystem())
      != nullptr);
  }

  SECTION("readScientificNotation")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/4270

    const auto data = R"(
{
"classname" "worldspawn"
"sounds" "1"
"MaxRange" "4096"
"mapversion" "220"
{
( 112 16 16 ) ( 112 16 17 ) ( 112 15 16 ) __TB_empty [ -1.8369701E-16 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1 
( 128 0 32 ) ( 128 0 33 ) ( 129 0 32 ) __TB_empty [ 1 -1.8369701e-16 0 0 ] [ 0 0 -1 0 ] 0 1 1 
( 112 16 16 ) ( 112 15 16 ) ( 113 16 16 ) __TB_empty [ 1.8369701e-16 1 0 0 ] [ -1 1.8369701E-16 0 0 ] 270 1 1 
( 128 0 80 ) ( 129 0 80 ) ( 128 -1 80 ) __TB_empty [ -1.8369701e-16 -1 0 0 ] [ -1 1.8369701E-16 0 0 ] 90 1 1 
( 112 16 16 ) ( 113 16 16 ) ( 112 16 17 ) __TB_empty [ -1 1.8369701E-16 0 0 ] [ 0 0 -1 0 ] 0 1 1 
( 128 0 32 ) ( 128 -1 32 ) ( 128 0 33 ) __TB_empty [ 1.8369701e-16 1 0 0 ] [ 0 0 -1 0 ] 0 1 1 
}
}
)";

    auto nodes =
      io::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status, taskManager);
    CHECK(nodes.size() == 1);
  }
}

} // namespace tb::mdl
