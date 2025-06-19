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
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Material.h"
#include "mdl/Texture.h"

#include "kdl/range_to_vector.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"

#include "vm/approx.h"
#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/vec.h"

#include <string>
#include <vector>

#include "Catch2.h"

namespace tb::mdl
{
namespace
{

bool canMoveBoundary(
  Brush brush,
  const vm::bbox3d& worldBounds,
  const size_t faceIndex,
  const vm::vec3d& delta)
{
  return brush.moveBoundary(worldBounds, faceIndex, delta, false)
         | kdl::transform([&]() { return worldBounds.contains(brush.bounds()); })
         | kdl::value_or(false);
}

void assertCanMoveVertices(
  Brush brush, const std::vector<vm::vec3d> vertexPositions, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);

  CHECK(brush.canTransformVertices(worldBounds, vertexPositions, transform));

  REQUIRE(brush.transformVertices(worldBounds, vertexPositions, transform).is_success());

  auto movedVertexPositions =
    brush.findClosestVertexPositions(transform * vertexPositions);
  movedVertexPositions =
    kdl::vec_sort_and_remove_duplicates(std::move(movedVertexPositions));

  auto expectedVertexPositions = transform * vertexPositions;
  expectedVertexPositions =
    kdl::vec_sort_and_remove_duplicates(std::move(expectedVertexPositions));

  CHECK(movedVertexPositions == expectedVertexPositions);
}

// "Move point" tests

void assertMovingVerticesDeletes(
  Brush brush, const std::vector<vm::vec3d> vertexPositions, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);

  CHECK(brush.canTransformVertices(worldBounds, vertexPositions, transform));

  REQUIRE(brush.transformVertices(worldBounds, vertexPositions, transform).is_success());
  const auto movedVertexPositions =
    brush.findClosestVertexPositions(transform * vertexPositions);
  CHECK(movedVertexPositions.empty());
}

void assertCanNotMoveVertices(
  const Brush& brush, const std::vector<vm::vec3d> vertexPositions, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);
  CHECK_FALSE(brush.canTransformVertices(worldBounds, vertexPositions, transform));
}

void assertCanMoveVertex(
  const Brush& brush, const vm::vec3d vertexPosition, const vm::vec3d delta)
{
  assertCanMoveVertices(brush, std::vector<vm::vec3d>{vertexPosition}, delta);
}

void assertMovingVertexDeletes(
  const Brush& brush, const vm::vec3d vertexPosition, const vm::vec3d delta)
{
  assertMovingVerticesDeletes(brush, std::vector<vm::vec3d>{vertexPosition}, delta);
}

void assertCanNotMoveVertex(
  const Brush& brush, const vm::vec3d vertexPosition, const vm::vec3d delta)
{
  assertCanNotMoveVertices(brush, std::vector<vm::vec3d>{vertexPosition}, delta);
}

void assertCanNotMoveEdges(
  const Brush& brush, const std::vector<vm::segment3d> edges, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);
  CHECK_FALSE(brush.canTransformEdges(worldBounds, edges, transform));
}

void assertCanMoveFaces(
  Brush brush, const std::vector<vm::polygon3d> movingFaces, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);

  const auto expectedMovedFaces =
    movingFaces
    | std::views::transform([&](const auto& face) { return face.transform(transform); })
    | kdl::to_vector;

  CHECK(brush.canTransformFaces(worldBounds, movingFaces, transform));
  CHECK(brush.transformFaces(worldBounds, movingFaces, transform).is_success());
  const auto movedFaces = brush.findClosestFacePositions(expectedMovedFaces);
  CHECK(movedFaces == expectedMovedFaces);
}

void assertCanNotMoveFaces(
  const Brush& brush, const std::vector<vm::polygon3d> movingFaces, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);
  CHECK_FALSE(brush.canTransformFaces(worldBounds, movingFaces, transform));
}

void assertCanMoveFace(
  const Brush& brush, const std::optional<size_t>& topFaceIndex, const vm::vec3d delta)
{
  REQUIRE(topFaceIndex);
  const auto& topFace = brush.face(*topFaceIndex);
  assertCanMoveFaces(brush, std::vector<vm::polygon3d>{topFace.polygon()}, delta);
}

void assertCanNotMoveFace(
  const Brush& brush, const std::optional<size_t>& topFaceIndex, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);

  REQUIRE(topFaceIndex);
  const auto& topFace = brush.face(*topFaceIndex);
  CHECK_FALSE(brush.canTransformFaces(worldBounds, {topFace.polygon()}, transform));
}

void assertCanMoveTopFace(const Brush& brush, const vm::vec3d delta)
{
  assertCanMoveFace(brush, brush.findFace(vm::vec3d{0, 0, 1}), delta);
}

void assertCanNotMoveTopFace(const Brush& brush, const vm::vec3d delta)
{
  assertCanNotMoveFace(brush, brush.findFace(vm::vec3d{0, 0, 1}), delta);
}

void assertCanNotMoveTopFaceBeyond127UnitsDown(const Brush& brush)
{
  assertCanMoveTopFace(brush, {0, 0, -127});
  assertCanNotMoveTopFace(brush, {0, 0, -128});
  assertCanNotMoveTopFace(brush, {0, 0, -129});

  assertCanMoveTopFace(brush, {256, 0, -127});
  assertCanNotMoveTopFace(brush, {256, 0, -128});
  assertCanNotMoveTopFace(brush, {256, 0, -129});
}

template <MapFormat F>
class UVLockTest
{
  MapFormat param = F;
};

} // namespace

TEST_CASE("BrushTest.constructBrushWithFaces")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  // build a cube with length 16 at the origin
  const auto brush =
    Brush::create(
      worldBounds,
      {
        // left
        createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{0, 1, 0}, vm::vec3d{0, 0, 1}),
        // right
        createParaxial(vm::vec3d{16, 0, 0}, vm::vec3d{16, 0, 1}, vm::vec3d{16, 1, 0}),
        // front
        createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::vec3d{1, 0, 0}),
        // back
        createParaxial(vm::vec3d{0, 16, 0}, vm::vec3d{1, 16, 0}, vm::vec3d{0, 16, 1}),
        // top
        createParaxial(vm::vec3d{0, 0, 16}, vm::vec3d{0, 1, 16}, vm::vec3d{1, 0, 16}),
        // bottom
        createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
      })
    | kdl::value();

  REQUIRE(brush.fullySpecified());
  REQUIRE(brush.faceCount() == 6u);
  CHECK(brush.findFace(vm::vec3d{1, 0, 0}));
  CHECK(brush.findFace(vm::vec3d{-1, 0, 0}));
  CHECK(brush.findFace(vm::vec3d{0, 1, 0}));
  CHECK(brush.findFace(vm::vec3d{0, -1, 0}));
  CHECK(brush.findFace(vm::vec3d{0, 0, 1}));
  CHECK(brush.findFace(vm::vec3d{0, 0, -1}));
}

TEST_CASE("BrushTest.constructBrushWithRedundantFaces")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  CHECK(Brush::create(
          worldBounds,
          {
            createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
            createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
            createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
          })
          .is_error());
}

TEST_CASE("BrushTest.cloneFaceAttributesFrom")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto brushBuilder = BrushBuilder{MapFormat::Valve, worldBounds};
  auto brush =
    brushBuilder.createCube(64.0, "left", "right", "front", "back", "top", "bottom")
    | kdl::value();

  const auto topFaceIndex = brush.findFace(vm::vec3d{0, 0, 1});
  REQUIRE(topFaceIndex != std::nullopt);

  auto& topFace = brush.face(*topFaceIndex);

  auto attributes = topFace.attributes();
  attributes.setXOffset(64.0f);
  attributes.setYOffset(-48.0f);
  topFace.setAttributes(attributes);

  auto newBrush = brush;
  newBrush.cloneFaceAttributesFrom(brush);

  CHECK(newBrush == brush);
}

