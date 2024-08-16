/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "MapDocumentTest.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/PatchNode.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "TestUtils.h"

#include "kdl/result.h"
#include "kdl/vector_utils.h"
#include "kdl/zip_iterator.h"

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/mat_io.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <vector>

#include "Catch2.h"

namespace TrenchBroom::View
{
namespace
{

void checkPlanePointsIntegral(const auto* brushNode)
{
  for (const auto& face : brushNode->brush().faces())
  {
    for (size_t i = 0; i < 3; i++)
    {
      const auto point = face.points()[i];
      CHECK(pointExactlyIntegral(point));
    }
  }
}

void checkVerticesIntegral(const auto* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* vertex : brush.vertices())
  {
    CHECK(pointExactlyIntegral(vertex->position()));
  }
}

void checkBoundsIntegral(const auto* brush)
{
  CHECK(pointExactlyIntegral(brush->logicalBounds().min));
  CHECK(pointExactlyIntegral(brush->logicalBounds().max));
}

void checkBrushIntegral(const auto* brush)
{
  checkPlanePointsIntegral(brush);
  checkVerticesIntegral(brush);
  checkBoundsIntegral(brush);
}

void checkTransformation(
  const Model::Node& node, const Model::Node& original, const vm::mat4x4d& transformation)
{
  CHECK(node.physicalBounds() == original.physicalBounds().transform(transformation));

  REQUIRE(node.childCount() == original.childCount());
  for (const auto& [nodeChild, originalChild] :
       kdl::make_zip_range(node.children(), original.children()))
  {
    checkTransformation(*nodeChild, *originalChild, transformation);
  }
}

} // namespace

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.flip")
{
  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
    | kdl::value()};
  auto* brushNode2 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
    | kdl::value()};

  checkBrushIntegral(brushNode1);
  checkBrushIntegral(brushNode2);

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectNodes({brushNode1, brushNode2});

  const auto boundsCenter = document->selectionBounds().center();
  CHECK(boundsCenter == vm::approx{vm::vec3{15.5, 15.5, 15.5}});

  document->flipObjects(boundsCenter, vm::axis::x);

  checkBrushIntegral(brushNode1);
  checkBrushIntegral(brushNode2);

  CHECK(brushNode1->logicalBounds() == vm::bbox3{{1.0, 0.0, 0.0}, {31.0, 31.0, 31.0}});
  CHECK(brushNode2->logicalBounds() == vm::bbox3{{0.0, 0.0, 0.0}, {1.0, 31.0, 31.0}});
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.transformObjects")
{
  using CreateNode = std::function<Model::Node*(const MapDocumentTest& test)>;
  const auto createNode = GENERATE_COPY(
    CreateNode{[](const auto& test) -> Model::Node* {
      auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
      auto* brushNode = test.createBrushNode();
      auto* patchNode = test.createPatchNode();
      auto* entityNode = new Model::EntityNode{Model::Entity{}};
      groupNode->addChildren({brushNode, patchNode, entityNode});
      return groupNode;
    }},
    CreateNode{
      [](const auto&) -> Model::Node* { return new Model::EntityNode{Model::Entity{}}; }},
    CreateNode{[](const auto& test) -> Model::Node* {
      auto* entityNode = new Model::EntityNode{Model::Entity{}};
      auto* brushNode = test.createBrushNode();
      auto* patchNode = test.createPatchNode();
      entityNode->addChildren({brushNode, patchNode});
      return entityNode;
    }},
    CreateNode{[](const auto& test) -> Model::Node* { return test.createBrushNode(); }},
    CreateNode{[](const auto& test) -> Model::Node* { return test.createPatchNode(); }});

  GIVEN("A node to transform")
  {
    auto* node = createNode(*this);
    CAPTURE(node->name());

    document->addNodes({{document->parentForNodes(), {node}}});

    const auto originalNode = std::unique_ptr<Model::Node>{
      node->cloneRecursively(document->worldBounds(), Model::SetLinkId::generate)};
    const auto transformation = vm::translation_matrix(vm::vec3d{1, 2, 3});

    WHEN("The node is transformed")
    {
      document->selectNodes({node});
      document->transformObjects("Transform Nodes", transformation);

      THEN("The transformation was applied to the node and its children")
      {
        checkTransformation(*node, *originalNode.get(), transformation);
      }

      AND_WHEN("The transformation is undone")
      {
        document->undoCommand();

        THEN("The node is back in its original state")
        {
          checkTransformation(*node, *originalNode.get(), vm::mat4x4d::identity());
        }
      }
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.rotate")
{
  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
    | kdl::value()};
  auto* brushNode2 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
    | kdl::value()};

  checkBrushIntegral(brushNode1);
  checkBrushIntegral(brushNode2);

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectNodes({brushNode1, brushNode2});

  const auto boundsCenter = document->selectionBounds().center();
  CHECK(boundsCenter == vm::vec3{15.5, 15.5, 15.5});

  // 90 degrees CCW about the Z axis through the center of the selection
  document->rotateObjects(boundsCenter, vm::vec3::pos_z(), vm::to_radians(90.0));

  checkBrushIntegral(brushNode1);
  checkBrushIntegral(brushNode2);

  const auto brush1ExpectedBounds = vm::bbox3{{0.0, 0.0, 0.0}, {31.0, 30.0, 31.0}};
  const auto brush2ExpectedBounds = vm::bbox3{{0.0, 30.0, 0.0}, {31.0, 31.0, 31.0}};

  // these should be exactly integral
  CHECK(brushNode1->logicalBounds() == brush1ExpectedBounds);
  CHECK(brushNode2->logicalBounds() == brush2ExpectedBounds);
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.rotateBrushEntity")
{
  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
    | kdl::value()};
  auto* brushNode2 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
    | kdl::value()};

  auto* entityNode = new Model::EntityNode{Model::Entity{{
    {"classname", "func_door"},
    {"angle", "45"},
  }}};

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->addNodes({{entityNode, {brushNode1, brushNode2}}});

  REQUIRE(*entityNode->entity().property("angle") == "45");

  SECTION("Rotating some brushes, but not all")
  {
    document->selectNodes({brushNode1});
    document->rotateObjects(
      document->selectionBounds().center(), vm::vec3::pos_z(), vm::to_radians(90.0));

    CHECK(*entityNode->entity().property("angle") == "45");
  }

  SECTION("Rotating all brushes")
  {
    document->selectNodes({brushNode1, brushNode2});
    document->rotateObjects(
      document->selectionBounds().center(), vm::vec3::pos_z(), vm::to_radians(90.0));

    CHECK(*entityNode->entity().property("angle") == "135");
  }

  SECTION("Rotating grouped brush entity")
  {
    document->selectNodes({entityNode});
    auto* groupNode = document->groupSelection("some_name");

    document->deselectAll();
    document->selectNodes({groupNode});
    document->rotateObjects(
      document->selectionBounds().center(), vm::vec3::pos_z(), vm::to_radians(90.0));

    CHECK(*entityNode->entity().property("angle") == "135");
  }
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.shearCube")
{
  const auto initialBBox = vm::bbox3{{100, 100, 100}, {200, 200, 200}};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode =
    new Model::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  CHECK_THAT(
    brushNode->brush().vertexPositions(),
    Catch::UnorderedEquals(std::vector<vm::vec3>{
      // bottom face
      {100, 100, 100},
      {200, 100, 100},
      {200, 200, 100},
      {100, 200, 100},
      // top face
      {100, 100, 200},
      {200, 100, 200},
      {200, 200, 200},
      {100, 200, 200},
    }));

  // Shear the -Y face by (50, 0, 0). That means the verts with Y=100 will get sheared.
  CHECK(document->shearObjects(initialBBox, vm::vec3::neg_y(), vm::vec3{50, 0, 0}));

  CHECK_THAT(
    brushNode->brush().vertexPositions(),
    Catch::UnorderedEquals(std::vector<vm::vec3>{
      // bottom face
      {150, 100, 100},
      {250, 100, 100},
      {200, 200, 100},
      {100, 200, 100},
      // top face
      {150, 100, 200},
      {250, 100, 200},
      {200, 200, 200},
      {100, 200, 200},
    }));
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.shearPillar")
{
  const auto initialBBox = vm::bbox3{{0, 0, 0}, {100, 100, 400}};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode =
    new Model::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  CHECK_THAT(
    brushNode->brush().vertexPositions(),
    Catch::UnorderedEquals(std::vector<vm::vec3>{
      // bottom face
      {0, 0, 0},
      {100, 0, 0},
      {100, 100, 0},
      {0, 100, 0},
      // top face
      {0, 0, 400},
      {100, 0, 400},
      {100, 100, 400},
      {0, 100, 400},
    }));

  // Shear the +Z face by (50, 0, 0). That means the verts with Z=400 will get sheared.
  CHECK(document->shearObjects(initialBBox, vm::vec3::pos_z(), vm::vec3{50, 0, 0}));

  CHECK_THAT(
    brushNode->brush().vertexPositions(),
    Catch::UnorderedEquals(std::vector<vm::vec3>{
      // bottom face
      {0, 0, 0},
      {100, 0, 0},
      {100, 100, 0},
      {0, 100, 0},
      // top face
      {50, 0, 400},
      {150, 0, 400},
      {150, 100, 400},
      {50, 100, 400},
    }));
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scaleObjects")
{
  const auto initialBBox = vm::bbox3{{-100, -100, -100}, {100, 100, 100}};
  const auto doubleBBox = vm::bbox3{2.0 * initialBBox.min, 2.0 * initialBBox.max};
  const auto invalidBBox = vm::bbox3{{0, -100, -100}, {0, 100, 100}};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode =
    new Model::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};
  const auto& brush = brushNode->brush();

  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  CHECK(brushNode->logicalBounds().size() == vm::vec3{200, 200, 200});
  CHECK(
    brush.face(*brush.findFace(vm::vec3::pos_z())).boundary()
    == vm::plane3{100.0, vm::vec3::pos_z()});

  // attempting an invalid scale has no effect
  CHECK_FALSE(document->scaleObjects(initialBBox, invalidBBox));
  CHECK(brushNode->logicalBounds().size() == vm::vec3{200, 200, 200});
  CHECK(
    brush.face(*brush.findFace(vm::vec3::pos_z())).boundary()
    == vm::plane3{100.0, vm::vec3::pos_z()});

  CHECK(document->scaleObjects(initialBBox, doubleBBox));
  CHECK(brushNode->logicalBounds().size() == vm::vec3{400, 400, 400});
  CHECK(
    brush.face(*brush.findFace(vm::vec3::pos_z())).boundary()
    == vm::plane3{200.0, vm::vec3::pos_z()});
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scaleObjectsInGroup")
{
  const auto initialBBox = vm::bbox3{{-100, -100, -100}, {100, 100, 100}};
  const auto doubleBBox = vm::bbox3{2.0 * initialBBox.min, 2.0 * initialBBox.max};
  const auto invalidBBox = vm::bbox3{{0, -100, -100}, {0, 100, 100}};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode =
    new Model::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});
  [[maybe_unused]] auto* group = document->groupSelection("my group");

  // attempting an invalid scale has no effect
  CHECK_FALSE(document->scaleObjects(initialBBox, invalidBBox));
  CHECK(brushNode->logicalBounds().size() == vm::vec3{200, 200, 200});

  CHECK(document->scaleObjects(initialBBox, doubleBBox));
  CHECK(brushNode->logicalBounds().size() == vm::vec3{400, 400, 400});
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scaleObjectsWithCenter")
{
  const auto initialBBox = vm::bbox3{{0, 0, 0}, {100, 100, 400}};
  const auto expectedBBox = vm::bbox3{{-50, 0, 0}, {150, 100, 400}};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode =
    new Model::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  const auto boundsCenter = initialBBox.center();
  CHECK(document->scaleObjects(boundsCenter, vm::vec3{2.0, 1.0, 1.0}));
  CHECK(brushNode->logicalBounds() == expectedBBox);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3784
TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.translateLinkedGroup")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto box = vm::bbox3{{0, 0, 0}, {64, 64, 64}};

  auto* brushNode1 =
    new Model::BrushNode{builder.createCuboid(box, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  auto* group = document->groupSelection("testGroup");
  document->selectNodes({group});

  auto* linkedGroup = document->createLinkedDuplicate();
  document->deselectAll();
  document->selectNodes({linkedGroup});
  REQUIRE_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{linkedGroup}));

  auto* linkedBrushNode = dynamic_cast<Model::BrushNode*>(linkedGroup->children().at(0));
  REQUIRE(linkedBrushNode != nullptr);

  setPref(Preferences::AlignmentLock, false);

  const auto delta = vm::vec3{0.125, 0, 0};
  REQUIRE(document->translateObjects(delta));

  auto getUVCoords =
    [](auto* brushNode, const vm::vec3& normal) -> std::vector<vm::vec2f> {
    const Model::BrushFace& face =
      brushNode->brush().face(*brushNode->brush().findFace(normal));
    return kdl::vec_transform(
      face.vertexPositions(), [&](auto x) { return face.uvCoords(x); });
  };

  // Brushes in linked groups should have alignment lock forced on
  CHECK(uvListsEqual(
    getUVCoords(brushNode1, vm::vec3::pos_z()),
    getUVCoords(linkedBrushNode, vm::vec3::pos_z())));

  PreferenceManager::instance().resetToDefault(Preferences::AlignmentLock);
}

} // namespace TrenchBroom::View
