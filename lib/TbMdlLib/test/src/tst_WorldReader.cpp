/*
 Copyright (C) 2010 Kristian Duske

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

#include "TestParserStatus.h"
#include "fs/DiskIO.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GameInfo.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "mdl/WorldReader.h"

#include "kd/task_manager.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <fmt/format.h>

#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("WorldReader")
{
  using namespace std::string_literals;

  auto taskManager = kdl::task_manager{};
  const auto worldBounds = vm::bbox3d{8192.0};
  auto status = TestParserStatus{};

  SECTION("Empty map")
  {
    const auto data = "";
    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world != nullptr);
    CHECK(world->childCount() == 1u);
    CHECK_FALSE(world->children().front()->hasChildren());
  }

  SECTION("Empty entity")
  {
    const auto data = "{}";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world != nullptr);
    CHECK(world->childCount() == 1u);
    CHECK(world->children().front()->childCount() == 1u);
  }

  SECTION("Worldspawn entity")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"message" "yay"
}
)";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->childCount() == 1u);
    auto* defaultLayer = dynamic_cast<mdl::LayerNode*>(worldNode->children().at(0));
    REQUIRE(defaultLayer != nullptr);
    REQUIRE(!defaultLayer->hasChildren());

    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("message"));
    CHECK(*worldNode->entity().property("message") == "yay");

    CHECK(!defaultLayer->layer().color().has_value());
    CHECK(!defaultLayer->locked());
    CHECK(!defaultLayer->hidden());
    CHECK(!defaultLayer->layer().omitFromExport());
  }

  SECTION("Default layer properties")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"_tb_layer_color" "0.0 1.0 0.0"
"_tb_layer_locked" "1"
"_tb_layer_hidden" "1"
"_tb_layer_omit_from_export" "1"
}
)";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    REQUIRE(world->childCount() == 1u);
    auto* defaultLayer = dynamic_cast<mdl::LayerNode*>(world->children().at(0));
    REQUIRE(defaultLayer != nullptr);

    CHECK(defaultLayer->layer().color() == Color{RgbF{0.0f, 1.0f, 0.0f}});
    CHECK(defaultLayer->locked());
    CHECK(defaultLayer->hidden());
    CHECK(defaultLayer->layer().omitFromExport());
  }

  SECTION("Worldspawn and one entity")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"message" "yay"
}
{
"classname" "info_player_deathmatch"
"origin" "1 22 -3"
"angle" " -1 "
}
)";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("message"));
    CHECK(*worldNode->entity().property("message") == "yay");

    CHECK(worldNode->childCount() == 1u);
    auto* defaultLayerNode = dynamic_cast<mdl::LayerNode*>(worldNode->children().front());
    CHECK(defaultLayerNode != nullptr);
    CHECK(defaultLayerNode->childCount() == 1u);
    CHECK(defaultLayerNode->layer().sortIndex() == mdl::Layer::defaultLayerSortIndex());

    auto* entityNode =
      static_cast<mdl::EntityNode*>(defaultLayerNode->children().front());
    CHECK(entityNode->entity().hasProperty("classname"));
    CHECK(*entityNode->entity().property("classname") == "info_player_deathmatch");
    CHECK(entityNode->entity().hasProperty("origin"));
    CHECK(*entityNode->entity().property("origin") == "1 22 -3");
    CHECK(entityNode->entity().hasProperty("angle"));
    CHECK(*entityNode->entity().property("angle") == " -1 ");
  }

  SECTION("Worldspawn and one brush")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) tex1 1 2 3 4 5
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) tex2 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) tex3 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) tex4 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) tex5 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) tex6 0 0 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);

    auto* brushNode = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brushNode, false);
    const auto& faces = brushNode->brush().faces();
    CHECK(faces.size() == 6u);

    const auto* face1 = findFaceByPoints(
      faces,
      vm::vec3d{0.0, 0.0, -16.0},
      vm::vec3d{0.0, 0.0, 0.0},
      vm::vec3d{64.0, 0.0, -16.0});
    CHECK(face1 != nullptr);
    CHECK(face1->attributes().materialName() == "tex1");
    CHECK(face1->attributes().xOffset() == 1.0);
    CHECK(face1->attributes().yOffset() == 2.0);
    CHECK(face1->attributes().rotation() == 3.0);
    CHECK(face1->attributes().xScale() == 4.0);
    CHECK(face1->attributes().yScale() == 5.0);

    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{0.0, 0.0, -16.0},
        vm::vec3d{0.0, 64.0, -16.0},
        vm::vec3d{0.0, 0.0, 0.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{0.0, 0.0, -16.0},
        vm::vec3d{64.0, 0.0, -16.0},
        vm::vec3d{0.0, 64.0, -16.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{64.0, 64.0, 0.0},
        vm::vec3d{0.0, 64.0, 0.0},
        vm::vec3d{64.0, 64.0, -16.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{64.0, 64.0, 0.0},
        vm::vec3d{64.0, 64.0, -16.0},
        vm::vec3d{64.0, 0.0, 0.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{64.0, 64.0, 0.0},
        vm::vec3d{64.0, 0.0, 0.0},
        vm::vec3d{0.0, 64.0, 0.0})
      != nullptr);
  }

  SECTION("Map and check face flags")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 22 -3 56.2 1.03433 -0.55
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);

    auto* brushNode = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brushNode, false);
    const auto& faces = brushNode->brush().faces();
    CHECK(faces.size() == 6u);

    const auto* face = findFaceByPoints(
      faces,
      vm::vec3d{0.0, 0.0, -16.0},
      vm::vec3d{0.0, 0.0, 0.0},
      vm::vec3d{64.0, 0.0, -16.0});
    CHECK(face != nullptr);
    CHECK(face->attributes().xOffset() == 22.0f);
    CHECK(face->attributes().xOffset() == 22.0f);
    CHECK(face->attributes().rotation() == 56.2f);
    CHECK(face->attributes().xScale() == 1.03433f);
    CHECK(face->attributes().yScale() == -0.55f);
  }

  SECTION("Curly brace in material name")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) {none 0 0 0 1 1
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);

    auto* brushNode = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brushNode, false);
    const auto& faces = brushNode->brush().faces();
    CHECK(faces.size() == 6u);

    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{0.0, 0.0, -16.0},
        vm::vec3d{0.0, 0.0, 0.0},
        vm::vec3d{64.0, 0.0, -16.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{0.0, 0.0, -16.0},
        vm::vec3d{0.0, 64.0, -16.0},
        vm::vec3d{0.0, 0.0, 0.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{0.0, 0.0, -16.0},
        vm::vec3d{64.0, 0.0, -16.0},
        vm::vec3d{0.0, 64.0, -16.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{64.0, 64.0, 0.0},
        vm::vec3d{0.0, 64.0, 0.0},
        vm::vec3d{64.0, 64.0, -16.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{64.0, 64.0, 0.0},
        vm::vec3d{64.0, 64.0, -16.0},
        vm::vec3d{64.0, 0.0, 0.0})
      != nullptr);
    CHECK(
      findFaceByPoints(
        faces,
        vm::vec3d{64.0, 64.0, 0.0},
        vm::vec3d{64.0, 0.0, 0.0},
        vm::vec3d{0.0, 64.0, 0.0})
      != nullptr);
  }

  SECTION("Valve220 brush")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) METAL4_5 [ 1 0 0 64 ] [ 0 -1 0 0 ] 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) METAL4_5 [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) METAL4_5 [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) METAL4_5 [ 1 0 0 64 ] [ 0 0 -1 0 ] 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) METAL4_5 [ 1 0 0 64 ] [ 0 0 -1 0 ] 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) METAL4_5 [ 1 0 0 64 ] [ 0 -1 0 0 ] 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Valve, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, true);
  }

  SECTION("Quake 2 brush format")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) attribsExplicit 56 -32 0 1 1 8 9 700
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) attribsOmitted 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) attribsExplicitlyZero 16 96 0 1 1 0 0 0
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1 0 0 0
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();

    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, false);

    SECTION("surface attributes for face attribsExplicit")
    {
      auto faceIndex = brush->brush().findFace("attribsExplicit");
      REQUIRE(faceIndex);

      auto& face = brush->brush().face(*faceIndex);

      CHECK(face.attributes().hasSurfaceAttributes());
      CHECK(face.attributes().surfaceContents() == 8);
      CHECK(face.attributes().surfaceFlags() == 9);
      CHECK(face.attributes().surfaceValue() == 700.0f);
    }

    SECTION("surface attributes for face attribsOmitted")
    {
      auto faceIndex = brush->brush().findFace("attribsOmitted");
      REQUIRE(faceIndex);

      auto& face = brush->brush().face(*faceIndex);

      CHECK(!face.attributes().hasSurfaceAttributes());
      CHECK(!face.attributes().surfaceContents());
      CHECK(!face.attributes().surfaceFlags());
      CHECK(!face.attributes().surfaceValue());
    }

    SECTION("surface attributes for face attribsExplicitlyZero")
    {
      auto faceIndex = brush->brush().findFace("attribsExplicitlyZero");
      REQUIRE(faceIndex);

      auto& face = brush->brush().face(*faceIndex);

      CHECK(face.attributes().hasSurfaceAttributes());
      CHECK(face.attributes().surfaceContents() == 0);
      CHECK(face.attributes().surfaceFlags() == 0);
      CHECK(face.attributes().surfaceValue() == 0.0f);
    }
  }

  SECTION("Quake 2 Valve220 brush format")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"mapversion" "220"
// brush 0
{
( 208 190 80 ) ( 208 -62 80 ) ( 208 190 -176 ) e1u2/basic1_1 [ -0.625 1 0 34 ] [ 0 0 -1 0 ] 32.6509 1 1 0 1 0
( 224 200 80 ) ( 208 190 80 ) ( 224 200 -176 ) e1u2/basic1_1 [ -1 0 0 32 ] [ 0 0 -1 0 ] 35.6251 1 1 0 1 0
( 224 200 -176 ) ( 208 190 -176 ) ( 224 -52 -176 ) e1u2/basic1_1 [ -1 0 0 32 ] [ 0.625 -1 0 -4 ] 35.6251 1 1 0 1 0
( 224 -52 80 ) ( 208 -62 80 ) ( 224 200 80 ) e1u2/basic1_1 [ 1 0 0 -32 ] [ 0.625 -1 0 -4 ] 324.375 1 1 0 1 0
( 224 -52 -176 ) ( 208 -62 -176 ) ( 224 -52 80 ) e1u2/basic1_1 [ 1 0 0 -23.7303 ] [ 0 0 -1 0 ] 35.6251 1 1 0 1 0
( 224 -52 80 ) ( 224 200 80 ) ( 224 -52 -176 ) e1u2/basic1_1 [ -0.625 1 0 44 ] [ 0 0 -1 0 ] 32.6509 1 1 0 1 0
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2_Valve, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, true);
  }

  SECTION("Quake 3 Valve220 brush format")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"mapversion" "220"
// brush 0
{
( 208 190 80 ) ( 208 -62 80 ) ( 208 190 -176 ) gothic_block/blocks18c_3 [ -0.625 1 0 34 ] [ 0 0 -1 0 ] 32.6509 0.25 0.25 0 0 0
( 224 200 80 ) ( 208 190 80 ) ( 224 200 -176 ) gothic_block/blocks18c_3 [ -1 0 0 32 ] [ 0 0 -1 0 ] 35.6251 0.25 0.25 0 1 0
( 224 200 -176 ) ( 208 190 -176 ) ( 224 -52 -176 ) gothic_block/blocks18c_3 [ -1 0 0 32 ] [ 0.625 -1 0 -4 ] 35.6251 0.25 0.25 0 0 0
( 224 -52 80 ) ( 208 -62 80 ) ( 224 200 80 ) gothic_block/blocks18c_3 [ 1 0 0 -32 ] [ 0.625 -1 0 -4 ] 324.375 0.25 0.25 0 0 0
( 224 -52 -176 ) ( 208 -62 -176 ) ( 224 -52 80 ) gothic_block/blocks18c_3 [ 1 0 0 -23.7303 ] [ 0 0 -1 0 ] 35.6251 0.25 0.25 0 0 0
( 224 -52 80 ) ( 224 200 80 ) ( 224 -52 -176 ) gothic_block/blocks18c_3 [ -0.625 1 0 44 ] [ 0 0 -1 0 ] 32.6509 0.25 0.25 0 0 0
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake3_Valve, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, true);
  }

  SECTION("Daikatana brush format")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3cw 56 -32 0 1 1 0 0 0 5 6 7
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1 1 2 3 8 9 10
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3cww 16 96 0 1 1
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1 0 0 0
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Daikatana, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);

    const auto* brushNode =
      static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brushNode, false);
    const auto& brush = brushNode->brush();

    const auto c_mf_v3cw_index = brush.findFace("rtz/c_mf_v3cw");
    const auto b_rc_v16w_index = brush.findFace("rtz/b_rc_v16w");
    const auto c_mf_v3cww_index = brush.findFace("rtz/c_mf_v3cww");
    REQUIRE(c_mf_v3cw_index);
    REQUIRE(b_rc_v16w_index);
    REQUIRE(c_mf_v3cww_index);

    CHECK(brush.face(*c_mf_v3cw_index).attributes().color() == Color{RgbB{5, 6, 7}});
    CHECK(brush.face(*b_rc_v16w_index).attributes().surfaceContents() == 1);
    CHECK(brush.face(*b_rc_v16w_index).attributes().surfaceFlags() == 2);
    CHECK(brush.face(*b_rc_v16w_index).attributes().surfaceValue() == 3.0);
    CHECK(brush.face(*b_rc_v16w_index).attributes().color() == Color{RgbB{8, 9, 10}});
    CHECK_FALSE(brush.face(*c_mf_v3cww_index).attributes().hasColor());
  }

  SECTION("Daikatana map header")
  {
    const auto data = R"(
////////////////////////////////////////////////////////////
// ldef 000 "Base Brush Layer"
////////////////////////////////////////////////////////////
{
"angle" "0"
"mapname" "Plague Poundings"
"cloud2speed" "2"
"lightningfreq" "1"
"classname" "worldspawn"
"sky" "e3m1"
"palette" "e3m1"
"episode" "3"
"ambient" "5"
"cloudname" "mtntile"
"musictrack" "E3C"
// brush 0  layer 000
{
( 1024 1520 0 ) ( 864 1520 160 ) ( 864 1728 160 ) e3m1/thatch2sno 49 0 90 1 1 134217728 16384 0
( 960 1488 48 ) ( 1008 1488 0 ) ( 1008 1872 0 ) e3m1/roof03 -83 45 -180 1 1 134217728 1024 0
( 1008 2152 -48 ) ( 1024 2152 -48 ) ( 944 2152 80 ) e3m1/rooftrim 32 13 135 1 -0.500000 134217728 0 0
( 944 1536 72 ) ( 944 1792 64 ) ( 944 1792 80 ) e3m1/rooftrim 32 -31 133 0.999905 -0.499926 134217728 0 0
( 1024 2144 -48 ) ( 1008 2144 -48 ) ( 1032 2120 -24 ) e3m1/rooftrim -18 -26 -135 0.999873 -0.499936 134217728 0 0
( 968 2120 -48 ) ( 944 2120 -48 ) ( 956 2120 80 ) e3m1/rooftrim -18 -26 -135 0.999873 -0.499936 134217728 0 0
}
}
)";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Daikatana, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, false);
  }

  SECTION("Standard brush with numeric material name")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) c_mf_v3c 56 -32 0 1 1
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) b_rc_v16w 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) 666 16 96 0 1 1
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) c_mf_v3c 56 96 0 1 1
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) c_mf_v3c 56 96 0 1 1
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) c_mf_v3c 16 96 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, false);
  }

  SECTION("Layer with brushes")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1
}
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1
}
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "My Layer"
"_tb_id" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 2u);

    auto* defaultLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(0));
    auto* myLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(1));
    CHECK(defaultLayerNode != nullptr);
    CHECK(myLayerNode != nullptr);

    CHECK(defaultLayerNode->layer().sortIndex() == mdl::Layer::defaultLayerSortIndex());
    // The layer didn't have a sort index (saved in an older version of TB), so it's
    // assigned 0
    CHECK(myLayerNode->layer().sortIndex() == 0);

    CHECK(defaultLayerNode->childCount() == 2u);
    CHECK(myLayerNode->childCount() == 1u);
    CHECK(!myLayerNode->hidden());
    CHECK(!myLayerNode->locked());
  }

  SECTION("Ordered layers")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 1"
"_tb_id" "1"
"_tb_layer_sort_index" "1"
"_tb_layer_locked" "1"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 0"
"_tb_id" "2"
"_tb_layer_sort_index" "0"
"_tb_layer_hidden" "1"
"_tb_layer_omit_from_export" "1"
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world->childCount() == 3u);

    // NOTE: They are listed in world->children() in file order, not sort index order
    auto* defaultLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(0));
    auto* sortNode1 = dynamic_cast<mdl::LayerNode*>(world->children().at(1));
    auto* sortNode0 = dynamic_cast<mdl::LayerNode*>(world->children().at(2));

    REQUIRE(defaultLayerNode != nullptr);
    REQUIRE(sortNode0 != nullptr);
    REQUIRE(sortNode1 != nullptr);

    CHECK(sortNode0->name() == "Sort Index 0");
    CHECK(sortNode1->name() == "Sort Index 1");

    CHECK(defaultLayerNode->layer().sortIndex() == mdl::Layer::defaultLayerSortIndex());
    CHECK(sortNode0->layer().sortIndex() == 0);
    CHECK(sortNode1->layer().sortIndex() == 1);

    CHECK(sortNode0->hidden());
    CHECK(!sortNode1->hidden());

    CHECK(!sortNode0->locked());
    CHECK(sortNode1->locked());

    CHECK(sortNode0->layer().omitFromExport());
    CHECK(!sortNode1->layer().omitFromExport());
  }

  SECTION("Ordered layers with gaps")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 5"
"_tb_id" "1"
"_tb_layer_sort_index" "5"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 3"
"_tb_id" "2"
"_tb_layer_sort_index" "3"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 1"
"_tb_id" "3"
"_tb_layer_sort_index" "1"
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 4u);

    // NOTE: They are listed in world->children() in file order, not sort index order
    auto* defaultLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(0));
    auto* sortNode5 = dynamic_cast<mdl::LayerNode*>(world->children().at(1));
    auto* sortNode3 = dynamic_cast<mdl::LayerNode*>(world->children().at(2));
    auto* sortNode1 = dynamic_cast<mdl::LayerNode*>(world->children().at(3));

    REQUIRE(nullptr != defaultLayerNode);
    REQUIRE(nullptr != sortNode1);
    REQUIRE(nullptr != sortNode3);
    REQUIRE(nullptr != sortNode5);

    CHECK(sortNode1->name() == "Sort Index 1");
    CHECK(sortNode3->name() == "Sort Index 3");
    CHECK(sortNode5->name() == "Sort Index 5");

    CHECK(defaultLayerNode->layer().sortIndex() == mdl::Layer::defaultLayerSortIndex());
    // We allow gaps in sort indices so they remain 1, 3, 5
    CHECK(sortNode1->layer().sortIndex() == 1);
    CHECK(sortNode3->layer().sortIndex() == 3);
    CHECK(sortNode5->layer().sortIndex() == 5);
  }

  SECTION("Ordered layers with gaps and duplicates")
  {
    const auto data = R"end(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index -1"
"_tb_id" "1"
"_tb_layer_sort_index" "-1"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 8"
"_tb_id" "2"
"_tb_layer_sort_index" "8"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 8 (second)"
"_tb_id" "3"
"_tb_layer_sort_index" "8"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 10"
"_tb_id" "4"
"_tb_layer_sort_index" "10"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 10 (second)"
"_tb_id" "5"
"_tb_layer_sort_index" "10"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Sort Index 12"
"_tb_id" "6"
"_tb_layer_sort_index" "12"
})end";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 7u);

    // NOTE: They are listed in world->children() in file order, not sort index order
    auto* defaultLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(0));
    auto* sortMinusOneNode = dynamic_cast<mdl::LayerNode*>(world->children().at(1));
    auto* sortNode8 = dynamic_cast<mdl::LayerNode*>(world->children().at(2));
    auto* sortNode8second = dynamic_cast<mdl::LayerNode*>(world->children().at(3));
    auto* sortNode10 = dynamic_cast<mdl::LayerNode*>(world->children().at(4));
    auto* sortNode10second = dynamic_cast<mdl::LayerNode*>(world->children().at(5));
    auto* sortNode12 = dynamic_cast<mdl::LayerNode*>(world->children().at(6));

    REQUIRE(nullptr != defaultLayerNode);
    REQUIRE(nullptr != sortMinusOneNode);
    REQUIRE(nullptr != sortNode8);
    REQUIRE(nullptr != sortNode8second);
    REQUIRE(nullptr != sortNode10);
    REQUIRE(nullptr != sortNode10second);
    REQUIRE(nullptr != sortNode12);

    CHECK(sortMinusOneNode->name() == "Sort Index -1");
    CHECK(sortNode8->name() == "Sort Index 8");
    CHECK(sortNode8second->name() == "Sort Index 8 (second)");
    CHECK(sortNode10->name() == "Sort Index 10");
    CHECK(sortNode10second->name() == "Sort Index 10 (second)");
    CHECK(sortNode12->name() == "Sort Index 12");

    CHECK(defaultLayerNode->layer().sortIndex() == mdl::Layer::defaultLayerSortIndex());

    // This one was invalid so it got moved to the end
    CHECK(sortMinusOneNode->layer().sortIndex() == 13);
    CHECK(sortNode8->layer().sortIndex() == 8);

    // This one was invalid so it got moved to the end
    CHECK(sortNode8second->layer().sortIndex() == 14);
    CHECK(sortNode10->layer().sortIndex() == 10);

    // This one was invalid so it got moved to the end
    CHECK(sortNode10second->layer().sortIndex() == 15);
    CHECK(sortNode12->layer().sortIndex() == 12);
  }

  SECTION("Layer with entity and brushes")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1
}
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1
}
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "My Layer"
"_tb_id" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
}
{
"classname" "func_door"
"_tb_layer" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 2u);
    CHECK(world->children().front()->childCount() == 2u); // default layer
    CHECK(world->children().back()->childCount() == 2u);  // My Layer
    CHECK(world->children().back()->children().back()->childCount() == 1u);
  }

  SECTION("Grouped entities and brushes")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1
}
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1
}
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "My Group"
"_tb_id" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
}
{
"classname" "func_door"
"_tb_group" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "My Subroup"
"_tb_id" "2"
"_tb_group" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 1u);

    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 3u);

    auto* myGroup = defaultLayer->children().back();
    CHECK(myGroup->childCount() == 3u);

    auto* mySubGroup = myGroup->children().back();
    CHECK(mySubGroup->childCount() == 1u);
  }

  SECTION("Parsed groups and layers retain their IDs")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Layer"
"_tb_id" "7"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 1"
"_tb_id" "7"
"_tb_layer" "7"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 2"
"_tb_id" "22"
}
)";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->childCount() == 2u);

    // NOTE: They are listed in world->children() in file order, not sort index order
    auto* defaultLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(0));
    auto* customLayerNode = dynamic_cast<mdl::LayerNode*>(world->children().at(1));

    REQUIRE(defaultLayerNode != nullptr);
    REQUIRE(customLayerNode != nullptr);

    auto* groupNode1 = dynamic_cast<mdl::GroupNode*>(customLayerNode->children().front());
    auto* groupNode2 =
      dynamic_cast<mdl::GroupNode*>(defaultLayerNode->children().front());

    REQUIRE(groupNode1 != nullptr);
    REQUIRE(groupNode2 != nullptr);

    CHECK(world->defaultLayer()->persistentId() == std::nullopt);
    CHECK(customLayerNode->persistentId() == 7u);
    CHECK(groupNode1->persistentId() == 7u);
    CHECK(groupNode2->persistentId() == 22u);
  }

  SECTION("Brush primitive")
  {
    const auto data = R"(
            {
                "classname" "worldspawn"
                {
                    brushDef
                    {
                        ( -64 64 64 ) ( 64 -64 64 ) ( -64 -64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
                        ( -64 64 64 ) ( 64 64 -64 ) ( 64 64 64 ) ( ( 0.015625 0 0 ) ( 0 0.015625 0 ) ) common/caulk 0 0 0
                        ( 64 64 64 ) ( 64 -64 -64 ) ( 64 -64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
                        ( 64 64 -64 ) ( -64 -64 -64 ) ( 64 -64 -64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
                        ( 64 -64 -64 ) ( -64 -64 64 ) ( 64 -64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
                        ( -64 -64 64 ) ( -64 64 -64 ) ( -64 64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
                    }
                }
            })";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake3, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    // TODO 2427: Assert one brush!
    CHECK(world->defaultLayer()->childCount() == 0u);
  }

  SECTION("Brush primitive and legacy brush")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
brushDef
{
( -64 64 64 ) ( 64 -64 64 ) ( -64 -64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
( -64 64 64 ) ( 64 64 -64 ) ( 64 64 64 ) ( ( 0.015625 0 0 ) ( 0 0.015625 0 ) ) common/caulk 0 0 0
( 64 64 64 ) ( 64 -64 -64 ) ( 64 -64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
( 64 64 -64 ) ( -64 -64 -64 ) ( 64 -64 -64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
( 64 -64 -64 ) ( -64 -64 64 ) ( 64 -64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
( -64 -64 64 ) ( -64 64 -64 ) ( -64 64 64 ) ( ( 0.015625 0 -0 ) ( -0 0.015625 0 ) ) common/caulk 0 0 0
}
}
{
( 64 64 64 ) ( 64 -64 64 ) ( -64 64 64 ) common/caulk 0 0 0 1 1 134217728 0 0
( 64 64 64 ) ( -64 64 64 ) ( 64 64 -64 ) common/caulk 0 0 0 1 1 134217728 0 0
( 64 64 64 ) ( 64 64 -64 ) ( 64 -64 64 ) common/caulk 0 0 0 1 1 134217728 0 0
( -64 -64 -64 ) ( 64 -64 -64 ) ( -64 64 -64 ) common/caulk 0 0 0 1 1 134217728 0 0
( -64 -64 -64 ) ( -64 -64 64 ) ( 64 -64 -64 ) common/caulk 0 0 0 1 1 134217728 0 0
( -64 -64 -64 ) ( -64 64 -64 ) ( -64 -64 64 ) common/caulk 0 0 0 1 1 134217728 0 0
}
})";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake3, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    // TODO 2427: Assert two brushes!
    CHECK(world->defaultLayer()->childCount() == 1u);
  }

  SECTION("Quake 3 patch")
  {
    const auto data = R"(
{
"classname" "worldspawn"
{
patchDef2
{
common/caulk
( 5 3 0 0 0 )
(
( (-64 -64 4 0   0 ) (-64 0 4 0   -0.25 ) (-64 64 4 0   -0.5 ) )
( (  0 -64 4 0.2 0 ) (  0 0 4 0.2 -0.25 ) (  0 64 4 0.2 -0.5 ) )
( ( 64 -64 4 0.4 0 ) ( 64 0 4 0.4 -0.25 ) ( 64 64 4 0.4 -0.5 ) )
( (128 -64 4 0.6 0 ) (128 0 4 0.6 -0.25 ) (128 64 4 0.6 -0.5 ) )
( (192 -64 4 0.8 0 ) (192 0 4 0.8 -0.25 ) (192 64 4 0.8 -0.5 ) )
)
}
}
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake3, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    CHECK(world->defaultLayer()->childCount() == 1u);

    const auto* patchNode =
      dynamic_cast<mdl::PatchNode*>(world->defaultLayer()->children().front());
    CHECK(patchNode != nullptr);

    const auto& patch = patchNode->patch();
    CHECK(patch.materialName() == "common/caulk");
    CHECK(patch.pointRowCount() == 5);
    CHECK(patch.pointColumnCount() == 3);

    CHECK_THAT(
      patch.controlPoints(),
      Equals(std::vector<mdl::BezierPatch::Point>{
        {-64, -64, 4, 0, 0},
        {-64, 0, 4, 0, -0.25},
        {-64, 64, 4, 0, -0.5},
        {0, -64, 4, 0.2, 0},
        {0, 0, 4, 0.2, -0.25},
        {0, 64, 4, 0.2, -0.5},
        {64, -64, 4, 0.4, 0},
        {64, 0, 4, 0.4, -0.25},
        {64, 64, 4, 0.4, -0.5},
        {128, -64, 4, 0.6, 0},
        {128, 0, 4, 0.6, -0.25},
        {128, 64, 4, 0.6, -0.5},
        {192, -64, 4, 0.8, 0},
        {192, 0, 4, 0.8, -0.25},
        {192, 64, 4, 0.8, -0.5},
      }));
  }

  SECTION("Multiple classnames")
  {
    // See https://github.com/TrenchBroom/TrenchBroom/issues/1485

    const auto data = R"(
{
"classname" "worldspawn"
"classname" "worldspawn"
})";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Quake2, {}};

    CHECK_NOTHROW(reader.read(worldBounds, status, taskManager));
  }

  SECTION("Escaped double quotation marks")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"message" "yay \"Mr. Robot!\""
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->childCount() == 1u);
    CHECK_FALSE(worldNode->children().front()->hasChildren());

    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("message"));
    CHECK(*worldNode->entity().property("message") == "yay \\\"Mr. Robot!\\\"");
  }

  SECTION("Property with unescaped path and trailing backslash")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"path" "c:\a\b\c\"
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->childCount() == 1u);
    CHECK_FALSE(worldNode->children().front()->hasChildren());

    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("path"));
    CHECK(*worldNode->entity().property("path") == "c:\\a\\b\\c\\");
  }

  SECTION("Property with escaped path and trailing backslash")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"path" "c:\\a\\b\\c\\"
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->childCount() == 1u);
    CHECK_FALSE(worldNode->children().front()->hasChildren());

    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("path"));
    CHECK(*worldNode->entity().property("path") == "c:\\\\a\\\\b\\\\c\\\\");
  }

  SECTION("Property with trailing escaped backslash")
  {
    const auto data = R"(
{
"classname" "worldspawn"
"message" "test\\"
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->childCount() == 1u);
    CHECK_FALSE(worldNode->children().front()->hasChildren());

    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("message"));
    CHECK(*worldNode->entity().property("message") == "test\\\\");
  }

  SECTION("Property with newline escape sequence")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1739
    const auto data = R"(
{
"classname" "worldspawn"
"message" "vm::line1\nvm::line2d"
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    CHECK(worldNode != nullptr);
    CHECK(worldNode->childCount() == 1u);
    CHECK_FALSE(worldNode->children().front()->hasChildren());

    CHECK(worldNode->entity().hasProperty(mdl::EntityPropertyKeys::Classname));
    CHECK(worldNode->entity().hasProperty("message"));
    CHECK(*worldNode->entity().property("message") == "vm::line1\\nvm::line2d");
  }

  SECTION("Heretic 2 map made in Quark")
  {
    const auto mapPath =
      std::filesystem::current_path() / "fixture/test/mdl/WorldReader/Heretic2Quark.map";
    const auto file = fs::Disk::openFile(mapPath) | kdl::value();
    auto fileReader = file->reader().buffer();

    auto worldReader = WorldReader{{}, fileReader.stringView(), mdl::MapFormat::Quake2, {}};
    auto worldResult = worldReader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    REQUIRE(worldNode != nullptr);
    REQUIRE(1u == worldNode->childCount());

    auto* layerNode = dynamic_cast<mdl::LayerNode*>(worldNode->children().at(0));
    REQUIRE(layerNode != nullptr);
    REQUIRE(1u == layerNode->childCount());

    auto* brushNode = dynamic_cast<mdl::BrushNode*>(layerNode->children().at(0));
    REQUIRE(brushNode != nullptr);

    CHECK(brushNode->logicalBounds() == vm::bbox3d{{-512, -512, -64}, {512, 512, 0}});
    for (const auto& face : brushNode->brush().faces())
    {
      CHECK("general/sand1" == face.attributes().materialName());
    }
  }

  SECTION("__TB_empty material name")
  {
    const auto data = R"(
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty 0 0 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty 0 0 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty 0 0 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty 0 0 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty 0 0 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty 0 0 0 1 1
}
})";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    REQUIRE(world->childCount() == 1u);

    auto* defaultLayer = dynamic_cast<mdl::LayerNode*>(world->children().front());
    REQUIRE(defaultLayer != nullptr);
    REQUIRE(defaultLayer->childCount() == 1u);

    auto* brush = dynamic_cast<mdl::BrushNode*>(defaultLayer->children().front());
    REQUIRE(brush != nullptr);

    for (const auto& face : brush->brush().faces())
    {
      CHECK(!face.attributes().materialName().empty());
      CHECK(face.attributes().materialName() == mdl::BrushFaceAttributes::NoMaterialName);
    }
  }

  SECTION("Quoted material names")
  {
    using NameInfo = std::tuple<std::string, std::string>;

    // clang-format off
  const auto 
  [materialName,      expectedName] = GENERATE(values<NameInfo>({
  {R"(some_name)",    R"(some_name)"},
  {R"("some name")",  R"(some name)"},
  {R"("some\\name")", R"(some\name)"},
  {R"("some\"name")", R"(some"name)"},
  {R"("")",           R"()"},
  }));
    // clang-format on

    CAPTURE(materialName, expectedName);

    const auto data = fmt::format(
      R"(
// entity 0
{{
"classname" "worldspawn"
// brush 0
{{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) {0} 0 0 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) {0} 0 0 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) {0} 0 0 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) {0} 0 0 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) {0} 0 0 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) {0} 0 0 0 1 1
}}
}})",
      materialName);


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& worldNode = worldResult.value();
    REQUIRE(worldNode != nullptr);
    REQUIRE(worldNode->childCount() == 1u);

    const auto* defaultLayerNode =
      dynamic_cast<mdl::LayerNode*>(worldNode->children().front());
    REQUIRE(defaultLayerNode != nullptr);
    REQUIRE(defaultLayerNode->childCount() == 1u);

    const auto* brushNode =
      dynamic_cast<mdl::BrushNode*>(defaultLayerNode->children().front());
    REQUIRE(brushNode != nullptr);

    CHECK(brushNode->brush().face(0).attributes().materialName() == expectedName);
  }

  SECTION("Linked groups")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 1"
"_tb_id" "1"
"_tb_linked_group_id" "abcd"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 2"
"_tb_id" "2"
"_tb_linked_group_id" "abcd"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}
            )";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    CHECK(world->defaultLayer()->childCount() == 2u);

    auto* groupNode1 =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children().front());
    auto* groupNode2 =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children().back());

    REQUIRE(groupNode1 != nullptr);
    REQUIRE(groupNode2 != nullptr);

    CHECK(groupNode1->linkId() == "abcd");
    CHECK(groupNode2->linkId() == "abcd");

    CHECK(
      groupNode1->group().transformation()
      == vm::translation_matrix(vm::vec3d{32.0, 0.0, 0.0}));
    CHECK(
      groupNode2->group().transformation()
      == vm::translation_matrix(vm::vec3d{32.0, 16.0, 0.0}));
  }

  SECTION("Orphaned linked groups")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 1"
"_tb_id" "1"
"_tb_linked_group_id" "abcd"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
            )";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    CHECK(world->defaultLayer()->childCount() == 1);

    auto* groupNode =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children().front());

    CHECK(groupNode != nullptr);
    CHECK(groupNode->linkId() == "abcd");
    CHECK(
      groupNode->group().transformation() == vm::translation_matrix(vm::vec3d{32, 0, 0}));
  }

  SECTION("Linked group with missing transformation")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 1"
"_tb_id" "1"
"_tb_linked_group_id" "1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 2"
"_tb_id" "2"
"_tb_linked_group_id" "1"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 3"
"_tb_id" "3"
"_tb_linked_group_id" "1"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}
            )";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    CHECK(world->defaultLayer()->childCount() == 3u);

    auto* groupNode1 =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[0]);
    auto* groupNode2 =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[1]);
    auto* groupNode3 =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[2]);

    REQUIRE(groupNode1 != nullptr);
    REQUIRE(groupNode2 != nullptr);
    REQUIRE(groupNode3 != nullptr);

    CHECK(groupNode1->linkId() == "1");
    CHECK(groupNode2->linkId() == "1");
    CHECK(groupNode3->linkId() == "1");

    CHECK(groupNode1->group().transformation() == vm::mat4x4d::identity());
    CHECK(
      groupNode2->group().transformation()
      == vm::translation_matrix(vm::vec3d{32.0, 16.0, 0.0}));
    CHECK(
      groupNode3->group().transformation()
      == vm::translation_matrix(vm::vec3d{32.0, 16.0, 0.0}));
  }

  SECTION("Group with unnecessary transformation")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group 1"
"_tb_id" "1"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
            )";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    CHECK(world->defaultLayer()->childCount() == 1u);

    auto* groupNode =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children().front());
    CHECK(groupNode != nullptr);

    CHECK(groupNode->group().transformation() == vm::mat4x4d{});
  }

  SECTION("Recursive linked groups")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_1_abcd"
"_tb_id" "1"
"_tb_linked_group_id" "abcd"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_1_1_abcd"
"_tb_id" "2"
"_tb_group" "1"
"_tb_linked_group_id" "abcd"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_2_xyz"
"_tb_id" "3"
"_tb_linked_group_id" "xyz"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_2_1_xyz"
"_tb_id" "4"
"_tb_group" "3"
"_tb_linked_group_id" "xyz"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_3_xyz"
"_tb_id" "5"
"_tb_linked_group_id" "xyz"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_3_1"
"_tb_id" "6"
"_tb_group" "5"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_4_fgh"
"_tb_id" "7"
"_tb_linked_group_id" "fgh"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_4_1"
"_tb_id" "8"
"_tb_group" "7"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "groupNode_4_1_1_fgh"
"_tb_id" "9"
"_tb_group" "8"
"_tb_linked_group_id" "fgh"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}
            )";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    REQUIRE(world->defaultLayer()->childCount() == 4u);

    const auto* groupNode_1_abcd =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[0]);

    REQUIRE(groupNode_1_abcd->childCount() == 1u);
    const auto* groupNode_1_2_abcd =
      dynamic_cast<mdl::GroupNode*>(groupNode_1_abcd->children().front());

    const auto* groupNode_2_xyz =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[1]);

    REQUIRE(groupNode_2_xyz->childCount() == 1u);
    const auto* groupNode_2_1_xyz =
      dynamic_cast<mdl::GroupNode*>(groupNode_2_xyz->children().front());

    const auto* groupNode_3_xyz =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[2]);

    const auto* groupNode_4_fgh =
      dynamic_cast<mdl::GroupNode*>(world->defaultLayer()->children()[3]);

    REQUIRE(groupNode_4_fgh->childCount() == 1u);
    const auto* groupNode_4_1 =
      dynamic_cast<mdl::GroupNode*>(groupNode_4_fgh->children().front());

    REQUIRE(groupNode_4_1->childCount() == 1u);
    const auto* groupNode_4_1_1_fgh =
      dynamic_cast<mdl::GroupNode*>(groupNode_4_1->children().front());

    CHECK(groupNode_1_abcd->linkId() == "abcd");
    CHECK(
      groupNode_1_abcd->group().transformation()
      == vm::translation_matrix(vm::vec3d{32, 0, 0}));
    CHECK(groupNode_1_2_abcd->linkId() != "abcd");
    CHECK(groupNode_1_2_abcd->group().transformation() == vm::mat4x4d::identity());

    CHECK(groupNode_2_xyz->linkId() == "xyz");
    CHECK(
      groupNode_2_xyz->group().transformation()
      == vm::translation_matrix(vm::vec3d{32, 0, 0}));
    CHECK(groupNode_2_1_xyz->linkId() != "xyz");
    CHECK(groupNode_2_1_xyz->group().transformation() == vm::mat4x4d::identity());
    CHECK(groupNode_3_xyz->linkId() == "xyz");
    CHECK(
      groupNode_3_xyz->group().transformation()
      == vm::translation_matrix(vm::vec3d{32, 0, 0}));

    CHECK(groupNode_4_fgh->linkId() == "fgh");
    CHECK(
      groupNode_4_fgh->group().transformation()
      == vm::translation_matrix(vm::vec3d{32, 0, 0}));
    CHECK(groupNode_4_1->group().transformation() == vm::mat4x4d::identity());
    CHECK(groupNode_4_1_1_fgh->linkId() != "fgh");
    CHECK(groupNode_4_1_1_fgh->group().transformation() == vm::mat4x4d::identity());
  }

  SECTION("Protected entity properties")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
{
"classname" "info_player_start"
"_tb_protected_properties" ""
}
{
"classname" "info_player_start"
"_tb_protected_properties" "origin;target"
}
{
"classname" "info_player_start"
"_tb_protected_properties" "with\;semicolon"
}
            )";


    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};

    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    CHECK(world->defaultLayer()->childCount() == 3u);

    SECTION("Empty list")
    {
      auto* entityNode =
        dynamic_cast<mdl::EntityNode*>(world->defaultLayer()->children()[0]);
      REQUIRE(entityNode != nullptr);

      CHECK_THAT(
        entityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{}));
    }

    SECTION("Two protected properties")
    {
      auto* entityNode =
        dynamic_cast<mdl::EntityNode*>(world->defaultLayer()->children()[1]);
      REQUIRE(entityNode != nullptr);

      CHECK_THAT(
        entityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{"origin", "target"}));
    }

    SECTION("Escaped semicolon")
    {
      auto* entityNode =
        dynamic_cast<mdl::EntityNode*>(world->defaultLayer()->children()[2]);
      REQUIRE(entityNode != nullptr);

      CHECK_THAT(
        entityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{"with;semicolon"}));
    }
  }

  SECTION("Empty map with unknown format")
  {
    const auto data = R"(
{
"classname" "worldspawn"
}
            )";


    auto worldResult = WorldReader::tryRead(
      {},
      data,
      {mdl::MapFormat::Standard, mdl::MapFormat::Valve},
      worldBounds,
      {},
      status,
      taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);
    CHECK(world->mapFormat() == mdl::MapFormat::Standard);
  }
}

TEST_CASE("WorldReader (Regression)", "[regression]")
{
  auto taskManager = kdl::task_manager{};
  const auto worldBounds = vm::bbox3d{8192.0};
  auto status = TestParserStatus{};

  SECTION("1424")
  {
    const auto data = R"(
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
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};
    auto world = reader.read(worldBounds, status, taskManager);
    CHECK(world != nullptr);
  }

  SECTION("Problematic brush 1")
  {
    const auto data = R"(
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
})";

    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};
    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);

    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);

    auto* brushNode = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brushNode, false);
    const auto& faces = brushNode->brush().faces();
    CHECK(faces.size() == 6u);
    CHECK(findFaceByPoints(faces, {308, 108, 176}, {308, 132, 176}, {252, 132, 176}));
    CHECK(findFaceByPoints(faces, {252, 132, 208}, {308, 132, 208}, {308, 108, 208}));
    CHECK(findFaceByPoints(faces, {288, 152, 176}, {288, 152, 208}, {288, 120, 208}));
    CHECK(findFaceByPoints(faces, {288, 122, 176}, {288, 122, 208}, {308, 102, 208}));
    CHECK(findFaceByPoints(faces, {308, 100, 176}, {308, 100, 208}, {324, 116, 208}));
    CHECK(findFaceByPoints(faces, {287, 152, 208}, {287, 152, 176}, {323, 116, 176}));
  }

  SECTION("Problematic brush 2")
  {
    const auto data = R"(
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
})";
    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};
    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);

    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, false);
  }

  SECTION("Problematic brush 3")
  {
    const auto data = R"(
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
})";
    auto reader = WorldReader{{}, data, mdl::MapFormat::Standard, {}};
    auto worldResult = reader.read(worldBounds, status, taskManager);
    REQUIRE(worldResult);

    const auto& world = worldResult.value();
    REQUIRE(world != nullptr);

    CHECK(world->childCount() == 1u);
    auto* defaultLayer = world->children().front();
    CHECK(defaultLayer->childCount() == 1u);
    auto* brush = static_cast<mdl::BrushNode*>(defaultLayer->children().front());
    checkBrushUVCoordSystem(brush, false);
  }
}

} // namespace tb::mdl