TEST_CASE("BrushTest.clip")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto left =
    createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{0, 1, 0}, vm::vec3d{0, 0, 1});
  const auto right =
    createParaxial(vm::vec3d{16, 0, 0}, vm::vec3d{16, 0, 1}, vm::vec3d{16, 1, 0});
  const auto front =
    createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::vec3d{1, 0, 0});
  const auto back =
    createParaxial(vm::vec3d{0, 16, 0}, vm::vec3d{1, 16, 0}, vm::vec3d{0, 16, 1});
  const auto top =
    createParaxial(vm::vec3d{0, 0, 16}, vm::vec3d{0, 1, 16}, vm::vec3d{1, 0, 16});
  const auto bottom =
    createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0});

  // build a cube with length 16 at the origin
  auto brush =
    Brush::create(worldBounds, {left, right, front, back, top, bottom}) | kdl::value();

  auto clip = createParaxial(vm::vec3d{8, 0, 0}, vm::vec3d{8, 0, 1}, vm::vec3d{8, 1, 0});
  CHECK(brush.clip(worldBounds, clip).is_success());

  CHECK(brush.faceCount() == 6u);
  CHECK(brush.findFace(left.boundary()));
  CHECK(brush.findFace(clip.boundary()));
  CHECK(brush.findFace(front.boundary()));
  CHECK(brush.findFace(back.boundary()));
  CHECK(brush.findFace(top.boundary()));
  CHECK(brush.findFace(bottom.boundary()));
  CHECK_FALSE(brush.findFace(right.boundary()));
}

TEST_CASE("BrushTest.moveBoundary")
{
  const auto worldBounds = vm::bbox3d{4096.0};
  auto brush =
    Brush::create(
      worldBounds,
      {
        createParaxial(
          vm::vec3d{0, 0, 0}, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 1}), // left
        createParaxial(
          vm::vec3d{16, 0, 0}, vm::vec3d{15, 0, 1}, vm::vec3d{16, 1, 0}), // right
        createParaxial(
          vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::vec3d{1, 0, 0}), // front
        createParaxial(
          vm::vec3d{0, 16, 0}, vm::vec3d{1, 16, 0}, vm::vec3d{0, 16, 1}), // back
        createParaxial(vm::vec3d{0, 0, 6}, vm::vec3d{0, 1, 6}, vm::vec3d{1, 0, 6}), // top
        createParaxial(
          vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}), // bottom
      })
    | kdl::value();

  REQUIRE(brush.faceCount() == 6u);

  const auto topFaceIndex = brush.findFace(vm::vec3d{0, 0, 1});
  REQUIRE(topFaceIndex);

  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3d{0, 0, +16}));
  CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3d{0, 0, -16}));
  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3d{0, 0, +2}));
  CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3d{0, 0, -6}));
  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3d{0, 0, +1}));
  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3d{0, 0, -5}));

  CHECK(brush.moveBoundary(worldBounds, *topFaceIndex, vm::vec3d{0, 0, 1}, false)
          .is_success());
  CHECK(worldBounds.contains(brush.bounds()));

  CHECK(brush.faces().size() == 6u);
  CHECK(brush.bounds().size().z() == 7.0);
}

TEST_CASE("BrushTest.resizePastWorldBounds")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  auto brush1 = builder.createBrush(
                  std::vector<vm::vec3d>{
                    {64, -64, 16},
                    {64, 64, 16},
                    {64, -64, -16},
                    {64, 64, -16},
                    {48, 64, 16},
                    {48, 64, -16},
                  },
                  "material")
                | kdl::value();

  const auto rightFaceIndex = brush1.findFace(vm::vec3d{1, 0, 0});
  REQUIRE(rightFaceIndex);

  CHECK(canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3d{16, 0, 0}));
  CHECK(!canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3d{8000, 0, 0}));
}

TEST_CASE("BrushTest.expand")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  auto brush1 =
    builder.createCuboid(vm::bbox3d{{-64, -64, -64}, {64, 64, 64}}, "material")
    | kdl::value();
  CHECK(brush1.expand(worldBounds, 6, true).is_success());

  const auto expandedBBox = vm::bbox3d{{-70, -70, -70}, {70, 70, 70}};
  const auto expectedVerticesArray = expandedBBox.vertices();
  const auto expectedVertices =
    std::vector<vm::vec3d>{expectedVerticesArray.begin(), expectedVerticesArray.end()};

  CHECK(brush1.bounds() == expandedBBox);
  CHECK_THAT(brush1.vertexPositions(), Catch::UnorderedEquals(expectedVertices));
}

TEST_CASE("BrushTest.contract")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  auto brush1 =
    builder.createCuboid(vm::bbox3d{{-64, -64, -64}, {64, 64, 64}}, "material")
    | kdl::value();
  CHECK(brush1.expand(worldBounds, -32, true).is_success());

  const auto expandedBBox = vm::bbox3d{{-32, -32, -32}, {32, 32, 32}};
  const auto expectedVerticesArray = expandedBBox.vertices();
  const auto expectedVertices =
    std::vector<vm::vec3d>{expectedVerticesArray.begin(), expectedVerticesArray.end()};

  CHECK(brush1.bounds() == expandedBBox);
  CHECK_THAT(brush1.vertexPositions(), Catch::UnorderedEquals(expectedVertices));
}

TEST_CASE("BrushTest.contractToZero")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  auto brush1 =
    builder.createCuboid(vm::bbox3d{{-64, -64, -64}, {64, 64, 64}}, "material")
    | kdl::value();
  CHECK(brush1.expand(worldBounds, -64, true).is_error());
}

TEST_CASE("BrushTest.moveVertex")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom")
               | kdl::value();

  const auto p1 = vm::vec3d{-32, -32, -32};
  const auto p2 = vm::vec3d{-32, -32, +32};
  const auto p3 = vm::vec3d{-32, +32, -32};
  const auto p4 = vm::vec3d{-32, +32, +32};
  const auto p5 = vm::vec3d{+32, -32, -32};
  const auto p6 = vm::vec3d{+32, -32, +32};
  const auto p7 = vm::vec3d{+32, +32, -32};
  const auto p8 = vm::vec3d{+32, +32, +32};
  const auto p9 = vm::vec3d{+16, +16, +32};

  auto oldVertexPositions = std::vector<vm::vec3d>{p8};
  const auto transform = vm::translation_matrix(p9 - p8);
  const auto inverse = vm::translation_matrix(p8 - p9);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx{p9});

  assertMaterial("left", brush, p1, p2, p4, p3);
  assertMaterial("right", brush, p5, p7, p6);
  assertMaterial("right", brush, p6, p7, p9);
  assertMaterial("front", brush, p1, p5, p6, p2);
  assertMaterial("back", brush, p3, p4, p7);
  assertMaterial("back", brush, p4, p9, p7);
  assertMaterial("top", brush, p2, p6, p9, p4);
  assertMaterial("bottom", brush, p1, p3, p7, p5);

  oldVertexPositions = std::move(newVertexPositions);
  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, inverse).is_success());
  newVertexPositions = brush.findClosestVertexPositions(inverse * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx{p8});

  assertMaterial("left", brush, p1, p2, p4, p3);
  assertMaterial("right", brush, p5, p7, p8, p6);
  assertMaterial("front", brush, p1, p5, p6, p2);
  assertMaterial("back", brush, p3, p4, p8, p7);
  assertMaterial("top", brush, p2, p6, p8, p4);
  assertMaterial("bottom", brush, p1, p3, p7, p5);
}

