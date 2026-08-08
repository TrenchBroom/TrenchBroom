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

#include "TestLogger.h"
#include "gl/Material.h"
#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityModel.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/EntityRotation.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/GameFileSystem.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/LoadEntityModel.h"
#include "mdl/MapFormat.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include "vm/approx.h"
#include "vm/bbox.h"
#include "vm/ray.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("EntityNode")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  SECTION("canAddChild")
  {
    auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(!entityNode.canAddChild(worldNode));
    CHECK(!entityNode.canAddChild(layerNode));
    CHECK(!entityNode.canAddChild(groupNode));
    CHECK(!entityNode.canAddChild(entityNode));
    CHECK(entityNode.canAddChild(brushNode));
    CHECK(entityNode.canAddChild(patchNode));
  }

  SECTION("canRemoveChild")
  {
    const auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(entityNode.canRemoveChild(worldNode));
    CHECK(worldNode.canRemoveChild(layerNode));
    CHECK(entityNode.canRemoveChild(groupNode));
    CHECK(entityNode.canRemoveChild(entityNode));
    CHECK(entityNode.canRemoveChild(brushNode));
    CHECK(entityNode.canRemoveChild(patchNode));
  }

  SECTION("pointEntity")
  {
    auto entityNode = EntityNode{Entity{}};
    auto brushNode1 = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
    auto brushNode2 = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    REQUIRE(entityNode.entity().pointEntity());
    entityNode.addChild(&brushNode1);
    CHECK(!entityNode.entity().pointEntity());
    entityNode.addChild(&brushNode2);
    CHECK(!entityNode.entity().pointEntity());

    entityNode.removeChild(&brushNode1);
    CHECK(!entityNode.entity().pointEntity());
    entityNode.removeChild(&brushNode2);
    CHECK(entityNode.entity().pointEntity());
  }

  SECTION("projectedArea")
  {
    const auto definition = EntityDefinition{
      "some_name",
      Color{},
      "",
      {},
      PointEntityDefinition{
        vm::bbox3d{{0, 0, 0}, {1, 2, 3}},
        {},
        {},
      },
    };

    auto entityNode = EntityNode{Entity{}};
    entityNode.setDefinition(&definition);

    CHECK(entityNode.projectedArea(vm::axis::x) == 6.0);
    CHECK(entityNode.projectedArea(vm::axis::y) == 3.0);
    CHECK(entityNode.projectedArea(vm::axis::z) == 2.0);
  }

  SECTION("pick")
  {
    // cube.bsp is a solid cube spanning {-32, -32, -32} to {32, 32, 32}
    const auto environmentConfig = EnvironmentConfig{};
    const auto& gameInfo = QuakeGameInfo;

    auto logger = TestLogger{};
    auto fs = GameFileSystem{};
    fs.initialize(
      environmentConfig,
      gameInfo.gameConfig,
      gameInfo.gamePathPreference.defaultValue,
      {},
      logger);

    const auto path = std::filesystem::path{"cube.bsp"};
    const auto loadMaterial = [](auto) -> gl::Material {
      throw std::runtime_error{"should not be called"};
    };

    auto model = loadEntityModelSync(
                   fs, gameInfo.gameConfig.materialConfig, path, loadMaterial, logger)
                   .value();

    auto entityNode = EntityNode{Entity{}};
    entityNode.setModel(&model);

    const auto editorContext = EditorContext{};

    // The entity's own (unmodeled) bounds are only {-8, -8, -8} to {8, 8, 8}, so a ray at
    // x = 20 misses that bbox test entirely and falls through to hit-testing the model.
    const auto rayOrigin = vm::vec3d{20, 0, 40};

    SECTION("hits the model when the ray misses the entity's default bounds")
    {
      auto pickResult = PickResult{};
      entityNode.pick(
        editorContext, vm::ray3d{rayOrigin, vm::vec3d{0, 0, -1}}, pickResult);

      REQUIRE(pickResult.size() == 1u);
      CHECK(pickResult.all().front().hitPoint() == vm::approx{vm::vec3d{20, 0, 32}});
    }

    SECTION("misses the model")
    {
      auto pickResult = PickResult{};
      entityNode.pick(
        editorContext, vm::ray3d{rayOrigin, vm::vec3d{0, 0, 1}}, pickResult);

      CHECK(pickResult.size() == 0u);
    }
  }

  SECTION("setEntity updates the origin and the logical bounds")
  {
    const auto newOrigin = vm::vec3d{10, 20, 30};
    const auto newBounds = vm::bbox3d{
      newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
      newOrigin + (EntityNode::DefaultBounds.size() / 2.0)};

    SECTION("when the node is not part of a world")
    {
      auto entityNode =
        EntityNode{Entity{{{EntityPropertyKeys::Classname, "something"}}}};

      entityNode.setEntity(Entity{{{"origin", "10 20 30"}}});
      CHECK(entityNode.entity().origin() == newOrigin);
      CHECK(entityNode.logicalBounds() == newBounds);
    }

    SECTION("when the node is part of a world")
    {
      // the world takes ownership of the entity node
      auto* entityNode =
        new EntityNode{Entity{{{EntityPropertyKeys::Classname, "something"}}}};

      auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
      worldNode.defaultLayer()->addChild(entityNode);

      entityNode->setEntity(Entity{{{"origin", "10 20 30"}}});
      CHECK(entityNode->entity().origin() == newOrigin);
      CHECK(entityNode->logicalBounds() == newBounds);
    }
  }
}

} // namespace tb::mdl
