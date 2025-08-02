/*
 Copyright (C) 2025 Kristian Duske

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
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/ParallelUVCoordSystem.h"
#include "mdl/TestGame.h"
#include "mdl/WorldNode.h"

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::mdl
{
namespace
{

bool hasEmptyName(const std::vector<std::string>& names)
{
  return std::ranges::any_of(names, [](const auto& s) { return s.empty(); });
}

} // namespace

TEST_CASE("Map_Geometry")
{
  auto taskManager = createTestTaskManager();
  auto logger = NullLogger{};
  auto map = Map{*taskManager, logger};

  SECTION("Transform a group containing a brush entity")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1715

    auto* brushNode1 = createBrushNode(map);
    map.addNodes({{map.parentForNodes(), {brushNode1}}});

    auto* entityNode = new EntityNode{Entity{}};
    map.addNodes({{map.parentForNodes(), {entityNode}}});
    map.reparentNodes({{entityNode, {brushNode1}}});

    map.selectNodes({brushNode1});

    auto* groupNode = map.groupSelectedNodes("test");
    CHECK(groupNode->selected());

    CHECK(map.translateSelection(vm::vec3d{16, 0, 0}));
    CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));

    map.undoCommand();

    CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));
  }

  SECTION("Rotating a group containing a brush entity")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1754

    auto* brushNode1 = createBrushNode(map);
    map.addNodes({{map.parentForNodes(), {brushNode1}}});

    auto* entityNode = new EntityNode{Entity{}};
    map.addNodes({{map.parentForNodes(), {entityNode}}});
    map.reparentNodes({{entityNode, {brushNode1}}});

    map.selectNodes({brushNode1});

    auto* groupNode = map.groupSelectedNodes("test");
    CHECK(groupNode->selected());

    CHECK_FALSE(entityNode->entity().hasProperty("origin"));
    CHECK(map.rotateSelection(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, 10.0));
    CHECK_FALSE(entityNode->entity().hasProperty("origin"));

    map.undoCommand();

    CHECK_FALSE(entityNode->entity().hasProperty("origin"));
  }

  SECTION("snapVertices")
  {
    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode != nullptr);

      map.deselectAll();

      SECTION("Can't snap to grid with both groups selected")
      {
        map.selectNodes({groupNode, linkedGroupNode});

        CHECK(
          map.transformSelection("", vm::translation_matrix(vm::vec3d{0.5, 0.5, 0.0})));

        // This could generate conflicts, because what snaps one group could misalign
        // another group in the link set. So, just reject the change.
        CHECK(!map.snapVertices(16.0));
      }
    }
  }

  SECTION("csgConvexMerge")
  {
    SECTION("Merge two brushes")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
        | kdl::value()};
      auto* brushNode2 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      map.addNodes({{entityNode, {brushNode1}}});
      map.addNodes({{map.parentForNodes(), {brushNode2}}});
      CHECK(entityNode->children().size() == 1u);

      map.selectNodes({brushNode1, brushNode2});
      CHECK(map.csgConvexMerge());
      CHECK(entityNode->children().size() == 1u);

      auto* brushNode3 = entityNode->children().front();
      CHECK(brushNode3->logicalBounds() == vm::bbox3d{{0, 0, 0}, {64, 64, 64}});
    }

    SECTION("Merge two faces")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
        | kdl::value()};
      auto* brushNode2 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      map.addNodes({{entityNode, {brushNode1}}});
      map.addNodes({{map.parentForNodes(), {brushNode2}}});
      CHECK(entityNode->children().size() == 1u);

      const auto faceIndex = 0u;
      const auto& face1 = brushNode1->brush().face(faceIndex);
      const auto& face2 = brushNode2->brush().face(faceIndex);

      map.selectBrushFaces({{brushNode1, faceIndex}, {brushNode2, faceIndex}});
      CHECK(map.csgConvexMerge());
      CHECK(
        entityNode->children().size()
        == 2u); // added to the parent of the first brush, original brush is not deleted

      auto* brushNode3 = entityNode->children().back();

      // check our assumption about the order of the entities' children
      assert(brushNode3 != brushNode1);
      assert(brushNode3 != brushNode2);

      const auto face1Verts = face1.vertexPositions();
      const auto face2Verts = face2.vertexPositions();

      const auto bounds = vm::merge(
        vm::bbox3d::merge_all(std::begin(face1Verts), std::end(face1Verts)),
        vm::bbox3d::merge_all(std::begin(face2Verts), std::end(face2Verts)));

      CHECK(brushNode3->logicalBounds() == bounds);
    }

    SECTION("Texture alignment")
    {
      REQUIRE(
        map.create(MapFormat::Valve, vm::bbox3d{8192.0}, std::make_shared<TestGame>())
          .is_success());

      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});

      auto texAlignment = ParallelUVCoordSystem{{1, 0, 0}, {0, 1, 0}};
      auto texAlignmentSnapshot = texAlignment.takeSnapshot();

      auto brush1 = builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
                    | kdl::value();
      brush1.face(*brush1.findFace(vm::vec3d{0, 0, 1}))
        .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

      auto brush2 = builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
                    | kdl::value();
      brush2.face(*brush2.findFace(vm::vec3d{0, 0, 1}))
        .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

      auto* brushNode1 = new BrushNode{std::move(brush1)};
      auto* brushNode2 = new BrushNode{std::move(brush2)};

      map.addNodes({{entityNode, {brushNode1}}});
      map.addNodes({{entityNode, {brushNode2}}});
      CHECK(entityNode->children().size() == 2u);

      map.selectNodes({brushNode1, brushNode2});
      CHECK(map.csgConvexMerge());
      CHECK(entityNode->children().size() == 1u);

      auto* brushNode3 = static_cast<BrushNode*>(entityNode->children()[0]);
      const auto& brush3 = brushNode3->brush();

      const auto& top = brush3.face(*brush3.findFace(vm::vec3d{0, 0, 1}));
      CHECK(top.uAxis() == vm::vec3d{1, 0, 0});
      CHECK(top.vAxis() == vm::vec3d{0, 1, 0});
    }
  }

  SECTION("csgSubtract")
  {
    SECTION("Subtract multiple brushes")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});

      auto* minuendNode = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}}, "material")
        | kdl::value()};
      auto* subtrahendNode1 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{32, 32, 64}}, "material")
        | kdl::value()};
      auto* subtrahendNode2 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{32, 32, 0}, vm::vec3d{64, 64, 64}}, "material")
        | kdl::value()};

      map.addNodes({{entityNode, {minuendNode, subtrahendNode1, subtrahendNode2}}});
      CHECK(entityNode->children().size() == 3u);

      // we want to compute minuend - {subtrahendNode1, subtrahendNode2}
      map.selectNodes({subtrahendNode1, subtrahendNode2});
      CHECK(map.csgSubtract());
      CHECK(entityNode->children().size() == 2u);

      auto* remainderNode1 = dynamic_cast<BrushNode*>(entityNode->children()[0]);
      auto* remainderNode2 = dynamic_cast<BrushNode*>(entityNode->children()[1]);
      CHECK(remainderNode1 != nullptr);
      CHECK(remainderNode2 != nullptr);

      const auto expectedBBox1 = vm::bbox3d{vm::vec3d{0, 32, 0}, vm::vec3d{32, 64, 64}};
      const auto expectedBBox2 = vm::bbox3d{vm::vec3d{32, 0, 0}, vm::vec3d{64, 32, 64}};

      if (remainderNode1->logicalBounds() != expectedBBox1)
      {
        std::swap(remainderNode1, remainderNode2);
      }

      CHECK(remainderNode1->logicalBounds() == expectedBBox1);
      CHECK(remainderNode2->logicalBounds() == expectedBBox2);
    }

    SECTION("Undo restores selection")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});

      auto* subtrahend1 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}}, "material")
        | kdl::value()};
      map.addNodes({{entityNode, {subtrahend1}}});

      map.selectNodes({subtrahend1});
      CHECK(map.csgSubtract());
      CHECK(entityNode->children().size() == 0u);
      CHECK_FALSE(map.selection().hasNodes());

      // check that the selection is restored after undo
      map.undoCommand();

      CHECK(map.selection().hasOnlyBrushes());
      CHECK_THAT(
        map.selection().brushes, Catch::Equals(std::vector<BrushNode*>{subtrahend1}));
    }

    SECTION("Texture alignment")
    {
      REQUIRE(
        map.create(MapFormat::Valve, vm::bbox3d{8192.0}, std::make_shared<TestGame>())
          .is_success());

      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});

      auto texAlignment = ParallelUVCoordSystem{vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}};
      auto texAlignmentSnapshot = texAlignment.takeSnapshot();

      auto brush1 = builder.createCuboid(
                      vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}}, "material")
                    | kdl::value();
      auto brush2 = builder.createCuboid(
                      vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 32}}, "material")
                    | kdl::value();
      brush2.face(*brush2.findFace(vm::vec3d{0, 0, 1}))
        .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

      auto* brushNode1 = new BrushNode{std::move(brush1)};
      auto* brushNode2 = new BrushNode{std::move(brush2)};

      map.addNodes({{entityNode, {brushNode1}}});
      map.addNodes({{entityNode, {brushNode2}}});
      CHECK(entityNode->children().size() == 2u);

      // we want to compute brush1 - brush2
      map.selectNodes({brushNode2});
      CHECK(map.csgSubtract());
      CHECK(entityNode->children().size() == 1u);

      auto* brushNode3 = static_cast<BrushNode*>(entityNode->children()[0]);
      const auto& brush3 = brushNode3->brush();

      CHECK(
        brushNode3->logicalBounds()
        == vm::bbox3d{vm::vec3d{0, 0, 32}, vm::vec3d{64, 64, 64}});

      // the material alignment from the top of brush2 should have transferred
      // to the bottom face of brush3
      const auto& top = brush3.face(*brush3.findFace(vm::vec3d{0, 0, -1}));
      CHECK(top.uAxis() == vm::vec3d{1, 0, 0});
      CHECK(top.vAxis() == vm::vec3d{0, 1, 0});
    }

    SECTION("Regression tests")
    {
      auto [game, gameConfig] = loadGame("Quake");

      // Test for https://github.com/TrenchBroom/TrenchBroom/issues/3755
      REQUIRE(map
                .load(
                  MapFormat::Valve,
                  vm::bbox3d{8192.0},
                  game,
                  "fixture/test/ui/MapDocumentTest/csgSubtractFailure.map")
                .is_success());

      REQUIRE(map.currentLayer()->childCount() == 2);
      auto* subtrahendNode =
        dynamic_cast<BrushNode*>(map.currentLayer()->children().at(1));
      REQUIRE(subtrahendNode);
      REQUIRE(subtrahendNode->brush().findFace("clip").has_value());

      // select the second object in the default layer (a clip brush) and subtract
      map.selectNodes({subtrahendNode});
      CHECK(map.csgSubtract());

      REQUIRE(map.currentLayer()->childCount() == 1);
      auto* result = dynamic_cast<BrushNode*>(map.currentLayer()->children().at(0));

      CHECK_THAT(
        result->brush().vertexPositions(),
        UnorderedApproxVecMatches(
          std::vector<vm::vec3d>{
            {-2852, 372, 248},
            {-2854, 372, 256},
            {-2854, 364, 256},
            {-2852, 364, 248},
            {-2840, 372, 248},
            {-2843.2, 372, 256},
            {-2843.2, 364, 256},
            {-2840, 364, 248}},
          0.001));
    }
  }

  SECTION("csgHollow")
  {
    auto [game, gameConfig] = loadGame("Quake");
    REQUIRE(map
              .load(
                MapFormat::Valve,
                vm::bbox3d{8192.0},
                game,
                "fixture/test/ui/MapDocumentTest/csgHollow.map")
              .is_success());

    REQUIRE(map.currentLayer()->childCount() == 2);
    REQUIRE(!map.modified());

    SECTION("A brush too small to be hollowed doesn't block the command")
    {
      map.selectAllNodes();
      CHECK(map.csgHollow());

      // One cube is too small to hollow, so it's left untouched.
      // The other is hollowed into 6 brushes.
      CHECK(map.currentLayer()->childCount() == 7);
      CHECK(map.modified());
    }

    SECTION("If no brushes are hollowed, the transaction isn't committed")
    {
      auto* smallBrushNode = map.currentLayer()->children().at(0);
      map.selectNodes({smallBrushNode});

      CHECK(!map.csgHollow());
      CHECK(map.currentLayer()->childCount() == 2);
      CHECK(!map.modified());
    }
  }
}

} // namespace tb::mdl