TEST_CASE("BrushTest.rotateVertices")
{
  const auto angle = GENERATE(35.0, 45.0, 72.0, 90.0, 180.0, 270.0);

  CAPTURE(angle);

  const auto worldBounds = vm::bbox3d{4096.0};

  const auto p1 = vm::vec3d{-32, -32, -32};
  const auto p2 = vm::vec3d{+32, -32, -32};
  const auto p3 = vm::vec3d{-32, +32, -32};
  const auto p4 = vm::vec3d{+32, +32, -32};
  const auto p5 = vm::vec3d{-32, -32, +32};
  const auto p6 = vm::vec3d{+32, -32, +32};
  const auto p7 = vm::vec3d{-32, +32, +32};
  const auto p8 = vm::vec3d{+32, +32, +32};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush =
    builder.createBrush(std::vector{p1, p2, p3, p4, p5, p6, p7, p8}, "some_material")
    | kdl::value();

  const auto oldVertexPositions = std::vector{p1, p2, p3, p4};
  const auto transform = vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(angle));

  REQUIRE(brush.canTransformVertices(worldBounds, oldVertexPositions, transform));
  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(
    newVertexPositions
    == std::vector{
      vm::approx(transform * p1),
      vm::approx(transform * p2),
      vm::approx(transform * p3),
      vm::approx(transform * p4),
    });
}

TEST_CASE("BrushTest.moveTetrahedronVertexToOpposideSide")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto top = vm::vec3d{0, 0, 16};

  const auto points = std::vector<vm::vec3d>{
    {-16, -16, 0},
    {+16, -16, 0},
    {0, +16, 0},
    top,
  };

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(points, "some_material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>{top};
  const auto transform = vm::translation_matrix(vm::vec3d{0, 0, -32});

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx{vm::vec3d{0, 0, -16}});
  CHECK(brush.fullySpecified());
}

TEST_CASE("BrushTest.moveVertexInwardWithoutMerges")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};
  const auto p9 = vm::vec3d{+56, +56, +56};

  const auto originalPositions = std::vector{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 15u);
  CHECK(brush.faceCount() == 9u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p6}));
  CHECK(brush.hasEdge({p4, p7}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p7}));
  CHECK(brush.hasEdge({p6, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p4}));
  CHECK(brush.hasFace({p5, p7, p6}));
  CHECK(brush.hasFace({p3, p4, p7}));
  CHECK(brush.hasFace({p9, p6, p7}));
  CHECK(brush.hasFace({p9, p4, p6}));
  CHECK(brush.hasFace({p9, p7, p4}));
}

TEST_CASE("BrushTest.moveVertexOutwardWithoutMerges")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};
  const auto p9 = vm::vec3d{+72, +72, +72};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 15u);
  CHECK(brush.faceCount() == 9u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p2, p9}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p3, p9}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p5, p9}));
  CHECK(brush.hasEdge({p6, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p9}));
  CHECK(brush.hasFace({p2, p9, p4}));
  CHECK(brush.hasFace({p3, p4, p9}));
  CHECK(brush.hasFace({p3, p9, p7}));
  CHECK(brush.hasFace({p5, p9, p6}));
  CHECK(brush.hasFace({p5, p7, p9}));
}

TEST_CASE("BrushTest.moveVertexWithOneOuterNeighbourMerge")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+56, +56, +56};
  const auto p9 = vm::vec3d{+56, +56, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 14u);
  CHECK(brush.faceCount() == 8u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p7}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p7}));
  CHECK(brush.hasEdge({p6, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p9, p4}));
  CHECK(brush.hasFace({p5, p7, p6}));
  CHECK(brush.hasFace({p3, p4, p7}));
  CHECK(brush.hasFace({p9, p6, p7}));
  CHECK(brush.hasFace({p9, p7, p4}));
}

TEST_CASE("BrushTest.moveVertexWithTwoOuterNeighbourMerges")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+56, +56, +56};
  const auto p9 = vm::vec3d{+64, +64, +56};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx{p9});

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 13u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p6}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p5, p7, p9, p6}));
  CHECK(brush.hasFace({p3, p4, p9, p7}));
  CHECK(brush.hasFace({p2, p6, p4}));
  CHECK(brush.hasFace({p9, p4, p6}));
}

TEST_CASE("BrushTest.moveVertexWithAllOuterNeighbourMerges")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+56, +56, +56};
  const auto p9 = vm::vec3d{+64, +64, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx{p9});

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 6u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p9, p4}));
  CHECK(brush.hasFace({p3, p4, p9, p7}));
  CHECK(brush.hasFace({p5, p7, p9, p6}));
}

TEST_CASE("BrushTest.moveVertexWithAllInnerNeighbourMerge")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};
  const auto p9 = vm::vec3d{0, 0, 0};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 0u);

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p6}));
  CHECK(brush.hasEdge({p4, p7}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p7}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p4}));
  CHECK(brush.hasFace({p3, p4, p7}));
  CHECK(brush.hasFace({p5, p7, p6}));
  CHECK(brush.hasFace({p4, p6, p7}));
}

TEST_CASE("BrushTest.moveVertexUpThroughPlane")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +56};
  const auto p9 = vm::vec3d{+64, +64, +72};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 13u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p2, p9}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p3, p4, p9, p7}));
  CHECK(brush.hasFace({p5, p7, p9, p6}));
  CHECK(brush.hasFace({p2, p9, p4}));
  CHECK(brush.hasFace({p2, p6, p9}));
}

TEST_CASE("BrushTest.moveVertexOntoEdge")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, 0};
  const auto p9 = vm::vec3d{0, 0, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 0u);

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p6}));
  CHECK(brush.hasEdge({p4, p7}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p7}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p4}));
  CHECK(brush.hasFace({p3, p4, p7}));
  CHECK(brush.hasFace({p5, p7, p6}));
  CHECK(brush.hasFace({p4, p6, p7}));
}

TEST_CASE("BrushTest.moveVertexOntoIncidentVertex")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
  const auto transform = vm::translation_matrix(p7 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p7));

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p6}));
  CHECK(brush.hasEdge({p4, p7}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p7}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p2, p6, p4}));
  CHECK(brush.hasFace({p3, p4, p7}));
  CHECK(brush.hasFace({p5, p7, p6}));
  CHECK(brush.hasFace({p4, p6, p7}));
}

TEST_CASE("BrushTest.moveVertexOntoIncidentVertexInOppositeDirection")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p7});
  const auto transform = vm::translation_matrix(p8 - p7);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p8));

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p8));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p5}));
  CHECK(brush.hasEdge({p3, p8}));
  CHECK(brush.hasEdge({p4, p8}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p8}));
  CHECK(brush.hasEdge({p6, p8}));

  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p2, p6, p8, p4}));
  CHECK(brush.hasFace({p1, p3, p5}));
  CHECK(brush.hasFace({p3, p4, p8}));
  CHECK(brush.hasFace({p5, p8, p6}));
  CHECK(brush.hasFace({p3, p8, p5}));
}

TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};
  const auto p9 = vm::vec3d{+80, +64, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p6});
  const auto transform = vm::translation_matrix(p9 - p6);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p5}));
  CHECK(brush.hasEdge({p2, p9}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p5, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p3, p4, p9, p7}));
  CHECK(brush.hasFace({p1, p5, p2}));
  CHECK(brush.hasFace({p2, p5, p9}));
  CHECK(brush.hasFace({p2, p9, p4}));
  CHECK(brush.hasFace({p5, p7, p9}));
}

TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex2")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};
  const auto p9 = vm::vec3d{+80, -64, +64};

  const auto originalPositions = std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
  const auto transform = vm::translation_matrix(p9 - p8);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 7u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p9));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p9}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p7}));
  CHECK(brush.hasEdge({p4, p9}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p5, p9}));
  CHECK(brush.hasEdge({p7, p9}));

  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p1, p5, p9, p2}));
  CHECK(brush.hasFace({p2, p9, p4}));
  CHECK(brush.hasFace({p3, p4, p7}));
  CHECK(brush.hasFace({p4, p9, p7}));
  CHECK(brush.hasFace({p5, p7, p9}));
}

TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithDeletingVertex")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};
  const auto p9 = vm::vec3d{+80, 0, +64};
  const auto p10 = vm::vec3d{+64, 0, +64};

  const auto originalPositions =
    std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8, p9};

  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

  const auto oldVertexPositions = std::vector<vm::vec3d>({p9});
  const auto transform = vm::translation_matrix(p10 - p9);

  CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform).is_success());
  const auto newVertexPositions =
    brush.findClosestVertexPositions(transform * oldVertexPositions);

  CHECK(newVertexPositions.size() == 0u);

  CHECK(brush.vertexCount() == 8u);
  CHECK(brush.edgeCount() == 12u);
  CHECK(brush.faceCount() == 6u);

  CHECK(brush.hasVertex(p1));
  CHECK(brush.hasVertex(p2));
  CHECK(brush.hasVertex(p3));
  CHECK(brush.hasVertex(p4));
  CHECK(brush.hasVertex(p5));
  CHECK(brush.hasVertex(p6));
  CHECK(brush.hasVertex(p7));
  CHECK(brush.hasVertex(p8));

  CHECK(brush.hasEdge({p1, p2}));
  CHECK(brush.hasEdge({p1, p3}));
  CHECK(brush.hasEdge({p1, p5}));
  CHECK(brush.hasEdge({p2, p4}));
  CHECK(brush.hasEdge({p2, p6}));
  CHECK(brush.hasEdge({p3, p4}));
  CHECK(brush.hasEdge({p3, p7}));
  CHECK(brush.hasEdge({p4, p8}));
  CHECK(brush.hasEdge({p5, p6}));
  CHECK(brush.hasEdge({p5, p7}));
  CHECK(brush.hasEdge({p6, p8}));
  CHECK(brush.hasEdge({p7, p8}));

  CHECK(brush.hasFace({p1, p2, p4, p3}));
  CHECK(brush.hasFace({p1, p3, p7, p5}));
  CHECK(brush.hasFace({p1, p5, p6, p2}));
  CHECK(brush.hasFace({p2, p6, p8, p4}));
  CHECK(brush.hasFace({p3, p4, p8, p7}));
  CHECK(brush.hasFace({p5, p7, p8, p6}));
}

TEST_CASE("BrushTest.moveVerticesPastWorldBounds")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  auto brush = builder.createCube(128.0, "material") | kdl::value();

  const auto allVertexPositions =
    brush.vertices()
    | std::views::transform([](const auto* vertex) { return vertex->position(); })
    | kdl::to_vector;

  CHECK(brush.canTransformVertices(
    worldBounds, allVertexPositions, vm::translation_matrix(vm::vec3d{16, 0, 0})));
  CHECK_FALSE(brush.canTransformVertices(
    worldBounds, allVertexPositions, vm::translation_matrix(vm::vec3d{8192, 0, 0})));
}

// NOTE: Different than movePolygonRemainingPoint, because in this case we allow
// point moves that flip the normal of the remaining polygon
TEST_CASE("BrushTest.movePointRemainingPolygon")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto peakPosition = vm::vec3d{0, 0, +64};
  const auto baseQuadVertexPositions = std::vector<vm::vec3d>{
    {-64, -64, -64}, // base quad
    {-64, +64, -64},
    {+64, +64, -64},
    {+64, -64, -64}};
  const auto vertexPositions =
    kdl::vec_concat(std::vector<vm::vec3d>{peakPosition}, baseQuadVertexPositions);

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  assertCanMoveVertex(brush, peakPosition, vm::vec3d{0, 0, -127});
  assertCanNotMoveVertex(
    brush, peakPosition, vm::vec3d{0, 0, -128}); // Onto the base quad plane
  assertCanMoveVertex(
    brush,
    peakPosition,
    vm::vec3d{0, 0, -129}); // Through the other side of the base quad

  // More detailed testing of the last assertion
  {
    auto brushCopy = brush;
    auto temp = baseQuadVertexPositions;
    std::reverse(temp.begin(), temp.end());
    const auto flippedBaseQuadVertexPositions = std::vector<vm::vec3d>{temp};

    const auto transform = vm::translation_matrix(vm::vec3d{0.0, 0.0, -129.0});

    CHECK(brushCopy.faceCount() == 5u);
    CHECK(brushCopy.findFace(vm::polygon3d{baseQuadVertexPositions}));
    CHECK_FALSE(brushCopy.findFace(vm::polygon3d{flippedBaseQuadVertexPositions}));
    CHECK(brushCopy.findFace(vm::vec3d{0, 0, -1}));
    CHECK_FALSE(brushCopy.findFace(vm::vec3d{0, 0, 1}));

    const auto oldVertexPositions = std::vector<vm::vec3d>{peakPosition};
    CHECK(brushCopy.canTransformVertices(worldBounds, oldVertexPositions, transform));
    REQUIRE(brushCopy.transformVertices(worldBounds, oldVertexPositions, transform)
              .is_success());
    const auto newVertexPositions =
      brushCopy.findClosestVertexPositions(transform * oldVertexPositions);
    CHECK(newVertexPositions == transform * oldVertexPositions);

    CHECK(brushCopy.faceCount() == 5u);
    CHECK_FALSE(brushCopy.findFace(vm::polygon3d{baseQuadVertexPositions}));
    CHECK(brushCopy.findFace(vm::polygon3d{flippedBaseQuadVertexPositions}));
    CHECK_FALSE(brushCopy.findFace(vm::vec3d{0, 0, -1}));
    CHECK(brushCopy.findFace(vm::vec3d{0, 0, 1}));
  }

  assertCanMoveVertex(brush, peakPosition, vm::vec3d{256, 0, -127});
  assertCanNotMoveVertex(
    brush, peakPosition, vm::vec3d{256, 0, -128}); // Onto the base quad plane
  assertCanMoveVertex(
    brush, peakPosition, vm::vec3d{256, 0, -129}); // Flips the normal of the base
                                                   // quad, without moving through it
}

TEST_CASE("BrushTest.movePointRemainingPolyhedron")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto peakPosition = vm::vec3d{0, 0, 128};
  const auto vertexPositions = std::vector<vm::vec3d>{
    {-64, -64, 0}, // base quad
    {-64, +64, 0},
    {+64, +64, 0},
    {+64, -64, 0},
    {-64, -64, 64}, // upper quad
    {-64, +64, 64},
    {+64, +64, 64},
    {+64, -64, 64},
    peakPosition};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  assertMovingVertexDeletes(
    brush, peakPosition, vm::vec3d{0, 0, -65}); // Move inside the remaining cuboid
  assertCanMoveVertex(
    brush,
    peakPosition,
    vm::vec3d{0, 0, -63}); // Slightly above the top of the cuboid is OK
  assertCanNotMoveVertex(
    brush,
    peakPosition,
    vm::vec3d{0, 0, -129}); // Through and out the other side is disallowed
}

// add vertex tests

// TODO: add tests for Brush::addVertex

// remove vertex tests

