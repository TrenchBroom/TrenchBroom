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
#include "PreferenceManager.h"
#include "Preferences.h"
#include "TestUtils.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/WorldNode.h"

#include "kdl/result.h"
#include "kdl/vector_utils.h"
#include "kdl/zip_iterator.h"

#include "vm/approx.h"

#include <vector>

#include "Catch2.h"

namespace tb::ui
{
namespace
{

bool checkPlanePointsIntegral(const auto* brushNode)
{
  return std::ranges::all_of(brushNode->brush().faces(), [](const auto& face) {
    return std::ranges::all_of(face.points(), pointExactlyIntegral);
  });
}

bool checkVerticesIntegral(const auto* brushNode)
{
  return std::ranges::all_of(brushNode->brush().vertices(), [](const auto& vertex) {
    return pointExactlyIntegral(vertex->position());
  });
}

bool checkBoundsIntegral(const auto* brush)
{
  return pointExactlyIntegral(brush->logicalBounds().min)
         && pointExactlyIntegral(brush->logicalBounds().max);
}

bool checkBrushIntegral(const auto* brush)
{
  return checkPlanePointsIntegral(brush) && checkVerticesIntegral(brush)
         && checkBoundsIntegral(brush);
}

void checkTransformation(
  const mdl::Node& node, const mdl::Node& original, const vm::mat4x4d& transformation)
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
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
    | kdl::value()};
  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
    | kdl::value()};

  CHECK(checkBrushIntegral(brushNode1));
  CHECK(checkBrushIntegral(brushNode2));

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectNodes({brushNode1, brushNode2});

  const auto boundsCenter = document->selectionBounds()->center();
  CHECK(boundsCenter == vm::approx{vm::vec3d{15.5, 15.5, 15.5}});

  document->flip(boundsCenter, vm::axis::x);

  CHECK(checkBrushIntegral(brushNode1));
  CHECK(checkBrushIntegral(brushNode2));

  CHECK(brushNode1->logicalBounds() == vm::bbox3d{{1.0, 0.0, 0.0}, {31.0, 31.0, 31.0}});
  CHECK(brushNode2->logicalBounds() == vm::bbox3d{{0.0, 0.0, 0.0}, {1.0, 31.0, 31.0}});
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.transform")
{
  using CreateNode = std::function<mdl::Node*(const MapDocumentTest& test)>;
  const auto createNode = GENERATE_COPY(
    CreateNode{[](const auto& test) -> mdl::Node* {
      auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
      auto* brushNode = test.createBrushNode();
      auto* patchNode = test.createPatchNode();
      auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
      groupNode->addChildren({brushNode, patchNode, entityNode});
      return groupNode;
    }},
    CreateNode{
      [](const auto&) -> mdl::Node* { return new mdl::EntityNode{mdl::Entity{}}; }},
    CreateNode{[](const auto& test) -> mdl::Node* {
      auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
      auto* brushNode = test.createBrushNode();
      auto* patchNode = test.createPatchNode();
      entityNode->addChildren({brushNode, patchNode});
      return entityNode;
    }},
    CreateNode{[](const auto& test) -> mdl::Node* { return test.createBrushNode(); }},
    CreateNode{[](const auto& test) -> mdl::Node* { return test.createPatchNode(); }});

  GIVEN("A node to transform")
  {
    auto* node = createNode(*this);
    CAPTURE(node->name());

    document->addNodes({{document->parentForNodes(), {node}}});

    const auto originalNode =
      std::unique_ptr<mdl::Node>{node->cloneRecursively(document->worldBounds())};
    const auto transformation = vm::translation_matrix(vm::vec3d{1, 2, 3});

    WHEN("The node is transformed")
    {
      document->selectNodes({node});
      document->transform("Transform Nodes", transformation);

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

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.translate")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  SECTION("linked group")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/3784

    const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

    auto* brushNode1 =
      new mdl::BrushNode{builder.createCuboid(box, "material") | kdl::value()};
    document->addNodes({{document->parentForNodes(), {brushNode1}}});
    document->selectNodes({brushNode1});

    auto* group = document->groupSelection("testGroup");
    document->selectNodes({group});

    auto* linkedGroup = document->createLinkedDuplicate();
    document->deselectAll();
    document->selectNodes({linkedGroup});
    REQUIRE_THAT(
      document->selection().nodes,
      Catch::UnorderedEquals(std::vector<mdl::Node*>{linkedGroup}));

    auto* linkedBrushNode = dynamic_cast<mdl::BrushNode*>(linkedGroup->children().at(0));
    REQUIRE(linkedBrushNode != nullptr);

    const auto setPref = TemporarilySetPref{Preferences::AlignmentLock, false};

    const auto delta = vm::vec3d{0.125, 0, 0};
    REQUIRE(document->translate(delta));

    auto getUVCoords =
      [](auto* brushNode, const vm::vec3d& normal) -> std::vector<vm::vec2f> {
      const mdl::BrushFace& face =
        brushNode->brush().face(*brushNode->brush().findFace(normal));
      return kdl::vec_transform(
        face.vertexPositions(), [&](auto x) { return face.uvCoords(x); });
    };

    // Brushes in linked groups should have alignment lock forced on
    CHECK(uvListsEqual(
      getUVCoords(brushNode1, vm::vec3d{0, 0, 1}),
      getUVCoords(linkedBrushNode, vm::vec3d{0, 0, 1})));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.rotate")
{
  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  SECTION("objects")
  {
    auto* brushNode1 = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
      | kdl::value()};
    auto* brushNode2 = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
      | kdl::value()};

    REQUIRE(checkBrushIntegral(brushNode1));
    REQUIRE(checkBrushIntegral(brushNode2));

    SECTION("two brushes")
    {
      document->addNodes({{document->parentForNodes(), {brushNode1, brushNode2}}});
      document->selectNodes({brushNode1, brushNode2});

      const auto boundsCenter = document->selectionBounds()->center();
      CHECK(boundsCenter == vm::vec3d{15.5, 15.5, 15.5});

      // 90 degrees CCW about the Z axis through the center of the selection
      document->rotate(boundsCenter, vm::vec3d{0, 0, 1}, vm::to_radians(90.0));

      CHECK(checkBrushIntegral(brushNode1));
      CHECK(checkBrushIntegral(brushNode2));

      const auto brush1ExpectedBounds = vm::bbox3d{{0.0, 0.0, 0.0}, {31.0, 30.0, 31.0}};
      const auto brush2ExpectedBounds = vm::bbox3d{{0.0, 30.0, 0.0}, {31.0, 31.0, 31.0}};

      // these should be exactly integral
      CHECK(brushNode1->logicalBounds() == brush1ExpectedBounds);
      CHECK(brushNode2->logicalBounds() == brush2ExpectedBounds);
    }

    SECTION("brush entity")
    {
      auto* entityNode = new mdl::EntityNode{mdl::Entity{{
        {"classname", "func_door"},
        {"angle", "45"},
      }}};

      document->addNodes({{document->parentForNodes(), {entityNode}}});
      document->addNodes({{entityNode, {brushNode1, brushNode2}}});

      REQUIRE(*entityNode->entity().property("angle") == "45");

      SECTION("Rotating some brushes, but not all")
      {
        document->selectNodes({brushNode1});
        document->rotate(
          document->selectionBounds()->center(),
          vm::vec3d{0, 0, 1},
          vm::to_radians(90.0));

        CHECK(*entityNode->entity().property("angle") == "45");
      }

      SECTION("Rotating all brushes")
      {
        document->selectNodes({brushNode1, brushNode2});
        document->rotate(
          document->selectionBounds()->center(),
          vm::vec3d{0, 0, 1},
          vm::to_radians(90.0));

        CHECK(*entityNode->entity().property("angle") == "135");
      }

      SECTION("Rotating grouped brush entity")
      {
        document->selectNodes({entityNode});
        auto* groupNode = document->groupSelection("some_name");

        document->deselectAll();
        document->selectNodes({groupNode});
        document->rotate(
          document->selectionBounds()->center(),
          vm::vec3d{0, 0, 1},
          vm::to_radians(90.0));

        CHECK(*entityNode->entity().property("angle") == "135");
      }
    }
  }

  SECTION("vertices")
  {
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(
        vm::bbox3d{{-32.0, -32.0, -32.0}, {32.0, 32.0, 32.0}}, "material")
      | kdl::value()};

    document->addNodes({{document->parentForNodes(), {brushNode}}});
    document->selectNodes({brushNode});

    auto& vertexHandles = document->vertexHandles();
    vertexHandles.addHandles(brushNode);
    vertexHandles.select(std::vector<vm::vec3d>{
      {-32, -32, 32},
      {-32, 32, 32},
      {32, -32, 32},
      {32, 32, 32},
    });

    document->rotate({0, 0, 0}, {0, 0, 1}, vm::to_radians(45.0));

    const auto& brush = brushNode->brush();
    const auto e = vm::constants<double>::almost_zero();
    const auto x = 45.254833995939407;

    CHECK(brush.hasVertex({-x, 0, +32}, e));
    CHECK(brush.hasVertex({+x, 0, +32}, e));
    CHECK(brush.hasVertex({0, -x, +32}, e));
    CHECK(brush.hasVertex({0, +x, +32}, e));

    CHECK(brush.hasVertex({-32, -32, -32}, e));
    CHECK(brush.hasVertex({-32, +32, -32}, e));
    CHECK(brush.hasVertex({+32, -32, -32}, e));
    CHECK(brush.hasVertex({+32, +32, -32}, e));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.shear")
{
  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  SECTION("cube")
  {
    const auto initialBBox = vm::bbox3d{{100, 100, 100}, {200, 200, 200}};

    auto* brushNode =
      new mdl::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

    document->addNodes({{document->parentForNodes(), {brushNode}}});
    document->selectNodes({brushNode});

    CHECK_THAT(
      brushNode->brush().vertexPositions(),
      Catch::UnorderedEquals(std::vector<vm::vec3d>{
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
    CHECK(document->shear(initialBBox, vm::vec3d{0, -1, 0}, vm::vec3d{50, 0, 0}));

    CHECK_THAT(
      brushNode->brush().vertexPositions(),
      Catch::UnorderedEquals(std::vector<vm::vec3d>{
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

  SECTION("pillar")
  {
    const auto initialBBox = vm::bbox3d{{0, 0, 0}, {100, 100, 400}};

    auto* brushNode =
      new mdl::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

    document->addNodes({{document->parentForNodes(), {brushNode}}});
    document->selectNodes({brushNode});

    CHECK_THAT(
      brushNode->brush().vertexPositions(),
      Catch::UnorderedEquals(std::vector<vm::vec3d>{
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
    CHECK(document->shear(initialBBox, vm::vec3d{0, 0, 1}, vm::vec3d{50, 0, 0}));

    CHECK_THAT(
      brushNode->brush().vertexPositions(),
      Catch::UnorderedEquals(std::vector<vm::vec3d>{
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
}

TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scale")
{
  const auto initialBBox = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};
  const auto doubleBBox = vm::bbox3d{2.0 * initialBBox.min, 2.0 * initialBBox.max};
  const auto invalidBBox = vm::bbox3d{{0, -100, -100}, {0, 100, 100}};

  auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode =
    new mdl::BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};
  const auto& brush = brushNode->brush();

  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  REQUIRE(brushNode->logicalBounds().size() == vm::vec3d{200, 200, 200});
  REQUIRE(
    brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).boundary()
    == vm::plane3d{100.0, vm::vec3d{0, 0, 1}});

  SECTION("single brush")
  {
    // attempting an invalid scale has no effect
    CHECK_FALSE(document->scale(initialBBox, invalidBBox));
    CHECK(brushNode->logicalBounds().size() == vm::vec3d{200, 200, 200});
    CHECK(
      brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).boundary()
      == vm::plane3d{100.0, vm::vec3d{0, 0, 1}});

    CHECK(document->scale(initialBBox, doubleBBox));
    CHECK(brushNode->logicalBounds().size() == vm::vec3d{400, 400, 400});
    CHECK(
      brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).boundary()
      == vm::plane3d{200.0, vm::vec3d{0, 0, 1}});
  }

  SECTION("in group")
  {
    [[maybe_unused]] auto* group = document->groupSelection("my group");

    // attempting an invalid scale has no effect
    CHECK_FALSE(document->scale(initialBBox, invalidBBox));
    CHECK(brushNode->logicalBounds().size() == vm::vec3d{200, 200, 200});

    CHECK(document->scale(initialBBox, doubleBBox));
    CHECK(brushNode->logicalBounds().size() == vm::vec3d{400, 400, 400});
  }

  SECTION("with off center origin")
  {
    const auto origin = vm::vec3d{50, 0, 0};
    CHECK(document->scale(origin, vm::vec3d{2.0, 1.0, 1.0}));
    CHECK(brushNode->logicalBounds() == vm::bbox3d{{-250, -100, -100}, {150, 100, 100}});
  }
}

} // namespace tb::ui
