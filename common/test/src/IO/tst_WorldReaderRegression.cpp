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

#include "Catch2.h"
#include "IO/TestParserStatus.h"
#include "IO/WorldReader.h"
#include "Model/BezierPatch.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>

#include <string>

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("WorldReaderTest.parseFailure_1424")
{
  const std::string data(R"(
{
"classname" "worldspawn"
"message" "yay"
{
( 0 0 0 ) ( 0 0 0 ) ( 0 0 0 ) __TB_empty -56 -72 -0 1 1
( 1320 512 152 ) ( 1280 512 192 ) ( 1320 504 152 ) grill_wall03b_h -0 -72 -0 1 1
( 1344 512 160 ) ( 1280 512 224 ) ( 1320 512 152 ) grill_wall03b_h -56 -72 -0 1 1
( 1320 512 152 ) ( 1320 504 152 ) ( 1344 512 160 ) grill_wall03b_h -56 -0 -0 1 1
( 0 0 0 ) ( 0 0 0 ) ( 0 0 0 ) __TB_empty -0 -72 -0 1 1
( 1320 504 152 ) ( 1280 505.37931034482756 197.51724137931035 ) ( 1344 512 160 ) grill_wall03b_h -56 -72 -0 1 1
}
})");

  const vm::bbox3 worldBounds(8192.0);

  IO::TestParserStatus status;
  WorldReader reader(data, Model::MapFormat::Standard, {});

  auto world = reader.read(worldBounds, status);
  CHECK(world != nullptr);
}

TEST_CASE("WorldReaderTest.parseProblematicBrush1")
{
  const std::string data(R"(
{
"classname" "worldspawn"
{
( 308 108 176 ) ( 308 132 176 ) ( 252 132 176 ) mt_sr_v13 -59 13 -90 1 1
( 252 132 208 ) ( 308 132 208 ) ( 308 108 208 ) mt_sr_v13 -59 13 -90 1 1
( 288 152 176 ) ( 288 152 208 ) ( 288 120 208 ) mt_sr_v13 -59 -110 -180 1 1
( 288 122 176 ) ( 288 122 208 ) ( 308 102 208 ) mt_sr_v13 -37 -111 -180 1 1
( 308 100 176 ) ( 308 100 208 ) ( 324 116 208 ) mt_sr_v13 -100 -111 0 1 -1
( 287 152 208 ) ( 287 152 176 ) ( 323 116 176 ) mt_sr_v13 -65 -111 -180 1 1
}
})");
  const vm::bbox3 worldBounds(8192.0);

  IO::TestParserStatus status;
  WorldReader reader(data, Model::MapFormat::Standard, {});

  auto world = reader.read(worldBounds, status);

  CHECK(world->childCount() == 1u);
  Model::Node* defaultLayer = world->children().front();
  CHECK(defaultLayer->childCount() == 1u);

  Model::BrushNode* brushNode =
    static_cast<Model::BrushNode*>(defaultLayer->children().front());
  checkBrushTexCoordSystem(brushNode, false);
  const auto& faces = brushNode->brush().faces();
  CHECK(faces.size() == 6u);
  CHECK(
    findFaceByPoints(
      faces,
      vm::vec3(308.0, 108.0, 176.0),
      vm::vec3(308.0, 132.0, 176.0),
      vm::vec3(252.0, 132.0, 176.0))
    != nullptr);
  CHECK(
    findFaceByPoints(
      faces,
      vm::vec3(252.0, 132.0, 208.0),
      vm::vec3(308.0, 132.0, 208.0),
      vm::vec3(308.0, 108.0, 208.0))
    != nullptr);
  CHECK(
    findFaceByPoints(
      faces,
      vm::vec3(288.0, 152.0, 176.0),
      vm::vec3(288.0, 152.0, 208.0),
      vm::vec3(288.0, 120.0, 208.0))
    != nullptr);
  CHECK(
    findFaceByPoints(
      faces,
      vm::vec3(288.0, 122.0, 176.0),
      vm::vec3(288.0, 122.0, 208.0),
      vm::vec3(308.0, 102.0, 208.0))
    != nullptr);
  CHECK(
    findFaceByPoints(
      faces,
      vm::vec3(308.0, 100.0, 176.0),
      vm::vec3(308.0, 100.0, 208.0),
      vm::vec3(324.0, 116.0, 208.0))
    != nullptr);
  CHECK(
    findFaceByPoints(
      faces,
      vm::vec3(287.0, 152.0, 208.0),
      vm::vec3(287.0, 152.0, 176.0),
      vm::vec3(323.0, 116.0, 176.0))
    != nullptr);
}

TEST_CASE("WorldReaderTest.parseProblematicBrush2")
{
  const std::string data(R"(
{
"classname" "worldspawn"
{
( -572 1078 128 ) ( -594 1088 128 ) ( -597 1072 96 ) mt_sr_v16 -64 0 -180 1 -1
( -572 1078 160 ) ( -572 1078 128 ) ( -590 1051 128 ) b_rc_v4 32 0 90 1 1
( -601 1056 160 ) ( -601 1056 128 ) ( -594 1088 128 ) b_rc_v4 32 0 90 1 1
( -590 1051 160 ) ( -590 1051 128 ) ( -601 1056 128 ) b_rc_v4 32 -16 90 1 1
( -512 1051 128 ) ( -624 1051 128 ) ( -568 1088 128 ) b_rc_v4 0 -16 90 1 1
( -559 1090 96 ) ( -598 1090 96 ) ( -598 1055 96 ) mt_sr_v13 -16 0 0 1 1
}
})");
  const vm::bbox3 worldBounds(8192.0);

  IO::TestParserStatus status;
  WorldReader reader(data, Model::MapFormat::Standard, {});

  auto world = reader.read(worldBounds, status);

  CHECK(world->childCount() == 1u);
  Model::Node* defaultLayer = world->children().front();
  CHECK(defaultLayer->childCount() == 1u);
  Model::BrushNode* brush =
    static_cast<Model::BrushNode*>(defaultLayer->children().front());
  checkBrushTexCoordSystem(brush, false);
}

TEST_CASE("WorldReaderTest.parseProblematicBrush3")
{
  const std::string data(R"(
{
"classname" "worldspawn"
{
( 256 1152 -96 ) ( 160 1152 -96 ) ( 160 1120 -96 ) b_rc_v4 31 -31 90 1 1
( -64 1120 64 ) ( -64 1184 64 ) ( -32 1184 32 ) b_rc_v4 31 -31 90 1 1
( -112 1120 32 ) ( 224 1120 32 ) ( 224 1120 -96 ) b_rc_v4 0 0 90 1 1
( -112 1184 -96 ) ( 264 1184 -96 ) ( 264 1184 32 ) b_rc_v4 -127 -32 90 1 1
( -64 1184 64 ) ( -64 1120 64 ) ( -64 1120 -96 ) b_rc_v4 -127 32 90 1 1
( -32 1136 32 ) ( -32 1152 -96 ) ( -32 1120 -96 ) b_rc_v4 0 32 90 1 1
}
})");
  const vm::bbox3 worldBounds(8192.0);

  IO::TestParserStatus status;
  WorldReader reader(data, Model::MapFormat::Standard, {});

  auto world = reader.read(worldBounds, status);

  CHECK(world->childCount() == 1u);
  Model::Node* defaultLayer = world->children().front();
  CHECK(defaultLayer->childCount() == 1u);
  Model::BrushNode* brush =
    static_cast<Model::BrushNode*>(defaultLayer->children().front());
  checkBrushTexCoordSystem(brush, false);
}
} // namespace IO
} // namespace TrenchBroom
