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
#include "PreferenceManager.h"
#include "Preferences.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Grid.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ParallelUVCoordSystem.h"
#include "mdl/VertexHandleManager.h"
#include "mdl/WorldNode.h"

#include "kdl/zip_iterator.h"

#include "vm/approx.h"

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::mdl
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
  const Node& node, const Node& original, const vm::mat4x4d& transformation)
{
  CHECK(node.physicalBounds() == original.physicalBounds().transform(transformation));

  REQUIRE(node.childCount() == original.childCount());
  for (const auto& [nodeChild, originalChild] :
       kdl::make_zip_range(node.children(), original.children()))
  {
    checkTransformation(*nodeChild, *originalChild, transformation);
  }
}

bool hasEmptyName(const std::vector<std::string>& names)
{
  return std::ranges::any_of(names, [](const auto& s) { return s.empty(); });
}

} // namespace

TEST_CASE("Map_Geometry")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  SECTION("transformSelection")
  {
    using CreateNode = std::function<Node*(const Map&)>;
    const auto createNode = GENERATE_COPY(
      CreateNode{[](const auto& m) -> Node* {
        auto* groupNode = new GroupNode{Group{"group"}};
        auto* brushNode = createBrushNode(m);
        auto* patchNode = createPatchNode();
        auto* entityNode = new EntityNode{Entity{}};
        groupNode->addChildren({brushNode, patchNode, entityNode});
        return groupNode;
      }},
      CreateNode{[](const auto&) -> Node* { return new EntityNode{Entity{}}; }},
      CreateNode{[](const auto& m) -> Node* {
        auto* entityNode = new EntityNode{Entity{}};
        auto* brushNode = createBrushNode(m);
        auto* patchNode = createPatchNode();
        entityNode->addChildren({brushNode, patchNode});
        return entityNode;
      }},
      CreateNode{[](const auto& m) -> Node* { return createBrushNode(m); }},
      CreateNode{[](const auto&) -> Node* { return createPatchNode(); }});

    GIVEN("A node to transform")
    {
      auto* node = createNode(map);
      CAPTURE(node->name());

      addNodes(map, {{parentForNodes(map), {node}}});

      const auto originalNode =
        std::unique_ptr<Node>{node->cloneRecursively(map.worldBounds())};
      const auto transformation = vm::translation_matrix(vm::vec3d{1, 2, 3});

      WHEN("The node is transformed")
      {
        selectNodes(map, {node});
        transformSelection(map, "Transform Nodes", transformation);

        THEN("The transformation was applied to the node and its children")
        {
          checkTransformation(*node, *originalNode.get(), transformation);
        }

        AND_WHEN("The transformation is undone")
        {
          map.undoCommand();

          THEN("The node is back in its original state")
          {
            checkTransformation(*node, *originalNode.get(), vm::mat4x4d::identity());
          }
        }
      }
    }
  }

  SECTION("translateSelection")
  {
    SECTION("Transform a group containing a brush entity")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/1715

      auto* brushNode1 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      reparentNodes(map, {{entityNode, {brushNode1}}});

      selectNodes(map, {brushNode1});

      auto* groupNode = groupSelectedNodes(map, "test");
      CHECK(groupNode->selected());

      CHECK(translateSelection(map, vm::vec3d{16, 0, 0}));
      CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));

      map.undoCommand();

      CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));
    }

    SECTION("Linked group")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3784

      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

      auto* brushNode1 =
        new BrushNode{builder.createCuboid(box, "material") | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      selectNodes(map, {brushNode1});

      auto* group = groupSelectedNodes(map, "testGroup");
      selectNodes(map, {group});

      auto* linkedGroup = createLinkedDuplicate(map);
      deselectAll(map);
      selectNodes(map, {linkedGroup});
      REQUIRE_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(std::vector<Node*>{linkedGroup}));

      auto* linkedBrushNode = dynamic_cast<BrushNode*>(linkedGroup->children().at(0));
      REQUIRE(linkedBrushNode != nullptr);

      const auto setPref = TemporarilySetPref{Preferences::AlignmentLock, false};

      const auto delta = vm::vec3d{0.125, 0, 0};
      REQUIRE(translateSelection(map, delta));

      auto getUVCoords =
        [](auto* brushNode, const vm::vec3d& normal) -> std::vector<vm::vec2f> {
        const BrushFace& face =
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

  SECTION("rotateSelection")
  {
    const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

    SECTION("objects")
    {
      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
        | kdl::value()};
      auto* brushNode2 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
        | kdl::value()};

      REQUIRE(checkBrushIntegral(brushNode1));
      REQUIRE(checkBrushIntegral(brushNode2));

      SECTION("two brushes")
      {
        addNodes(map, {{parentForNodes(map), {brushNode1, brushNode2}}});
        selectNodes(map, {brushNode1, brushNode2});

        const auto boundsCenter = map.selectionBounds()->center();
        CHECK(boundsCenter == vm::vec3d{15.5, 15.5, 15.5});

        // 90 degrees CCW about the Z axis through the center of the selection
        rotateSelection(map, boundsCenter, vm::vec3d{0, 0, 1}, vm::to_radians(90.0));

        CHECK(checkBrushIntegral(brushNode1));
        CHECK(checkBrushIntegral(brushNode2));

        const auto brush1ExpectedBounds = vm::bbox3d{{0.0, 0.0, 0.0}, {31.0, 30.0, 31.0}};
        const auto brush2ExpectedBounds =
          vm::bbox3d{{0.0, 30.0, 0.0}, {31.0, 31.0, 31.0}};

        // these should be exactly integral
        CHECK(brushNode1->logicalBounds() == brush1ExpectedBounds);
        CHECK(brushNode2->logicalBounds() == brush2ExpectedBounds);
      }

      SECTION("brush entity")
      {
        auto* entityNode = new EntityNode{Entity{{
          {"classname", "func_door"},
          {"angle", "45"},
        }}};

        addNodes(map, {{parentForNodes(map), {entityNode}}});
        addNodes(map, {{entityNode, {brushNode1, brushNode2}}});

        REQUIRE(*entityNode->entity().property("angle") == "45");

        SECTION("Rotating some brushes, but not all")
        {
          selectNodes(map, {brushNode1});
          rotateSelection(
            map,
            map.selectionBounds()->center(),
            vm::vec3d{0, 0, 1},
            vm::to_radians(90.0));

          CHECK(*entityNode->entity().property("angle") == "45");
        }

        SECTION("Rotating all brushes")
        {
          selectNodes(map, {brushNode1, brushNode2});
          rotateSelection(
            map,
            map.selectionBounds()->center(),
            vm::vec3d{0, 0, 1},
            vm::to_radians(90.0));

          CHECK(*entityNode->entity().property("angle") == "135");
        }

        SECTION("Rotating grouped brush entity")
        {
          selectNodes(map, {entityNode});
          auto* groupNode = groupSelectedNodes(map, "some_name");

          deselectAll(map);
          selectNodes(map, {groupNode});
          rotateSelection(
            map,
            map.selectionBounds()->center(),
            vm::vec3d{0, 0, 1},
            vm::to_radians(90.0));

          CHECK(*entityNode->entity().property("angle") == "135");
        }
      }
    }

    SECTION("vertices")
    {
      auto* brushNode = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{{-32.0, -32.0, -32.0}, {32.0, 32.0, 32.0}}, "material")
        | kdl::value()};

      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto& vertexHandles = map.vertexHandles();
      vertexHandles.addHandles(brushNode);
      vertexHandles.select(std::vector<vm::vec3d>{
        {-32, -32, 32},
        {-32, 32, 32},
        {32, -32, 32},
        {32, 32, 32},
      });

      rotateSelection(map, {0, 0, 0}, {0, 0, 1}, vm::to_radians(45.0));

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

    SECTION("Rotating a group containing a brush entity")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/1754

      auto* brushNode1 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      reparentNodes(map, {{entityNode, {brushNode1}}});

      selectNodes(map, {brushNode1});

      auto* groupNode = groupSelectedNodes(map, "test");
      CHECK(groupNode->selected());

      CHECK_FALSE(entityNode->entity().hasProperty("origin"));
      CHECK(rotateSelection(map, vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, 10.0));
      CHECK_FALSE(entityNode->entity().hasProperty("origin"));

      map.undoCommand();

      CHECK_FALSE(entityNode->entity().hasProperty("origin"));
    }
  }

  SECTION("scaleSelection")
  {
    const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

    const auto initialBBox = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};
    const auto doubleBBox = vm::bbox3d{2.0 * initialBBox.min, 2.0 * initialBBox.max};
    const auto invalidBBox = vm::bbox3d{{0, -100, -100}, {0, 100, 100}};

    auto* brushNode =
      new BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};
    const auto& brush = brushNode->brush();

    addNodes(map, {{parentForNodes(map), {brushNode}}});
    selectNodes(map, {brushNode});

    REQUIRE(brushNode->logicalBounds().size() == vm::vec3d{200, 200, 200});
    REQUIRE(
      brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).boundary()
      == vm::plane3d{100.0, vm::vec3d{0, 0, 1}});

    SECTION("single brush")
    {
      // attempting an invalid scale has no effect
      CHECK_FALSE(scaleSelection(map, initialBBox, invalidBBox));
      CHECK(brushNode->logicalBounds().size() == vm::vec3d{200, 200, 200});
      CHECK(
        brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).boundary()
        == vm::plane3d{100.0, vm::vec3d{0, 0, 1}});

      CHECK(scaleSelection(map, initialBBox, doubleBBox));
      CHECK(brushNode->logicalBounds().size() == vm::vec3d{400, 400, 400});
      CHECK(
        brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).boundary()
        == vm::plane3d{200.0, vm::vec3d{0, 0, 1}});
    }

    SECTION("in group")
    {
      [[maybe_unused]] auto* group = groupSelectedNodes(map, "my group");

      // attempting an invalid scale has no effect
      CHECK_FALSE(scaleSelection(map, initialBBox, invalidBBox));
      CHECK(brushNode->logicalBounds().size() == vm::vec3d{200, 200, 200});

      CHECK(scaleSelection(map, initialBBox, doubleBBox));
      CHECK(brushNode->logicalBounds().size() == vm::vec3d{400, 400, 400});
    }

    SECTION("with off center origin")
    {
      const auto origin = vm::vec3d{50, 0, 0};
      CHECK(scaleSelection(map, origin, vm::vec3d{2.0, 1.0, 1.0}));
      CHECK(
        brushNode->logicalBounds() == vm::bbox3d{{-250, -100, -100}, {150, 100, 100}});
    }
  }

  SECTION("shearSelection")
  {
    const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

    SECTION("cube")
    {
      const auto initialBBox = vm::bbox3d{{100, 100, 100}, {200, 200, 200}};

      auto* brushNode =
        new BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        Catch::Matchers::UnorderedEquals(std::vector<vm::vec3d>{
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

      // Shear the -Y face by (50, 0, 0). That means the verts with Y=100 will get
      // sheared.
      CHECK(shearSelection(map, initialBBox, vm::vec3d{0, -1, 0}, vm::vec3d{50, 0, 0}));

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        Catch::Matchers::UnorderedEquals(std::vector<vm::vec3d>{
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
        new BrushNode{builder.createCuboid(initialBBox, "material") | kdl::value()};

      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        Catch::Matchers::UnorderedEquals(std::vector<vm::vec3d>{
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

      // Shear the +Z face by (50, 0, 0). That means the verts with Z=400 will get
      // sheared.
      CHECK(shearSelection(map, initialBBox, vm::vec3d{0, 0, 1}, vm::vec3d{50, 0, 0}));

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        Catch::Matchers::UnorderedEquals(std::vector<vm::vec3d>{
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

  SECTION("flipSelection")
  {
    const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

    auto* brushNode1 = new BrushNode{
      builder.createCuboid(vm::bbox3d{{0.0, 0.0, 0.0}, {30.0, 31.0, 31.0}}, "material")
      | kdl::value()};
    auto* brushNode2 = new BrushNode{
      builder.createCuboid(vm::bbox3d{{30.0, 0.0, 0.0}, {31.0, 31.0, 31.0}}, "material")
      | kdl::value()};

    CHECK(checkBrushIntegral(brushNode1));
    CHECK(checkBrushIntegral(brushNode2));

    addNodes(map, {{parentForNodes(map), {brushNode1}}});
    addNodes(map, {{parentForNodes(map), {brushNode2}}});

    selectNodes(map, {brushNode1, brushNode2});

    const auto boundsCenter = map.selectionBounds()->center();
    CHECK(boundsCenter == vm::approx{vm::vec3d{15.5, 15.5, 15.5}});

    flipSelection(map, boundsCenter, vm::axis::x);

    CHECK(checkBrushIntegral(brushNode1));
    CHECK(checkBrushIntegral(brushNode2));

    CHECK(brushNode1->logicalBounds() == vm::bbox3d{{1.0, 0.0, 0.0}, {31.0, 31.0, 31.0}});
    CHECK(brushNode2->logicalBounds() == vm::bbox3d{{0.0, 0.0, 0.0}, {1.0, 31.0, 31.0}});
  }

  SECTION("snapVertices")
  {
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

      SECTION("Can't snap to grid with both groups selected")
      {
        selectNodes(map, {groupNode, linkedGroupNode});

        CHECK(
          transformSelection(map, "", vm::translation_matrix(vm::vec3d{0.5, 0.5, 0.0})));

        // This could generate conflicts, because what snaps one group could misalign
        // another group in the link set. So, just reject the change.
        CHECK(!snapVertices(map, 16.0));
      }
    }
  }

  SECTION("snapVertices")
  {
    SECTION("Don't crash when snapping vertices")
    {
      // see https://github.com/TrenchBroom/TrenchBroom/issues/2244
      selectAllNodes(map);
      removeSelectedNodes(map);

      const auto brush = R"(
// Game: Quake
// Format: Standard
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -96 -0 116 ) ( -96 -64 116 ) ( -96 -64 172 ) karch1 -0 -0 -0 1 1
( -96 -0 172 ) ( -96 -64 172 ) ( -116 -64 144 ) karch1 -84 176 -0 1 1
( -116 -64 144 ) ( -96 -64 116 ) ( -96 -0 116 ) karch_sup6 2 -64 -0 1 1
( -96 -0 116 ) ( -96 -0 172 ) ( -116 -0 144 ) karch1 -0 -0 -0 1 1
( -96 -64 172 ) ( -96 -64 116 ) ( -116 -64 144 ) karch1 -0 -0 -0 1 1
}
})";
      paste(map, brush);
      selectAllNodes(map);

      CHECK(map.selection().brushes.size() == 1u);
      CHECK_NOTHROW(snapVertices(map, map.grid().actualSize()));
    }
  }

  SECTION("csgConvexMerge")
  {
    SECTION("Merge two brushes")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
        | kdl::value()};
      auto* brushNode2 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{entityNode, {brushNode1}}});
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      CHECK(entityNode->children().size() == 1u);

      selectNodes(map, {brushNode1, brushNode2});
      CHECK(csgConvexMerge(map));
      CHECK(entityNode->children().size() == 1u);

      auto* brushNode3 = entityNode->children().front();
      CHECK(brushNode3->logicalBounds() == vm::bbox3d{{0, 0, 0}, {64, 64, 64}});
    }

    SECTION("Merge two faces")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
        | kdl::value()};
      auto* brushNode2 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{entityNode, {brushNode1}}});
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      CHECK(entityNode->children().size() == 1u);

      const auto faceIndex = 0u;
      const auto& face1 = brushNode1->brush().face(faceIndex);
      const auto& face2 = brushNode2->brush().face(faceIndex);

      selectBrushFaces(map, {{brushNode1, faceIndex}, {brushNode2, faceIndex}});
      CHECK(csgConvexMerge(map));
      CHECK(
        entityNode->children().size()
        == 2u); // added to the parent of the first brush, original brush is not deleted

      auto* brushNode3 = entityNode->children().back();

      // check our assumption about the order of the entities' children
      assert(brushNode3 != brushNode1);
      assert(brushNode3 != brushNode2);

      const auto face1Verts = face1.vertexPositions();
      const auto face2Verts = face2.vertexPositions();

      const auto bounds = vm::merge(
        vm::bbox3d::merge_all(std::begin(face1Verts), std::end(face1Verts)),
        vm::bbox3d::merge_all(std::begin(face2Verts), std::end(face2Verts)));

      CHECK(brushNode3->logicalBounds() == bounds);
    }

    SECTION("Texture alignment")
    {
      fixture.create({.mapFormat = MapFormat::Valve});

      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      auto texAlignment = ParallelUVCoordSystem{{1, 0, 0}, {0, 1, 0}};
      auto texAlignmentSnapshot = texAlignment.takeSnapshot();

      auto brush1 = builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
                    | kdl::value();
      brush1.face(*brush1.findFace(vm::vec3d{0, 0, 1}))
        .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

      auto brush2 = builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
                    | kdl::value();
      brush2.face(*brush2.findFace(vm::vec3d{0, 0, 1}))
        .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

      auto* brushNode1 = new BrushNode{std::move(brush1)};
      auto* brushNode2 = new BrushNode{std::move(brush2)};

      addNodes(map, {{entityNode, {brushNode1}}});
      addNodes(map, {{entityNode, {brushNode2}}});
      CHECK(entityNode->children().size() == 2u);

      selectNodes(map, {brushNode1, brushNode2});
      CHECK(csgConvexMerge(map));
      CHECK(entityNode->children().size() == 1u);

      auto* brushNode3 = static_cast<BrushNode*>(entityNode->children()[0]);
      const auto& brush3 = brushNode3->brush();

      const auto& top = brush3.face(*brush3.findFace(vm::vec3d{0, 0, 1}));
      CHECK(top.uAxis() == vm::vec3d{1, 0, 0});
      CHECK(top.vAxis() == vm::vec3d{0, 1, 0});
    }
  }

  SECTION("csgSubtract")
  {
    SECTION("Subtract multiple brushes")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      auto* minuendNode = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}}, "material")
        | kdl::value()};
      auto* subtrahendNode1 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{32, 32, 64}}, "material")
        | kdl::value()};
      auto* subtrahendNode2 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{32, 32, 0}, vm::vec3d{64, 64, 64}}, "material")
        | kdl::value()};

      addNodes(map, {{entityNode, {minuendNode, subtrahendNode1, subtrahendNode2}}});
      CHECK(entityNode->children().size() == 3u);

      // we want to compute minuend - {subtrahendNode1, subtrahendNode2}
      selectNodes(map, {subtrahendNode1, subtrahendNode2});
      CHECK(csgSubtract(map));
      CHECK(entityNode->children().size() == 2u);

      auto* remainderNode1 = dynamic_cast<BrushNode*>(entityNode->children()[0]);
      auto* remainderNode2 = dynamic_cast<BrushNode*>(entityNode->children()[1]);
      CHECK(remainderNode1 != nullptr);
      CHECK(remainderNode2 != nullptr);

      const auto expectedBBox1 = vm::bbox3d{vm::vec3d{0, 32, 0}, vm::vec3d{32, 64, 64}};
      const auto expectedBBox2 = vm::bbox3d{vm::vec3d{32, 0, 0}, vm::vec3d{64, 32, 64}};

      if (remainderNode1->logicalBounds() != expectedBBox1)
      {
        std::swap(remainderNode1, remainderNode2);
      }

      CHECK(remainderNode1->logicalBounds() == expectedBBox1);
      CHECK(remainderNode2->logicalBounds() == expectedBBox2);
    }

    SECTION("Undo restores selection")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      auto* subtrahend1 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{entityNode, {subtrahend1}}});

      selectNodes(map, {subtrahend1});
      CHECK(csgSubtract(map));
      CHECK(entityNode->children().size() == 0u);
      CHECK_FALSE(map.selection().hasNodes());

      // check that the selection is restored after undo
      map.undoCommand();

      CHECK(map.selection().hasOnlyBrushes());
      CHECK_THAT(
        map.selection().brushes,
        Catch::Matchers::Equals(std::vector<BrushNode*>{subtrahend1}));
    }

    SECTION("Texture alignment")
    {
      fixture.create({.mapFormat = MapFormat::Valve});

      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      auto texAlignment = ParallelUVCoordSystem{vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}};
      auto texAlignmentSnapshot = texAlignment.takeSnapshot();

      auto brush1 = builder.createCuboid(
                      vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}}, "material")
                    | kdl::value();
      auto brush2 = builder.createCuboid(
                      vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 32}}, "material")
                    | kdl::value();
      brush2.face(*brush2.findFace(vm::vec3d{0, 0, 1}))
        .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

      auto* brushNode1 = new BrushNode{std::move(brush1)};
      auto* brushNode2 = new BrushNode{std::move(brush2)};

      addNodes(map, {{entityNode, {brushNode1}}});
      addNodes(map, {{entityNode, {brushNode2}}});
      CHECK(entityNode->children().size() == 2u);

      // we want to compute brush1 - brush2
      selectNodes(map, {brushNode2});
      CHECK(csgSubtract(map));
      CHECK(entityNode->children().size() == 1u);

      auto* brushNode3 = static_cast<BrushNode*>(entityNode->children()[0]);
      const auto& brush3 = brushNode3->brush();

      CHECK(
        brushNode3->logicalBounds()
        == vm::bbox3d{vm::vec3d{0, 0, 32}, vm::vec3d{64, 64, 64}});

      // the material alignment from the top of brush2 should have transferred
      // to the bottom face of brush3
      const auto& top = brush3.face(*brush3.findFace(vm::vec3d{0, 0, -1}));
      CHECK(top.uAxis() == vm::vec3d{1, 0, 0});
      CHECK(top.vAxis() == vm::vec3d{0, 1, 0});
    }

    SECTION("Regression tests")
    {
      fixture.load(
        "fixture/test/ui/MapDocumentTest/csgSubtractFailure.map",
        {.mapFormat = MapFormat::Valve, .game = LoadGameFixture{"Quake"}});

      REQUIRE(map.editorContext().currentLayer()->childCount() == 2);
      auto* subtrahendNode =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(1));
      REQUIRE(subtrahendNode);
      REQUIRE(subtrahendNode->brush().findFace("clip").has_value());

      // select the second object in the default layer (a clip brush) and subtract
      selectNodes(map, {subtrahendNode});
      CHECK(csgSubtract(map));

      REQUIRE(map.editorContext().currentLayer()->childCount() == 1);
      auto* result =
        dynamic_cast<BrushNode*>(map.editorContext().currentLayer()->children().at(0));

      CHECK_THAT(
        result->brush().vertexPositions(),
        UnorderedApproxVecMatches(
          std::vector<vm::vec3d>{
            {-2852, 372, 248},
            {-2854, 372, 256},
            {-2854, 364, 256},
            {-2852, 364, 248},
            {-2840, 372, 248},
            {-2843.2, 372, 256},
            {-2843.2, 364, 256},
            {-2840, 364, 248}},
          0.001));
    }
  }

  SECTION("csgHollow")
  {
    fixture.load(
      "fixture/test/ui/MapDocumentTest/csgHollow.map",
      {.mapFormat = MapFormat::Valve, .game = LoadGameFixture{"Quake"}});

    REQUIRE(map.editorContext().currentLayer()->childCount() == 2);
    REQUIRE(!map.modified());

    SECTION("A brush too small to be hollowed doesn't block the command")
    {
      selectAllNodes(map);
      CHECK(csgHollow(map));

      // One cube is too small to hollow, so it's left untouched.
      // The other is hollowed into 6 brushes.
      CHECK(map.editorContext().currentLayer()->childCount() == 7);
      CHECK(map.modified());
    }

    SECTION("If no brushes are hollowed, the transaction isn't committed")
    {
      auto* smallBrushNode = map.editorContext().currentLayer()->children().at(0);
      selectNodes(map, {smallBrushNode});

      CHECK(!csgHollow(map));
      CHECK(map.editorContext().currentLayer()->childCount() == 2);
      CHECK(!map.modified());
    }
  }
}

} // namespace tb::mdl
