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

#include "Error.h"
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"
#include "Renderer/OrthographicCamera.h"
#include "View/InputState.h"
#include "View/MapDocumentTest.h"
#include "View/PickRequest.h"
#include "View/SelectionTool.h"

#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include "vm/ray.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "SelectionToolTest.clicking")
{
  const auto* world = document->world();
  auto builder = Model::BrushBuilder{
    world->mapFormat(),
    document->worldBounds(),
    document->game()->config().faceAttribsConfig.defaults};

  auto tool = SelectionTool{document};

  GIVEN("A group node")
  {
    auto* brushNode = new Model::BrushNode{builder.createCube(32.0, "some_face").value()};
    auto* entityNode = new Model::EntityNode{{}, {{"origin", "64 0 0"}}};
    auto* groupNode = new Model::GroupNode(Model::Group{"some_group"});

    document->addNodes({{document->parentForNodes(), {groupNode}}});
    document->addNodes({{groupNode, {brushNode, entityNode}}});

    auto camera = Renderer::OrthographicCamera{};

    AND_GIVEN("A pick ray that points at the top face of the brush")
    {
      camera.moveTo({0, 0, 32});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3{camera.pickRay({0, 0, 0})};

      auto pickResult = Model::PickResult{};
      document->pick(pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      REQUIRE(document->selectedBrushFaces().empty());

      auto inputState = InputState{};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I click once")
      {
        inputState.mouseDown(MouseButtons::MBLeft);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::MBLeft);

        THEN("The group gets selected")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(document->selectedNodes() == Model::NodeCollection{{groupNode}});
        }
      }

      WHEN("I double click")
      {
        inputState.mouseDown(MouseButtons::MBLeft);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::MBLeft);

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
    auto brush = builder
                   .createCube(
                     32.0,
                     "left_face",
                     "right_face",
                     "front_face",
                     "back_face",
                     "top_face",
                     "bottom_face")
                   .value();
    auto* brushNode = new Model::BrushNode{std::move(brush)};

    const auto topFaceIndex = brushNode->brush().findFace("top_face").value();
    const auto frontFaceIndex = brushNode->brush().findFace("front_face").value();

    auto* entityNode = new Model::EntityNode{{}, {{"origin", "64 0 0"}}};

    document->addNodes({{document->parentForNodes(), {brushNode, entityNode}}});

    auto camera = Renderer::OrthographicCamera{};

    AND_GIVEN("A pick ray that points at the top face of the brush")
    {
      camera.moveTo({0, 0, 32});
      camera.setDirection({0, 0, -1}, {0, 1, 0});

      const auto pickRay = vm::ray3{camera.pickRay({0, 0, 0})};

      auto pickResult = Model::PickResult{};
      document->pick(pickRay, pickResult);
      REQUIRE(pickResult.all().size() == 1);

      REQUIRE(document->selectedBrushFaces().empty());

      auto inputState = InputState{};
      inputState.setPickRequest({pickRay, camera});
      inputState.setPickResult(std::move(pickResult));

      WHEN("I shift click once")
      {
        inputState.setModifierKeys(ModifierKeys::MKShift);
        inputState.mouseDown(MouseButtons::MBLeft);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::MBLeft);

        THEN("The top face get selected")
        {
          CHECK(
            document->selectedBrushFaces()
            == std::vector<Model::BrushFaceHandle>{{brushNode, topFaceIndex}});
          CHECK(document->selectedNodes().empty());
        }

        AND_WHEN("I shift click on the selected face again")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The top face remains selected")
          {
            CHECK(
              document->selectedBrushFaces()
              == std::vector<Model::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
          }
        }

        AND_WHEN("I shift+ctrl click on the selected face again")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The top face gets deselected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
          }
        }
      }

      WHEN("I click once")
      {
        inputState.mouseDown(MouseButtons::MBLeft);
        tool.mouseClick(inputState);
        inputState.mouseUp(MouseButtons::MBLeft);

        THEN("The brush gets selected")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(document->selectedNodes() == Model::NodeCollection{{brushNode}});
        }

        AND_WHEN("I click on the selected brushagain")
        {
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The brush remains selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == Model::NodeCollection{{brushNode}});
          }
        }

        AND_WHEN("I ctrl click on the selected brush again")
        {
          inputState.setModifierKeys(ModifierKeys::MKCtrlCmd);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The brush gets deselected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
          }
        }
      }

      WHEN("I shift double click")
      {
        inputState.setModifierKeys(ModifierKeys::MKShift);
        inputState.mouseDown(MouseButtons::MBLeft);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::MBLeft);

        THEN("All brush faces are selected")
        {
          CHECK(document->selectedBrushFaces().size() == 6);
          CHECK(document->selectedNodes().empty());
        }
      }

      WHEN("I double click")
      {
        inputState.mouseDown(MouseButtons::MBLeft);
        tool.mouseDoubleClick(inputState);
        inputState.mouseUp(MouseButtons::MBLeft);

        THEN("All nodes are selected")
        {
          CHECK(document->selectedBrushFaces().empty());
          CHECK(
            document->selectedNodes() == Model::NodeCollection{{brushNode, entityNode}});
        }
      }

      AND_GIVEN("The front face of the brush is selected")
      {
        document->selectBrushFaces({{brushNode, frontFaceIndex}});

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The top face get selected")
          {
            CHECK(
              document->selectedBrushFaces()
              == std::vector<Model::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I shift+ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("Both the front and the top faces are selected")
          {
            CHECK_THAT(
              document->selectedBrushFaces(),
              Catch::Matchers::UnorderedEquals(std::vector<Model::BrushFaceHandle>{
                {brushNode, topFaceIndex}, {brushNode, frontFaceIndex}}));
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The brush gets selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == Model::NodeCollection{{brushNode}});
          }
        }

        WHEN("I ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKCtrlCmd);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The brush gets selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == Model::NodeCollection{{brushNode}});
          }
        }
      }

      AND_GIVEN("The entity is selected")
      {
        document->selectNodes({entityNode});

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The top face get selected")
          {
            CHECK(
              document->selectedBrushFaces()
              == std::vector<Model::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I shift+ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The top face get selected")
          {
            CHECK(
              document->selectedBrushFaces()
              == std::vector<Model::BrushFaceHandle>{{brushNode, topFaceIndex}});
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The brush gets selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes() == Model::NodeCollection{{brushNode}});
          }
        }

        WHEN("I ctrl click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKCtrlCmd);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("The brush and entity both get selected")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(
              document->selectedNodes()
              == Model::NodeCollection{{entityNode, brushNode}});
          }
        }
      }

      AND_GIVEN("The top face is hidden")
      {

        auto hiddenTag = Model::Tag{"hidden", {}};

        auto newBrush = brushNode->brush();
        newBrush.face(topFaceIndex).addTag(hiddenTag);
        document->swapNodeContents(
          "Set Tag", {{brushNode, Model::NodeContents{std::move(newBrush)}}});

        REQUIRE(brushNode->brush().face(topFaceIndex).hasTag(hiddenTag));

        document->editorContext().setHiddenTags(hiddenTag.type());
        REQUIRE_FALSE(document->editorContext().visible(
          brushNode, brushNode->brush().face(topFaceIndex)));

        WHEN("I shift click once")
        {
          inputState.setModifierKeys(ModifierKeys::MKShift);
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

          THEN("Nothing happens")
          {
            CHECK(document->selectedBrushFaces().empty());
            CHECK(document->selectedNodes().empty());
          }
        }

        WHEN("I click once")
        {
          inputState.mouseDown(MouseButtons::MBLeft);
          tool.mouseClick(inputState);
          inputState.mouseUp(MouseButtons::MBLeft);

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
} // namespace View
} // namespace TrenchBroom
