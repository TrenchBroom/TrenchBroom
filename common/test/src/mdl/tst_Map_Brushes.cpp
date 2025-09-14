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

#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "ui/MapDocument.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("Map_Brushes")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  SECTION("createBrush")
  {
    fixture.create();

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
    fixture.create();

    SECTION("Setting all attributes")
    {
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
          .color = Color{1.0f, 1.0f, 1.0f, 1.0f},
        });

      {
        const auto& firstAttrs = brushNode->brush().face(firstFaceIndex).attributes();
        CHECK(firstAttrs.materialName() == "first");
        CHECK(firstAttrs.xOffset() == 32.0f);
        CHECK(firstAttrs.yOffset() == 64.0f);
        CHECK(firstAttrs.rotation() == 90.0f);
        CHECK(firstAttrs.xScale() == 2.0f);
        CHECK(firstAttrs.yScale() == 4.0f);
        CHECK(firstAttrs.surfaceFlags() == 63u);
        CHECK(firstAttrs.surfaceContents() == 12u);
        CHECK(firstAttrs.surfaceValue() == 3.14f);
        CHECK(firstAttrs.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
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
          .color = Color{0.5f, 0.5f, 0.5f, 0.5f},
        });

      {
        const auto& secondAttrs = brushNode->brush().face(secondFaceIndex).attributes();
        CHECK(secondAttrs.materialName() == "second");
        CHECK(secondAttrs.xOffset() == 16.0f);
        CHECK(secondAttrs.yOffset() == 48.0f);
        CHECK(secondAttrs.rotation() == 45.0f);
        CHECK(secondAttrs.xScale() == 1.0f);
        CHECK(secondAttrs.yScale() == 1.0f);
        CHECK(secondAttrs.surfaceFlags() == 18u);
        CHECK(secondAttrs.surfaceContents() == 2048u);
        CHECK(secondAttrs.surfaceValue() == 1.0f);
        CHECK(secondAttrs.color() == Color{0.5f, 0.5f, 0.5f, 0.5f});
      }

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, thirdFaceIndex}});

      setBrushFaceAttributes(
        map, copyAll(brushNode->brush().face(secondFaceIndex).attributes()));

      CHECK(
        brushNode->brush().face(thirdFaceIndex).attributes()
        == brushNode->brush().face(secondFaceIndex).attributes());

      auto thirdFaceContentsFlags =
        brushNode->brush().face(thirdFaceIndex).attributes().surfaceContents();

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, secondFaceIndex}});

      setBrushFaceAttributes(
        map, copyAll(brushNode->brush().face(firstFaceIndex).attributes()));

      CHECK(
        brushNode->brush().face(secondFaceIndex).attributes()
        == brushNode->brush().face(firstFaceIndex).attributes());

      deselectAll(map);
      selectBrushFaces(map, {{brushNode, thirdFaceIndex}});
      setBrushFaceAttributes(
        map,
        copyAllExceptContentFlags(brushNode->brush().face(firstFaceIndex).attributes()));

      {
        const auto& firstAttrs = brushNode->brush().face(firstFaceIndex).attributes();
        const auto& newThirdAttrs = brushNode->brush().face(thirdFaceIndex).attributes();
        CHECK(newThirdAttrs.materialName() == firstAttrs.materialName());
        CHECK(newThirdAttrs.xOffset() == firstAttrs.xOffset());
        CHECK(newThirdAttrs.yOffset() == firstAttrs.yOffset());
        CHECK(newThirdAttrs.rotation() == firstAttrs.rotation());
        CHECK(newThirdAttrs.xScale() == firstAttrs.xScale());
        CHECK(newThirdAttrs.yScale() == firstAttrs.yScale());
        CHECK(newThirdAttrs.surfaceFlags() == firstAttrs.surfaceFlags());
        CHECK(newThirdAttrs.surfaceContents() == thirdFaceContentsFlags);
        CHECK(newThirdAttrs.surfaceValue() == firstAttrs.surfaceValue());
        CHECK(newThirdAttrs.color() == firstAttrs.color());
      }
    }

    SECTION("Undo and redo")
    {
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

      fixture.load(
        "fixture/test/ui/ChangeBrushFaceAttributesTest/lavaAndWater.map",
        {.game = LoadGameFixture{"Quake2"}});

      REQUIRE(map.editorContext().currentLayer() != nullptr);

      auto* lavabrush =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(0));
      REQUIRE(lavabrush);
      CHECK(!lavabrush->brush().face(0).attributes().hasSurfaceAttributes());
      CHECK(
        lavabrush->brush().face(0).resolvedSurfaceContents()
        == LavaFlag); // comes from the .wal texture

      auto* waterbrush =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(1));
      REQUIRE(waterbrush);
      CHECK(!waterbrush->brush().face(0).attributes().hasSurfaceAttributes());
      CHECK(
        waterbrush->brush().face(0).resolvedSurfaceContents()
        == WaterFlag); // comes from the .wal texture

      SECTION(
        "Transfer face attributes except content flags from waterbrush to lavabrush")
      {
        selectNodes(map, {lavabrush});

        CHECK(setBrushFaceAttributes(
          map, copyAllExceptContentFlags(waterbrush->brush().face(0).attributes())));

        SECTION("Check lavabrush is now inheriting the water content flags")
        {
          // Note: the contents flag wasn't transferred, but because lavabrushes's
          // content flag was "Inherit", it stays "Inherit" and now inherits the water
          // contents
          CHECK(!lavabrush->brush().face(0).attributes().hasSurfaceAttributes());
          CHECK(lavabrush->brush().face(0).resolvedSurfaceContents() == WaterFlag);
          CHECK(lavabrush->brush().face(0).attributes().materialName() == "watertest");
        }
      }

      SECTION(
        "Setting a content flag when the existing one is inherited keeps the existing "
        "one")
      {
        selectNodes(map, {lavabrush});

        CHECK(setBrushFaceAttributes(map, {.surfaceContents = SetFlagBits{WaterFlag}}));

        CHECK(lavabrush->brush().face(0).attributes().hasSurfaceAttributes());
        CHECK(
          lavabrush->brush().face(0).resolvedSurfaceContents() == (WaterFlag | LavaFlag));
      }
    }

    SECTION("Setting a material keeps the surface flags unset")
    {
      fixture.create({.mapFormat = MapFormat::Valve, .game = LoadGameFixture{"Quake"}});

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      selectNodes(map, {brushNode});
      CHECK(!brushNode->brush().face(0).attributes().hasSurfaceAttributes());

      setBrushFaceAttributes(map, {.materialName = "something_else"});

      CHECK(brushNode->brush().face(0).attributes().materialName() == "something_else");
      CHECK(!brushNode->brush().face(0).attributes().hasSurfaceAttributes());
    }

    SECTION("Reset attributes to defaults")
    {
      auto defaultFaceAttrs = BrushFaceAttributes{BrushFaceAttributes::NoMaterialName};
      defaultFaceAttrs.setXScale(0.5f);
      defaultFaceAttrs.setYScale(2.0f);

      auto gameConfig = MockGameConfig{};
      gameConfig.faceAttribsConfig.defaults = defaultFaceAttrs;

      fixture.create(
        {.mapFormat = MapFormat::Valve, .game = MockGameFixture{std::move(gameConfig)}});

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const size_t faceIndex = 0u;
      const auto initialX = brushNode->brush().face(faceIndex).uAxis();
      const auto initialY = brushNode->brush().face(faceIndex).vAxis();

      selectBrushFaces(map, {{brushNode, faceIndex}});

      for (size_t i = 0; i < 5; ++i)
      {
        setBrushFaceAttributes(map, {.rotation = AddValue{2.0f}});
      }

      REQUIRE(brushNode->brush().face(faceIndex).attributes().rotation() == 10.0f);

      setBrushFaceAttributes(map, resetAll(defaultFaceAttrs));

      CHECK(brushNode->brush().face(faceIndex).attributes().xOffset() == 0.0f);
      CHECK(brushNode->brush().face(faceIndex).attributes().yOffset() == 0.0f);
      CHECK(brushNode->brush().face(faceIndex).attributes().rotation() == 0.0f);
      CHECK(
        brushNode->brush().face(faceIndex).attributes().xScale()
        == defaultFaceAttrs.xScale());
      CHECK(
        brushNode->brush().face(faceIndex).attributes().yScale()
        == defaultFaceAttrs.yScale());

      CHECK(brushNode->brush().face(faceIndex).uAxis() == initialX);
      CHECK(brushNode->brush().face(faceIndex).vAxis() == initialY);
    }

    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

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

        CHECK(setBrushFaceAttributes(map, {.materialName = "abc"}));

        // check that the brushes in both linked groups got a material
        for (auto* g : std::vector<GroupNode*>{groupNode, linkedGroupNode})
        {
          auto* brush = dynamic_cast<BrushNode*>(g->children().at(0));
          REQUIRE(brush != nullptr);

          auto attrs = brush->brush().face(0).attributes();
          CHECK(attrs.materialName() == "abc");
        }
      }
    }
  }
}

} // namespace tb::mdl
