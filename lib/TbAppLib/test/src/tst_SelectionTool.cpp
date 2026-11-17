/*
 Copyright (C) 2022 Kristian Duske

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

#include "base/PreferenceManager.h"
#include "gl/FontManager.h"
#include "gl/OrthographicCamera.h"
#include "gl/ShaderManager.h"
#include "gl/TestGl.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GameInfo.h"
#include "mdl/Grid.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Hit.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/Selection.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "prefs/Preferences.h"
#include "render/RenderContext.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/SelectionTool.h"

#include "kd/invoke.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/vector_utils.h"

#include "vm/bbox.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

TEST_CASE("SelectionTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  SECTION("tool returns itself")
  {
    auto tool = SelectionTool{document};
    CHECK(&tool.tool() == static_cast<Tool*>(&tool));

    const auto& constTool = tool;
    CHECK(&constTool.tool() == static_cast<const Tool*>(&tool));
  }

  SECTION("cancel always returns false")
  {
    auto tool = SelectionTool{document};
    CHECK(!tool.cancel());
  }

  SECTION("unhandledInput")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    GIVEN("A brush node")
    {
      addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::BrushNode>(
          builder.createCube(32.0, "some_face") | kdl::value()));

      auto camera = gl::OrthographicCamera{};

      AND_GIVEN("A pick ray that points at the top face of the brush")
      {
        camera.moveTo({0, 0, 32});
        camera.setDirection({0, 0, -1}, {0, 1, 0});

        const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

        auto pickResult = mdl::PickResult{};
        pick(map, pickRay, pickResult);
        REQUIRE(pickResult.all().size() == 1);

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setPickRequest({pickRay, camera});
        inputState.setPickResult(std::move(pickResult));

        WHEN("I ctrl click with the right mouse button")
        {
          inputState.setModifierKeys(ModifierKeys::CtrlCmd);
          inputState.mouseDown(MouseButtons::Right);

          THEN("Neither clicks nor drags are handled")
          {
            CHECK(!tool.mouseClick(inputState));
            CHECK(!tool.mouseDoubleClick(inputState));
            CHECK(tool.acceptMouseDrag(inputState) == nullptr);
            CHECK(!map.selection().hasAny());
          }
        }

        WHEN("I shift+alt double click with the right mouse button")
        {
          inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
          inputState.mouseDown(MouseButtons::Right);

          THEN("The double click is not handled")
          {
            CHECK(!tool.mouseDoubleClick(inputState));
            CHECK(!map.selection().hasAny());
          }
        }

        WHEN("I ctrl+alt click")
        {
          inputState.setModifierKeys(ModifierKeys::CtrlCmd | ModifierKeys::Alt);
          inputState.mouseDown(MouseButtons::Left);

          THEN("Neither clicks nor drags are handled")
          {
            CHECK(!tool.mouseClick(inputState));
            CHECK(!tool.mouseDoubleClick(inputState));
            CHECK(tool.acceptMouseDrag(inputState) == nullptr);
            CHECK(!map.selection().hasAny());
          }
        }

        WHEN("Selection changes are blocked")
        {
          map.editorContext().setBlockSelection(true);
          inputState.mouseDown(MouseButtons::Left);

          THEN("Neither clicks nor drags are handled")
          {
            CHECK(!tool.mouseClick(inputState));

            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
            CHECK(!tool.mouseDoubleClick(inputState));

            inputState.setModifierKeys(ModifierKeys::CtrlCmd);
            CHECK(tool.acceptMouseDrag(inputState) == nullptr);

            CHECK(!map.selection().hasAny());
          }
        }
      }
    }
  }

  SECTION("clicking")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    GIVEN("A group node")
    {

      auto* groupNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::GroupNode>(mdl::Group{"some_group"}));
      addNodes(
        map,
        *groupNode,
        kdl::vec_from<std::unique_ptr<mdl::Node>>(
          std::make_unique<mdl::BrushNode>(
            builder.createCube(32.0, "some_face") | kdl::value()),
          std::make_unique<mdl::EntityNode>(mdl::Entity{{{"origin", "64 0 0"}}})));

      auto camera = gl::OrthographicCamera{};

      AND_GIVEN("A pick ray that points at the top face of the brush")
      {
        camera.moveTo({0, 0, 32});
        camera.setDirection({0, 0, -1}, {0, 1, 0});

        const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

        auto pickResult = mdl::PickResult{};
        pick(map, pickRay, pickResult);
        REQUIRE(pickResult.all().size() == 1);

        REQUIRE(map.selection().brushFaces.empty());

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setPickRequest({pickRay, camera});
        inputState.setPickResult(std::move(pickResult));

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The group gets selected")
          {
            CHECK(map.selection().brushFaces.empty());
            CHECK(map.selection() == mdl::makeSelection(map, {groupNode}));
          }
        }

        WHEN("I double click")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseDoubleClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The group is opened")
          {
            CHECK(map.selection().brushFaces.empty());
            CHECK(!map.selection().hasNodes());
            CHECK(map.editorContext().currentGroup() == groupNode);
          }
        }
      }

      AND_GIVEN("An entity outside of the group and the group is open")
      {
        addNode(
          map,
          parentForNodes(map),
          std::make_unique<mdl::EntityNode>(mdl::Entity{{{"origin", "0 128 0"}}}));

        openGroup(map, *groupNode);
        REQUIRE(map.editorContext().currentGroup() == groupNode);

        AND_GIVEN("A pick ray that points at the top face of the brush in the group")
        {
          camera.moveTo({0, 0, 32});
          camera.setDirection({0, 0, -1}, {0, 1, 0});

          const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

          auto pickResult = mdl::PickResult{};
          pick(map, pickRay, pickResult);
          REQUIRE(pickResult.all().size() == 1);

          auto inputState = InputState{0.0f, 0.0f};
          inputState.setPickRequest({pickRay, camera});
          inputState.setPickResult(std::move(pickResult));

          WHEN("I double click")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("All nodes in the group are selected and the group remains open")
            {
              CHECK(
                map.selection()
                == mdl::makeSelection(
                  map, groupNode->children() | kdl::ranges::to<std::vector>()));
              CHECK(map.editorContext().currentGroup() == groupNode);
            }
          }
        }

        AND_GIVEN("A pick ray that points at the entity outside of the group")
        {
          camera.moveTo({0, 128, 32});
          camera.setDirection({0, 0, -1}, {0, 1, 0});

          const auto pickRay = vm::ray3d{camera.pickRay({0, 128, 0})};

          auto pickResult = mdl::PickResult{};
          pick(map, pickRay, pickResult);
          REQUIRE(pickResult.all().size() == 1);

          auto inputState = InputState{0.0f, 0.0f};
          inputState.setPickRequest({pickRay, camera});
          inputState.setPickResult(std::move(pickResult));

          WHEN("I double click")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The group is closed")
            {
              CHECK(!map.selection().hasAny());
              CHECK(map.editorContext().currentGroup() == nullptr);
            }
          }
        }
      }
    }

    GIVEN("A brush node and an entity node")
    {
      auto brush = builder.createCube(
                     32.0,
                     "left_face",
                     "right_face",
                     "front_face",
                     "back_face",
                     "top_face",
                     "bottom_face")
                   | kdl::value();
      auto brushNodePtr = std::make_unique<mdl::BrushNode>(std::move(brush));
      auto* brushNode = brushNodePtr.get();

      const auto topFaceIndex = *brushNode->brush().findFace("top_face");
      const auto frontFaceIndex = *brushNode->brush().findFace("front_face");

      auto entityNodePtr =
        std::make_unique<mdl::EntityNode>(mdl::Entity{{{"origin", "64 0 0"}}});
      auto* entityNode = entityNodePtr.get();

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<mdl::Node>>(
          std::move(brushNodePtr), std::move(entityNodePtr)));

      auto camera = gl::OrthographicCamera{};

      AND_GIVEN("A pick ray that points at the top face of the brush")
      {
        camera.moveTo({0, 0, 32});
        camera.setDirection({0, 0, -1}, {0, 1, 0});

        const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

        auto pickResult = mdl::PickResult{};
        pick(map, pickRay, pickResult);
        REQUIRE(pickResult.all().size() == 1);

        REQUIRE(map.selection().brushFaces.empty());

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setPickRequest({pickRay, camera});
        inputState.setPickResult(std::move(pickResult));

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The top face get selected")
          {
            CHECK(
              map.selection().brushFaces
              == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
            CHECK(!map.selection().hasNodes());
          }

          AND_WHEN("I shift click on the selected face again")
          {
            inputState.setModifierKeys(ModifierKeys::Shift);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The top face remains selected")
            {
              CHECK(
                map.selection().brushFaces
                == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
              CHECK(!map.selection().hasNodes());
            }
          }

          AND_WHEN("I shift+ctrl click on the selected face again")
          {
            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The top face gets deselected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(!map.selection().hasNodes());
            }
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The brush gets selected")
          {
            CHECK(map.selection().brushFaces.empty());
            CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
          }

          AND_WHEN("I click on the selected brushagain")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The brush remains selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
            }
          }

          AND_WHEN("I ctrl click on the selected brush again")
          {
            inputState.setModifierKeys(ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The brush gets deselected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(!map.selection().hasNodes());
            }
          }
        }

        WHEN("I shift double click")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseDoubleClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("All brush faces are selected")
          {
            CHECK(map.selection().brushFaces.size() == 6);
            CHECK(!map.selection().hasNodes());
          }
        }

        WHEN("I double click")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseDoubleClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("All nodes are selected")
          {
            CHECK(map.selection().brushFaces.empty());
            CHECK(map.selection() == mdl::makeSelection(map, {brushNode, entityNode}));
          }
        }

        AND_GIVEN("The brush is selected")
        {
          selectNodes(map, {brushNode});

          WHEN("I shift+ctrl click once")
          {
            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("All faces except for the top face are selected")
            {
              CHECK_THAT(
                map.selection().brushFaces,
                UnorderedEquals(kdl::vec_erase(
                  mdl::toHandles(*brushNode),
                  mdl::BrushFaceHandle{*brushNode, topFaceIndex})));
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I shift+ctrl double click")
          {
            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("All brush faces are selected")
            {
              CHECK(map.selection().brushFaces.size() == 6);
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I ctrl double click")
          {
            inputState.setModifierKeys(ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("All nodes are selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {brushNode, entityNode}));
            }
          }
        }

        AND_GIVEN("The front face of the brush is selected")
        {
          selectBrushFaces(map, {{*brushNode, frontFaceIndex}});

          WHEN("I shift click once")
          {
            inputState.setModifierKeys(ModifierKeys::Shift);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The top face get selected")
            {
              CHECK(
                map.selection().brushFaces
                == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I shift+ctrl click once")
          {
            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("Both the front and the top faces are selected")
            {
              CHECK_THAT(
                map.selection().brushFaces,
                UnorderedEquals(std::vector<mdl::BrushFaceHandle>{
                  {*brushNode, topFaceIndex}, {*brushNode, frontFaceIndex}}));
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("A brush node with a coplanar face exists and its front face is selected")
          {
            auto coplanarBrush =
              builder.createCube(
                32.0,
                "left_face",
                "right_face",
                "front_face",
                "back_face",
                "top_face",
                "bottom_face")
              | kdl::and_then([&](auto b) {
                  return b.transform(
                           map.worldBounds(),
                           vm::translation_matrix(vm::vec3d{32, 0, 0}),
                           false)
                         | kdl::transform([&]() { return std::move(b); });
                })
              | kdl::value();

            auto* coplanarBrushNode = addNode(
              map,
              parentForNodes(map),
              std::make_unique<mdl::BrushNode>(std::move(coplanarBrush)));

            const auto coplanarTopFaceIndex =
              *coplanarBrushNode->brush().findFace("top_face");
            mdl::selectBrushFaces(map, {{*brushNode, frontFaceIndex}});

            AND_WHEN("I select all adjacent coplanar faces")
            {
              inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
              inputState.mouseDown(MouseButtons::Left);
              tool.mouseDoubleClick(inputState);
              inputState.mouseUp(MouseButtons::Left);

              THEN("The clicked face's coplanar region is selected")
              {
                CHECK_THAT(
                  map.selection().brushFaces,
                  UnorderedEquals(std::vector<mdl::BrushFaceHandle>{
                    {*brushNode, topFaceIndex},
                    {*coplanarBrushNode, coplanarTopFaceIndex}}));
                CHECK(!map.selection().hasNodes());
              }
            }

            AND_WHEN("I add all adjacent coplanar faces")
            {
              inputState.setModifierKeys(
                ModifierKeys::CtrlCmd | ModifierKeys::Shift | ModifierKeys::Alt);
              inputState.mouseDown(MouseButtons::Left);
              tool.mouseDoubleClick(inputState);
              inputState.mouseUp(MouseButtons::Left);

              THEN("The clicked face's coplanar region is added to the selection")
              {
                CHECK_THAT(
                  map.selection().brushFaces,
                  UnorderedEquals(std::vector<mdl::BrushFaceHandle>{
                    {*brushNode, frontFaceIndex},
                    {*brushNode, topFaceIndex},
                    {*coplanarBrushNode, coplanarTopFaceIndex}}));
                CHECK(!map.selection().hasNodes());
              }
            }
          }

          WHEN("I click once")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The brush gets selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
            }
          }

          WHEN("I ctrl click once")
          {
            inputState.setModifierKeys(ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The brush gets selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
            }
          }

          WHEN("I ctrl double click")
          {
            inputState.setModifierKeys(ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("All nodes are selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {brushNode, entityNode}));
            }
          }
        }

        AND_GIVEN("The entity is selected")
        {
          selectNodes(map, {entityNode});

          WHEN("I shift click once")
          {
            inputState.setModifierKeys(ModifierKeys::Shift);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The top face get selected")
            {
              CHECK(
                map.selection().brushFaces
                == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I shift+ctrl click once")
          {
            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The top face get selected")
            {
              CHECK(
                map.selection().brushFaces
                == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I click once")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The brush gets selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
            }
          }

          WHEN("I ctrl click once")
          {
            inputState.setModifierKeys(ModifierKeys::CtrlCmd);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("The brush and entity both get selected")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(map.selection() == mdl::makeSelection(map, {entityNode, brushNode}));
            }
          }
        }

        AND_GIVEN("The top face is hidden")
        {

          auto hiddenTag = mdl::Tag{"hidden", {}};

          auto newBrush = brushNode->brush();
          newBrush.face(topFaceIndex).addTag(hiddenTag);
          updateNodeContents(
            map, "Set Tag", {{brushNode, mdl::NodeContents{std::move(newBrush)}}});

          REQUIRE(brushNode->brush().face(topFaceIndex).hasTag(hiddenTag));

          map.editorContext().setHiddenTags(hiddenTag.type());
          REQUIRE_FALSE(map.editorContext().visible(
            *brushNode, brushNode->brush().face(topFaceIndex)));

          WHEN("I shift click once")
          {
            inputState.setModifierKeys(ModifierKeys::Shift);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("Nothing happens")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I click once")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("Nothing happens")
            {
              CHECK(map.selection().brushFaces.empty());
              CHECK(!map.selection().hasNodes());
            }
          }

          WHEN("I shift double click")
          {
            inputState.setModifierKeys(ModifierKeys::Shift);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("Nothing happens")
            {
              CHECK(!map.selection().hasAny());
            }
          }

          WHEN("I shift+alt double click")
          {
            inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("Nothing happens")
            {
              CHECK(!map.selection().hasAny());
            }
          }

          WHEN("I double click")
          {
            inputState.mouseDown(MouseButtons::Left);
            tool.mouseDoubleClick(inputState);
            inputState.mouseUp(MouseButtons::Left);

            THEN("Nothing happens")
            {
              CHECK(!map.selection().hasAny());
            }
          }
        }
      }
    }

    GIVEN("A selected node")
    {

      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::EntityNode>(mdl::Entity{{{"origin", "64 0 0"}}}));
      mdl::selectNodes(map, {entityNode});

      auto camera = gl::OrthographicCamera{};

      AND_GIVEN("A pick ray that points into the void")
      {
        camera.moveTo({0, 0, 32});
        camera.setDirection({0, 0, 1}, {0, 1, 0});

        const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

        auto pickResult = mdl::PickResult{};
        pick(map, pickRay, pickResult);
        REQUIRE(pickResult.all().empty());

        REQUIRE(map.selection().nodes == std::vector<mdl::Node*>{entityNode});

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setPickRequest({pickRay, camera});
        inputState.setPickResult(std::move(pickResult));

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The node is deselected")
          {
            CHECK(!map.selection().hasNodes());
          }
        }

        WHEN("I shift+ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(map.selection().nodes == std::vector<mdl::Node*>{entityNode});
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The node is deselected")
          {
            CHECK(!map.selection().hasNodes());
          }
        }

        WHEN("I ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::CtrlCmd);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(map.selection().nodes == std::vector<mdl::Node*>{entityNode});
          }
        }

        WHEN("I shift double click")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseDoubleClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(map.selection().nodes == std::vector<mdl::Node*>{entityNode});
          }
        }

        WHEN("I shift+alt double click")
        {
          inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
          inputState.mouseDown(MouseButtons::Left);
          CHECK(tool.mouseDoubleClick(inputState));
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(map.selection().nodes == std::vector<mdl::Node*>{entityNode});
          }
        }
      }
    }

    GIVEN("A selected brush face")
    {
      auto brushNodePtr = std::make_unique<mdl::BrushNode>(
        builder.createCube(32.0, "some_face") | kdl::value());
      auto* brushNode = brushNodePtr.get();
      const auto faceHandle = mdl::BrushFaceHandle{*brushNode, 0};

      addNode(map, parentForNodes(map), std::move(brushNodePtr));
      mdl::selectBrushFaces(map, {faceHandle});

      auto camera = gl::OrthographicCamera{};

      AND_GIVEN("A pick ray that points into the void")
      {
        camera.moveTo({0, 0, 32});
        camera.setDirection({0, 0, 1}, {0, 1, 0});

        const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

        auto pickResult = mdl::PickResult{};
        pick(map, pickRay, pickResult);
        REQUIRE(pickResult.all().empty());

        REQUIRE(map.selection().brushFaces == std::vector{faceHandle});

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setPickRequest({pickRay, camera});
        inputState.setPickResult(std::move(pickResult));

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The face is deselected")
          {
            CHECK(map.selection().brushFaces.empty());
          }
        }

        WHEN("I shift+ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(map.selection().brushFaces == std::vector{faceHandle});
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The face is deselected")
          {
            CHECK(map.selection().brushFaces.empty());
          }
        }

        WHEN("I ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::CtrlCmd);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(map.selection().brushFaces == std::vector{faceHandle});
          }
        }
      }
    }
  }

  SECTION("clickingThroughHidden")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    GIVEN("A brush visible behind the hidden face of another brush")
    {
      auto visibleBrush = builder.createCube(
                            32.0,
                            "left_face",
                            "right_face",
                            "front_face",
                            "back_face",
                            "top_face",
                            "bottom_face")
                          | kdl::value();
      auto visibleBrushNodePtr =
        std::make_unique<mdl::BrushNode>(std::move(visibleBrush));
      auto* visibleBrushNode = visibleBrushNodePtr.get();
      const auto visibleTopFaceIndex = *visibleBrushNode->brush().findFace("top_face");

      auto hiddenBrush = builder.createCube(
                           64.0,
                           "left_face",
                           "right_face",
                           "front_face",
                           "back_face",
                           "top_face",
                           "bottom_face")
                         | kdl::value();
      auto hiddenBrushNodePtr = std::make_unique<mdl::BrushNode>(std::move(hiddenBrush));
      auto* hiddenBrushNode = hiddenBrushNodePtr.get();
      const auto hiddenTopFaceIndex = *hiddenBrushNode->brush().findFace("top_face");

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<mdl::Node>>(
          std::move(visibleBrushNodePtr), std::move(hiddenBrushNodePtr)));

      const auto hiddenTag = mdl::Tag{"hidden", {}};
      auto taggedBrush = hiddenBrushNode->brush();
      taggedBrush.face(hiddenTopFaceIndex).addTag(hiddenTag);
      updateNodeContents(
        map, "Set Tag", {{hiddenBrushNode, mdl::NodeContents{std::move(taggedBrush)}}});

      map.editorContext().setHiddenTags(hiddenTag.type());

      REQUIRE(hiddenBrushNode->brush().face(hiddenTopFaceIndex).hasTag(hiddenTag));
      CHECK(!map.editorContext().visible(
        *hiddenBrushNode, hiddenBrushNode->brush().face(hiddenTopFaceIndex)));

      auto camera = gl::OrthographicCamera{};
      AND_GIVEN("A pick ray that points at the top face of the brushes")
      {
        camera.moveTo({0, 0, 128});
        camera.setDirection({0, 0, -1}, {0, 1, 0});

        const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

        auto pickResult = mdl::PickResult{};
        pick(map, pickRay, pickResult);
        CHECK(pickResult.all().size() == 2);
        REQUIRE(map.selection().brushFaces.empty());

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setPickRequest({pickRay, camera});
        inputState.setPickResult(std::move(pickResult));

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The top face of the visible brush get selected")
          {
            CHECK(!map.selection().hasNodes());
            CHECK(
              map.selection().brushFaces
              == std::vector<mdl::BrushFaceHandle>{
                {*visibleBrushNode, visibleTopFaceIndex}});
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The visible brush gets selected")
          {
            CHECK(map.selection().brushFaces.empty());
            CHECK(map.selection() == mdl::makeSelection(map, {visibleBrushNode}));
          }
        }
      }
    }
  }

  SECTION("scrolling")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    GIVEN("Two nested brushes along the pick ray")
    {
      // the larger brush's top face is higher up, so it is hit first (at the smaller
      // distance) by a ray looking straight down; the smaller brush's top face is hit
      // second
      auto largeBrushNodePtr = std::make_unique<mdl::BrushNode>(
        builder.createCube(64.0, "material") | kdl::value());
      auto* largeBrushNode = largeBrushNodePtr.get();
      auto smallBrushNodePtr = std::make_unique<mdl::BrushNode>(
        builder.createCube(32.0, "material") | kdl::value());
      auto* smallBrushNode = smallBrushNodePtr.get();
      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<mdl::Node>>(
          std::move(largeBrushNodePtr), std::move(smallBrushNodePtr)));
      selectNodes(map, {largeBrushNode});

      auto camera = gl::OrthographicCamera{};
      camera.moveTo({0, 0, 128});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 2);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I scroll forward with just CtrlCmd held")
      {
        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
        tool.mouseScroll(inputState);

        THEN("The selection drills through to the next node under the ray")
        {
          CHECK(map.selection() == mdl::makeSelection(map, {smallBrushNode}));
        }
      }

      WHEN("I scroll backward with just CtrlCmd held")
      {
        deselectAll(map);
        selectNodes(map, {smallBrushNode});

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.scroll(ScrollSource::Mouse, 0.0f, -1.0f);
        tool.mouseScroll(inputState);

        THEN("The selection drills through to the previous node under the ray")
        {
          CHECK(map.selection() == mdl::makeSelection(map, {largeBrushNode}));
        }
      }

      WHEN("I scroll with CtrlCmd+Alt held")
      {
        const auto sizeBefore = map.grid().size();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
        tool.mouseScroll(inputState);

        THEN("The grid size changes instead of drilling the selection")
        {
          CHECK(map.grid().size() != sizeBefore);
          CHECK(map.selection() == mdl::makeSelection(map, {largeBrushNode}));
        }
      }

      WHEN("I scroll with no modifiers")
      {
        const auto sizeBefore = map.grid().size();

        inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
        tool.mouseScroll(inputState);

        THEN("Neither the grid size nor the selection changes")
        {
          CHECK(map.grid().size() == sizeBefore);
          CHECK(map.selection() == mdl::makeSelection(map, {largeBrushNode}));
        }
      }

      WHEN("I scroll backward with CtrlCmd+Alt held")
      {
        const auto sizeBefore = map.grid().size();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        inputState.scroll(ScrollSource::Mouse, 0.0f, -1.0f);
        tool.mouseScroll(inputState);

        THEN("The grid size increases")
        {
          CHECK(map.grid().size() == sizeBefore + 1);
        }
      }

      WHEN("I scroll horizontally with CtrlCmd+Alt held")
      {
        const auto sizeBefore = map.grid().size();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        inputState.scroll(ScrollSource::Mouse, 1.0f, 0.0f);
        tool.mouseScroll(inputState);

        THEN("The grid size does not change")
        {
          CHECK(map.grid().size() == sizeBefore);
        }
      }

      WHEN("I scroll forward with just CtrlCmd held and the last node is selected")
      {
        deselectAll(map);
        selectNodes(map, {smallBrushNode});

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
        tool.mouseScroll(inputState);

        THEN("The selection does not change")
        {
          CHECK(map.selection() == mdl::makeSelection(map, {smallBrushNode}));
        }
      }

      WHEN("I scroll backward with just CtrlCmd held and the first node is selected")
      {
        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.scroll(ScrollSource::Mouse, 0.0f, -1.0f);
        tool.mouseScroll(inputState);

        THEN("The selection does not change")
        {
          CHECK(map.selection() == mdl::makeSelection(map, {largeBrushNode}));
        }
      }

      WHEN("I scroll with just CtrlCmd held and no node under the ray is selected")
      {
        deselectAll(map);

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
        tool.mouseScroll(inputState);

        THEN("Nothing gets selected")
        {
          CHECK(!map.selection().hasAny());
        }
      }

      AND_GIVEN("The mouse wheel is inverted")
      {
        setPref(Preferences::CameraMouseWheelInvert, true);
        auto resetPref = kdl::invoke_later{[] {
          PreferenceManager::instance().resetToDefault(
            Preferences::CameraMouseWheelInvert);
        }};

        WHEN("I scroll forward with just CtrlCmd held")
        {
          deselectAll(map);
          selectNodes(map, {smallBrushNode});

          inputState.setModifierKeys(ModifierKeys::CtrlCmd);
          inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
          tool.mouseScroll(inputState);

          THEN("The selection drills through to the previous node under the ray")
          {
            CHECK(map.selection() == mdl::makeSelection(map, {largeBrushNode}));
          }
        }

        WHEN("I scroll forward with CtrlCmd+Alt held")
        {
          const auto sizeBefore = map.grid().size();

          inputState.setModifierKeys(ModifierKeys::CtrlCmd | ModifierKeys::Alt);
          inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
          tool.mouseScroll(inputState);

          THEN("The grid size increases")
          {
            CHECK(map.grid().size() == sizeBefore + 1);
          }
        }
      }
    }
  }

  SECTION("dragging")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    GIVEN("A brush node and an entity node")
    {
      auto brush = builder.createCube(
                     32.0,
                     "left_face",
                     "right_face",
                     "front_face",
                     "back_face",
                     "top_face",
                     "bottom_face")
                   | kdl::value();
      auto brushNodePtr = std::make_unique<mdl::BrushNode>(std::move(brush));
      auto* brushNode = brushNodePtr.get();

      const auto topFaceIndex = *brushNode->brush().findFace("top_face");
      const auto frontFaceIndex = *brushNode->brush().findFace("front_face");

      auto entityNodePtr =
        std::make_unique<mdl::EntityNode>(mdl::Entity{{{"origin", "64 0 0"}}});
      auto* entityNode = entityNodePtr.get();

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<mdl::Node>>(
          std::move(brushNodePtr), std::move(entityNodePtr)));

      auto camera = gl::OrthographicCamera{};
      camera.moveTo({0, 0, 32});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I drag without CtrlCmd held")
      {
        inputState.mouseDown(MouseButtons::Left);

        THEN("No drag is accepted")
        {
          CHECK(tool.acceptMouseDrag(inputState) == nullptr);
        }
      }

      WHEN("I shift+ctrl drag starting on the top face")
      {
        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        THEN("The top face is selected")
        {
          CHECK(
            map.selection().brushFaces
            == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
        }

        AND_WHEN("The drag moves onto the front face")
        {
          auto frontPickResult = mdl::PickResult{};
          frontPickResult.addHit(mdl::Hit{
            mdl::BrushNode::BrushHitType,
            0.0,
            vm::vec3d{0, 0, 0},
            mdl::BrushFaceHandle{*brushNode, frontFaceIndex}});

          auto dragInputState = InputState{0.0f, 0.0f};
          dragInputState.setPickRequest({pickRay, camera});
          dragInputState.setPickResult(std::move(frontPickResult));
          dragInputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
          dragInputState.mouseDown(MouseButtons::Left);

          CHECK(tracker->update(dragInputState));

          THEN("Both faces end up selected")
          {
            CHECK_THAT(
              map.selection().brushFaces,
              UnorderedEquals(std::vector<mdl::BrushFaceHandle>{
                {*brushNode, topFaceIndex}, {*brushNode, frontFaceIndex}}));
          }

          tracker->end(dragInputState);
        }

        AND_WHEN("The drag moves into the void")
        {
          auto dragInputState = InputState{0.0f, 0.0f};
          dragInputState.setPickRequest({pickRay, camera});
          dragInputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
          dragInputState.mouseDown(MouseButtons::Left);

          CHECK(tracker->update(dragInputState));

          THEN("The selection does not change")
          {
            CHECK(
              map.selection().brushFaces
              == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
          }

          tracker->end(dragInputState);
        }

        AND_WHEN("The drag stays on the top face")
        {
          CHECK(tracker->update(inputState));

          THEN("The selection does not change")
          {
            CHECK(
              map.selection().brushFaces
              == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
          }

          tracker->end(inputState);
        }
      }

      WHEN("I ctrl drag starting on the brush")
      {
        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        THEN("The brush is selected")
        {
          CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
        }

        AND_WHEN("The drag moves onto the entity")
        {
          auto entityPickResult = mdl::PickResult{};
          entityPickResult.addHit(mdl::Hit{
            mdl::EntityNode::EntityHitType, 0.0, vm::vec3d{64, 0, 0}, entityNode});

          auto dragInputState = InputState{0.0f, 0.0f};
          dragInputState.setPickRequest({pickRay, camera});
          dragInputState.setPickResult(std::move(entityPickResult));
          dragInputState.setModifierKeys(ModifierKeys::CtrlCmd);
          dragInputState.mouseDown(MouseButtons::Left);

          CHECK(tracker->update(dragInputState));

          THEN("Both the brush and the entity end up selected")
          {
            CHECK(map.selection() == mdl::makeSelection(map, {brushNode, entityNode}));
          }

          tracker->cancel();
        }

        AND_WHEN("The drag moves into the void")
        {
          auto dragInputState = InputState{0.0f, 0.0f};
          dragInputState.setPickRequest({pickRay, camera});
          dragInputState.setModifierKeys(ModifierKeys::CtrlCmd);
          dragInputState.mouseDown(MouseButtons::Left);

          CHECK(tracker->update(dragInputState));

          THEN("The selection does not change")
          {
            CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
          }

          tracker->cancel();
        }

        AND_WHEN("The drag stays on the brush")
        {
          CHECK(tracker->update(inputState));

          THEN("The selection does not change")
          {
            CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
          }

          tracker->cancel();
        }

        AND_WHEN("The drag ends")
        {
          tracker->end(inputState);

          THEN("The drag is recorded as a single undoable step")
          {
            CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
            REQUIRE(map.undoCommandName() != nullptr);
            CHECK(*map.undoCommandName() == "Drag Select Objects");
          }
        }

        AND_WHEN("The drag is cancelled")
        {
          tracker->cancel();

          THEN("The selection is restored")
          {
            CHECK(!map.selection().hasAny());
          }
        }
      }

      WHEN("I shift+ctrl drag starting on the top face while the brush is selected")
      {
        selectNodes(map, {brushNode});

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        THEN("Only the top face is selected")
        {
          CHECK(!map.selection().hasNodes());
          CHECK(
            map.selection().brushFaces
            == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
        }

        tracker->end(inputState);
      }

      WHEN("I shift+ctrl drag starting on the selected top face")
      {
        selectBrushFaces(map, {{*brushNode, topFaceIndex}});

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        THEN("The top face remains selected")
        {
          CHECK(
            map.selection().brushFaces
            == std::vector<mdl::BrushFaceHandle>{{*brushNode, topFaceIndex}});
        }

        tracker->end(inputState);
      }

      WHEN("I ctrl drag starting on the brush while a face is selected")
      {
        selectBrushFaces(map, {{*brushNode, frontFaceIndex}});

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        THEN("Only the brush is selected")
        {
          CHECK(map.selection().brushFaces.empty());
          CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
        }

        tracker->end(inputState);
      }

      WHEN("I ctrl drag starting on the selected brush")
      {
        selectNodes(map, {brushNode});

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        THEN("The brush remains selected")
        {
          CHECK(map.selection() == mdl::makeSelection(map, {brushNode}));
        }

        tracker->end(inputState);
      }

      WHEN("I shift+ctrl drag starting in the void")
      {
        inputState.setPickResult(mdl::PickResult{});
        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        THEN("No drag is accepted")
        {
          CHECK(tool.acceptMouseDrag(inputState) == nullptr);
          CHECK(!map.selection().hasAny());
        }
      }

      WHEN("I ctrl drag starting in the void")
      {
        inputState.setPickResult(mdl::PickResult{});
        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        THEN("No drag is accepted")
        {
          CHECK(tool.acceptMouseDrag(inputState) == nullptr);
          CHECK(!map.selection().hasAny());
        }
      }
    }
  }

  SECTION("setRenderOptions")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    auto camera = gl::OrthographicCamera{};
    camera.moveTo({0, 0, 32});
    camera.setDirection({0, 0, -1}, {0, 1, 0});

    const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

    auto testGl = gl::TestGl{};
    auto fontManager = gl::FontManager{[](const auto& path) { return path; }};
    auto shaderManager = gl::ShaderManager{[](const auto& path) { return path; }};
    auto renderContext = render::RenderContext{
      testGl, render::RenderMode::Render3D, camera, fontManager, shaderManager};

    GIVEN("A brush node under the pick ray")
    {
      auto* brushNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::BrushNode>(
          builder.createCube(32.0, "some_face") | kdl::value()));

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("The brush is not selected")
      {
        tool.setRenderOptions(inputState, renderContext);

        THEN("The selection guide is not shown")
        {
          CHECK(!renderContext.showSelectionGuide());
        }
      }

      WHEN("The brush is selected")
      {
        selectNodes(map, {brushNode});
        tool.setRenderOptions(inputState, renderContext);

        THEN("The selection guide is shown")
        {
          CHECK(renderContext.showSelectionGuide());
        }
      }

      WHEN("The brush is selected, but the pick ray hits nothing")
      {
        selectNodes(map, {brushNode});
        inputState.setPickResult(mdl::PickResult{});
        tool.setRenderOptions(inputState, renderContext);

        THEN("The selection guide is not shown")
        {
          CHECK(!renderContext.showSelectionGuide());
        }
      }
    }

    GIVEN("A closed group containing a brush node under the pick ray")
    {
      auto* groupNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::GroupNode>(mdl::Group{"some_group"}));
      addNode(
        map,
        *groupNode,
        std::make_unique<mdl::BrushNode>(
          builder.createCube(32.0, "some_face") | kdl::value()));

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("The group is selected")
      {
        selectNodes(map, {groupNode});
        tool.setRenderOptions(inputState, renderContext);

        THEN("The selection guide is shown")
        {
          CHECK(renderContext.showSelectionGuide());
        }
      }
    }
  }

  SECTION("undoRedo")
  {
    const auto& worldNode = map.worldNode();
    auto builder = mdl::BrushBuilder{
      worldNode.mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

    auto tool = SelectionTool{document};

    auto camera = gl::OrthographicCamera{};
    camera.moveTo({0, 0, 128});
    camera.setDirection({0, 0, -1}, {0, 1, 0});

    const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

    GIVEN("A brush node and an entity node")
    {
      auto* brushNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::BrushNode>(
          builder.createCube(
            32.0,
            "left_face",
            "right_face",
            "front_face",
            "back_face",
            "top_face",
            "bottom_face")
          | kdl::value()));
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::EntityNode>(mdl::Entity{{{"origin", "64 0 0"}}}));

      const auto frontFaceIndex = *brushNode->brush().findFace("front_face");

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I click the brush while a face is selected")
      {
        selectBrushFaces(map, {{*brushNode, frontFaceIndex}});
        const auto selectionBefore = map.selection();

        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I ctrl click the brush while a face is selected")
      {
        selectBrushFaces(map, {{*brushNode, frontFaceIndex}});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift click the top face while the entity is selected")
      {
        selectNodes(map, {entityNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift+ctrl click the top face while the entity is selected")
      {
        selectNodes(map, {entityNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift+ctrl click the top face while the brush is selected")
      {
        selectNodes(map, {brushNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift double click the top face while the entity is selected")
      {
        selectNodes(map, {entityNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift+ctrl double click the top face while the entity is selected")
      {
        selectNodes(map, {entityNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift+alt double click the top face while the entity is selected")
      {
        selectNodes(map, {entityNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I shift+ctrl drag across two faces while the brush is selected")
      {
        selectNodes(map, {brushNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        auto frontPickResult = mdl::PickResult{};
        frontPickResult.addHit(mdl::Hit{
          mdl::BrushNode::BrushHitType,
          0.0,
          vm::vec3d{0, 0, 0},
          mdl::BrushFaceHandle{*brushNode, frontFaceIndex}});
        inputState.setPickResult(std::move(frontPickResult));

        tracker->update(inputState);
        tracker->end(inputState);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter.brushFaces.size() == 2);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I ctrl drag across the brush and the entity while a face is selected")
      {
        selectBrushFaces(map, {{*brushNode, frontFaceIndex}});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = tool.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        auto entityPickResult = mdl::PickResult{};
        entityPickResult.addHit(
          mdl::Hit{mdl::EntityNode::EntityHitType, 0.0, vm::vec3d{64, 0, 0}, entityNode});
        inputState.setPickResult(std::move(entityPickResult));

        tracker->update(inputState);
        tracker->end(inputState);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter == mdl::makeSelection(map, {brushNode, entityNode}));

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }
    }

    GIVEN("A brush entity with two brushes and a brush outside of it")
    {
      auto* entityNode = addNode(
        map, parentForNodes(map), std::make_unique<mdl::EntityNode>(mdl::Entity{}));
      addNode(
        map,
        *entityNode,
        std::make_unique<mdl::BrushNode>(
          builder.createCuboid(vm::bbox3d{{-16, -16, -16}, {16, 16, 16}}, "some_face")
          | kdl::value()));
      addNode(
        map,
        *entityNode,
        std::make_unique<mdl::BrushNode>(
          builder.createCuboid(vm::bbox3d{{48, -16, -16}, {80, 16, 16}}, "some_face")
          | kdl::value()));
      auto* otherBrushNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::BrushNode>(
          builder.createCuboid(vm::bbox3d{{112, -16, -16}, {144, 16, 16}}, "some_face")
          | kdl::value()));

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I double click a brush of the entity while the other brush is selected")
      {
        selectNodes(map, {otherBrushNode});
        const auto selectionBefore = map.selection();

        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I ctrl double click a brush of the entity while a face is selected")
      {
        selectBrushFaces(map, {{*otherBrushNode, 0}});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }
    }

    GIVEN("A group containing a brush")
    {
      auto* groupNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::GroupNode>(mdl::Group{"some_group"}));
      auto* brushNode = addNode(
        map,
        *groupNode,
        std::make_unique<mdl::BrushNode>(
          builder.createCube(32.0, "some_face") | kdl::value()));

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I double click the selected group")
      {
        selectNodes(map, {groupNode});
        const auto selectionBefore = map.selection();

        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(map.editorContext().currentGroup() == groupNode);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.editorContext().currentGroup() == nullptr);
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.editorContext().currentGroup() == groupNode);
          CHECK(map.selection() == selectionAfter);
        }
      }

      WHEN("I double click the void while the group is open and its brush is selected")
      {
        openGroup(map, *groupNode);
        selectNodes(map, {brushNode});
        const auto selectionBefore = map.selection();

        inputState.setPickResult(mdl::PickResult{});
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        const auto selectionAfter = map.selection();
        REQUIRE(map.editorContext().currentGroup() == nullptr);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.editorContext().currentGroup() == groupNode);
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.editorContext().currentGroup() == nullptr);
          CHECK(map.selection() == selectionAfter);
        }
      }
    }

    GIVEN("Two nested brushes along the pick ray")
    {
      auto* largeBrushNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::BrushNode>(
          builder.createCube(64.0, "material") | kdl::value()));
      addNode(
        map,
        parentForNodes(map),
        std::make_unique<mdl::BrushNode>(
          builder.createCube(32.0, "material") | kdl::value()));

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 2);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I drill the selection")
      {
        selectNodes(map, {largeBrushNode});
        const auto selectionBefore = map.selection();

        inputState.setModifierKeys(ModifierKeys::CtrlCmd);
        inputState.scroll(ScrollSource::Mouse, 0.0f, 1.0f);
        tool.mouseScroll(inputState);

        const auto selectionAfter = map.selection();
        REQUIRE(selectionAfter != selectionBefore);

        THEN("Undo and redo each take a single step")
        {
          map.undoCommand();
          CHECK(map.selection() == selectionBefore);

          map.redoCommand();
          CHECK(map.selection() == selectionAfter);
        }
      }
    }
  }
}

} // namespace tb::ui
