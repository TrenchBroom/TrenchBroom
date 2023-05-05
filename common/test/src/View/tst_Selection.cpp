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

#include "Exceptions.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/NodeCollection.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <kdl/result.h>

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.allSelectedEntityNodes")
{
  GIVEN("A document with multiple entity nodes in various configurations")
  {
    auto* topLevelEntityNode = new Model::EntityNode{Model::Entity{}};

    auto* emptyGroupNode = new Model::GroupNode{Model::Group{"empty"}};
    auto* groupNodeWithEntity = new Model::GroupNode{Model::Group{"group"}};
    auto* groupedEntityNode = new Model::EntityNode{Model::Entity{}};
    groupNodeWithEntity->addChild(groupedEntityNode);

    auto* topLevelBrushNode = createBrushNode();
    auto* topLevelPatchNode = createPatchNode();

    auto* topLevelBrushEntityNode = new Model::EntityNode{Model::Entity{}};
    auto* brushEntityBrushNode = createBrushNode();
    auto* brushEntityPatchNode = createPatchNode();
    topLevelBrushEntityNode->addChildren({brushEntityBrushNode, brushEntityPatchNode});

    document->addNodes(
      {{document->parentForNodes(),
        {topLevelEntityNode,
         topLevelBrushEntityNode,
         topLevelBrushNode,
         topLevelPatchNode,
         emptyGroupNode,
         groupNodeWithEntity}}});

    document->deselectAll();

    WHEN("Nothing is selected")
    {
      THEN("The world node is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(
            std::vector<Model::EntityNodeBase*>{document->world()}));
      }
    }

    WHEN("A top level brush node is selected")
    {
      document->selectNodes({topLevelBrushNode});

      THEN("The world node is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(
            std::vector<Model::EntityNodeBase*>{document->world()}));
      }
    }

    WHEN("A top level patch node is selected")
    {
      document->selectNodes({topLevelPatchNode});

      THEN("The world node is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(
            std::vector<Model::EntityNodeBase*>{document->world()}));
      }
    }

    WHEN("An empty group node is selected")
    {
      document->selectNodes({emptyGroupNode});

      THEN("An empty vector is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{}));
      }
    }

    WHEN("A group node containing an entity node is selected")
    {
      document->selectNodes({groupNodeWithEntity});

      THEN("The grouped entity node is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(
            std::vector<Model::EntityNodeBase*>{groupedEntityNode}));
      }

      AND_WHEN("A top level entity node is selected")
      {
        document->selectNodes({topLevelEntityNode});

        THEN("The top level entity node and the grouped entity node are returned")
        {
          CHECK_THAT(
            document->allSelectedEntityNodes(),
            Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
              groupedEntityNode, topLevelEntityNode}));
        }
      }
    }

    WHEN("An empty top level entity node is selected")
    {
      document->selectNodes({topLevelEntityNode});

      THEN("That entity node is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(
            std::vector<Model::EntityNodeBase*>{topLevelEntityNode}));
      }
    }

    WHEN("A node in a brush entity node is selected")
    {
      const auto selectBrushNode =
        [](auto* brushNode, auto* patchNode) -> std::tuple<Model::Node*, Model::Node*> {
        return {brushNode, patchNode};
      };
      const auto selectPatchNode =
        [](auto* brushNode, auto* patchNode) -> std::tuple<Model::Node*, Model::Node*> {
        return {patchNode, brushNode};
      };
      const auto selectNodes = GENERATE_COPY(selectBrushNode, selectPatchNode);

      const auto [nodeToSelect, otherNode] =
        selectNodes(brushEntityBrushNode, brushEntityPatchNode);

      CAPTURE(nodeToSelect->name(), otherNode->name());

      document->selectNodes({nodeToSelect});

      THEN("The containing entity node is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(
            std::vector<Model::EntityNodeBase*>{topLevelBrushEntityNode}));
      }

      AND_WHEN("Another node in the same entity node is selected")
      {
        document->selectNodes({otherNode});

        THEN("The containing entity node is returned only once")
        {
          CHECK_THAT(
            document->allSelectedEntityNodes(),
            Catch::Matchers::UnorderedEquals(
              std::vector<Model::EntityNodeBase*>{topLevelBrushEntityNode}));
        }
      }

      AND_WHEN("A top level entity node is selected")
      {
        document->selectNodes({topLevelEntityNode});

        THEN("The top level entity node and the brush entity node are returned")
        {
          CHECK_THAT(
            document->allSelectedEntityNodes(),
            Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
              topLevelBrushEntityNode, topLevelEntityNode}));
        }
      }
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouching")
{
  Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
  Model::BrushNode* brushNode1 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  Model::BrushNode* brushNode2 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  Model::BrushNode* brushNode3 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());

  transformNode(
    *brushNode2,
    vm::translation_matrix(vm::vec3{10.0, 0.0, 0.0}),
    document->worldBounds());
  transformNode(
    *brushNode3,
    vm::translation_matrix(vm::vec3{100.0, 0.0, 0.0}),
    document->worldBounds());

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  REQUIRE(brushNode1->intersects(brushNode2));
  REQUIRE(brushNode2->intersects(brushNode1));

  REQUIRE(!brushNode1->intersects(brushNode3));
  REQUIRE(!brushNode3->intersects(brushNode1));

  document->selectNodes({brushNode1});
  document->selectTouching(false);

  using Catch::Matchers::UnorderedEquals;
  CHECK_THAT(
    document->selectedNodes().brushes(),
    UnorderedEquals(std::vector<Model::BrushNode*>{brushNode2}));
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2476
TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouching_2476")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const Model::BrushBuilder builder(
    document->world()->mapFormat(), document->worldBounds());
  const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

  auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new Model::BrushNode(
    builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectAllNodes();

  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::UnorderedEquals(std::vector<Model::BrushNode*>{brushNode1, brushNode2}));
  CHECK_THAT(
    document->currentLayer()->children(),
    Catch::Equals(std::vector<Model::Node*>{brushNode1, brushNode2}));

  document->selectTouching(true);

  // only this next line was failing
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::UnorderedEquals(std::vector<Model::BrushNode*>{}));
  CHECK_THAT(
    document->currentLayer()->children(), Catch::Equals(std::vector<Model::Node*>{}));

  // brush1 and brush2 are deleted
  CHECK(brushNode1->parent() == nullptr);
  CHECK(brushNode2->parent() == nullptr);
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouchingWithGroup")
{
  document->selectAllNodes();
  document->deleteObjects();
  assert(document->selectedNodes().nodeCount() == 0);

  Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
  document->addNodes({{document->world(), {layer}}});

  Model::GroupNode* group = new Model::GroupNode(Model::Group("Unnamed"));
  document->addNodes({{layer, {group}}});

  Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
  const vm::bbox3 brushBounds(
    vm::vec3(-32.0, -32.0, -32.0), vm::vec3(+32.0, +32.0, +32.0));

  Model::BrushNode* brush =
    new Model::BrushNode(builder.createCuboid(brushBounds, "texture").value());
  document->addNodes({{group, {brush}}});

  const vm::bbox3 selectionBounds(
    vm::vec3(-16.0, -16.0, -48.0), vm::vec3(+16.0, +16.0, +48.0));

  Model::BrushNode* selectionBrush =
    new Model::BrushNode(builder.createCuboid(selectionBounds, "texture").value());
  document->addNodes({{layer, {selectionBrush}}});

  document->selectNodes({selectionBrush});
  document->selectTouching(true);

  CHECK(document->selectedNodes().nodeCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectInsideWithGroup")
{
  document->selectAllNodes();
  document->deleteObjects();
  assert(document->selectedNodes().nodeCount() == 0);

  Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
  document->addNodes({{document->world(), {layer}}});

  Model::GroupNode* group = new Model::GroupNode(Model::Group("Unnamed"));
  document->addNodes({{layer, {group}}});

  Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
  const vm::bbox3 brushBounds(
    vm::vec3(-32.0, -32.0, -32.0), vm::vec3(+32.0, +32.0, +32.0));

  Model::BrushNode* brush =
    new Model::BrushNode(builder.createCuboid(brushBounds, "texture").value());
  document->addNodes({{group, {brush}}});

  const vm::bbox3 selectionBounds(
    vm::vec3(-48.0, -48.0, -48.0), vm::vec3(+48.0, +48.0, +48.0));

  Model::BrushNode* selectionBrush =
    new Model::BrushNode(builder.createCuboid(selectionBounds, "texture").value());
  document->addNodes({{layer, {selectionBrush}}});

  document->selectNodes({selectionBrush});
  document->selectInside(true);

  CHECK(document->selectedNodes().nodeCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTall")
{
  using Catch::Matchers::UnorderedEquals;

  Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
  Model::BrushNode* brushNode1 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  Model::BrushNode* brushNode2 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  Model::BrushNode* brushNode3 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());

  transformNode(
    *brushNode2,
    vm::translation_matrix(vm::vec3{0.0, 0.0, -500.0}),
    document->worldBounds());
  transformNode(
    *brushNode3,
    vm::translation_matrix(vm::vec3{100.0, 0.0, 0.0}),
    document->worldBounds());

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  REQUIRE(!brushNode1->intersects(brushNode2));
  REQUIRE(!brushNode1->intersects(brushNode3));

  document->selectNodes({brushNode1});

  SECTION("z camera")
  {
    document->selectTall(vm::axis::z);

    CHECK_THAT(
      document->selectedNodes().brushes(),
      UnorderedEquals(std::vector<Model::BrushNode*>{brushNode2}));
  }
  SECTION("x camera")
  {
    document->selectTall(vm::axis::x);

    CHECK_THAT(
      document->selectedNodes().brushes(),
      UnorderedEquals(std::vector<Model::BrushNode*>{brushNode3}));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectInverse")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const Model::BrushBuilder builder(
    document->world()->mapFormat(), document->worldBounds());
  const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

  auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new Model::BrushNode(
    builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  auto* brushNode3 = new Model::BrushNode(
    builder.createCuboid(box.translate(vm::vec3(2, 2, 2)), "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  auto* patchNode = createPatchNode();
  document->addNodes({{document->parentForNodes(), {patchNode}}});

  document->selectNodes({brushNode1, brushNode2});
  Model::EntityNode* brushEnt = document->createBrushEntity(m_brushEntityDef);

  document->deselectAll();

  // worldspawn {
  //   brushEnt { brush1, brush2 },
  //   brush3
  //   patch
  // }

  document->selectNodes({brushNode1});
  REQUIRE(brushNode1->selected());
  REQUIRE(!brushNode2->selected());
  REQUIRE(!brushNode3->selected());
  REQUIRE(!brushEnt->selected());
  REQUIRE(!patchNode->selected());

  document->selectInverse();

  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode2, brushNode3, patchNode}));
  CHECK(!brushNode1->selected());
  CHECK(brushNode2->selected());
  CHECK(brushNode3->selected());
  CHECK(!brushEnt->selected());
  CHECK(patchNode->selected());
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3826
TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouchingInsideNestedGroup")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  auto* brushNode1 = createBrushNode();
  auto* brushNode2 = createBrushNode();

  auto* outerGroup = new Model::GroupNode{Model::Group{"outerGroup"}};
  auto* innerGroup = new Model::GroupNode{Model::Group{"innerGroup"}};

  document->addNodes({{document->parentForNodes(), {outerGroup}}});
  document->addNodes({{outerGroup, {innerGroup}}});
  document->addNodes({{innerGroup, {brushNode1}}});
  document->addNodes({{innerGroup, {brushNode2}}});

  // worldspawn {
  //   outerGroup {
  //     innerGroup { brush1, brush2 }
  //   }
  // }

  outerGroup->open();
  innerGroup->open();
  document->selectNodes({brushNode1});

  document->selectTouching(false);

  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::UnorderedEquals(std::vector<Model::BrushNode*>{brushNode2}));
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectSiblings")
{
  document->selectAllNodes();
  document->deleteObjects();

  const Model::BrushBuilder builder(
    document->world()->mapFormat(), document->worldBounds());
  const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

  auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new Model::BrushNode(
    builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  auto* brushNode3 = new Model::BrushNode(
    builder.createCuboid(box.translate(vm::vec3(2, 2, 2)), "texture").value());
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  auto* patchNode = createPatchNode();
  document->addNodes({{document->parentForNodes(), {patchNode}}});

  document->selectNodes({brushNode1, brushNode2});
  document->createBrushEntity(m_brushEntityDef);

  document->deselectAll();

  // worldspawn {
  //   brushEnt { brush1, brush2 },
  //   brush3
  //   patch
  // }

  SECTION("Brush in default layer")
  {
    document->selectNodes({brushNode3});
    REQUIRE_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode3}));

    document->selectSiblings();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{brushNode1, brushNode2, brushNode3, patchNode}));

    document->undoCommand();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode3}));
  }

  SECTION("Brush in brush entity")
  {
    document->selectNodes({brushNode1});
    REQUIRE_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode1}));

    document->selectSiblings();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode1, brushNode2}));

    document->undoCommand();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode1}));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.updateLastSelectionBounds")
{
  auto* entityNode = new Model::EntityNode({}, {{"classname", "point_entity"}});
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  REQUIRE(!entityNode->logicalBounds().is_empty());

  document->selectAllNodes();

  auto bounds = document->selectionBounds();
  document->deselectAll();
  CHECK(document->lastSelectionBounds() == bounds);

  document->deselectAll();
  CHECK(document->lastSelectionBounds() == bounds);

  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  document->selectNodes({brushNode});
  CHECK(document->lastSelectionBounds() == bounds);

  bounds = brushNode->logicalBounds();

  document->deselectAll();
  CHECK(document->lastSelectionBounds() == bounds);
}

