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

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Matchers.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/UvAlignment.h"
#include "mdl/UvCoordSystem.h"

#include "kd/result.h"

#include "vm/approx.h"

#include <limits>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

auto& getFace(const BrushNode& brushNode, const size_t faceIndex)
{
  return brushNode.brush().face(faceIndex);
}

} // namespace

TEST_CASE("Map_Brushes")
{
  auto fixture = MapFixture{};

  SECTION("createBrush")
  {
    auto& map = fixture.create();

    SECTION("valid brush")
    {
      const auto points = std::vector<vm::vec3d>{
        {-64, -64, -64},
        {-64, -64, +64},
        {-64, +64, -64},
        {-64, +64, +64},
        {+64, -64, -64},
        {+64, -64, +64},
        {+64, +64, -64},
        {+64, +64, +64},
      };

      CHECK(createBrush(map, points));

      REQUIRE(map.selection().brushes.size() == 1);

      const auto* brushNode = map.selection().brushes.front();
      CHECK(std::ranges::all_of(
        points, [&](const auto& point) { return brushNode->brush().hasVertex(point); }));
    }

    SECTION("invalid brush")
    {
      const auto points = std::vector<vm::vec3d>{
        {-64, -64, -64},
        {-64, -64, +64},
        {-64, +64, -64},
        {-64, +64, +64},
      };

      CHECK(!createBrush(map, points));
      CHECK(map.selection().brushes.empty());
    }
  }

  SECTION("setBrushFaceAttributes")
  {
    SECTION("Setting all attributes")
    {
      auto& map = fixture.create();

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{&parentForNodes(map), {brushNode}}});

      const size_t firstFaceIndex = 0u;
      const size_t secondFaceIndex = 1u;
      const size_t thirdFaceIndex = 2u;

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, firstFaceIndex}});

      setBrushFaceAttributes(
        map,
        {
          .materialName = "first",
          .xOffset = SetValue{32.0f},
          .yOffset = SetValue{64.0f},
          .rotation = SetValue{90.0f},
          .xScale = SetValue{2.0f},
          .yScale = SetValue{4.0f},
          .surfaceFlags = SetFlags{63u},
          .surfaceContents = SetFlags{12u},
          .surfaceValue = SetValue{3.14f},
          .color = RgbaF{1.0f, 1.0f, 1.0f, 1.0f},
        });

      {
        const auto& firstFace = getFace(*brushNode, firstFaceIndex);
        CHECK(firstFace.materialName() == "first");
        CHECK(firstFace.uvAttributes().offset.x() == 32.0f);
        CHECK(firstFace.uvAttributes().offset.y() == 64.0f);
        CHECK(firstFace.uvAttributes().rotation == 90.0f);
        CHECK(firstFace.uvAttributes().scale.x() == 2.0f);
        CHECK(firstFace.uvAttributes().scale.y() == 4.0f);
        CHECK(firstFace.surfaceAttributes().flags == 63u);
        CHECK(firstFace.surfaceAttributes().contents == 12u);
        CHECK(firstFace.surfaceAttributes().value == 3.14f);
        CHECK(
          firstFace.surfaceAttributes().color == Color{RgbaF{1.0f, 1.0f, 1.0f, 1.0f}});
      }

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, secondFaceIndex}});

      setBrushFaceAttributes(
        map,
        {
          .materialName = "second",
          .xOffset = SetValue{16.0f},
          .yOffset = SetValue{48.0f},
          .rotation = SetValue{45.0f},
          .xScale = SetValue{1.0f},
          .yScale = SetValue{1.0f},
          .surfaceFlags = SetFlags{18u},
          .surfaceContents = SetFlags{2048u},
          .surfaceValue = SetValue{1.0f},
          .color = RgbaF{0.5f, 0.5f, 0.5f, 0.5f},
        });

      {
        const auto& secondFace = getFace(*brushNode, secondFaceIndex);
        CHECK(secondFace.materialName() == "second");
        CHECK(secondFace.uvAttributes().offset.x() == 16.0f);
        CHECK(secondFace.uvAttributes().offset.y() == 48.0f);
        CHECK(secondFace.uvAttributes().rotation == 45.0f);
        CHECK(secondFace.uvAttributes().scale.x() == 1.0f);
        CHECK(secondFace.uvAttributes().scale.y() == 1.0f);
        CHECK(secondFace.surfaceAttributes().flags == 18u);
        CHECK(secondFace.surfaceAttributes().contents == 2048u);
        CHECK(secondFace.surfaceAttributes().value == 1.0f);
        CHECK(
          secondFace.surfaceAttributes().color == Color{RgbaF{0.5f, 0.5f, 0.5f, 0.5f}});
      }

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, thirdFaceIndex}});

      setBrushFaceAttributes(map, copyAll(getFace(*brushNode, secondFaceIndex)));

      CHECK(
        getFace(*brushNode, thirdFaceIndex).materialName()
        == getFace(*brushNode, secondFaceIndex).materialName());
      CHECK_THAT(
        getFace(*brushNode, thirdFaceIndex),
        MatchesBrushFaceAttributes(getFace(*brushNode, secondFaceIndex)));

      auto thirdFaceContentsFlags =
        getFace(*brushNode, thirdFaceIndex).surfaceAttributes().contents;

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, secondFaceIndex}});

      setBrushFaceAttributes(map, copyAll(getFace(*brushNode, firstFaceIndex)));

      CHECK(
        getFace(*brushNode, secondFaceIndex).materialName()
        == getFace(*brushNode, firstFaceIndex).materialName());
      CHECK_THAT(
        getFace(*brushNode, secondFaceIndex),
        MatchesBrushFaceAttributes(getFace(*brushNode, firstFaceIndex)));

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, thirdFaceIndex}});
      setBrushFaceAttributes(
        map, copyAllExceptContentFlags(getFace(*brushNode, firstFaceIndex)));

      {
        const auto& firstFace = getFace(*brushNode, firstFaceIndex);
        const auto& newThirdFace = getFace(*brushNode, thirdFaceIndex);
        CHECK(newThirdFace.materialName() == firstFace.materialName());
        CHECK(
          newThirdFace.uvAttributes().offset.x() == firstFace.uvAttributes().offset.x());
        CHECK(
          newThirdFace.uvAttributes().offset.y() == firstFace.uvAttributes().offset.y());
        CHECK(newThirdFace.uvAttributes().rotation == firstFace.uvAttributes().rotation);
        CHECK(
          newThirdFace.uvAttributes().scale.x() == firstFace.uvAttributes().scale.x());
        CHECK(
          newThirdFace.uvAttributes().scale.y() == firstFace.uvAttributes().scale.y());
        CHECK(
          newThirdFace.surfaceAttributes().flags == firstFace.surfaceAttributes().flags);
        CHECK(newThirdFace.surfaceAttributes().contents == thirdFaceContentsFlags);
        CHECK(
          newThirdFace.surfaceAttributes().value == firstFace.surfaceAttributes().value);
        CHECK(
          newThirdFace.surfaceAttributes().color == firstFace.surfaceAttributes().color);
      }
    }

    SECTION("Undo and redo")
    {
      auto& map = fixture.create();

      auto* brushNode = createBrushNode(map, "original");
      addNodes(map, {{&parentForNodes(map), {brushNode}}});

      for (const auto& face : brushNode->brush().faces())
      {
        REQUIRE(face.materialName() == "original");
      }

      selectNodes(map, {brushNode});

      setBrushFaceAttributes(map, {.materialName = "material"});
      for (const auto& face : brushNode->brush().faces())
      {
        REQUIRE(face.materialName() == "material");
      }

      map.undoCommand();
      for (const auto& face : brushNode->brush().faces())
      {
        CHECK(face.materialName() == "original");
      }

      map.redoCommand();
      for (const auto& face : brushNode->brush().faces())
      {
        CHECK(face.materialName() == "material");
      }
    }

    SECTION("Quake 2 format")
    {
      const int WaterFlag = 32;
      const int LavaFlag = 8;

      auto& map = fixture.load("test/mdl/Map/lavaAndWater.map", Quake2FixtureConfig);

      REQUIRE(map.editorContext().currentLayer() != nullptr);

      auto* lavabrush =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(0));
      REQUIRE(lavabrush);
      CHECK(getFace(*lavabrush, 0).surfaceAttributes().empty());
      CHECK(
        getFace(*lavabrush, 0).resolvedSurfaceContents()
        == LavaFlag); // comes from the .wal texture

      auto* waterbrush =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(1));
      REQUIRE(waterbrush);
      CHECK(getFace(*waterbrush, 0).surfaceAttributes().empty());
      CHECK(
        getFace(*waterbrush, 0).resolvedSurfaceContents()
        == WaterFlag); // comes from the .wal texture

      SECTION(
        "Transfer face attributes except content flags from waterbrush to lavabrush")
      {
        selectNodes(map, {lavabrush});

        CHECK(setBrushFaceAttributes(
          map, copyAllExceptContentFlags(getFace(*waterbrush, 0))));

        SECTION("Check lavabrush is now inheriting the water content flags")
        {
          // Note: the contents flag wasn't transferred, but because lavabrushes's
          // content flag was "Inherit", it stays "Inherit" and now inherits the water
          // contents
          CHECK(getFace(*lavabrush, 0).surfaceAttributes().empty());
          CHECK(getFace(*lavabrush, 0).resolvedSurfaceContents() == WaterFlag);
          CHECK(getFace(*lavabrush, 0).materialName() == "watertest");
        }
      }

      SECTION(
        "Setting a content flag when the existing one is inherited keeps the existing "
        "one")
      {
        selectNodes(map, {lavabrush});

        CHECK(setBrushFaceAttributes(map, {.surfaceContents = SetFlagBits{WaterFlag}}));

        CHECK(!getFace(*lavabrush, 0).surfaceAttributes().empty());
        CHECK(getFace(*lavabrush, 0).resolvedSurfaceContents() == (WaterFlag | LavaFlag));
      }
    }

    SECTION("Setting a material keeps the surface flags unset")
    {
      auto& map = fixture.create(QuakeFixtureConfig);

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{&parentForNodes(map), {brushNode}}});

      selectNodes(map, {brushNode});
      CHECK(getFace(*brushNode, 0).surfaceAttributes().empty());

      setBrushFaceAttributes(map, {.materialName = "something_else"});

      CHECK(getFace(*brushNode, 0).materialName() == "something_else");
      CHECK(getFace(*brushNode, 0).surfaceAttributes().empty());
    }

    SECTION("Reset attributes to defaults")
    {
      const auto defaultUvAttrs = UvAttributes{.scale = {0.5f, 2.0f}};

      auto fixtureConfig = MapFixtureConfig{};
      fixtureConfig.gameInfo.gameConfig.faceAttribsConfig.defaultUvAttributes =
        defaultUvAttrs;

      auto& map = fixture.create(fixtureConfig);

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{&parentForNodes(map), {brushNode}}});

      const size_t faceIndex = 0u;
      const auto initialX = getFace(*brushNode, faceIndex).uAxis();
      const auto initialY = getFace(*brushNode, faceIndex).vAxis();

      selectBrushFaces(map, {{brushNode, faceIndex}});

      // NOLINTNEXTLINE(misc-const-correctness)
      for (size_t i = 0; i < 5; ++i)
      {
        setBrushFaceAttributes(map, {.rotation = AddValue{2.0f}});
      }

      REQUIRE(getFace(*brushNode, faceIndex).uvAttributes().rotation == 10.0f);

      setBrushFaceAttributes(map, resetAll(defaultUvAttrs));

      CHECK(getFace(*brushNode, faceIndex).uvAttributes().offset.x() == 0.0f);
      CHECK(getFace(*brushNode, faceIndex).uvAttributes().offset.y() == 0.0f);
      CHECK(getFace(*brushNode, faceIndex).uvAttributes().rotation == 0.0f);
      CHECK(
        getFace(*brushNode, faceIndex).uvAttributes().scale.x()
        == defaultUvAttrs.scale.x());
      CHECK(
        getFace(*brushNode, faceIndex).uvAttributes().scale.y()
        == defaultUvAttrs.scale.y());

      CHECK(getFace(*brushNode, faceIndex).uAxis() == initialX);
      CHECK(getFace(*brushNode, faceIndex).vAxis() == initialY);
    }

    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto& map = fixture.create();

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{&parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode != nullptr);

      deselectAll(map);

      SECTION("Can select two linked groups and apply a material")
      {
        selectNodes(map, {groupNode, linkedGroupNode});

        REQUIRE(setBrushFaceAttributes(map, {.materialName = "abc"}));

        // check that the brushes in both linked groups got a material
        for (auto* g : std::vector<GroupNode*>{groupNode, linkedGroupNode})
        {
          auto* brush = dynamic_cast<BrushNode*>(g->children().at(0));
          REQUIRE(brush != nullptr);

          CHECK(getFace(*brush, 0).materialName() == "abc");
        }
      }
    }

    SECTION("rejects an invalid value")
    {
      auto& map = fixture.create();

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{&parentForNodes(map), {brushNode}}});

      const size_t faceIndex = 0u;
      deselectAll(map);
      selectBrushFaces(map, {{brushNode, faceIndex}});

      const auto originalFace = getFace(*brushNode, faceIndex);
      const auto canUndoBefore = map.canUndoCommand();

      const auto nan = std::numeric_limits<float>::quiet_NaN();
      CHECK(!setBrushFaceAttributes(map, {.rotation = SetValue{nan}}));

      CHECK_THAT(
        getFace(*brushNode, faceIndex), MatchesBrushFaceAttributes(originalFace));
      CHECK(map.canUndoCommand() == canUndoBefore);
    }
  }

  SECTION("copyUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto sourceFaceIndex = brushNode->brush().findFace(vm::vec3d{0, -1, 0});
    REQUIRE(sourceFaceIndex);

    const auto targetFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(targetFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *sourceFaceIndex}});
    REQUIRE(setBrushFaceAttributes(
      map,
      {
        .xOffset = SetValue{13.0f},
        .yOffset = SetValue{17.0f},
        .rotation = SetValue{22.0f},
        .xScale = SetValue{1.2f},
        .yScale = SetValue{0.8f},
      }));

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *targetFaceIndex}});
    REQUIRE(setBrushFaceAttributes(
      map,
      {
        .xOffset = SetValue{2.0f},
        .yOffset = SetValue{3.0f},
        .rotation = SetValue{90.0f},
        .xScale = SetValue{1.0f},
        .yScale = SetValue{1.0f},
      }));

    const auto originalTargetFace = getFace(*brushNode, *targetFaceIndex);
    const auto originalTargetUAxis = getFace(*brushNode, *targetFaceIndex).uAxis();
    const auto originalTargetVAxis = getFace(*brushNode, *targetFaceIndex).vAxis();

    const auto& sourceFace = getFace(*brushNode, *sourceFaceIndex);
    const auto sourceSnapshot = sourceFace.takeUvCoordSystemSnapshot();
    const auto sourceUvAttributes = sourceFace.uvAttributes();
    const auto sourcePlane = sourceFace.boundary();

    CHECK(copyUv(
      map, *sourceSnapshot, sourceUvAttributes, sourcePlane, WrapStyle::Projection));

    auto expectedUvAttributes = originalTargetFace.uvAttributes();
    expectedUvAttributes.offset = {0.36245f, 0.501574f};
    const auto& expectedSurfaceAttributes = originalTargetFace.surfaceAttributes();

    const auto& targetFace = getFace(*brushNode, *targetFaceIndex);
    CHECK_THAT(
      targetFace,
      MatchesBrushFaceAttributes(
        originalTargetFace.materialName(),
        expectedUvAttributes,
        expectedSurfaceAttributes));
    CHECK(targetFace.uAxis() == vm::approx{vm::vec3d{0, -1, 0}});
    CHECK(targetFace.vAxis() == vm::approx{vm::vec3d{-0.374607, 0, -0.927184}});

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneTargetFace = getFace(*brushNode, *targetFaceIndex);
      CHECK_THAT(undoneTargetFace, MatchesBrushFaceAttributes(originalTargetFace));
      CHECK(undoneTargetFace.uAxis() == vm::approx{originalTargetUAxis});
      CHECK(undoneTargetFace.vAxis() == vm::approx{originalTargetVAxis});

      map.redoCommand();

      const auto& redoneTargetFace = getFace(*brushNode, *targetFaceIndex);
      CHECK_THAT(
        redoneTargetFace,
        MatchesBrushFaceAttributes(
          originalTargetFace.materialName(),
          expectedUvAttributes,
          expectedSurfaceAttributes));
      CHECK(redoneTargetFace.uAxis() == vm::approx{vm::vec3d{0, -1, 0}});
      CHECK(redoneTargetFace.vAxis() == vm::approx{vm::vec3d{-0.374607, 0, -0.927184}});
    }
  }

  SECTION("translateUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(
      map,
      {
        .xOffset = SetValue{10.0f},
        .yOffset = SetValue{20.0f},
      }));

    const auto cameraUp = vm::vec3f{0, 1, 0};
    const auto cameraRight = vm::vec3f{1, 0, 0};
    const auto delta = vm::vec2f{4.0f, 8.0f};

    const auto originalFace = getFace(*brushNode, *faceIndex);
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex)
      .translateUv(vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, delta);
    const auto expectedFaceCopy = expectedBrush.face(*faceIndex);
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(translateUv(map, cameraUp, cameraRight, delta));

    const auto& movedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(movedFace, MatchesBrushFaceAttributes(expectedFaceCopy));
    CHECK(movedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(movedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex),
      MatchesBrushFaceAttributes(originalOtherFace));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFaceCopy));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));
    }
  }

  SECTION("rotateUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    SECTION("accepts valid rotations")
    {
      const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
      REQUIRE(faceIndex);

      const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
      REQUIRE(otherFaceIndex);

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, *faceIndex}});

      REQUIRE(setBrushFaceAttributes(map, {.rotation = SetValue{10.0f}}));

      const auto originalFace = getFace(*brushNode, *faceIndex);
      const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
      const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

      const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

      auto expectedBrush = brushNode->brush();
      REQUIRE(expectedBrush.face(*faceIndex).rotateUv(15.0f).is_success());
      const auto expectedFaceCopy = expectedBrush.face(*faceIndex);
      const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
      const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

      REQUIRE(rotateUv(map, 15.0f));

      const auto& rotatedFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(rotatedFace, MatchesBrushFaceAttributes(expectedFaceCopy));
      CHECK(rotatedFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(rotatedFace.vAxis() == vm::approx{expectedVAxis});

      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      SECTION("Undo and redo")
      {
        map.undoCommand();

        const auto& undoneFace = getFace(*brushNode, *faceIndex);
        CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
        CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
        CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
        CHECK_THAT(
          getFace(*brushNode, *otherFaceIndex),
          MatchesBrushFaceAttributes(originalOtherFace));

        map.redoCommand();

        const auto& redoneFace = getFace(*brushNode, *faceIndex);
        CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFaceCopy));
        CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
        CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
        CHECK_THAT(
          getFace(*brushNode, *otherFaceIndex),
          MatchesBrushFaceAttributes(originalOtherFace));
      }
    }

    SECTION("rejects an invalid rotations")
    {
      const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
      REQUIRE(faceIndex);

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, *faceIndex}});

      const auto max = std::numeric_limits<float>::max();
      REQUIRE(setBrushFaceAttributes(map, {.rotation = SetValue{max}}));

      const auto originalFace = getFace(*brushNode, *faceIndex);
      const auto canUndoBefore = map.canUndoCommand();

      // rotating the already-huge rotation by another huge angle overflows it to
      // infinity -- this is the original crash this branch set out to fix
      CHECK(!rotateUv(map, max));

      CHECK_THAT(
        getFace(*brushNode, *faceIndex), MatchesBrushFaceAttributes(originalFace));
      CHECK(map.canUndoCommand() == canUndoBefore);
    }
  }

  SECTION("shearUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    const auto factors = vm::vec2f{0.25f, -0.5f};

    const auto originalFace = getFace(*brushNode, *faceIndex);
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex).shearUv(factors);
    const auto expectedFaceCopy = expectedBrush.face(*faceIndex);
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(shearUv(map, factors));

    const auto& shearedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(shearedFace, MatchesBrushFaceAttributes(expectedFaceCopy));
    CHECK(shearedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(shearedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex),
      MatchesBrushFaceAttributes(originalOtherFace));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFaceCopy));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));
    }
  }

  SECTION("flipUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(
      map,
      {
        .xScale = SetValue{2.0f},
        .yScale = SetValue{3.0f},
      }));

    const auto cameraUp = vm::vec3f{0, 1, 0};
    const auto cameraRight = vm::vec3f{1, 0, 0};
    const auto flipDirection = vm::direction::left;

    const auto originalFace = getFace(*brushNode, *faceIndex);
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex)
      .flipUv(vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, flipDirection);
    const auto expectedFaceCopy = expectedBrush.face(*faceIndex);
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(flipUv(map, cameraUp, cameraRight, flipDirection));

    const auto& flippedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(flippedFace, MatchesBrushFaceAttributes(expectedFaceCopy));
    CHECK(flippedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(flippedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex),
      MatchesBrushFaceAttributes(originalOtherFace));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFaceCopy));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));
    }
  }

  SECTION("alignUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, -1, 0});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(map, {.rotation = SetValue{0.0f}}));

    const auto originalFace = getFace(*brushNode, *faceIndex);
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

    auto expectedBrush = brushNode->brush();
    evaluate(
      align(expectedBrush.face(*faceIndex), UvPolicy::next),
      expectedBrush.face(*faceIndex))
      | kdl::ignore();
    const auto expectedFaceCopy = expectedBrush.face(*faceIndex);
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    alignUv(map, UvPolicy::next);

    const auto& alignedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(alignedFace, !MatchesBrushFaceAttributes(originalFace));
    CHECK_THAT(alignedFace, MatchesBrushFaceAttributes(expectedFaceCopy));
    CHECK(alignedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(alignedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex),
      MatchesBrushFaceAttributes(originalOtherFace));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFaceCopy));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));
    }
  }

  SECTION("justifyUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, -1, 0});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(
      map,
      {
        .xOffset = SetValue{7.0f},
        .yOffset = SetValue{11.0f},
      }));

    const auto originalFace = getFace(*brushNode, *faceIndex);
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

    auto expectedBrush = brushNode->brush();
    evaluate(
      justify(expectedBrush.face(*faceIndex), UvAxis::u, UvSign::plus, UvPolicy::best),
      expectedBrush.face(*faceIndex))
      | kdl::ignore();
    const auto expectedFaceCopy = expectedBrush.face(*faceIndex);
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    justifyUv(map, UvJustifyDirection::Left, UvPolicy::best);

    const auto& justifiedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(justifiedFace, !MatchesBrushFaceAttributes(originalFace));
    CHECK_THAT(justifiedFace, MatchesBrushFaceAttributes(expectedFaceCopy));
    CHECK(justifiedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(justifiedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex),
      MatchesBrushFaceAttributes(originalOtherFace));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFaceCopy));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));
    }
  }

  SECTION("fitUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, -1, 0});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(
      map,
      {
        .xOffset = SetValue{5.0f},
        .yOffset = SetValue{9.0f},
        .xScale = SetValue{1.0f},
      }));

    const auto originalFace = getFace(*brushNode, *faceIndex);
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFace = getFace(*brushNode, *otherFaceIndex);

    auto expectedBrush = brushNode->brush();
    auto& expectedFace = expectedBrush.face(*faceIndex);

    const auto invariantVertex = anchorVertex(expectedFace, UvAxis::u, UvSign::minus);
    const auto previousUvCoords = vm::vec2f{
      expectedFace.toUvCoordSystemMatrix(
        expectedFace.uvAttributes().offset, expectedFace.uvAttributes().scale)
      * invariantVertex};

    evaluate(
      fit(expectedFace, UvAxis::u, UvPolicy::next, UvFitMode::fitToFace), expectedFace)
      | kdl::ignore();

    const auto newUvCoords = vm::vec2f{
      expectedFace.toUvCoordSystemMatrix(
        expectedFace.uvAttributes().offset, expectedFace.uvAttributes().scale)
      * invariantVertex};
    const auto delta = previousUvCoords - newUvCoords;

    evaluate(
      {
        .xOffset = AddValue{delta.x()},
        .yOffset = AddValue{delta.y()},
      },
      expectedFace)
      | kdl::ignore();

    const auto expectedUAxis = expectedFace.uAxis();
    const auto expectedVAxis = expectedFace.vAxis();

    fitUv(map, UvFitDirection::Horizontal, UvPolicy::next, UvFitMode::fitToFace);

    const auto& fittedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(fittedFace, !MatchesBrushFaceAttributes(originalFace));
    CHECK_THAT(fittedFace, MatchesBrushFaceAttributes(expectedFace));
    CHECK(fittedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(fittedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex),
      MatchesBrushFaceAttributes(originalOtherFace));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(undoneFace, MatchesBrushFaceAttributes(originalFace));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace, MatchesBrushFaceAttributes(expectedFace));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex),
        MatchesBrushFaceAttributes(originalOtherFace));
    }
  }

  SECTION("autoFitUv")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushNode}}});

    const auto iFront = *brushNode->brush().findFace(vm::vec3d{0, -1, 0});
    REQUIRE(iFront);

    const auto iRight = *brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(iRight);

    const auto iTop = *brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(iTop);

    SECTION("Aligns when any selected face is not aligned")
    {
      // front face is not aligned (rotation == 15)
      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}});
      REQUIRE(setBrushFaceAttributes(
        map,
        {
          .xOffset = SetValue{5.0f},
          .yOffset = SetValue{9.0f},
          .rotation = SetValue{15.0f},
          .xScale = SetValue{1.3f},
          .yScale = SetValue{0.8f},
        }));

      // right face is aligned (rotation == 0)
      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iRight}});
      REQUIRE(setBrushFaceAttributes(
        map,
        {
          .xOffset = SetValue{15.5f},
          .yOffset = SetValue{15.5f},
          .rotation = SetValue{0.0f},
          .xScale = SetValue{32.0f},
          .yScale = SetValue{32.0f},
        }));

      REQUIRE(!isAligned(getFace(*brushNode, iFront)));
      REQUIRE(getFace(*brushNode, iRight).uAxis() == vm::approx{vm::vec3d{0, 1, 0}});
      REQUIRE(getFace(*brushNode, iRight).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      const auto originalTopFace = getFace(*brushNode, iTop);

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}, {brushNode, iRight}});
      autoFitUv(map);

      // front face is now aligned
      CHECK(getFace(*brushNode, iFront).uAxis() == vm::approx{vm::vec3d{1, 0, 0}});
      CHECK(getFace(*brushNode, iFront).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      // right face remains aligned
      CHECK(getFace(*brushNode, iRight).uAxis() == vm::approx{vm::vec3d{0, 1, 0}});
      CHECK(getFace(*brushNode, iRight).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      // top face was not affected
      CHECK_THAT(getFace(*brushNode, iTop), MatchesBrushFaceAttributes(originalTopFace));
    }

    SECTION(
      "Does not realign when selected faces are aligned but not all fitted and justified")
    {
      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}});
      REQUIRE(setBrushFaceAttributes(
        map,
        {
          .xOffset = SetValue{7.0f},
          .yOffset = SetValue{11.0f},
          .rotation = SetValue{0.0f},
          .xScale = SetValue{1.4f},
          .yScale = SetValue{0.9f},
        }));

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iRight}});
      REQUIRE(setBrushFaceAttributes(
        map,
        {
          .xOffset = SetValue{15.5f},
          .yOffset = SetValue{15.5f},
          .rotation = SetValue{0.0f},
          .xScale = SetValue{32.0f},
          .yScale = SetValue{32.0f},
        }));

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}, {brushNode, iRight}});

      REQUIRE(isAligned(getFace(*brushNode, iFront)));
      REQUIRE(!isJustified(getFace(*brushNode, iFront), UvAxis::u, UvSign::plus));
      REQUIRE(!isJustified(getFace(*brushNode, iFront), UvAxis::v, UvSign::plus));
      REQUIRE(!isFitted(getFace(*brushNode, iFront), UvAxis::u));
      REQUIRE(!isFitted(getFace(*brushNode, iFront), UvAxis::v));

      REQUIRE(isAligned(getFace(*brushNode, iRight)));
      REQUIRE(isJustified(getFace(*brushNode, iRight), UvAxis::u, UvSign::plus));
      REQUIRE(isJustified(getFace(*brushNode, iRight), UvAxis::v, UvSign::plus));
      REQUIRE(isFitted(getFace(*brushNode, iRight), UvAxis::u));
      REQUIRE(isFitted(getFace(*brushNode, iRight), UvAxis::v));

      REQUIRE(getFace(*brushNode, iRight).uAxis() == vm::approx{vm::vec3d{0, 1, 0}});
      REQUIRE(getFace(*brushNode, iRight).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      autoFitUv(map);

      CHECK(getFace(*brushNode, iFront).uAxis() == vm::approx{vm::vec3d{1, 0, 0}});
      CHECK(getFace(*brushNode, iFront).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});
      CHECK(getFace(*brushNode, iRight).uAxis() == vm::approx{vm::vec3d{0, 1, 0}});
      CHECK(getFace(*brushNode, iRight).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      CHECK(isAligned(getFace(*brushNode, iFront)));
      CHECK(isJustified(getFace(*brushNode, iFront), UvAxis::u, UvSign::plus));
      CHECK(isJustified(getFace(*brushNode, iFront), UvAxis::v, UvSign::plus));
      CHECK(isFitted(getFace(*brushNode, iFront), UvAxis::u));
      CHECK(isFitted(getFace(*brushNode, iFront), UvAxis::v));

      CHECK(isAligned(getFace(*brushNode, iRight)));
      CHECK(isJustified(getFace(*brushNode, iRight), UvAxis::u, UvSign::plus));
      CHECK(isJustified(getFace(*brushNode, iRight), UvAxis::v, UvSign::plus));
      CHECK(isFitted(getFace(*brushNode, iRight), UvAxis::u));
      CHECK(isFitted(getFace(*brushNode, iRight), UvAxis::v));
    }

    SECTION("Undo and Redo")
    {
      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}});
      REQUIRE(setBrushFaceAttributes(
        map,
        {
          .xOffset = SetValue{5.0f},
          .yOffset = SetValue{9.0f},
          .rotation = SetValue{15.0f},
          .xScale = SetValue{1.3f},
          .yScale = SetValue{0.8f},
        }));

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iRight}});
      REQUIRE(setBrushFaceAttributes(
        map,
        {
          .xOffset = SetValue{15.5f},
          .yOffset = SetValue{15.5f},
          .rotation = SetValue{0.0f},
          .xScale = SetValue{32.0f},
          .yScale = SetValue{32.0f},
        }));

      const auto originalFrontFace = getFace(*brushNode, iFront);
      const auto originalRightFace = getFace(*brushNode, iRight);

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}, {brushNode, iRight}});
      autoFitUv(map);

      const auto modifiedFrontFace = getFace(*brushNode, iFront);
      const auto modifiedRightFace = getFace(*brushNode, iRight);

      REQUIRE_THAT(modifiedFrontFace, !MatchesBrushFaceAttributes(originalFrontFace));
      REQUIRE_THAT(modifiedRightFace, !MatchesBrushFaceAttributes(originalRightFace));

      map.undoCommand();

      REQUIRE_THAT(
        getFace(*brushNode, iFront), MatchesBrushFaceAttributes(originalFrontFace));
      REQUIRE_THAT(
        getFace(*brushNode, iRight), MatchesBrushFaceAttributes(originalRightFace));

      map.redoCommand();

      REQUIRE_THAT(
        getFace(*brushNode, iFront), MatchesBrushFaceAttributes(modifiedFrontFace));
      REQUIRE_THAT(
        getFace(*brushNode, iRight), MatchesBrushFaceAttributes(modifiedRightFace));
    }
  }
}

} // namespace tb::mdl