TEST_CASE("BrushTest.removeSingleVertex")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createCube(64.0, "asdf") | kdl::value();

  CHECK(brush.removeVertices(worldBounds, {vm::vec3d{+32, +32, +32}}).is_success());

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, -32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, +32, -32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

  CHECK(brush.removeVertices(worldBounds, {vm::vec3d{+32, +32, -32}}).is_success());

  CHECK(brush.vertexCount() == 6u);
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, -32, +32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, -32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

  CHECK(brush.removeVertices(worldBounds, {vm::vec3d{+32, -32, +32}}).is_success());

  CHECK(brush.vertexCount() == 5u);
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, -32, +32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, -32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

  CHECK(brush.removeVertices(worldBounds, {vm::vec3d{-32, -32, -32}}).is_success());

  CHECK(brush.vertexCount() == 4u);
  CHECK_FALSE(brush.hasVertex(vm::vec3d{-32, -32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
  CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
  CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, -32, +32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, -32}));
  CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

  CHECK_FALSE(brush.canRemoveVertices(worldBounds, {vm::vec3d{-32, -32, +32}}));
  CHECK_FALSE(brush.canRemoveVertices(worldBounds, {vm::vec3d{-32, +32, -32}}));
  CHECK_FALSE(brush.canRemoveVertices(worldBounds, {vm::vec3d{-32, +32, +32}}));
  CHECK_FALSE(brush.canRemoveVertices(worldBounds, {vm::vec3d{+32, -32, -32}}));
}

TEST_CASE("BrushTest.removeMultipleVertices")
{
  const auto worldBounds = vm::bbox3d{4096.0};
  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  const auto vertices = std::vector<vm::vec3d>{
    {-32, -32, -32},
    {-32, -32, +32},
    {-32, +32, -32},
    {-32, +32, +32},
    {+32, -32, -32},
    {+32, -32, +32},
    {+32, +32, -32},
    {+32, +32, +32},
  };

  for (size_t i = 0; i < 6; ++i)
  {
    for (size_t j = i + 1; j < 7; ++j)
    {
      for (size_t k = j + 1; k < 8; ++k)
      {
        const auto toRemove =
          std::vector<vm::vec3d>{vertices[i], vertices[j], vertices[k]};

        auto brush = builder.createBrush(vertices, "asdf") | kdl::value();
        CHECK(brush.canRemoveVertices(worldBounds, toRemove));
        CHECK(brush.removeVertices(worldBounds, toRemove).is_success());

        for (size_t l = 0; l < 8; ++l)
        {
          if (l != i && l != j && l != k)
          {
            CHECK(brush.hasVertex(vertices[l]));
          }
        }
      }
    }
  }
}

// "Move edge" tests

TEST_CASE("BrushTest.moveEdge")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom")
               | kdl::value();

  const auto p1 = vm::vec3d{-32, -32, -32};
  const auto p2 = vm::vec3d{-32, -32, +32};
  const auto p3 = vm::vec3d{-32, +32, -32};
  const auto p4 = vm::vec3d{-32, +32, +32};
  const auto p5 = vm::vec3d{+32, -32, -32};
  const auto p6 = vm::vec3d{+32, -32, +32};
  const auto p7 = vm::vec3d{+32, +32, -32};
  const auto p8 = vm::vec3d{+32, +32, +32};
  const auto p1_2 = vm::vec3d{-32, -32, -16};
  const auto p2_2 = vm::vec3d{-32, -32, +48};

  assertMaterial("left", brush, p1, p2, p4, p3);
  assertMaterial("right", brush, p5, p7, p8, p6);
  assertMaterial("front", brush, p1, p5, p6, p2);
  assertMaterial("back", brush, p3, p4, p8, p7);
  assertMaterial("top", brush, p2, p6, p8, p4);
  assertMaterial("bottom", brush, p1, p3, p7, p5);

  const auto originalEdge = vm::segment(p1, p2);
  auto oldEdgePositions = std::vector<vm::segment3d>{originalEdge};

  const auto transform = vm::translation_matrix(p1_2 - p1);
  const auto inverse = vm::translation_matrix(p1 - p1_2);

  CHECK(brush.transformEdges(worldBounds, oldEdgePositions, transform).is_success());
  auto newEdgePositions = brush.findClosestEdgePositions(kdl::vec_transform(
    oldEdgePositions, [&](const auto& s) { return s.transform(transform); }));

  CHECK(newEdgePositions == std::vector<vm::segment3d>{{p1_2, p2_2}});

  assertMaterial("left", brush, p1_2, p2_2, p4, p3);
  assertMaterial("right", brush, p5, p7, p8, p6);
  assertMaterial("front", brush, p1_2, p5, p6, p2_2);
  assertMaterial("back", brush, p3, p4, p8, p7);
  assertMaterial("top", brush, p2_2, p6, p8);
  assertMaterial("top", brush, p2_2, p8, p4);
  assertMaterial("bottom", brush, p1_2, p3, p5);
  assertMaterial("bottom", brush, p3, p7, p5);

  CHECK(brush.canTransformEdges(worldBounds, newEdgePositions, inverse));

  oldEdgePositions = std::move(newEdgePositions);
  CHECK(brush.transformEdges(worldBounds, oldEdgePositions, inverse).is_success());
  newEdgePositions = brush.findClosestEdgePositions(kdl::vec_transform(
    oldEdgePositions, [&](const auto& s) { return s.transform(inverse); }));

  CHECK(newEdgePositions == std::vector<vm::segment3d>{originalEdge});

  assertMaterial("left", brush, p1, p2, p4, p3);
  assertMaterial("right", brush, p5, p7, p8, p6);
  assertMaterial("front", brush, p1, p5, p6, p2);
  assertMaterial("back", brush, p3, p4, p8, p7);
  assertMaterial("top", brush, p2, p6, p8, p4);
  assertMaterial("bottom", brush, p1, p3, p7, p5);
}

static void assertCanMoveEdges(
  Brush brush, const std::vector<vm::segment3d> edges, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);

  const auto expectedMovedEdges =
    edges | std::views::transform([&](const auto& edge) { return edge.translate(delta); })
    | kdl::to_vector;

  CHECK(brush.canTransformEdges(worldBounds, edges, transform));
  CHECK(brush.transformEdges(worldBounds, edges, transform).is_success());
  const auto movedEdges = brush.findClosestEdgePositions(
    kdl::vec_transform(edges, [&](const auto& s) { return s.translate(delta); }));
  CHECK(movedEdges == expectedMovedEdges);
}

TEST_CASE("BrushTest.moveEdgeRemainingPolyhedron")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  // Taller than the cube, starts to the left of the +-64 unit cube
  const auto edge = vm::segment3d{{-128, 0, -128}, {-128, 0, +128}};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush =
    builder.createCube(128, BrushFaceAttributes::NoMaterialName) | kdl::value();
  CHECK(brush.addVertex(worldBounds, edge.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge.end()).is_success());

  CHECK(brush.vertexCount() == 10u);

  assertCanMoveEdges(brush, {edge}, {+63, 0, 0});
  assertCanNotMoveEdges(brush, {edge}, {+64, 0, 0});  // On the side of the cube
  assertCanNotMoveEdges(brush, {edge}, {+128, 0, 0}); // Center of the cube

  assertCanMoveVertices(brush, asVertexList({edge}), {+63, 0, 0});
  assertCanMoveVertices(brush, asVertexList({edge}), {+64, 0, 0});
  assertCanMoveVertices(brush, asVertexList({edge}), {+128, 0, 0});
}