TEST_CASE_METHOD(
  MapDocumentTest, "SelectionCommandTest.faceSelectionUndoAfterTranslationUndo")
{
  Model::BrushNode* brushNode = createBrushNode();
  CHECK(brushNode->logicalBounds().center() == vm::vec3::zero());

  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto topFaceIndex = brushNode->brush().findFace(vm::vec3::pos_z());
  REQUIRE(topFaceIndex);

  // select the top face
  document->selectBrushFaces({{brushNode, *topFaceIndex}});
  CHECK_THAT(
    document->selectedBrushFaces(),
    Catch::Equals(std::vector<Model::BrushFaceHandle>{{brushNode, *topFaceIndex}}));

  // deselect it
  document->deselectBrushFaces({{brushNode, *topFaceIndex}});
  CHECK_THAT(
    document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

  // select the brush
  document->selectNodes({brushNode});
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::Equals(std::vector<Model::BrushNode*>{brushNode}));

  // translate the brush
  document->translateObjects(vm::vec3(10.0, 0.0, 0.0));
  CHECK(brushNode->logicalBounds().center() == vm::vec3(10.0, 0.0, 0.0));

  // Start undoing changes

  document->undoCommand();
  CHECK(brushNode->logicalBounds().center() == vm::vec3::zero());
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::Equals(std::vector<Model::BrushNode*>{brushNode}));
  CHECK_THAT(
    document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

  document->undoCommand();
  CHECK_THAT(
    document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{}));
  CHECK_THAT(
    document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

  document->undoCommand();
  CHECK_THAT(
    document->selectedBrushFaces(),
    Catch::Equals(std::vector<Model::BrushFaceHandle>{{brushNode, *topFaceIndex}}));
}
} // namespace View
} // namespace TrenchBroom
