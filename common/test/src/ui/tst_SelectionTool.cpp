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

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "render/OrthographicCamera.h"
#include "ui/InputState.h"
#include "ui/MapDocumentTest.h"
#include "ui/PickRequest.h"
#include "ui/SelectionTool.h"

#include "kdl/result.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "SelectionToolTest.clicking")
{
  const auto* world = document->world();
  auto builder = mdl::BrushBuilder{
    world->mapFormat(),
    document->worldBounds(),
    document->game()->config().faceAttribsConfig.defaults};

  auto tool = SelectionTool{document};

  GIVEN("A group node")
  {
    auto* brushNode =
      new mdl::BrushNode{builder.createCube(32.0, "some_face") | kdl::value()};
    auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"origin", "64 0 0"}}}};
    auto* groupNode = new mdl::GroupNode(mdl::Group{"some_group"});

    document->addNodes({{document->parentForNodes(), {groupNode}}});
    document->addNodes({{groupNode, {brushNode, entityNode}}});

    auto camera = render::OrthographicCamera{};

    AND_GIVEN("A pick ray that points at the top face of the brush")
    {
      camera.moveTo({0, 0, 32});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

      auto pickResult = mdl::PickResult{};
      document->pick(pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      REQUIRE(document->selectedBrushFaces().empty());

      auto inputState = InputState{};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I click once")
      {
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        THEN("The group gets selected")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(document->selectedNodes() == mdl::makeNodeCollection({groupNode}));
        }
      }

      WHEN("I double click")
      {
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        THEN("The group is opened")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(document->selectedNodes().empty());
          CHECK(document->currentGroup() == groupNode);
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
    auto* brushNode = new mdl::BrushNode{std::move(brush)};

    const auto topFaceIndex = *brushNode->brush().findFace("top_face");
    const auto frontFaceIndex = *brushNode->brush().findFace("front_face");

    auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"origin", "64 0 0"}}}};

    document->addNodes({{document->parentForNodes(), {brushNode, entityNode}}});

    auto camera = render::OrthographicCamera{};

    AND_GIVEN("A pick ray that points at the top face of the brush")
    {
      camera.moveTo({0, 0, 32});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

      auto pickResult = mdl::PickResult{};
      document->pick(pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      REQUIRE(document->selectedBrushFaces().empty());

      auto inputState = InputState{};
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
            document->selectedBrushFaces()
            == std::vector<mdl::BrushFaceHandle>{{brushNode, topFaceIndex}});
          CHECK(document->selectedNodes().empty());
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
              document->selectedBrushFaces()
              == std::vector<mdl::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
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
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
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
          CHECK(document->selectedBrushFaces().empty());
          CHECK(document->selectedNodes() == mdl::makeNodeCollection({brushNode}));
        }

        AND_WHEN("I click on the selected brushagain")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The brush remains selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == mdl::makeNodeCollection({brushNode}));
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
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
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
          CHECK(document->selectedBrushFaces().size() == 6);
          CHECK(document->selectedNodes().empty());
        }
      }

      WHEN("I double click")
      {
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        THEN("All nodes are selected")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(
            document->selectedNodes()
            == mdl::makeNodeCollection({brushNode, entityNode}));
        }
      }

      AND_GIVEN("The front face of the brush is selected")
      {
        document->selectBrushFaces({{brushNode, frontFaceIndex}});

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The top face get selected")
          {
            CHECK(
              document->selectedBrushFaces()
              == std::vector<mdl::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
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
              document->selectedBrushFaces(),
              Catch::Matchers::UnorderedEquals(std::vector<mdl::BrushFaceHandle>{
                {brushNode, topFaceIndex}, {brushNode, frontFaceIndex}}));
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The brush gets selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == mdl::makeNodeCollection({brushNode}));
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
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == mdl::makeNodeCollection({brushNode}));
          }
        }
      }

      AND_GIVEN("The entity is selected")
      {
        document->selectNodes({entityNode});

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The top face get selected")
          {
            CHECK(
              document->selectedBrushFaces()
              == std::vector<mdl::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
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
              document->selectedBrushFaces()
              == std::vector<mdl::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("The brush gets selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == mdl::makeNodeCollection({brushNode}));
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
            CHECK(document->selectedBrushFaces().empty());
            CHECK(
              document->selectedNodes()
              == mdl::makeNodeCollection({entityNode, brushNode}));
          }
        }
      }

      AND_GIVEN("The top face is hidden")
      {

        auto hiddenTag = mdl::Tag{"hidden", {}};

        auto newBrush = brushNode->brush();
        newBrush.face(topFaceIndex).addTag(hiddenTag);
        document->swapNodeContents(
          "Set Tag", {{brushNode, mdl::NodeContents{std::move(newBrush)}}});

        REQUIRE(brushNode->brush().face(topFaceIndex).hasTag(hiddenTag));

        document->editorContext().setHiddenTags(hiddenTag.type());
        REQUIRE_FALSE(document->editorContext().visible(
          brushNode, brushNode->brush().face(topFaceIndex)));

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::Shift);
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::Left);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::Left);

          THEN("Nothing happens")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
          }
        }
      }
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionToolTest.clickingThroughHidden")
{
  const auto* world = document->world();
  auto builder = mdl::BrushBuilder{
    world->mapFormat(),
    document->worldBounds(),
    document->game()->config().faceAttribsConfig.defaults};

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
    auto* visibleBrushNode = new mdl::BrushNode{std::move(visibleBrush)};
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
    auto* hiddenBrushNode = new mdl::BrushNode{std::move(hiddenBrush)};
    const auto hiddenTopFaceIndex = *hiddenBrushNode->brush().findFace("top_face");

    document->addNodes(
      {{document->parentForNodes(), {visibleBrushNode, hiddenBrushNode}}});

    const auto hiddenTag = mdl::Tag{"hidden", {}};
    auto taggedBrush = hiddenBrushNode->brush();
    taggedBrush.face(hiddenTopFaceIndex).addTag(hiddenTag);
    document->swapNodeContents(
      "Set Tag", {{hiddenBrushNode, mdl::NodeContents{std::move(taggedBrush)}}});

    document->editorContext().setHiddenTags(hiddenTag.type());

    REQUIRE(hiddenBrushNode->brush().face(hiddenTopFaceIndex).hasTag(hiddenTag));
    CHECK_FALSE(document->editorContext().visible(
      hiddenBrushNode, hiddenBrushNode->brush().face(hiddenTopFaceIndex)));

    auto camera = render::OrthographicCamera{};
    AND_GIVEN("A pick ray that points at the top face of the brushes")
    {
      camera.moveTo({0, 0, 128});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3d{camera.pickRay({0, 0, 0})};

      auto pickResult = mdl::PickResult{};
      document->pick(pickRay, pickResult);
      CHECK(pickResult.all().size() == 2);
      REQUIRE(document->selectedBrushFaces().empty());

      auto inputState = InputState{};
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
          CHECK(document->selectedNodes().empty());
          CHECK(
            document->selectedBrushFaces()
            == std::vector<mdl::BrushFaceHandle>{
              {visibleBrushNode, visibleTopFaceIndex}});
        }
      }

      WHEN("I click once")
      {
        inputState.mouseDown(MouseButtons::Left);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::Left);

        THEN("The visible brush gets selected")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(document->selectedNodes() == mdl::makeNodeCollection({visibleBrushNode}));
        }
      }
    }
  }
}
} // namespace tb::ui