// Same as above, but moving 2 edges
TEST_CASE("BrushTest.moveEdgesRemainingPolyhedron")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  // Taller than the cube, starts to the left of the +-64 unit cube
  const auto edge1 = vm::segment3d{{-128, -32, -128}, {-128, -32, +128}};
  const auto edge2 = vm::segment3d{{-128, +32, -128}, {-128, +32, +128}};
  const auto movingEdges = std::vector<vm::segment3d>{edge1, edge2};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush =
    builder.createCube(128, BrushFaceAttributes::NoMaterialName) | kdl::value();
  CHECK(brush.addVertex(worldBounds, edge1.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge1.end()).is_success());
  CHECK(brush.addVertex(worldBounds, edge2.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge2.end()).is_success());

  CHECK(brush.vertexCount() == 12u);

  assertCanMoveEdges(brush, movingEdges, {+63, 0, 0});
  assertCanNotMoveEdges(brush, movingEdges, {+64, 0, 0});  // On the side of the cube
  assertCanNotMoveEdges(brush, movingEdges, {+128, 0, 0}); // Center of the cube

  assertCanMoveVertices(brush, asVertexList(movingEdges), {+63, 0, 0});
  assertCanMoveVertices(brush, asVertexList(movingEdges), {+64, 0, 0});
  assertCanMoveVertices(brush, asVertexList(movingEdges), {+128, 0, 0});
}

// "Move face" tests

TEST_CASE("BrushTest.moveFace")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createCube(64.0, "asdf") | kdl::value();

  const auto face = vm::polygon3d{
    {-32, -32, +32},
    {+32, -32, +32},
    {+32, +32, +32},
    {-32, +32, +32},
  };
  const auto transform = vm::translation_matrix(vm::vec3d{-16, -16, 0});
  const auto inverse = vm::translation_matrix(vm::vec3d{+16, +16, 0});

  CHECK(brush.canTransformFaces(worldBounds, {face}, transform));

  auto oldFacePositions = std::vector<vm::polygon3d>{face};
  CHECK(brush.transformFaces(worldBounds, oldFacePositions, transform).is_success());
  auto newFacePositions = brush.findClosestFacePositions(kdl::vec_transform(
    oldFacePositions, [&](const auto& f) { return f.transform(transform); }));

  CHECK(newFacePositions.size() == 1u);
  CHECK(newFacePositions[0].hasVertex({-48, -48, +32}));
  CHECK(newFacePositions[0].hasVertex({-48, +16, +32}));
  CHECK(newFacePositions[0].hasVertex({+16, +16, +32}));
  CHECK(newFacePositions[0].hasVertex({+16, -48, +32}));

  oldFacePositions = std::move(newFacePositions);
  CHECK(brush.transformFaces(worldBounds, oldFacePositions, inverse).is_success());
  newFacePositions = brush.findClosestFacePositions(kdl::vec_transform(
    oldFacePositions, [&](const auto& f) { return f.transform(inverse); }));

  CHECK(newFacePositions.size() == 1u);
  CHECK(newFacePositions[0].vertices().size() == 4u);
  for (size_t i = 0; i < 4; ++i)
  {
    CHECK(newFacePositions[0].hasVertex(face.vertices()[i]));
  }
}

TEST_CASE("BrushNodeTest.cannotMoveFace")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush =
    builder.createCuboid(vm::vec3d{128, 128, 32}, BrushFaceAttributes::NoMaterialName)
    | kdl::value();

  const auto face = vm::polygon3d{
    {-64, -64, -16},
    {+64, -64, -16},
    {+64, -64, +16},
    {-64, -64, +16},
  };

  const auto transform = vm::translation_matrix(vm::vec3d{0, 128, 0});
  CHECK_FALSE(brush.canTransformFaces(worldBounds, {face}, transform));
}

TEST_CASE("BrushTest.movePolygonRemainingPoint")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto vertexPositions = std::vector<vm::vec3d>{
    {-64, -64, +64}, // top quad
    {-64, +64, +64},
    {+64, -64, +64},
    {+64, +64, +64},

    {0, 0, -64}, // bottom point
  };

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingEdge")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto vertexPositions = std::vector<vm::vec3d>{
    {-64, -64, +64}, // top quad
    {-64, +64, +64},
    {+64, -64, +64},
    {+64, +64, +64},

    {-64, 0, -64}, // bottom edge, on the z=-64 plane
    {+64, 0, -64}};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingPolygon")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush =
    builder.createCube(128.0, BrushFaceAttributes::NoMaterialName) | kdl::value();

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingPolygon2")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  // Same brush as movePolygonRemainingPolygon, but this particular order of vertices
  // triggers a failure in Brush::doCanMoveVertices where the polygon inserted into the
  // "remaining" BrushGeometry gets the wrong normal.
  const auto vertexPositions = std::vector<vm::vec3d>{
    {+64, +64, +64},
    {+64, -64, +64},
    {+64, -64, -64},
    {+64, +64, -64},
    {-64, -64, +64},
    {-64, -64, -64},
    {-64, +64, -64},
    {-64, +64, +64}};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();
  CHECK(brush.bounds() == vm::bbox3d{{-64, -64, -64}, {64, 64, 64}});

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingPolygon_DisallowVertexCombining")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  //       z = +192  //
  // |\              //
  // | \             //
  // |  \  z = +64   //
  // |   |           //
  // |___| z = -64   //
  //                 //

  const auto vertexPositions = std::vector<vm::vec3d>{
    {-64, -64, +192}, // top quad, slanted
    {-64, +64, +192},
    {+64, -64, +64},
    {+64, +64, +64},

    {-64, -64, -64}, // bottom quad
    {-64, +64, -64},
    {+64, -64, -64},
    {+64, +64, -64},
  };

  const auto topFaceNormal = vm::vec3d{sqrt(2.0) / 2.0, 0.0, sqrt(2.0) / 2.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  const auto topFaceIndex = brush.findFace(topFaceNormal);
  assertCanMoveFace(brush, topFaceIndex, vm::vec3d{0, 0, -127});
  // Merge 2 verts of the moving polygon with 2 in the remaining polygon, should be
  // allowed
  assertCanMoveFace(brush, topFaceIndex, {0, 0, -128});
  assertCanNotMoveFace(brush, topFaceIndex, vm::vec3d{0, 0, -129});
}

TEST_CASE("BrushTest.movePolygonRemainingPolyhedron")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  //   _   z = +64   //
  //  / \            //
  // /   \           //
  // |   | z = -64   //
  // |   |           //
  // |___| z = -192  //
  //                 //

  const auto smallerTopPolygon = std::vector<vm::vec3d>{
    {-32, -32, +64}, // smaller top polygon
    {-32, +32, +64},
    {+32, -32, +64},
    {+32, +32, +64}};
  const auto cubeTopFace = std::vector<vm::vec3d>{
    {-64, -64, -64}, // top face of cube
    {-64, +64, -64},
    {+64, -64, -64},
    {+64, +64, -64},
  };
  const auto cubeBottomFace = std::vector<vm::vec3d>{
    {-64, -64, -192}, // bottom face of cube
    {-64, +64, -192},
    {+64, -64, -192},
    {+64, +64, -192},
  };

  const auto vertexPositions =
    kdl::vec_concat(smallerTopPolygon, cubeTopFace, cubeBottomFace);

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  // Try to move the top face down along the Z axis
  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
  // Move top through the polyhedron and out the bottom
  assertCanNotMoveTopFace(brush, {0, 0, -257});

  // Move the smaller top polygon as 4 separate vertices
  assertCanMoveVertices(brush, smallerTopPolygon, {0, 0, -127});
  assertMovingVerticesDeletes(brush, smallerTopPolygon, {0, 0, -128});
  assertMovingVerticesDeletes(brush, smallerTopPolygon, {0, 0, -129});
  // Move through the polyhedron and out the bottom
  assertCanNotMoveVertices(brush, smallerTopPolygon, {0, 0, -257});

  // Move top face along the X axis
  assertCanMoveTopFace(brush, {32, 0, 0});
  assertCanMoveTopFace(brush, {256, 0.0, 0.0});
  // Causes face merging and a vert to be deleted at z=-64
  assertCanMoveTopFace(brush, {-32, -32, 0});
}

