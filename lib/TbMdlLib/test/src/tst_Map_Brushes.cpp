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
#include "mdl/UVAttributes.h"
#include "mdl/UVCoordSystem.h"
#include "mdl/UpdateBrushFaceAttributes.h"

#include "vm/approx.h"

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
      addNodes(map, {{parentForNodes(map), {brushNode}}});

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
        const auto& firstAttrs = getFace(*brushNode, firstFaceIndex).attributes();
        const auto firstUVAttrs = getFace(*brushNode, firstFaceIndex).uvAttributes();
        CHECK(firstAttrs.materialName() == "first");
        CHECK(firstUVAttrs.offset.x() == 32.0f);
        CHECK(firstUVAttrs.offset.y() == 64.0f);
        CHECK(firstUVAttrs.rotation == 90.0f);
        CHECK(firstUVAttrs.scale.x() == 2.0f);
        CHECK(firstUVAttrs.scale.y() == 4.0f);
        CHECK(firstAttrs.surfaceFlags() == 63u);
        CHECK(firstAttrs.surfaceContents() == 12u);
        CHECK(firstAttrs.surfaceValue() == 3.14f);
        CHECK(firstAttrs.color() == Color{RgbaF{1.0f, 1.0f, 1.0f, 1.0f}});
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
        const auto& secondAttrs = getFace(*brushNode, secondFaceIndex).attributes();
        const auto secondUVAttrs = getFace(*brushNode, secondFaceIndex).uvAttributes();
        CHECK(secondAttrs.materialName() == "second");
        CHECK(secondUVAttrs.offset.x() == 16.0f);
        CHECK(secondUVAttrs.offset.y() == 48.0f);
        CHECK(secondUVAttrs.rotation == 45.0f);
        CHECK(secondUVAttrs.scale.x() == 1.0f);
        CHECK(secondUVAttrs.scale.y() == 1.0f);
        CHECK(secondAttrs.surfaceFlags() == 18u);
        CHECK(secondAttrs.surfaceContents() == 2048u);
        CHECK(secondAttrs.surfaceValue() == 1.0f);
        CHECK(secondAttrs.color() == Color{RgbaF{0.5f, 0.5f, 0.5f, 0.5f}});
      }

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, thirdFaceIndex}});

      setBrushFaceAttributes(
        map,
        copyAll(
          getFace(*brushNode, secondFaceIndex).attributes(),
          getFace(*brushNode, secondFaceIndex).uvAttributes()));

      CHECK(
        getFace(*brushNode, thirdFaceIndex).attributes()
        == getFace(*brushNode, secondFaceIndex).attributes());
      CHECK(
        getFace(*brushNode, thirdFaceIndex).uvAttributes()
        == getFace(*brushNode, secondFaceIndex).uvAttributes());

      auto thirdFaceContentsFlags =
        getFace(*brushNode, thirdFaceIndex).attributes().surfaceContents();

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, secondFaceIndex}});

      setBrushFaceAttributes(
        map,
        copyAll(
          getFace(*brushNode, firstFaceIndex).attributes(),
          getFace(*brushNode, firstFaceIndex).uvAttributes()));

      CHECK(
        getFace(*brushNode, secondFaceIndex).attributes()
        == getFace(*brushNode, firstFaceIndex).attributes());
      CHECK(
        getFace(*brushNode, secondFaceIndex).uvAttributes()
        == getFace(*brushNode, firstFaceIndex).uvAttributes());

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, thirdFaceIndex}});
      setBrushFaceAttributes(
        map,
        copyAllExceptContentFlags(
          getFace(*brushNode, firstFaceIndex).attributes(),
          getFace(*brushNode, firstFaceIndex).uvAttributes()));

      {
        const auto& firstAttrs = getFace(*brushNode, firstFaceIndex).attributes();
        const auto firstUVAttrs = getFace(*brushNode, firstFaceIndex).uvAttributes();
        const auto& newThirdAttrs = getFace(*brushNode, thirdFaceIndex).attributes();
        const auto newThirdUVAttrs = getFace(*brushNode, thirdFaceIndex).uvAttributes();
        CHECK(newThirdAttrs.materialName() == firstAttrs.materialName());
        CHECK(newThirdUVAttrs.offset.x() == firstUVAttrs.offset.x());
        CHECK(newThirdUVAttrs.offset.y() == firstUVAttrs.offset.y());
        CHECK(newThirdUVAttrs.rotation == firstUVAttrs.rotation);
        CHECK(newThirdUVAttrs.scale.x() == firstUVAttrs.scale.x());
        CHECK(newThirdUVAttrs.scale.y() == firstUVAttrs.scale.y());
        CHECK(newThirdAttrs.surfaceFlags() == firstAttrs.surfaceFlags());
        CHECK(newThirdAttrs.surfaceContents() == thirdFaceContentsFlags);
        CHECK(newThirdAttrs.surfaceValue() == firstAttrs.surfaceValue());
        CHECK(newThirdAttrs.color() == firstAttrs.color());
      }
    }

    SECTION("Undo and redo")
    {
      auto& map = fixture.create();

      auto* brushNode = createBrushNode(map, "original");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      for (const auto& face : brushNode->brush().faces())
      {
        REQUIRE(face.attributes().materialName() == "original");
      }

      selectNodes(map, {brushNode});

      setBrushFaceAttributes(map, {.materialName = "material"});
      for (const auto& face : brushNode->brush().faces())
      {
        REQUIRE(face.attributes().materialName() == "material");
      }

      map.undoCommand();
      for (const auto& face : brushNode->brush().faces())
      {
        CHECK(face.attributes().materialName() == "original");
      }

      map.redoCommand();
      for (const auto& face : brushNode->brush().faces())
      {
        CHECK(face.attributes().materialName() == "material");
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
      CHECK(!getFace(*lavabrush, 0).attributes().hasSurfaceAttributes());
      CHECK(
        getFace(*lavabrush, 0).resolvedSurfaceContents()
        == LavaFlag); // comes from the .wal texture

      auto* waterbrush =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(1));
      REQUIRE(waterbrush);
      CHECK(!getFace(*waterbrush, 0).attributes().hasSurfaceAttributes());
      CHECK(
        getFace(*waterbrush, 0).resolvedSurfaceContents()
        == WaterFlag); // comes from the .wal texture

      SECTION(
        "Transfer face attributes except content flags from waterbrush to lavabrush")
      {
        selectNodes(map, {lavabrush});

        CHECK(setBrushFaceAttributes(
          map,
          copyAllExceptContentFlags(
            getFace(*waterbrush, 0).attributes(),
            getFace(*waterbrush, 0).uvAttributes())));

        SECTION("Check lavabrush is now inheriting the water content flags")
        {
          // Note: the contents flag wasn't transferred, but because lavabrushes's
          // content flag was "Inherit", it stays "Inherit" and now inherits the water
          // contents
          CHECK(!getFace(*lavabrush, 0).attributes().hasSurfaceAttributes());
          CHECK(getFace(*lavabrush, 0).resolvedSurfaceContents() == WaterFlag);
          CHECK(getFace(*lavabrush, 0).attributes().materialName() == "watertest");
        }
      }

      SECTION(
        "Setting a content flag when the existing one is inherited keeps the existing "
        "one")
      {
        selectNodes(map, {lavabrush});

        CHECK(setBrushFaceAttributes(map, {.surfaceContents = SetFlagBits{WaterFlag}}));

        CHECK(getFace(*lavabrush, 0).attributes().hasSurfaceAttributes());
        CHECK(getFace(*lavabrush, 0).resolvedSurfaceContents() == (WaterFlag | LavaFlag));
      }
    }

    SECTION("Setting a material keeps the surface flags unset")
    {
      auto& map = fixture.create(QuakeFixtureConfig);

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      selectNodes(map, {brushNode});
      CHECK(!getFace(*brushNode, 0).attributes().hasSurfaceAttributes());

      setBrushFaceAttributes(map, {.materialName = "something_else"});

      CHECK(getFace(*brushNode, 0).attributes().materialName() == "something_else");
      CHECK(!getFace(*brushNode, 0).attributes().hasSurfaceAttributes());
    }

    SECTION("Reset attributes to defaults")
    {
      const auto defaultFaceAttrs =
        BrushFaceAttributes{BrushFaceAttributes::NoMaterialName};
      const auto defaultUVAttrs = UVAttributes{.scale = vm::vec2f{0.5f, 2.0f}};

      auto fixtureConfig = MapFixtureConfig{};
      fixtureConfig.gameInfo.gameConfig.faceAttribsConfig.defaults = defaultFaceAttrs;
      fixtureConfig.gameInfo.gameConfig.faceAttribsConfig.uvDefaults = defaultUVAttrs;

      auto& map = fixture.create(fixtureConfig);

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

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

      setBrushFaceAttributes(map, resetAll(defaultUVAttrs));

      CHECK(getFace(*brushNode, faceIndex).uvAttributes().offset.x() == 0.0f);
      CHECK(getFace(*brushNode, faceIndex).uvAttributes().offset.y() == 0.0f);
      CHECK(getFace(*brushNode, faceIndex).uvAttributes().rotation == 0.0f);
      CHECK(
        getFace(*brushNode, faceIndex).uvAttributes().scale.x()
        == defaultUVAttrs.scale.x());
      CHECK(
        getFace(*brushNode, faceIndex).uvAttributes().scale.y()
        == defaultUVAttrs.scale.y());

      CHECK(getFace(*brushNode, faceIndex).uAxis() == initialX);
      CHECK(getFace(*brushNode, faceIndex).vAxis() == initialY);
    }

    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto& map = fixture.create();

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
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

          auto attrs = getFace(*brush, 0).attributes();
          CHECK(attrs.materialName() == "abc");
        }
      }
    }
  }

  SECTION("copyUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

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

    const auto originalTargetFaceAttributes =
      getFace(*brushNode, *targetFaceIndex).attributes();
    const auto originalTargetUVAttributes =
      getFace(*brushNode, *targetFaceIndex).uvAttributes();
    const auto originalTargetUAxis = getFace(*brushNode, *targetFaceIndex).uAxis();
    const auto originalTargetVAxis = getFace(*brushNode, *targetFaceIndex).vAxis();

    const auto& sourceFace = getFace(*brushNode, *sourceFaceIndex);
    const auto sourceSnapshot = sourceFace.takeUVCoordSystemSnapshot();
    const auto sourceUVAttributes = sourceFace.uvAttributes();
    const auto sourcePlane = sourceFace.boundary();

    CHECK(copyUV(
      map, *sourceSnapshot, sourceUVAttributes, sourcePlane, WrapStyle::Projection));

    const auto expectedAttributes = originalTargetFaceAttributes;
    auto expectedUVAttributes = originalTargetUVAttributes;
    expectedUVAttributes.offset = vm::vec2f{0.36245f, 0.501574f};

    const auto& targetFace = getFace(*brushNode, *targetFaceIndex);
    CHECK_THAT(targetFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(targetFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(targetFace.uAxis() == vm::approx{vm::vec3d{0, -1, 0}});
    CHECK(targetFace.vAxis() == vm::approx{vm::vec3d{-0.374607, 0, -0.927184}});

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneTargetFace = getFace(*brushNode, *targetFaceIndex);
      CHECK_THAT(
        undoneTargetFace.attributes(),
        MatchesBrushFaceAttributes(originalTargetFaceAttributes));
      CHECK_THAT(
        undoneTargetFace.uvAttributes(), MatchesUVAttributes(originalTargetUVAttributes));
      CHECK(undoneTargetFace.uAxis() == vm::approx{originalTargetUAxis});
      CHECK(undoneTargetFace.vAxis() == vm::approx{originalTargetVAxis});

      map.redoCommand();

      const auto& redoneTargetFace = getFace(*brushNode, *targetFaceIndex);
      CHECK_THAT(
        redoneTargetFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(
        redoneTargetFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneTargetFace.uAxis() == vm::approx{vm::vec3d{0, -1, 0}});
      CHECK(redoneTargetFace.vAxis() == vm::approx{vm::vec3d{-0.374607, 0, -0.927184}});
    }
  }

  SECTION("translateUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

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

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex)
      .translateUV(vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, delta);
    const auto expectedAttributes = expectedBrush.face(*faceIndex).attributes();
    const auto expectedUVAttributes = expectedBrush.face(*faceIndex).uvAttributes();
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(translateUV(map, cameraUp, cameraRight, delta));

    const auto& movedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(movedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(movedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(movedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(movedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("rotateUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(map, {.rotation = SetValue{10.0f}}));

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex).rotateUV(15.0f);
    const auto expectedAttributes = expectedBrush.face(*faceIndex).attributes();
    const auto expectedUVAttributes = expectedBrush.face(*faceIndex).uvAttributes();
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(rotateUV(map, 15.0f));

    const auto& rotatedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(rotatedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(rotatedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(rotatedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(rotatedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("shearUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    const auto factors = vm::vec2f{0.25f, -0.5f};

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex).shearUV(factors);
    const auto expectedAttributes = expectedBrush.face(*faceIndex).attributes();
    const auto expectedUVAttributes = expectedBrush.face(*faceIndex).uvAttributes();
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(shearUV(map, factors));

    const auto& shearedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(shearedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(shearedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(shearedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(shearedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("flipUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

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

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    expectedBrush.face(*faceIndex)
      .flipUV(vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, flipDirection);
    const auto expectedAttributes = expectedBrush.face(*faceIndex).attributes();
    const auto expectedUVAttributes = expectedBrush.face(*faceIndex).uvAttributes();
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    REQUIRE(flipUV(map, cameraUp, cameraRight, flipDirection));

    const auto& flippedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(flippedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(flippedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(flippedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(flippedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("alignUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    const auto faceIndex = brushNode->brush().findFace(vm::vec3d{0, -1, 0});
    REQUIRE(faceIndex);

    const auto otherFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(otherFaceIndex);

    deselectAll(map);
    selectBrushFaces(map, {{brushNode, *faceIndex}});

    REQUIRE(setBrushFaceAttributes(map, {.rotation = SetValue{0.0f}}));

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    evaluate(
      align(expectedBrush.face(*faceIndex), UvPolicy::next),
      expectedBrush.face(*faceIndex));
    const auto expectedAttributes = expectedBrush.face(*faceIndex).attributes();
    const auto expectedUVAttributes = expectedBrush.face(*faceIndex).uvAttributes();
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    alignUV(map, UvPolicy::next);

    const auto& alignedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(
      alignedFace.uvAttributes(), !MatchesUVAttributes(originalFaceUVAttributes));
    CHECK_THAT(alignedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(alignedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(alignedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(alignedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("justifyUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

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

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    evaluate(
      justify(expectedBrush.face(*faceIndex), UvAxis::u, UvSign::plus, UvPolicy::best),
      expectedBrush.face(*faceIndex));
    const auto expectedAttributes = expectedBrush.face(*faceIndex).attributes();
    const auto expectedUVAttributes = expectedBrush.face(*faceIndex).uvAttributes();
    const auto expectedUAxis = expectedBrush.face(*faceIndex).uAxis();
    const auto expectedVAxis = expectedBrush.face(*faceIndex).vAxis();

    justifyUV(map, UvJustifyDirection::Left, UvPolicy::best);

    const auto& justifiedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(
      justifiedFace.uvAttributes(), !MatchesUVAttributes(originalFaceUVAttributes));
    CHECK_THAT(
      justifiedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(justifiedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(justifiedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(justifiedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("fitUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

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

    const auto originalFaceAttributes = getFace(*brushNode, *faceIndex).attributes();
    const auto originalFaceUVAttributes = getFace(*brushNode, *faceIndex).uvAttributes();
    const auto originalUAxis = getFace(*brushNode, *faceIndex).uAxis();
    const auto originalVAxis = getFace(*brushNode, *faceIndex).vAxis();

    const auto originalOtherFaceAttributes =
      getFace(*brushNode, *otherFaceIndex).attributes();
    const auto originalOtherFaceUVAttributes =
      getFace(*brushNode, *otherFaceIndex).uvAttributes();

    auto expectedBrush = brushNode->brush();
    auto& expectedFace = expectedBrush.face(*faceIndex);

    const auto invariantVertex = anchorVertex(expectedFace, UvAxis::u, UvSign::minus);
    const auto previousUvCoords = vm::vec2f{
      expectedFace.toUVCoordSystemMatrix(
        expectedFace.uvAttributes().offset, expectedFace.uvAttributes().scale)
      * invariantVertex};

    evaluate(fit(expectedFace, UvAxis::u, UvPolicy::next), expectedFace);

    const auto newUvCoords = vm::vec2f{
      expectedFace.toUVCoordSystemMatrix(
        expectedFace.uvAttributes().offset, expectedFace.uvAttributes().scale)
      * invariantVertex};
    const auto delta = previousUvCoords - newUvCoords;

    evaluate(
      {
        .xOffset = AddValue{delta.x()},
        .yOffset = AddValue{delta.y()},
      },
      expectedFace);

    const auto expectedAttributes = expectedFace.attributes();
    const auto expectedUVAttributes = expectedFace.uvAttributes();
    const auto expectedUAxis = expectedFace.uAxis();
    const auto expectedVAxis = expectedFace.vAxis();

    fitUV(map, UvFitDirection::Horizontal, UvPolicy::next);

    const auto& fittedFace = getFace(*brushNode, *faceIndex);
    CHECK_THAT(fittedFace.uvAttributes(), !MatchesUVAttributes(originalFaceUVAttributes));
    CHECK_THAT(fittedFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
    CHECK_THAT(fittedFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
    CHECK(fittedFace.uAxis() == vm::approx{expectedUAxis});
    CHECK(fittedFace.vAxis() == vm::approx{expectedVAxis});

    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).attributes(),
      MatchesBrushFaceAttributes(originalOtherFaceAttributes));
    CHECK_THAT(
      getFace(*brushNode, *otherFaceIndex).uvAttributes(),
      MatchesUVAttributes(originalOtherFaceUVAttributes));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      const auto& undoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(
        undoneFace.attributes(), MatchesBrushFaceAttributes(originalFaceAttributes));
      CHECK_THAT(
        undoneFace.uvAttributes(), MatchesUVAttributes(originalFaceUVAttributes));
      CHECK(undoneFace.uAxis() == vm::approx{originalUAxis});
      CHECK(undoneFace.vAxis() == vm::approx{originalVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));

      map.redoCommand();

      const auto& redoneFace = getFace(*brushNode, *faceIndex);
      CHECK_THAT(redoneFace.attributes(), MatchesBrushFaceAttributes(expectedAttributes));
      CHECK_THAT(redoneFace.uvAttributes(), MatchesUVAttributes(expectedUVAttributes));
      CHECK(redoneFace.uAxis() == vm::approx{expectedUAxis});
      CHECK(redoneFace.vAxis() == vm::approx{expectedVAxis});
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).attributes(),
        MatchesBrushFaceAttributes(originalOtherFaceAttributes));
      CHECK_THAT(
        getFace(*brushNode, *otherFaceIndex).uvAttributes(),
        MatchesUVAttributes(originalOtherFaceUVAttributes));
    }
  }

  SECTION("autoFitUV")
  {
    auto& map = fixture.create(QuakeFixtureConfig);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

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

      const auto originalTopAttributes = getFace(*brushNode, iTop).attributes();
      const auto originalTopUVAttributes = getFace(*brushNode, iTop).uvAttributes();

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}, {brushNode, iRight}});
      autoFitUV(map);

      // front face is now aligned
      CHECK(getFace(*brushNode, iFront).uAxis() == vm::approx{vm::vec3d{1, 0, 0}});
      CHECK(getFace(*brushNode, iFront).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      // right face remains aligned
      CHECK(getFace(*brushNode, iRight).uAxis() == vm::approx{vm::vec3d{0, 1, 0}});
      CHECK(getFace(*brushNode, iRight).vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

      // top face was not affected
      CHECK_THAT(
        getFace(*brushNode, iTop).attributes(),
        MatchesBrushFaceAttributes(originalTopAttributes));
      CHECK_THAT(
        getFace(*brushNode, iTop).uvAttributes(),
        MatchesUVAttributes(originalTopUVAttributes));
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

      autoFitUV(map);

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

      const auto originalFrontUVAttributes = getFace(*brushNode, iFront).uvAttributes();
      const auto originalRightUVAttributes = getFace(*brushNode, iRight).uvAttributes();

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, iFront}, {brushNode, iRight}});
      autoFitUV(map);

      const auto modifiedFrontUVAttributes = getFace(*brushNode, iFront).uvAttributes();
      const auto modifiedRightUVAttributes = getFace(*brushNode, iRight).uvAttributes();

      REQUIRE(modifiedFrontUVAttributes != originalFrontUVAttributes);
      REQUIRE(modifiedRightUVAttributes != originalRightUVAttributes);

      map.undoCommand();

      REQUIRE(getFace(*brushNode, iFront).uvAttributes() == originalFrontUVAttributes);
      REQUIRE(getFace(*brushNode, iRight).uvAttributes() == originalRightUVAttributes);

      map.redoCommand();

      REQUIRE(getFace(*brushNode, iFront).uvAttributes() == modifiedFrontUVAttributes);
      REQUIRE(getFace(*brushNode, iRight).uvAttributes() == modifiedRightUVAttributes);
    }
  }
}

} // namespace tb::mdl
