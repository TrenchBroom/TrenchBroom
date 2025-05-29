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

#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/NodeCollection.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include "kdl/result.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.allSelectedEntityNodes")
{
  GIVEN("A document with multiple entity nodes in various configurations")
  {
    auto* topLevelEntityNode = new mdl::EntityNode{mdl::Entity{}};

    auto* emptyGroupNode = new mdl::GroupNode{mdl::Group{"empty"}};
    auto* groupNodeWithEntity = new mdl::GroupNode{mdl::Group{"group"}};
    auto* groupedEntityNode = new mdl::EntityNode{mdl::Entity{}};
    groupNodeWithEntity->addChild(groupedEntityNode);

    auto* topLevelBrushNode = createBrushNode();
    auto* topLevelPatchNode = createPatchNode();

    auto* topLevelBrushEntityNode = new mdl::EntityNode{mdl::Entity{}};
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
            std::vector<mdl::EntityNodeBase*>{document->world()}));
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
            std::vector<mdl::EntityNodeBase*>{document->world()}));
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
            std::vector<mdl::EntityNodeBase*>{document->world()}));
      }
    }

    WHEN("An empty group node is selected")
    {
      document->selectNodes({emptyGroupNode});

      THEN("An empty vector is returned")
      {
        CHECK_THAT(
          document->allSelectedEntityNodes(),
          Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityNodeBase*>{}));
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
            std::vector<mdl::EntityNodeBase*>{groupedEntityNode}));
      }

      AND_WHEN("A top level entity node is selected")
      {
        document->selectNodes({topLevelEntityNode});

        THEN("The top level entity node and the grouped entity node are returned")
        {
          CHECK_THAT(
            document->allSelectedEntityNodes(),
            Catch::Matchers::UnorderedEquals(
              std::vector<mdl::EntityNodeBase*>{groupedEntityNode, topLevelEntityNode}));
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
            std::vector<mdl::EntityNodeBase*>{topLevelEntityNode}));
      }
    }

    WHEN("A node in a brush entity node is selected")
    {
      const auto selectBrushNode =
        [](auto* brushNode, auto* patchNode) -> std::tuple<mdl::Node*, mdl::Node*> {
        return {brushNode, patchNode};
      };
      const auto selectPatchNode =
        [](auto* brushNode, auto* patchNode) -> std::tuple<mdl::Node*, mdl::Node*> {
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
            std::vector<mdl::EntityNodeBase*>{topLevelBrushEntityNode}));
      }

      AND_WHEN("Another node in the same entity node is selected")
      {
        document->selectNodes({otherNode});

        THEN("The containing entity node is returned only once")
        {
          CHECK_THAT(
            document->allSelectedEntityNodes(),
            Catch::Matchers::UnorderedEquals(
              std::vector<mdl::EntityNodeBase*>{topLevelBrushEntityNode}));
        }
      }

      AND_WHEN("A top level entity node is selected")
      {
        document->selectNodes({topLevelEntityNode});

        THEN("The top level entity node and the brush entity node are returned")
        {
          CHECK_THAT(
            document->allSelectedEntityNodes(),
            Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityNodeBase*>{
              topLevelBrushEntityNode, topLevelEntityNode}));
        }
      }
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouching")
{
  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
  auto* brushNode2 = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
  auto* brushNode3 = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};

  transformNode(
    *brushNode2,
    vm::translation_matrix(vm::vec3d{10.0, 0.0, 0.0}),
    document->worldBounds());
  transformNode(
    *brushNode3,
    vm::translation_matrix(vm::vec3d{100.0, 0.0, 0.0}),
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
    UnorderedEquals(std::vector<mdl::BrushNode*>{brushNode2}));
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2476
TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouching_2476")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

  auto* brushNode1 =
    new mdl::BrushNode{builder.createCuboid(box, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectAllNodes();

  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::UnorderedEquals(std::vector<mdl::BrushNode*>{brushNode1, brushNode2}));
  CHECK_THAT(
    document->currentLayer()->children(),
    Catch::Equals(std::vector<mdl::Node*>{brushNode1, brushNode2}));

  document->selectTouching(true);

  // only this next line was failing
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::UnorderedEquals(std::vector<mdl::BrushNode*>{}));
  CHECK_THAT(
    document->currentLayer()->children(), Catch::Equals(std::vector<mdl::Node*>{}));

  // brush1 and brush2 are deleted
  CHECK(brushNode1->parent() == nullptr);
  CHECK(brushNode2->parent() == nullptr);
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouchingWithGroup")
{
  document->selectAllNodes();
  document->remove();
  assert(document->selectedNodes().nodeCount() == 0);

  auto* layer = new mdl::LayerNode{mdl::Layer{"Layer 1"}};
  document->addNodes({{document->world(), {layer}}});

  auto* group = new mdl::GroupNode{mdl::Group{"Unnamed"}};
  document->addNodes({{layer, {group}}});

  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto brushBounds = vm::bbox3d{{-32.0, -32.0, -32.0}, {+32.0, +32.0, +32.0}};

  auto* brush =
    new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
  document->addNodes({{group, {brush}}});

  const auto selectionBounds = vm::bbox3d{{-16.0, -16.0, -48.0}, {+16.0, +16.0, +48.0}};

  auto* selectionBrush =
    new mdl::BrushNode{builder.createCuboid(selectionBounds, "material") | kdl::value()};
  document->addNodes({{layer, {selectionBrush}}});

  document->selectNodes({selectionBrush});
  document->selectTouching(true);

  CHECK(document->selectedNodes().nodeCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectInsideWithGroup")
{
  document->selectAllNodes();
  document->remove();
  assert(document->selectedNodes().nodeCount() == 0);

  auto* layer = new mdl::LayerNode{mdl::Layer{"Layer 1"}};
  document->addNodes({{document->world(), {layer}}});

  auto* group = new mdl::GroupNode{mdl::Group{"Unnamed"}};
  document->addNodes({{layer, {group}}});

  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto brushBounds = vm::bbox3d{{-32.0, -32.0, -32.0}, {+32.0, +32.0, +32.0}};

  auto* brush =
    new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
  document->addNodes({{group, {brush}}});

  const auto selectionBounds = vm::bbox3d{{-48.0, -48.0, -48.0}, {+48.0, +48.0, +48.0}};

  auto* selectionBrush =
    new mdl::BrushNode{builder.createCuboid(selectionBounds, "material") | kdl::value()};
  document->addNodes({{layer, {selectionBrush}}});

  document->selectNodes({selectionBrush});
  document->selectInside(true);

  CHECK(document->selectedNodes().nodeCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTall")
{
  using Catch::Matchers::UnorderedEquals;

  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
  auto* brushNode2 = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
  auto* brushNode3 = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};

  transformNode(
    *brushNode2,
    vm::translation_matrix(vm::vec3d{0.0, 0.0, -500.0}),
    document->worldBounds());
  transformNode(
    *brushNode3,
    vm::translation_matrix(vm::vec3d{100.0, 0.0, 0.0}),
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
      UnorderedEquals(std::vector<mdl::BrushNode*>{brushNode2}));
  }
  SECTION("x camera")
  {
    document->selectTall(vm::axis::x);

    CHECK_THAT(
      document->selectedNodes().brushes(),
      UnorderedEquals(std::vector<mdl::BrushNode*>{brushNode3}));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectInverse")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

  auto* brushNode1 =
    new mdl::BrushNode{builder.createCuboid(box, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  auto* brushNode3 = new mdl::BrushNode{
    builder.createCuboid(box.translate({2, 2, 2}), "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  auto* patchNode = createPatchNode();
  document->addNodes({{document->parentForNodes(), {patchNode}}});

  document->selectNodes({brushNode1, brushNode2});
  mdl::EntityNode* brushEnt = document->createBrushEntity(*m_brushEntityDef);

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
    Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode2, brushNode3, patchNode}));
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
  document->remove();

  auto* brushNode1 = createBrushNode();
  auto* brushNode2 = createBrushNode();

  auto* outerGroup = new mdl::GroupNode{mdl::Group{"outerGroup"}};
  auto* innerGroup = new mdl::GroupNode{mdl::Group{"innerGroup"}};

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
    Catch::UnorderedEquals(std::vector<mdl::BrushNode*>{brushNode2}));
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectSiblings")
{
  document->selectAllNodes();
  document->remove();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

  auto* brushNode1 =
    new mdl::BrushNode{builder.createCuboid(box, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  auto* brushNode3 = new mdl::BrushNode{
    builder.createCuboid(box.translate({2, 2, 2}), "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  auto* patchNode = createPatchNode();
  document->addNodes({{document->parentForNodes(), {patchNode}}});

  document->selectNodes({brushNode1, brushNode2});
  document->createBrushEntity(*m_brushEntityDef);

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
      Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode3}));

    document->selectSiblings();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(
        std::vector<mdl::Node*>{brushNode1, brushNode2, brushNode3, patchNode}));

    document->undoCommand();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode3}));
  }

  SECTION("Brush in brush entity")
  {
    document->selectNodes({brushNode1});
    REQUIRE_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode1}));

    document->selectSiblings();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode1, brushNode2}));

    document->undoCommand();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode1}));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.updateLastSelectionBounds")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "point_entity"}}}};
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
  auto* brushNode = createBrushNode();
  CHECK(brushNode->logicalBounds().center() == vm::vec3d{0, 0, 0});

  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto topFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
  REQUIRE(topFaceIndex);

  // select the top face
  document->selectBrushFaces({{brushNode, *topFaceIndex}});
  CHECK_THAT(
    document->selectedBrushFaces(),
    Catch::Equals(std::vector<mdl::BrushFaceHandle>{{brushNode, *topFaceIndex}}));

  // deselect it
  document->deselectBrushFaces({{brushNode, *topFaceIndex}});
  CHECK_THAT(
    document->selectedBrushFaces(), Catch::Equals(std::vector<mdl::BrushFaceHandle>{}));

  // select the brush
  document->selectNodes({brushNode});
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::Equals(std::vector<mdl::BrushNode*>{brushNode}));

  // translate the brush
  document->translate(vm::vec3d{10.0, 0.0, 0.0});
  CHECK(brushNode->logicalBounds().center() == vm::vec3d{10.0, 0.0, 0.0});

  // Start undoing changes

  document->undoCommand();
  CHECK(brushNode->logicalBounds().center() == vm::vec3d{0, 0, 0});
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::Equals(std::vector<mdl::BrushNode*>{brushNode}));
  CHECK_THAT(
    document->selectedBrushFaces(), Catch::Equals(std::vector<mdl::BrushFaceHandle>{}));

  document->undoCommand();
  CHECK_THAT(
    document->selectedNodes().brushes(), Catch::Equals(std::vector<mdl::BrushNode*>{}));
  CHECK_THAT(
    document->selectedBrushFaces(), Catch::Equals(std::vector<mdl::BrushFaceHandle>{}));

  document->undoCommand();
  CHECK_THAT(
    document->selectedBrushFaces(),
    Catch::Equals(std::vector<mdl::BrushFaceHandle>{{brushNode, *topFaceIndex}}));
}

} // namespace tb::ui