TEST_CASE("BrushTest.moveTwoFaces")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  //               //
  // |\    z = 64  //
  // | \           //
  // |  \          //
  // A|   \ z = 0   //
  // |   /         //
  // |__/C         //
  //  B    z = -64 //
  //               //

  const auto leftPolygon = std::vector<vm::vec3d>{
    // A
    {-32, -32, +64},
    {-32, +32, +64},
    {-32, +32, -64},
    {-32, -32, -64},
  };
  const auto bottomPolygon = std::vector<vm::vec3d>{
    // B
    {-32, -32, -64},
    {-32, +32, -64},
    {+0, +32, -64},
    {+0, -32, -64},
  };
  const auto bottomRightPolygon = std::vector<vm::vec3d>{
    // C
    {+0, -32, -64},
    {+0, +32, -64},
    {+32, +32, +0},
    {+32, -32, +0},
  };

  const auto vertexPositions =
    kdl::vec_concat(leftPolygon, bottomPolygon, bottomRightPolygon);

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush = builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
               | kdl::value();

  CHECK(brush.hasFace(vm::polygon3d{leftPolygon}));
  CHECK(brush.hasFace(vm::polygon3d{bottomPolygon}));
  CHECK(brush.hasFace(vm::polygon3d{bottomRightPolygon}));

  assertCanMoveFaces(
    brush, {vm::polygon3d{leftPolygon}, vm::polygon3d{bottomPolygon}}, {0, 0, 63});
  // Merges B and C
  assertCanNotMoveFaces(
    brush, {vm::polygon3d{leftPolygon}, vm::polygon3d{bottomPolygon}}, {0, 0, 64});
}

// "Move polyhedron" tests

TEST_CASE("BrushNodeTest.movePolyhedronRemainingEdge")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  // Edge to the left of the cube, shorter, extends down to Z=-256
  const auto edge = vm::segment3d{{-128, 0, -256}, {-128, 0, 0}};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto brush =
    builder.createCube(128, BrushFaceAttributes::NoMaterialName) | kdl::value();
  CHECK(brush.addVertex(worldBounds, edge.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge.end()).is_success());

  CHECK(brush.vertexCount() == 10u);

  const auto cubeTopIndex = brush.findFace(vm::vec3d{0, 0, 1});
  const auto cubeBottomIndex = brush.findFace(vm::vec3d{0, 0, -1});
  const auto cubeRightIndex = brush.findFace(vm::vec3d{1, 0, 0});
  const auto cubeLeftIndex = brush.findFace(vm::vec3d{-1, 0, 0});
  const auto cubeBackIndex = brush.findFace(vm::vec3d{0, 1, 0});
  const auto cubeFrontIndex = brush.findFace(vm::vec3d{0, -1, 0});

  CHECK(cubeTopIndex);
  CHECK_FALSE(cubeBottomIndex); // no face here, part of the wedge connecting to `edge`
  CHECK(cubeRightIndex);
  CHECK_FALSE(cubeLeftIndex); // no face here, part of the wedge connecting to `edge`
  CHECK(cubeFrontIndex);
  CHECK(cubeBackIndex);

  const auto& cubeTop = brush.face(*cubeTopIndex);
  const auto& cubeRight = brush.face(*cubeRightIndex);
  const auto& cubeFront = brush.face(*cubeFrontIndex);
  const auto& cubeBack = brush.face(*cubeBackIndex);

  const auto movingFaces = std::vector<vm::polygon3d>{
    cubeTop.polygon(),
    cubeRight.polygon(),
    cubeFront.polygon(),
    cubeBack.polygon(),
  };

  assertCanMoveFaces(brush, movingFaces, {32, 0, 0});  // away from `edge`
  assertCanMoveFaces(brush, movingFaces, {-63, 0, 0}); // towards `edge`, not touching
  assertCanMoveFaces(brush, movingFaces, {-64, 0, 0}); // towards `edge`, touching
  assertCanMoveFaces(brush, movingFaces, {-65, 0, 0}); // towards `edge`, covering

  // Move the cube down 64 units, so the top vertex of `edge` is on the same plane as
  // `cubeTop` This will turn `cubeTop` from a quad into a pentagon
  assertCanNotMoveFaces(brush, movingFaces, {0, 0, -64});
  assertCanMoveVertices(brush, asVertexList(movingFaces), {0, 0, -64});

  // Make edge poke through the top face
  assertCanNotMoveFaces(brush, movingFaces, {-192, 0, -128});
  assertCanNotMoveVertices(brush, asVertexList(movingFaces), {-192, 0, -128});
}

// UV Lock tests

TEST_CASE("moveFaceWithUVLock")
{
  auto format = GENERATE(MapFormat::Valve, MapFormat::Standard);

  const auto worldBounds = vm::bbox3d{4096.0};

  auto textureResource = createTextureResource(Texture{64, 64});
  auto testMaterial = Material{"testMaterial", std::move(textureResource)};

  auto builder = BrushBuilder{format, worldBounds};
  auto brush = builder.createCube(64.0, "") | kdl::value();
  for (auto& face : brush.faces())
  {
    face.setMaterial(&testMaterial);
  }

  const auto delta = vm::vec3d{+8, 0, 0};
  const auto transform = vm::translation_matrix(delta);

  const auto polygonToMove =
    vm::polygon3d{brush.face(*brush.findFace(vm::vec3d{0, 0, 1})).vertexPositions()};
  CHECK(brush.canTransformFaces(worldBounds, {polygonToMove}, transform));

  // move top face by x=+8
  auto changed = brush;
  auto changedWithUVLock = brush;

  REQUIRE(
    changed.transformFaces(worldBounds, {polygonToMove}, transform, false).is_success());
  REQUIRE(changedWithUVLock.transformFaces(worldBounds, {polygonToMove}, transform, true)
            .is_success());

  // The move should be equivalent to shearing by this matrix
  const auto M = vm::shear_bbox_matrix(brush.bounds(), vm::vec3d{0, 0, 1}, delta);

  for (auto& oldFace : brush.faces())
  {
    const auto oldUVCoords = kdl::vec_transform(
      oldFace.vertexPositions(), [&](auto x) { return oldFace.uvCoords(x); });
    const auto shearedVertexPositions =
      kdl::vec_transform(oldFace.vertexPositions(), [&](auto x) { return M * x; });
    const auto shearedPolygon = vm::polygon3d{shearedVertexPositions};

    const auto normal = oldFace.boundary().normal;

    // The brush modified without alignment lock is expected to have changed UV's on
    // some faces, but not on others
    {
      const auto newFaceIndex = changed.findFace(shearedPolygon);
      REQUIRE(newFaceIndex);
      const auto& newFace = changed.face(*newFaceIndex);
      const auto newUVCoords = kdl::vec_transform(
        shearedVertexPositions, [&](auto x) { return newFace.uvCoords(x); });
      if (
        normal == vm::vec3d{0, 0, 1} || normal == vm::vec3d{0, 1, 0}
        || normal == vm::vec3d{0, -1, 0})
      {
        CHECK_FALSE(uvListsEqual(oldUVCoords, newUVCoords));
        // TODO: actually check the UV's
      }
      else
      {
        CHECK(uvListsEqual(oldUVCoords, newUVCoords));
      }
    }

    // UV's should all be the same when using alignment lock (with Valve format).
    // Standard format can only do UV lock on the top face, which is not sheared.
    {
      const auto newFaceWithUVLockIndex = changedWithUVLock.findFace(shearedPolygon);
      REQUIRE(newFaceWithUVLockIndex);
      const auto& newFaceWithUVLock = changedWithUVLock.face(*newFaceWithUVLockIndex);
      const auto newUVCoordsWithUVLock = kdl::vec_transform(
        shearedVertexPositions, [&](auto x) { return newFaceWithUVLock.uvCoords(x); });
      if (normal == vm::vec3d{0, 0, 1} || (format == MapFormat::Valve))
      {
        CHECK(uvListsEqual(oldUVCoords, newUVCoordsWithUVLock));
      }
    }
  }
}

TEST_CASE("BrushTest.subtractCuboidFromCuboid")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto minuendMaterial = std::string{"minuend"};
  const auto subtrahendMaterial = std::string{"subtrahend"};
  const auto defaultMaterial = std::string{"default"};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  const auto minuend =
    builder.createCuboid(vm::bbox3d{{-32, -16, -32}, {32, 16, 32}}, minuendMaterial)
    | kdl::value();
  const auto subtrahend =
    builder.createCuboid(vm::bbox3d{{-16, -32, -64}, {16, 32, 0}}, subtrahendMaterial)
    | kdl::value();

  const auto fragments =
    minuend.subtract(MapFormat::Standard, worldBounds, defaultMaterial, subtrahend)
    | kdl::fold | kdl::value();
  CHECK(fragments.size() == 3u);

  const Brush* left = nullptr;
  const Brush* top = nullptr;
  const Brush* right = nullptr;

  for (const auto& brush : fragments)
  {
    if (brush.findFace(vm::plane3d{32.0, vm::vec3d{-1, 0, 0}}))
    {
      left = &brush;
    }
    else if (brush.findFace(vm::plane3d{32.0, vm::vec3d{1, 0, 0}}))
    {
      right = &brush;
    }
    else if (brush.findFace(vm::plane3d{16.0, vm::vec3d{-1, 0, 0}}))
    {
      top = &brush;
    }
  }

  CHECK(left != nullptr);
  CHECK(top != nullptr);
  CHECK(right != nullptr);

  // left brush faces
  CHECK(left->faceCount() == 6u);
  CHECK(left->findFace(vm::plane3d{-16.0, vm::vec3d{1, 0, 0}}));
  CHECK(left->findFace(vm::plane3d{+32.0, vm::vec3d{-1, 0, 0}}));
  CHECK(left->findFace(vm::plane3d{+16.0, vm::vec3d{0, 1, 0}}));
  CHECK(left->findFace(vm::plane3d{+16.0, vm::vec3d{0, -1, 0}}));
  CHECK(left->findFace(vm::plane3d{+32.0, vm::vec3d{0, 0, 1}}));
  CHECK(left->findFace(vm::plane3d{+32.0, vm::vec3d{0, 0, -1}}));

  // left brush materials
  CHECK(
    left->face(*left->findFace(vm::vec3d{1, 0, 0})).attributes().materialName()
    == subtrahendMaterial);
  CHECK(
    left->face(*left->findFace(vm::vec3d{-1, 0, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    left->face(*left->findFace(vm::vec3d{0, 1, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    left->face(*left->findFace(vm::vec3d{0, -1, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    left->face(*left->findFace(vm::vec3d{0, 0, 1})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    left->face(*left->findFace(vm::vec3d{0, 0, -1})).attributes().materialName()
    == minuendMaterial);

  // top brush faces
  CHECK(top->faceCount() == 6u);
  CHECK(top->findFace(vm::plane3d{16.0, vm::vec3d{1, 0, 0}}));
  CHECK(top->findFace(vm::plane3d{16.0, vm::vec3d{-1, 0, 0}}));
  CHECK(top->findFace(vm::plane3d{16.0, vm::vec3d{0, 1, 0}}));
  CHECK(top->findFace(vm::plane3d{16.0, vm::vec3d{0, -1, 0}}));
  CHECK(top->findFace(vm::plane3d{32.0, vm::vec3d{0, 0, 1}}));
  CHECK(top->findFace(vm::plane3d{.0, vm::vec3d{0, 0, -1}}));

  // top brush materials
  CHECK(
    top->face(*top->findFace(vm::vec3d{1, 0, 0})).attributes().materialName()
    == subtrahendMaterial);
  CHECK(
    top->face(*top->findFace(vm::vec3d{-1, 0, 0})).attributes().materialName()
    == subtrahendMaterial);
  CHECK(
    top->face(*top->findFace(vm::vec3d{0, 1, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    top->face(*top->findFace(vm::vec3d{0, -1, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    top->face(*top->findFace(vm::vec3d{0, 0, 1})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    top->face(*top->findFace(vm::vec3d{0, 0, -1})).attributes().materialName()
    == subtrahendMaterial);

  // right brush faces
  CHECK(right->faceCount() == 6u);
  CHECK(right->findFace(vm::plane3d{+32.0, vm::vec3d{1, 0, 0}}));
  CHECK(right->findFace(vm::plane3d{-16.0, vm::vec3d{-1, 0, 0}}));
  CHECK(right->findFace(vm::plane3d{+16.0, vm::vec3d{0, 1, 0}}));
  CHECK(right->findFace(vm::plane3d{+16.0, vm::vec3d{0, -1, 0}}));
  CHECK(right->findFace(vm::plane3d{+32.0, vm::vec3d{0, 0, 1}}));
  CHECK(right->findFace(vm::plane3d{+32.0, vm::vec3d{0, 0, -1}}));

  // right brush materials
  CHECK(
    right->face(*right->findFace(vm::vec3d{1, 0, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    right->face(*right->findFace(vm::vec3d{-1, 0, 0})).attributes().materialName()
    == subtrahendMaterial);
  CHECK(
    right->face(*right->findFace(vm::vec3d{0, 1, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    right->face(*right->findFace(vm::vec3d{0, -1, 0})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    right->face(*right->findFace(vm::vec3d{0, 0, 1})).attributes().materialName()
    == minuendMaterial);
  CHECK(
    right->face(*right->findFace(vm::vec3d{0, 0, -1})).attributes().materialName()
    == minuendMaterial);
}

TEST_CASE("BrushTest.subtractDisjoint")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto brush1Bounds = vm::bbox3d{{-8, -8, -8}, {8, 8, 8}};
  const auto brush2Bounds = vm::bbox3d{{124, 124, -4}, {132, 132, +4}};
  CHECK_FALSE(brush1Bounds.intersects(brush2Bounds));

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  const auto brush1 = builder.createCuboid(brush1Bounds, "material") | kdl::value();
  const Brush brush2 = builder.createCuboid(brush2Bounds, "material") | kdl::value();

  const auto fragments =
    brush1.subtract(MapFormat::Standard, worldBounds, "material", brush2) | kdl::fold
    | kdl::value();
  CHECK(fragments.size() == 1u);

  const auto& subtraction = fragments.at(0);
  CHECK_THAT(
    subtraction.vertexPositions(), Catch::UnorderedEquals(brush1.vertexPositions()));
}

TEST_CASE("BrushTest.subtractEnclosed")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  const auto brush1Bounds = vm::bbox3d{{-8, -8, -8}, {8, 8, 8}};
  const auto brush2Bounds = vm::bbox3d{{-9, -9, -9}, {9, 9, 9}};
  CHECK(brush1Bounds.intersects(brush2Bounds));

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  const auto brush1 = builder.createCuboid(brush1Bounds, "material") | kdl::value();
  const auto brush2 = builder.createCuboid(brush2Bounds, "material") | kdl::value();

  const auto fragments =
    brush1.subtract(MapFormat::Standard, worldBounds, "material", brush2) | kdl::fold
    | kdl::value();
  CHECK(fragments.empty());
}

} // namespace tb::mdl
