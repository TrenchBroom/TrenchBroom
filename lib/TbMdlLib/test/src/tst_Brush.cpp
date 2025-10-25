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

#include "Matchers.h"
#include "TestParserStatus.h"
#include "fs/TestUtils.h"
#include "gl/Material.h"
#include "gl/Texture.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/NodeReader.h"
#include "mdl/TestUtils.h"

#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/result_fold.h"
#include "kd/vector_utils.h"

#include "vm/approx.h"
#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

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

  REQUIRE(brush.transformVertices(worldBounds, vertexPositions, transform));

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

  REQUIRE(brush.transformVertices(worldBounds, vertexPositions, transform));
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

void assertCanMoveEdges(
  Brush brush, const std::vector<vm::segment3d> edges, const vm::vec3d delta)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  const auto transform = vm::translation_matrix(delta);

  const auto expectedMovedEdges =
    edges | std::views::transform([&](const auto& edge) { return edge.translate(delta); })
    | kdl::ranges::to<std::vector>();

  CHECK(brush.canTransformEdges(worldBounds, edges, transform));
  CHECK(brush.transformEdges(worldBounds, edges, transform));
  const auto movedEdges = brush.findClosestEdgePositions(
    edges | std::views::transform([&](const auto& s) { return s.translate(delta); })
    | kdl::ranges::to<std::vector>());
  CHECK(movedEdges == expectedMovedEdges);
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
    | kdl::ranges::to<std::vector>();

  CHECK(brush.canTransformFaces(worldBounds, movingFaces, transform));
  CHECK(brush.transformFaces(worldBounds, movingFaces, transform));
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

void assertCannotSnapTo(
  const std::string& data, const double gridSize, kdl::task_manager& taskManager)
{
  const auto worldBounds = vm::bbox3d{8192.0};

  auto status = TestParserStatus{};

  const auto nodes =
    NodeReader::read({}, data, MapFormat::Standard, worldBounds, {}, status, taskManager);
  REQUIRE(nodes);
  CHECK(nodes.value().size() == 1u);

  auto brush = static_cast<BrushNode*>(nodes.value().front())->brush();
  CHECK_FALSE(brush.canSnapVertices(worldBounds, gridSize));

  kdl::col_delete_all(nodes.value());
}

void assertCannotSnap(const std::string& data, kdl::task_manager& taskManager)
{
  assertCannotSnapTo(data, 1, taskManager);
}

void assertSnapTo(
  const std::string& data, const double gridSize, kdl::task_manager& taskManager)
{
  const auto worldBounds = vm::bbox3d{8192.0};

  auto status = TestParserStatus{};

  const auto nodes =
    NodeReader::read({}, data, MapFormat::Standard, worldBounds, {}, status, taskManager);
  REQUIRE(nodes);
  REQUIRE(nodes.value().size() == 1u);

  auto brush = static_cast<BrushNode*>(nodes.value().front())->brush();
  CHECK(brush.canSnapVertices(worldBounds, gridSize));

  CHECK(brush.snapVertices(worldBounds, gridSize));
  CHECK(brush.fullySpecified());

  // Ensure they were actually snapped
  {
    for (const auto* vertex : brush.vertices())
    {
      const vm::vec3d& pos = vertex->position();
      CHECK(vm::is_integral(pos, 0.001));
    }
  }

  kdl::col_delete_all(nodes.value());
}

void assertSnapToInteger(const std::string& data, kdl::task_manager& taskManager)
{
  assertSnapTo(data, 1, taskManager);
}

template <MapFormat F>
class UVLockTest
{
  MapFormat param = F;
};

} // namespace

TEST_CASE("Brush")
{
  SECTION("constructor")
  {
    SECTION("With faces")
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

    SECTION("With redundant faces")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      CHECK(
        Brush::create(
          worldBounds,
          {
            createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
            createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
            createParaxial(vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}),
          })
          .is_error());
    }
  }

  SECTION("cloneFaceAttributesFrom")
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

  SECTION("clip")
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

    auto clip =
      createParaxial(vm::vec3d{8, 0, 0}, vm::vec3d{8, 0, 1}, vm::vec3d{8, 1, 0});
    CHECK(brush.clip(worldBounds, clip));

    CHECK(brush.faceCount() == 6u);
    CHECK(brush.findFace(left.boundary()));
    CHECK(brush.findFace(clip.boundary()));
    CHECK(brush.findFace(front.boundary()));
    CHECK(brush.findFace(back.boundary()));
    CHECK(brush.findFace(top.boundary()));
    CHECK(brush.findFace(bottom.boundary()));
    CHECK_FALSE(brush.findFace(right.boundary()));
  }

  SECTION("moveBoundary")
  {
    SECTION("Move faces successfully")
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
            createParaxial(
              vm::vec3d{0, 0, 6}, vm::vec3d{0, 1, 6}, vm::vec3d{1, 0, 6}), // top
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

      CHECK(brush.moveBoundary(worldBounds, *topFaceIndex, vm::vec3d{0, 0, 1}, false));
      CHECK(worldBounds.contains(brush.bounds()));

      CHECK(brush.faces().size() == 6u);
      CHECK(brush.bounds().size().z() == 7.0);
    }

    SECTION("Can't move faces past world bounds")
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
      CHECK(
        !canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3d{8000, 0, 0}));
    }
  }

  SECTION("expand")
  {
    SECTION("Expand outwards")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

      auto brush1 =
        builder.createCuboid(vm::bbox3d{{-64, -64, -64}, {64, 64, 64}}, "material")
        | kdl::value();
      CHECK(brush1.expand(worldBounds, 6, true));

      const auto expandedBBox = vm::bbox3d{{-70, -70, -70}, {70, 70, 70}};
      const auto expectedVerticesArray = expandedBBox.vertices();
      const auto expectedVertices = std::vector<vm::vec3d>{
        expectedVerticesArray.begin(), expectedVerticesArray.end()};

      CHECK(brush1.bounds() == expandedBBox);
      CHECK_THAT(brush1.vertexPositions(), UnorderedEquals(expectedVertices));
    }

    SECTION("Expand inwards")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

      auto brush1 =
        builder.createCuboid(vm::bbox3d{{-64, -64, -64}, {64, 64, 64}}, "material")
        | kdl::value();
      CHECK(brush1.expand(worldBounds, -32, true));

      const auto expandedBBox = vm::bbox3d{{-32, -32, -32}, {32, 32, 32}};
      const auto expectedVerticesArray = expandedBBox.vertices();
      const auto expectedVertices = std::vector<vm::vec3d>{
        expectedVerticesArray.begin(), expectedVerticesArray.end()};

      CHECK(brush1.bounds() == expandedBBox);
      CHECK_THAT(brush1.vertexPositions(), UnorderedEquals(expectedVertices));
    }

    SECTION("Can't make invalid brush by expanding")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

      auto brush1 =
        builder.createCuboid(vm::bbox3d{{-64, -64, -64}, {64, 64, 64}}, "material")
        | kdl::value();
      CHECK(brush1.expand(worldBounds, -64, true).is_error());
    }
  }

  SECTION("transformVertices")
  {
    SECTION("Move vertex onto adjacent vertex and back")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush =
        builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom")
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

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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
      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, inverse));
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

    SECTION("Move a vertx of a tetrahedron onto the opposing side")
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

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
      auto newVertexPositions =
        brush.findClosestVertexPositions(transform * oldVertexPositions);

      CHECK(newVertexPositions.size() == 1u);
      CHECK(newVertexPositions[0] == vm::approx{vm::vec3d{0, 0, -16}});
      CHECK(brush.fullySpecified());
    }

    SECTION("Move a vertex inward without merging anything")
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

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex outward without merging anything")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex up and merge one incident face")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex outward and merge two incident faces")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex outward and merge all incident faces")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>{p8};
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex inward and merge all incident faces, removing the vertex")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move vertex upwards and re-split the incident face")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex onto an edge")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex onto an adjacent vertex")
    {
      const auto p1 = vm::vec3d{-64, -64, -64};
      const auto p2 = vm::vec3d{-64, -64, +64};
      const auto p3 = vm::vec3d{-64, +64, -64};
      const auto p4 = vm::vec3d{-64, +64, +64};
      const auto p5 = vm::vec3d{+64, -64, -64};
      const auto p6 = vm::vec3d{+64, -64, +64};
      const auto p7 = vm::vec3d{+64, +64, -64};
      const auto p8 = vm::vec3d{+64, +64, +64};

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
      const auto transform = vm::translation_matrix(p7 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex onto an adjacent vertex (opposite direction)")
    {
      const auto p1 = vm::vec3d{-64, -64, -64};
      const auto p2 = vm::vec3d{-64, -64, +64};
      const auto p3 = vm::vec3d{-64, +64, -64};
      const auto p4 = vm::vec3d{-64, +64, +64};
      const auto p5 = vm::vec3d{+64, -64, -64};
      const auto p6 = vm::vec3d{+64, -64, +64};
      const auto p7 = vm::vec3d{+64, +64, -64};
      const auto p8 = vm::vec3d{+64, +64, +64};

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p7});
      const auto transform = vm::translation_matrix(p8 - p7);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex and merge colinear edges without removing the vertex")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p6});
      const auto transform = vm::translation_matrix(p9 - p6);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex and merge colinear edges without removing the vertex 2")
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

      const auto originalPositions =
        std::vector<vm::vec3d>{p1, p2, p3, p4, p5, p6, p7, p8};

      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createBrush(originalPositions, "material") | kdl::value();

      const auto oldVertexPositions = std::vector<vm::vec3d>({p8});
      const auto transform = vm::translation_matrix(p9 - p8);

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Move a vertex and merge colinear edges, removing the vertex")
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

      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Can't move vertices past world bounds")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

      auto brush = builder.createCube(128.0, "material") | kdl::value();

      const auto allVertexPositions =
        brush.vertices()
        | std::views::transform([](const auto* vertex) { return vertex->position(); })
        | kdl::ranges::to<std::vector>();

      CHECK(brush.canTransformVertices(
        worldBounds, allVertexPositions, vm::translation_matrix(vm::vec3d{16, 0, 0})));
      CHECK_FALSE(brush.canTransformVertices(
        worldBounds, allVertexPositions, vm::translation_matrix(vm::vec3d{8192, 0, 0})));
    }

    SECTION("Can't move a vertex through a brush with more than four vertices")
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
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

    SECTION("Move a tetrahedron vertex, flipping the orientation")
    {
      // NOTE: Different than the previous one, because in this case we allow
      // point moves that flip the normal of the remaining polygon
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
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
        std::ranges::reverse(temp);
        const auto flippedBaseQuadVertexPositions = std::vector<vm::vec3d>{temp};

        const auto transform = vm::translation_matrix(vm::vec3d{0.0, 0.0, -129.0});

        CHECK(brushCopy.faceCount() == 5u);
        CHECK(brushCopy.findFace(vm::polygon3d{baseQuadVertexPositions}));
        CHECK_FALSE(brushCopy.findFace(vm::polygon3d{flippedBaseQuadVertexPositions}));
        CHECK(brushCopy.findFace(vm::vec3d{0, 0, -1}));
        CHECK_FALSE(brushCopy.findFace(vm::vec3d{0, 0, 1}));

        const auto oldVertexPositions = std::vector<vm::vec3d>{peakPosition};
        CHECK(brushCopy.canTransformVertices(worldBounds, oldVertexPositions, transform));
        REQUIRE(brushCopy.transformVertices(worldBounds, oldVertexPositions, transform));
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

    SECTION("Rotate vertices")
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
      const auto transform =
        vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(angle));

      REQUIRE(brush.canTransformVertices(worldBounds, oldVertexPositions, transform));
      CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
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
  }

  SECTION("removeVertices")
  {
    SECTION("Remove a single vertex")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush = builder.createCube(64.0, "asdf") | kdl::value();

      CHECK(brush.removeVertices(worldBounds, {vm::vec3d{+32, +32, +32}}));

      CHECK(brush.vertexCount() == 7u);
      CHECK(brush.hasVertex(vm::vec3d{-32, -32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{+32, -32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{+32, +32, -32}));
      CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

      CHECK(brush.removeVertices(worldBounds, {vm::vec3d{+32, +32, -32}}));

      CHECK(brush.vertexCount() == 6u);
      CHECK(brush.hasVertex(vm::vec3d{-32, -32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{+32, -32, +32}));
      CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, -32}));
      CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

      CHECK(brush.removeVertices(worldBounds, {vm::vec3d{+32, -32, +32}}));

      CHECK(brush.vertexCount() == 5u);
      CHECK(brush.hasVertex(vm::vec3d{-32, -32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, -32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, +32, -32}));
      CHECK(brush.hasVertex(vm::vec3d{-32, +32, +32}));
      CHECK(brush.hasVertex(vm::vec3d{+32, -32, -32}));
      CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, -32, +32}));
      CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, -32}));
      CHECK_FALSE(brush.hasVertex(vm::vec3d{+32, +32, +32}));

      CHECK(brush.removeVertices(worldBounds, {vm::vec3d{-32, -32, -32}}));

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

    SECTION("Remove multiple vertices")
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
            CHECK(brush.removeVertices(worldBounds, toRemove));

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
  }

  SECTION("transformEdges")
  {
    SECTION("Move a single edge")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush =
        builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom")
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

      CHECK(brush.transformEdges(worldBounds, oldEdgePositions, transform));
      auto newEdgePositions = brush.findClosestEdgePositions(
        oldEdgePositions
        | std::views::transform([&](const auto& s) { return s.transform(transform); })
        | kdl::ranges::to<std::vector>());

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
      CHECK(brush.transformEdges(worldBounds, oldEdgePositions, inverse));
      newEdgePositions = brush.findClosestEdgePositions(
        oldEdgePositions
        | std::views::transform([&](const auto& s) { return s.transform(inverse); })
        | kdl::ranges::to<std::vector>());

      CHECK(newEdgePositions == std::vector<vm::segment3d>{originalEdge});

      assertMaterial("left", brush, p1, p2, p4, p3);
      assertMaterial("right", brush, p5, p7, p8, p6);
      assertMaterial("front", brush, p1, p5, p6, p2);
      assertMaterial("back", brush, p3, p4, p8, p7);
      assertMaterial("top", brush, p2, p6, p8, p4);
      assertMaterial("bottom", brush, p1, p3, p7, p5);
    }

    SECTION("Can't move an edge if it would be removed")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      // Taller than the cube, starts to the left of the +-64 unit cube
      const auto edge = vm::segment3d{{-128, 0, -128}, {-128, 0, +128}};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush =
        builder.createCube(128, BrushFaceAttributes::NoMaterialName) | kdl::value();
      CHECK(brush.addVertex(worldBounds, edge.start()));
      CHECK(brush.addVertex(worldBounds, edge.end()));

      CHECK(brush.vertexCount() == 10u);

      assertCanMoveEdges(brush, {edge}, {+63, 0, 0});
      assertCanNotMoveEdges(brush, {edge}, {+64, 0, 0});  // On the side of the cube
      assertCanNotMoveEdges(brush, {edge}, {+128, 0, 0}); // Center of the cube

      assertCanMoveVertices(brush, asVertexList({edge}), {+63, 0, 0});
      assertCanMoveVertices(brush, asVertexList({edge}), {+64, 0, 0});
      assertCanMoveVertices(brush, asVertexList({edge}), {+128, 0, 0});
    }

    SECTION("Can't move two edges if any of them would be removed")
    {
      // Same as above, but moving 2 edges
      const auto worldBounds = vm::bbox3d{4096.0};

      // Taller than the cube, starts to the left of the +-64 unit cube
      const auto edge1 = vm::segment3d{{-128, -32, -128}, {-128, -32, +128}};
      const auto edge2 = vm::segment3d{{-128, +32, -128}, {-128, +32, +128}};
      const auto movingEdges = std::vector<vm::segment3d>{edge1, edge2};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush =
        builder.createCube(128, BrushFaceAttributes::NoMaterialName) | kdl::value();
      CHECK(brush.addVertex(worldBounds, edge1.start()));
      CHECK(brush.addVertex(worldBounds, edge1.end()));
      CHECK(brush.addVertex(worldBounds, edge2.start()));
      CHECK(brush.addVertex(worldBounds, edge2.end()));

      CHECK(brush.vertexCount() == 12u);

      assertCanMoveEdges(brush, movingEdges, {+63, 0, 0});
      assertCanNotMoveEdges(brush, movingEdges, {+64, 0, 0});  // On the side of the cube
      assertCanNotMoveEdges(brush, movingEdges, {+128, 0, 0}); // Center of the cube

      assertCanMoveVertices(brush, asVertexList(movingEdges), {+63, 0, 0});
      assertCanMoveVertices(brush, asVertexList(movingEdges), {+64, 0, 0});
      assertCanMoveVertices(brush, asVertexList(movingEdges), {+128, 0, 0});
    }
  }

  SECTION("transformFaces")
  {
    SECTION("Move a single face")
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
      CHECK(brush.transformFaces(worldBounds, oldFacePositions, transform));
      auto newFacePositions = brush.findClosestFacePositions(
        oldFacePositions
        | std::views::transform([&](const auto& f) { return f.transform(transform); })
        | kdl::ranges::to<std::vector>());

      CHECK(newFacePositions.size() == 1u);
      CHECK(newFacePositions[0].hasVertex({-48, -48, +32}));
      CHECK(newFacePositions[0].hasVertex({-48, +16, +32}));
      CHECK(newFacePositions[0].hasVertex({+16, +16, +32}));
      CHECK(newFacePositions[0].hasVertex({+16, -48, +32}));

      oldFacePositions = std::move(newFacePositions);
      CHECK(brush.transformFaces(worldBounds, oldFacePositions, inverse));
      newFacePositions = brush.findClosestFacePositions(
        oldFacePositions
        | std::views::transform([&](const auto& f) { return f.transform(inverse); })
        | kdl::ranges::to<std::vector>());

      CHECK(newFacePositions.size() == 1u);
      CHECK(newFacePositions[0].vertices().size() == 4u);
      for (size_t i = 0; i < 4; ++i)
      {
        CHECK(newFacePositions[0].hasVertex(face.vertices()[i]));
      }
    }

    SECTION("Cannot move face if remaining brush is empty")
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

    SECTION("Cannot move a face of a tetrahedron beyond the opposing vertex")
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
        | kdl::value();

      assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
    }

    SECTION("Cannot move a face of a brush beyond an opposing edge")
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
        | kdl::value();

      assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
    }

    SECTION("Cannot move a face of a cube beyond the opposing face")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush =
        builder.createCube(128.0, BrushFaceAttributes::NoMaterialName) | kdl::value();

      assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
    }

    SECTION("Cannot move a face of a cube beyond the opposing face 2")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      // Same brush as above, but this particular order of vertices triggers a failure in
      // Brush::doCanMoveVertices where the polygon inserted into the "remaining"
      // BrushGeometry gets the wrong normal.
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
        | kdl::value();
      CHECK(brush.bounds() == vm::bbox3d{{-64, -64, -64}, {64, 64, 64}});

      assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
    }

    SECTION("Move a face of a brush so that two vertices are merged")
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
        | kdl::value();

      const auto topFaceIndex = brush.findFace(topFaceNormal);
      assertCanMoveFace(brush, topFaceIndex, vm::vec3d{0, 0, -127});
      // Merge 2 verts of the moving polygon with 2 in the remaining polygon, should be
      // allowed
      assertCanMoveFace(brush, topFaceIndex, {0, 0, -128});
      assertCanNotMoveFace(brush, topFaceIndex, vm::vec3d{0, 0, -129});
    }

    SECTION("Cannot move a face of a brush if incident faces would be removed")
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
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

    SECTION("Move two faces at once")
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
      auto brush =
        builder.createBrush(vertexPositions, BrushFaceAttributes::NoMaterialName)
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

    SECTION("Move several faces at once")
    {
      const auto worldBounds = vm::bbox3d{4096.0};

      // Edge to the left of the cube, shorter, extends down to Z=-256
      const auto edge = vm::segment3d{{-128, 0, -256}, {-128, 0, 0}};

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto brush =
        builder.createCube(128, BrushFaceAttributes::NoMaterialName) | kdl::value();
      CHECK(brush.addVertex(worldBounds, edge.start()));
      CHECK(brush.addVertex(worldBounds, edge.end()));

      CHECK(brush.vertexCount() == 10u);

      const auto cubeTopIndex = brush.findFace(vm::vec3d{0, 0, 1});
      const auto cubeBottomIndex = brush.findFace(vm::vec3d{0, 0, -1});
      const auto cubeRightIndex = brush.findFace(vm::vec3d{1, 0, 0});
      const auto cubeLeftIndex = brush.findFace(vm::vec3d{-1, 0, 0});
      const auto cubeBackIndex = brush.findFace(vm::vec3d{0, 1, 0});
      const auto cubeFrontIndex = brush.findFace(vm::vec3d{0, -1, 0});

      CHECK(cubeTopIndex);
      CHECK_FALSE(
        cubeBottomIndex); // no face here, part of the wedge connecting to `edge`
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

    SECTION("Move face with UV lock")
    {
      auto format = GENERATE(MapFormat::Valve, MapFormat::Standard);

      const auto worldBounds = vm::bbox3d{4096.0};

      auto textureResource = gl::createTextureResource(gl::Texture{64, 64});
      auto testMaterial = gl::Material{"testMaterial", std::move(textureResource)};

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

      REQUIRE(changed.transformFaces(worldBounds, {polygonToMove}, transform, false));
      REQUIRE(
        changedWithUVLock.transformFaces(worldBounds, {polygonToMove}, transform, true));

      // The move should be equivalent to shearing by this matrix
      const auto M = vm::shear_bbox_matrix(brush.bounds(), vm::vec3d{0, 0, 1}, delta);

      for (auto& oldFace : brush.faces())
      {
        const auto oldUVCoords =
          oldFace.vertexPositions()
          | std::views::transform([&](auto x) { return oldFace.uvCoords(x); })
          | kdl::ranges::to<std::vector>();

        const auto shearedVertexPositions =
          oldFace.vertexPositions() | std::views::transform([&](auto x) { return M * x; })
          | kdl::ranges::to<std::vector>();

        const auto shearedPolygon = vm::polygon3d{shearedVertexPositions};

        const auto normal = oldFace.boundary().normal;

        // The brush modified without alignment lock is expected to have changed UV's on
        // some faces, but not on others
        {
          const auto newFaceIndex = changed.findFace(shearedPolygon);
          REQUIRE(newFaceIndex);

          const auto& newFace = changed.face(*newFaceIndex);
          const auto newUVCoords =
            shearedVertexPositions
            | std::views::transform([&](auto x) { return newFace.uvCoords(x); })
            | kdl::ranges::to<std::vector>();

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
          const auto newUVCoordsWithUVLock =
            shearedVertexPositions
            | std::views::transform([&](auto x) { return newFaceWithUVLock.uvCoords(x); })
            | kdl::ranges::to<std::vector>();

          if (normal == vm::vec3d{0, 0, 1} || (format == MapFormat::Valve))
          {
            CHECK(uvListsEqual(oldUVCoords, newUVCoordsWithUVLock));
          }
        }
      }
    }
  }

  SECTION("subtract")
  {
    SECTION("Subtract cuboid from cuboid")
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

    SECTION("Subtract disjoint brushes")
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
        subtraction.vertexPositions(), UnorderedEquals(brush1.vertexPositions()));
    }

    SECTION("Subtract contained brushes")
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
  }
}

/*
 Regex to turn a face definition into a c++ statement to add a face to a vector of
 faces: Find:
 \(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*[^\n]+
 Replace: faces.push_back(createParaxial(vm::vec3d{$1, $2, $3}, vm::vec3d{$4, $5, $6},
 vm::vec3d{$7, $8, $9}));
 */

TEST_CASE("Brush (Regression)", "[regression]")
{
  auto taskManager = kdl::task_manager{};

  const auto worldBounds = vm::bbox3d{8192.0};

  SECTION("constructWithFailingFaces")
  {
    const auto brush =
      Brush::create(
        worldBounds,
        {
          createParaxial({-192, 704, 128}, {-156, 650, 128}, {-156, 650, 160}),
          createParaxial({-202, 604, 160}, {-164, 664, 128}, {-216, 613, 128}),
          createParaxial({-156, 650, 128}, {-202, 604, 128}, {-202, 604, 160}),
          createParaxial({-192, 704, 160}, {-256, 640, 160}, {-256, 640, 128}),
          createParaxial({-256, 640, 160}, {-202, 604, 160}, {-202, 604, 128}),
          createParaxial({-217, 672, 160}, {-161, 672, 160}, {-161, 603, 160}),
          createParaxial({-161, 603, 128}, {-161, 672, 128}, {-217, 672, 128}),
        })
      | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 7u);
  }

  SECTION("constructWithFailingFaces2")
  {
    const auto brush =
      Brush::create(
        worldBounds,
        {
          createParaxial({3488, 1152, 1340}, {3488, 1248, 1344}, {3488, 1344, 1340}),
          createParaxial({3232, 1344, 1576}, {3232, 1152, 1576}, {3232, 1152, 1256}),
          createParaxial({3488, 1344, 1576}, {3264, 1344, 1576}, {3264, 1344, 1256}),
          createParaxial({3280, 1152, 1576}, {3504, 1152, 1576}, {3504, 1152, 1256}),
          createParaxial({3488, 1248, 1344}, {3488, 1152, 1340}, {3232, 1152, 1340}),
          createParaxial({3488, 1248, 1344}, {3232, 1248, 1344}, {3232, 1344, 1340}),
          createParaxial({3488, 1152, 1340}, {3360, 1152, 1344}, {3424, 1344, 1342}),
          createParaxial({3360, 1152, 1344}, {3232, 1152, 1340}, {3296, 1344, 1342}),
          createParaxial({3504, 1344, 1280}, {3280, 1344, 1280}, {3280, 1152, 1280}),
        })
      | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 9u);
  }

  SECTION("constructWithFailingFaces3")
  {
    const auto brush =
      Brush::create(
        worldBounds,
        {
          createParaxial({-32, -1088, 896}, {-64, -1120, 896}, {-64, -1120, 912}),
          createParaxial({-32, -832, 896}, {-32, -1088, 896}, {-32, -1088, 912}),
          createParaxial({-64, -848, 912}, {-64, -1120, 912}, {-64, -1120, 896}),
          createParaxial({-32, -896, 896}, {-32, -912, 912}, {-64, -912, 912}),
          createParaxial({-64, -1088, 912}, {-64, -848, 912}, {-32, -848, 912}),
          createParaxial({-64, -864, 896}, {-32, -864, 896}, {-32, -832, 896}),
        })
      | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 6u);
  }

  SECTION("constructWithFailingFaces4")
  {
    const auto brush =
      Brush::create(
        worldBounds,
        {
          createParaxial({-1268, 272, 2524}, {-1268, 272, 2536}, {-1268, 288, 2540}),
          createParaxial({-1280, 265, 2534}, {-1268, 272, 2524}, {-1268, 288, 2528}),
          createParaxial({-1268, 288, 2528}, {-1280, 288, 2540}, {-1280, 265, 2534}),
          createParaxial({-1268, 288, 2540}, {-1280, 288, 2540}, {-1280, 288, 2536}),
          createParaxial({-1268, 265, 2534}, {-1280, 265, 2534}, {-1280, 288, 2540}),
          createParaxial({-1268, 265, 2534}, {-1268, 272, 2524}, {-1280, 265, 2534}),
        })
      | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 6u);
  }

  SECTION("constructWithFailingFaces5")
  {
    /* from jam6_ericwtronyn
     Interestingly, the order in which the faces appear in the map file is okay, but when
     they get reordered during load, the resulting order leads to a crash. The order below
     is the reordered one.
     */

    const Brush brush =
      Brush::create(
        worldBounds,
        {
          createParaxial({1296, 896, 944}, {1296, 1008, 1056}, {1280, 1008, 1008}),
          createParaxial({1296, 1008, 1168}, {1296, 1008, 1056}, {1296, 896, 944}),
          createParaxial({1280, 1008, 1008}, {1280, 1008, 1168}, {1280, 896, 1056}),
          createParaxial({1280, 1008, 1168}, {1280, 1008, 1008}, {1296, 1008, 1056}),
          createParaxial({1296, 1008, 1168}, {1296, 896, 1056}, {1280, 896, 1056}),
          createParaxial({1280, 896, 896}, {1280, 896, 1056}, {1296, 896, 1056}),
        })
      | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 6u);
  }

  SECTION("constructWithFailingFaces6")
  {
    /* from 768_negke
     {
     ( -80 -80 -3840  ) ( -80 -80 -3824  ) ( -32 -32 -3808 ) mmetal1_2b 0 0 0 1 1 // front
     / right ( -96 -32 -3840  ) ( -96 -32 -3824  ) ( -80 -80 -3824 ) mmetal1_2 0 0 0 1 1
     // left ( -96 -32 -3824  ) ( -32 -32 -3808  ) ( -80 -80 -3824 ) mmetal1_2b 0 0 0 1 1
     // top ( -32 -32 -3840  ) ( -32 -32 -3808  ) ( -96 -32 -3824 ) mmetal1_2b 0 0 0 1 1
     // back ( -32 -32 -3840  ) ( -96 -32 -3840  ) ( -80 -80 -3840 ) mmetal1_2b 0 0 0 1 1
     // bottom
     }
     */

    const auto brush = Brush::create(
                         worldBounds,
                         {
                           createParaxial(
                             vm::vec3d{-80, -80, -3840},
                             vm::vec3d{-80, -80, -3824},
                             vm::vec3d{-32, -32, -3808}),
                           createParaxial(
                             vm::vec3d{-96, -32, -3840},
                             vm::vec3d{-96, -32, -3824},
                             vm::vec3d{-80, -80, -3824}),
                           createParaxial(
                             vm::vec3d{-96, -32, -3824},
                             vm::vec3d{-32, -32, -3808},
                             vm::vec3d{-80, -80, -3824}),
                           createParaxial(
                             vm::vec3d{-32, -32, -3840},
                             vm::vec3d{-32, -32, -3808},
                             vm::vec3d{-96, -32, -3824}),
                           createParaxial(
                             vm::vec3d{-32, -32, -3840},
                             vm::vec3d{-96, -32, -3840},
                             vm::vec3d{-80, -80, -3840}),
                         })
                       | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 5u);
  }

  SECTION("constructBrushWithManySides")
  {
    /*
     See https://github.com/TrenchBroom/TrenchBroom/issues/1153
     The faces have been reordered according to BrushFace::sortFaces and all
     non-interesting faces have been removed from the brush.

     {
     ( 624 688 -456 ) ( 656 760 -480 ) ( 624 680 -480 ) face7 8 0 180 1 -1
     ( 536 792 -480 ) ( 536 792 -432 ) ( 488 720 -480 ) face12 48 0 180 1 -1
     ( 568 656 -464 ) ( 568 648 -480 ) ( 520 672 -456 ) face14 -32 0 -180 1 -1
     ( 520 672 -456 ) ( 520 664 -480 ) ( 488 720 -452 ) face15 8 0 180 1 -1
     ( 560 728 -440 ) ( 488 720 -452 ) ( 536 792 -432 ) face17 -32 -8 -180 1 1
     ( 568 656 -464 ) ( 520 672 -456 ) ( 624 688 -456 ) face19 -32 -8 -180 1 1
     ( 560 728 -440 ) ( 624 688 -456 ) ( 520 672 -456 ) face20 -32 -8 -180 1 1 // assert
     ( 600 840 -480 ) ( 536 792 -480 ) ( 636 812 -480 ) face22 -32 -8 -180 1 1
     }
     */

    const auto brush =
      Brush::create(
        worldBounds,
        {
          createParaxial({624, 688, -456}, {656, 760, -480}, {624, 680, -480}, "face7"),
          createParaxial({536, 792, -480}, {536, 792, -432}, {488, 720, -480}, "face12"),
          createParaxial({568, 656, -464}, {568, 648, -480}, {520, 672, -456}, "face14"),
          createParaxial({520, 672, -456}, {520, 664, -480}, {488, 720, -452}, "face15"),
          createParaxial({560, 728, -440}, {488, 720, -452}, {536, 792, -432}, "face17"),
          createParaxial({568, 656, -464}, {520, 672, -456}, {624, 688, -456}, "face19"),
          createParaxial({560, 728, -440}, {624, 688, -456}, {520, 672, -456}, "face20"),
          createParaxial({600, 840, -480}, {536, 792, -480}, {636, 812, -480}, "face22"),
        })
      | kdl::value();

    REQUIRE(brush.fullySpecified());
    CHECK(brush.faceCount() == 8u);
  }

  SECTION("constructBrushAfterRotateFail")
  {
    /*
     See https://github.com/TrenchBroom/TrenchBroom/issues/1173

     This is the brush after rotation. Rebuilding the geometry should assert.
     */

    const auto brush =
      Brush::create(
        worldBounds,
        {
          createParaxial(
            {-729.68857812925364, -128, 2061.2927432882448},
            {-910.70791411301013, 128, 2242.3120792720015},
            {-820.19824612113155, -128, 1970.7830752963655}),
          createParaxial(
            {-639.17891013737574, -640, 1970.7830752963669},
            {-729.68857812925364, -128, 2061.2927432882448},
            {-729.68857812925364, -640, 1880.2734073044885}),
          createParaxial(
            {-639.17891013737574, -1024, 1970.7830752963669},
            {-820.19824612113177, -640, 2151.8024112801227},
            {-639.17891013737574, -640, 1970.7830752963669}),
          createParaxial(
            {-639.17891013737574, -1024, 1970.7830752963669},
            {-639.17891013737574, -640, 1970.7830752963669},
            {-729.68857812925364, -1024, 1880.2734073044885}),
          createParaxial(
            {-1001.2175821048878, -128, 2151.8024112801222},
            {-910.70791411301013, -128, 2242.3120792720015},
            {-910.70791411300991, -640, 2061.2927432882443}),
          createParaxial(
            {-639.17891013737574, -1024, 1970.7830752963669},
            {-729.68857812925364, -1024, 1880.2734073044885},
            {-820.19824612113177, -640, 2151.8024112801227}), // assertion failure here
          createParaxial(
            {-1001.2175821048878, -128, 2151.8024112801222},
            {-1001.2175821048878, 128, 2151.8024112801222},
            {-910.70791411301013, -128, 2242.3120792720015}),
          createParaxial(
            {-729.68857812925364, -1024, 1880.2734073044885},
            {-729.68857812925364, -640, 1880.2734073044885},
            {-910.70791411300991, -640, 2061.2927432882443}),
        })
      | kdl::value();

    CHECK(brush.fullySpecified());
  }

  SECTION("moveVertexFailing1")
  {
    const auto p1 = vm::vec3d{-64, -64, 0};
    const auto p2 = vm::vec3d{+64, -64, 0};
    const auto p3 = vm::vec3d{0, +64, 0};
    const auto p4 = vm::vec3d{0, 0, +32};

    const auto oldPositions = std::vector<vm::vec3d>{p1, p2, p3, p4};

    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
    auto brush = builder.createBrush(oldPositions, "material") | kdl::value();

    for (size_t i = 0; i < oldPositions.size(); ++i)
    {
      for (size_t j = 0; j < oldPositions.size(); ++j)
      {
        if (i != j)
        {
          CHECK_FALSE(brush.canTransformVertices(
            worldBounds,
            {oldPositions[i]},
            vm::translation_matrix(oldPositions[j] - oldPositions[i])));
        }
      }
    }
  }

  SECTION("moveVertexFail_2158")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/2158
    const auto data = R"(
      {
      ( 320 256 320 ) ( 384 192 320 ) ( 352 224 384 ) sky1 0 96 0 1 1
      ( 384 128 320 ) ( 320 64 320 ) ( 352 96 384 ) sky1 0 96 0 1 1
      ( 384 32 320 ) ( 384 32 384 ) ( 384 256 384 ) sky1 0 96 0 1 1
      ( 192 192 320 ) ( 256 256 320 ) ( 224 224 384 ) sky1 0 96 0 1 1
      ( 256 64 320 ) ( 192 128 320 ) ( 224 96 384 ) sky1 0 96 0 1 1
      ( 192 32 384 ) ( 192 32 320 ) ( 192 256 320 ) sky1 0 96 0 1 1
      ( 384 256 320 ) ( 384 256 384 ) ( 192 256 384 ) sky1 0 96 0 1 1
      ( 320 64 320 ) ( 256 64 320 ) ( 288 64 384 ) sky1 0 96 0 1 1
      ( 192 64 352 ) ( 192 240 352 ) ( 368 240 352 ) sky1 0 0 0 1 1
      ( 384 240 320 ) ( 208 240 320 ) ( 208 64 320 ) sky1 0 0 0 1 1
      })";

    auto status = TestParserStatus{};

    const auto nodes =
      NodeReader::read({}, data, MapFormat::Standard, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    CHECK(nodes.value().size() == 1u);

    auto brush = static_cast<BrushNode*>(nodes.value().front())->brush();
    const auto p = vm::vec3d{192, 128, 352};

    const auto oldVertexPositions = std::vector{p};
    const auto transform = vm::translation_matrix(4.0 * 16.0 * vm::vec3d{0, -1, 0});
    CHECK(brush.transformVertices(worldBounds, oldVertexPositions, transform));
    auto newVertexPositions =
      brush.findClosestVertexPositions(transform * oldVertexPositions);

    CHECK(newVertexPositions.size() == 1u);
    CHECK(newVertexPositions.front() == vm::approx(transform * p));

    kdl::col_delete_all(nodes.value());
  }

  SECTION("moveVerticesFail_2158")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/2158
    const auto data = R"({
    ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 30.92560005187988281 0.960906982421875 5.59741020202636719 0.98696297407150269 0.98029798269271851
    ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 67.89600372314453125 -61.20909881591796875 13.658599853515625 0.85491102933883667 1.12606000900268555
    ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) rock4_2 -3.77819991111755371 -44.42710113525390625 7.24881982803344727 0.95510202646255493 1.04886996746063232
    ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) rock4_2 77.66159820556640625 -86.74199676513671875 173.0970001220703125 1.15471994876861572 -1.11249995231628418
    ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 115.52100372314453125 55.40819931030273438 157.998992919921875 1.19368994235992432 -1.0113600492477417
    ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 29.0522003173828125 16.1511993408203125 198.899993896484375 0.90696299076080322 -1.06921005249023438
    ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1163.51855729802718997 -1820.79407602155902168 554.17919393113811566 ) rock4_2 -52.78820037841796875 -84.4026031494140625 200.2100067138671875 0.88777101039886475 -0.97177797555923462
    ( 1163.51855729802718997 -1820.79407602155902168 554.17919393113811566 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 72.63649749755859375 102.17099761962890625 80.11309814453125 0.87609797716140747 -1.61881005764007568
    ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) rock4_2 -0.7561039924621582 32.18519973754882812 75.325897216796875 0.90074300765991211 -1.72079002857208252
    ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 85.426300048828125 -37.61460113525390625 170.2440032958984375 0.94236099720001221 -1.08232998847961426
    ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 -15.04969978332519531 -12.76039981842041016 176.2700042724609375 0.93921899795532227 -1.1466900110244751
    ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) rock4_2 -1.02465999126434326 60.25889968872070312 159.8549957275390625 0.78085201978683472 -1.21036994457244873
    ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 86.87239837646484375 40.37289810180664062 129.878997802734375 0.66983801126480103 -2.06800007820129395
    ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 77.13539886474609375 119.01000213623046875 358.319000244140625 1.14928996562957764 1.19559001922607422
    ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 -19.27930068969726562 17.50340080261230469 148.16400146484375 1.01748001575469971 -0.89703798294067383
    ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) rock4_2 -43.56299972534179688 -73.20639801025390625 350.87200927734375 0.98191499710083008 1.14552998542785645
    ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -57.5941009521484375 20.35930061340332031 349.8599853515625 0.91973602771759033 1.05388998985290527
    ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) rock4_2 84.4087982177734375 44.97620010375976562 5.90301990509033203 0.94212800264358521 1.00434005260467529
    ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) rock4_2 -4.20452976226806641 26.958099365234375 7.14522981643676758 0.90771502256393433 1.01380002498626709
    ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) rock4_2 -81.561798095703125 -95.4485015869140625 40.62070083618164062 0.5180240273475647 1.46343004703521729
    ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) rock4_2 52.8777008056640625 -9.35947036743164062 58.6305999755859375 0.61474400758743286 1.24004995822906494
    ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) rock4_2 -45.87020111083984375 -44.08499908447265625 41.31510162353515625 0.53462702035903931 1.54106998443603516
    ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) rock4_2 8.81488037109375 37.412200927734375 6.29719018936157227 0.96984899044036865 0.99895197153091431
    ( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) rock4_2 5.92700004577636719 4.41837978363037109 8.78011035919189453 0.7744939923286438 1.05709004402160645
    ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) rock4_2 0.02703860029578209 11.37539958953857422 8.51169967651367188 0.77832400798797607 1.01610994338989258
    ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) rock4_2 75.124298095703125 3.1680600643157959 8.79839038848876953 0.75931602716445923 1.01523995399475098
    ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 -13.265899658203125 -8.93752956390380859 11.75290012359619141 0.59300100803375244 0.97339397668838501
    ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 5.71436023712158203 66.92310333251953125 162.699005126953125 0.74939501285552979 -1.05348002910614014
    ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 47.94699859619140625 80.93849945068359375 350.2969970703125 0.99699199199676514 0.93575799465179443
    ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) rock4_2 -17.06769943237304688 76.29920196533203125 226.9109954833984375 0.86038202047348022 -0.97620397806167603
    ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 17.15080070495605469 78.2032012939453125 226.90899658203125 0.86016601324081421 -0.97621601819992065
    ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 67.65200042724609375 17.70070075988769531 124.0709991455078125 0.93583697080612183 0.99498897790908813
    ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 34.074798583984375 -67.4031982421875 5.12918996810913086 0.89313501119613647 0.99598902463912964
    ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 9.72570991516113281 95.0894012451171875 350.1099853515625 0.99535101652145386 0.97052502632141113
    ( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -2.58533000946044922 7.69421005249023438 349.858001708984375 0.99317502975463867 0.99086099863052368
    ( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 0.29211398959159851 -1.12084996700286865 349.87799072265625 0.99334698915481567 0.98575097322463989
    ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -3.78198003768920898 21.7248992919921875 349.865997314453125 0.9932439923286438 0.99966299533843994
    })";

    auto status = TestParserStatus{};

    auto nodes = NodeReader::read({}, 
      data, MapFormat::Standard, vm::bbox3d{4096.0}, {}, status, taskManager);
    REQUIRE(nodes);
    CHECK(nodes.value().size() == 1u);

    auto brush = static_cast<BrushNode*>(nodes.value().front())->brush();

    const auto vertexPositions = std::vector{
      brush.findClosestVertexPosition(
        {1169.1764156206966, -1800.2961013859342, 568.79748529920892}),
      brush.findClosestVertexPosition(
        {1164.1689509627774, -1797.7259237617193, 578.31488545196294}),
      brush.findClosestVertexPosition(
        {1163.5185572994671, -1820.7940760208414, 554.17919392904093}),
      brush.findClosestVertexPosition(
        {1120.5128684458623, -1855.3192739534061, 574.53563498325116})};

    const auto transform = vm::translation_matrix(vm::vec3d{16, 0, 0});
    CHECK(brush.canTransformVertices(worldBounds, vertexPositions, transform));
    CHECK_NOTHROW(brush.transformVertices(worldBounds, vertexPositions, transform));

    kdl::col_delete_all(nodes.value());
  }

  SECTION("removeVertexWithCorrectMaterials_2082")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/2082

    const auto data = R"(
{
( 32 -32 -0 ) ( -16 -32 -0 ) ( -16 -32 32 ) *04water1 [ -1 0 0 -0.941193 ] [ 0 0 -1 -0 ] 125.468 1 1
( -16 -32 32 ) ( -16 -32 -0 ) ( -32 -16 -0 ) *04mwat2 [ -1 0 0 -0.941193 ] [ 0 0 -1 -0 ] 125.468 1 1
( 32 32 -0 ) ( 32 -32 -0 ) ( 32 -32 32 ) *04water2 [ -2.22045e-16 -1 0 -24.9412 ] [ 0 0 -1 -0 ] 125.468 1 1
( 32 -32 32 ) ( -16 -32 32 ) ( 32 -0 64 ) *teleport [ 0 0 -1 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 -0 64 ) ( -16 -32 32 ) ( -32 -16 32 ) *slime1 [ 0 -1 -2.22045e-16 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 32 -0 ) ( -16 32 -0 ) ( -32 -16 -0 ) *lava1 [ 1 0 0 -0 ] [ 0 -1 0 0.999998 ] -0 1 1
( 32 -0 64 ) ( -16 32 32 ) ( 32 32 32 ) *slime [ 0 -1 2.22045e-16 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 32 32 ) ( -16 32 32 ) ( -16 32 -0 ) *04awater1 [ 0.894427 -0.447214 0 18.9966 ] [ 0 0 -1 -0 ] -0 1 1
( -16 32 -0 ) ( -16 32 32 ) ( -32 -16 32 ) *04mwat1 [ -2.22045e-16 1 0 39588 ] [ 0 0 -1 -0 ] 125.468 1 1
( -32 -16 32 ) ( -16 32 32 ) ( 32 -0 64 ) *slime0 [ -2.43359e-08 -1 0 0.999985 ] [ -1 2.43359e-08 0 -0 ] 90 1 1
}
)";

    auto status = TestParserStatus{};

    auto nodes =
      NodeReader::read({}, data, MapFormat::Valve, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    CHECK(nodes.value().size() == 1u);

    auto brush = static_cast<BrushNode*>(nodes.value().front())->brush();

    const auto p1 = vm::vec3d{32, 32, 0};
    const auto p2 = vm::vec3d{-16, 32, 0};
    const auto p3 = vm::vec3d{-32, -16, 0};
    const auto p4 = vm::vec3d{-16, -32, 0};
    const auto p5 = vm::vec3d{32, -32, 0};

    const auto p6 = vm::vec3d{32, 32, 32};
    const auto p7 = vm::vec3d{-16, 32, 32}; // this vertex will be deleted
    const auto p8 = vm::vec3d{-32, -16, 32};
    const auto p9 = vm::vec3d{-16, -32, 32};
    const auto p10 = vm::vec3d{32, -32, 32};

    const auto p11 = vm::vec3d{32, 0, 64};

    // Make sure that the faces have the materials we expect before the vertex is deleted.

    // side faces
    assertMaterial("*04awater1", brush, std::vector{p1, p2, p7, p6});
    assertMaterial("*04mwat1", brush, std::vector{p2, p3, p8, p7});
    assertMaterial("*04mwat2", brush, std::vector{p3, p4, p9, p8});
    assertMaterial("*04water1", brush, std::vector{p4, p5, p10, p9});
    assertMaterial("*04water2", brush, std::vector{p5, p1, p6, p11, p10});

    // bottom face
    assertMaterial("*lava1", brush, std::vector{p5, p4, p3, p2, p1});

    // top faces
    assertMaterial("*slime", brush, std::vector{p6, p7, p11});
    assertMaterial("*slime0", brush, std::vector{p7, p8, p11});
    assertMaterial("*slime1", brush, std::vector{p8, p9, p11});
    assertMaterial("*teleport", brush, std::vector{p9, p10, p11});

    // delete the vertex
    CHECK(brush.canRemoveVertices(worldBounds, std::vector{p7}));
    CHECK(brush.removeVertices(worldBounds, std::vector{p7}));

    // assert the structure and materials

    // side faces
    assertMaterial("*04awater1", brush, std::vector{p1, p2, p6});
    assertMaterial("*04mwat1", brush, std::vector{p2, p3, p8});
    assertMaterial("*04mwat2", brush, std::vector{p3, p4, p9, p8});
    assertMaterial("*04water1", brush, std::vector{p4, p5, p10, p9});
    assertMaterial("*04water2", brush, std::vector{p5, p1, p6, p11, p10});

    // bottom face
    assertMaterial("*lava1", brush, std::vector{p5, p4, p3, p2, p1});

    // top faces
    assertMaterial("*slime", brush, std::vector{p6, p2, p11});
    assertMaterial("*slime0", brush, std::vector{p2, p8, p11});
    // failure, becomes *slime0:
    assertMaterial("*slime1", brush, std::vector{p8, p9, p11});
    assertMaterial("*teleport", brush, std::vector{p9, p10, p11});

    kdl::col_delete_all(nodes.value());
  }

  SECTION("snapIssue1198")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1198
    const auto data = R"(
      {
      ( 167.63423 -46.88446 472.36551 ) ( 66.06285 -1.98675 573.93711 ) ( 139.12681 -168.36963 500.87299 ) rock_1736 -158 527 166.79401 0.97488 -0.85268 //TX1
      ( 208 -298.77704 309.53674 ) ( 208 -283.89740 159.77713 ) ( 208 -425.90924 294.65701 ) rock_1736 -261 -291 186.67561 1 1.17558 //TX1
      ( -495.37965 -970.19919 2420.40004 ) ( -369.12126 -979.60987 2439.22145 ) ( -516.42274 -1026.66357 2533.32892 ) skill_ground -2752 -44 100.55540 0.89744 -0.99664 //TX1
      ( 208 -103.52284 489.43151 ) ( 208 -63.04567 610.86296 ) ( 80 -103.52284 489.43151 ) rock_1736 208 516 0 -1 0.94868 //TX1
      ( -450.79344 -2050.77028 440.48261 ) ( -333.56544 -2071.81325 487.37381 ) ( -470.33140 -2177.02858 432.66743 ) skill_ground -2100 -142 261.20348 0.99813 0.93021 //TX1
      ( -192.25073 -2050.77026 159.49851 ) ( -135.78626 -2071.81323 272.42748 ) ( -201.66146 -2177.02856 140.67705 ) skill_ground -2010 513 188.47871 0.99729 -0.89685 //TX1
      ( 181.06874 -76.56186 495.11416 ) ( 172.37248 -56.19832 621.18438 ) ( 63.35341 -126.83229 495.11416 ) rock_1736 197 503 0 -0.91965 0.98492 //TX1
      ( 171.46251 -48.09583 474.98238 ) ( 129.03154 -21.91225 616.98017 ) ( 105.41315 -157.70143 477.82758 ) rock_1736 -71 425 178.51302 0.85658 -1.11429 //TX1
      ( -37.21422 -6.81390 22.01408 ) ( -12.34518 -24.34492 146.34503 ) ( -92.55376 -122.11616 16.82534 ) skill_ground -6 23 182.57664 0.90171 -0.97651 //TX1
      ( -975.92228 -1778.45799 1072.52401 ) ( -911.46425 -1772.13654 1182.92865 ) ( -1036.18913 -1883.59588 1113.72975 ) skill_ground -2320 426 158.59875 0.88222 -0.82108 //TX1
      ( -984.28431 -1006.06166 2136.35663 ) ( -881.58265 -976.76783 2206.91312 ) ( -1039.55007 -1059.19179 2238.85958 ) skill_ground -2580 152 118.33189 0.90978 -0.96784 //TX1
      ( -495.37960 -2050.77026 672 ) ( -369.12118 -2071.81323 672 ) ( -516.42263 -2177.02856 672 ) skill_ground -2104 -151 260.53769 1 1 //TX1
      ( 0 -192 512 ) ( 0 -192 640 ) ( 128 -192 512 ) skill_ground 0 512 0 1 1 //TX1
      ( 0 0 512 ) ( 0 -128 512 ) ( 128 0 512 ) skill_ground 0 0 0 1 -1 //TX1
      })";
    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1202")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1202
    const auto data = R"(
      {
      ( -384 -1440 416 ) ( -384 -1440 544 ) ( -512 -1440 416 ) skip -384 416 0 -1 1 //TX1
      ( -479.20200 -1152 448 ) ( -388.69232 -1242.50967 448 ) ( -607.20203 -1152 448 ) skip -476 1631 -45 1 -0.70711 //TX2
      ( -202.75913 -1259.70123 365.61488 ) ( -293.26877 -1169.19156 365.61487 ) ( -2889239 -13453450 408.28175 ) city6_8 747 1097 135 1 0.94281 //TX2
      ( -672 -1664 112 ) ( -800 -1664 112 ) ( -672 -1664 240 ) bricka2_4 -672 112 0 -1 1 //TX2
      ( -166.47095 -1535.24850 432 ) ( -294.41554 -15391482 432 ) ( -38.47095 -1663.24847 432 ) bricka2_4 -212 1487 181.68613 1 12899 //TX2
      ( 96 -2840.62573 176 ) ( 96 -3021.64502 176 ) ( 96 -2840.62573 304 ) bricka2_4 -2009 176 0 -1.41421 1 //TX2
      ( -128 -288 176 ) ( -128 -160 176 ) ( -128 -288 304 ) bricka2_4 288 176 0 1 1 //TX2
      })";
    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1203")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1203
    const auto data = R"({
    ( -2255.07542 -1621.75354 1184 ) ( -2340.26373 -1524.09826 1184 ) ( -2255.07542 -1621.75354 1312 ) metal5_6 2126 1184 0 0.76293 1 //TX2
    ( -2274.59294 -1572.67199 1077.14252 ) ( -2216.18139 -1643.55025 1214.27523 ) ( -2179.93925 -1486.72565 1086.37772 ) metal1_2 -86 -3857 66.92847 1.16449 -0.65206 //TX2
    ( -2294.68465 -1559.17687 1145.06418 ) ( -2209.49633 -1656.83209 1145.06409 ) ( -2226.47948 -1499.67881 1009.29941 ) metal1_2 -2044 -1080 180.00005 0.76293 1.06066 //TX2
    ( -2277.90664 -1569.35830 1229.87757 ) ( -2219.49502 -1640.23662 1092.74492 ) ( -2183.25294 -1483.41196 1220.64238 ) metal1_2 1738 -2475 -66.92843 1.16449 0.65206 //TX2
    ( -2291.16152 -1556.10351 1161.99537 ) ( -2205.97305 -1653.75857 1161.99532 ) ( -2222.95604 -1496.60517 1297.75964 ) metal1_2 -2040 1096 180.00003 0.76293 -1.06066 //TX2
    ( -2081.99036 -1805.83188 1184 ) ( -2022.45370 -1920.93607 1184 ) ( -2195.68224 -1864.63800 1184 ) skinsore -640 2679 -62.65012 1.01242 -1 //TX2
    ( -2243.07853 -1621.15697 1184 ) ( -2243.07799 -1621.15750 1312 ) ( -2152.56935 -1530.64682 1184 ) metal5_6 2293 1184 0 0.70711 1 //TX1
    ( -2288.33311 -1643.78464 1184 ) ( -2197.82344 -1553.27497 1184 ) ( -2288.33311 -1643.78464 1312 ) metal5_6 2325 1184 0 0.70711 1 //TX2
    ( -2243.76171 -1610.43983 1184 ) ( -2243.76171 -1610.43983 1312 ) ( -2327.90482 -1513.98290 1184 ) metal5_6 2137 1184 0 0.75357 1 //TX1
    })";
    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1205")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1205
    const auto data = R"(
    {
    ( 304 -895.52890 1232 ) ( 304 -763.64662 1232 ) ( 304 -895.52890 1104 ) bookshelf1w 1232 -869 -90 1 1.03033 //TX1
    ( -23.76447 -759.76453 1232 ) ( 69.49032 -666.50962 1232 ) ( -23.76447 -759.76453 1104 ) bookshelf1w 1232 -1043 -90 1 0.72855 //TX1
    ( -139.64675 -480 1232 ) ( -7.76448 -480 1232 ) ( -139.64675 -480 1104 ) bookshelf1w 1232 -136 -90 1 1.03033 //TX1
    ( -42.50967 -245.49033 1232 ) ( 50.74518 -338.74518 1232 ) ( -42.50967 -245.49033 1104 ) bookshelf1w 1232 337 -90 1 -0.72855 //TX1
    ( 323.88225 -320 1232 ) ( 191.99998 -320 1232 ) ( 323.88225 -320 1104 ) bookshelf1w 1232 -314 -90 1 -1.03033 //TX1
    ( 144 -168.23550 1232 ) ( 144 -300.11777 1232 ) ( 144 -168.23550 1104 ) bookshelf1w 1232 163 -90 1 -1.03033 //TX1
    ( 303.99988 -432.00012 1248.00050 ) ( 278.89702 -432.00012 1373.51482 ) ( 303.99988 -304.00012 1248.00050 ) rfslte1 432 1273 0 1 0.98058 //TX1
    ( 303.99995 -367.99981 1248 ) ( 286.42119 -385.57861 1373.56263 ) ( 213.49015 -277.49027 1248 ) rfslte1 430 1272 0 -0.70711 0.98096 //TX1
    ( 256 -320 1247.99999 ) ( 256 -345.10286 1373.51432 ) ( 128 -320.00005 1247.99999 ) rfslte1 256 1273 0 -1 0.98058 //TX1
    ( 191.99988 -320.00012 1248.00049 ) ( 209.57867 -337.57891 1373.56311 ) ( 101.49021 -410.50979 1248.00049 ) rfslte1 -453 1272 0 -0.70711 0.98096 //TX1
    ( 144 -368 1248.00049 ) ( 169.10289 -368 1373.51481 ) ( 144 -496 1248.00049 ) rfslte1 -368 1273 0 -1 0.98058 //TX1
    ( 144 -432 1248.00049 ) ( 161.57879 -414.42121 1373.56311 ) ( 234.50967 -522.50967 1248.00049 ) rfslte1 -611 1272 0 -0.70711 0.98096 //TX1
    ( 192 -480 1248.00049 ) ( 192 -454.89711 1373.51481 ) ( 320 -480 1248.00049 ) rfslte1 -192 1273 0 1 0.98058 //TX1
    ( 256 -480 1248.00049 ) ( 238.42121 -462.42121 1373.56311 ) ( 346.50967 -389.49033 1248.00049 ) rfslte1 679 1272 0 0.70711 0.98096 //TX1
    ( 144 -320 1232 ) ( 144 -448 1232 ) ( 272 -320 1232 ) rfslte1 -144 320 0 1 -1 //TX1
    ( 285.25483 -226.74517 1232 ) ( 191.99999 -320.00001 1232 ) ( 285.25483 -226.74517 1104 ) bookshelf1w 1232 311 -90 1 -0.72855 //TX1
    ( 304 -368 1232 ) ( 210.74516 -274.74516 1232 ) ( 304 -368 1104 ) bookshelf1w 1232 -505 -90 1 0.72855 //TX1
    })";
    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1206")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1206
    const auto data = R"(
    {
    ( -637.50000 1446.44631 1339.47316 ) ( -637.50000 1560.93298 1396.71649 ) ( -765.50000 1446.44631 1339.47316 ) column01_3 -638 1617 0 -1 0.89443 //TX1
    ( -632.50000 1438.33507 1340.33194 ) ( -632.50000 1538.28627 1260.37098 ) ( -760.50000 1438.33507 1340.33194 ) column01_3 -632 1842 0 -1 0.78087 //TX1
    ( -646 1397.33116 1362.08442 ) ( -646 1511.81782 1304.84109 ) ( -518 1397.33116 1362.08442 ) column01_3 646 1562 0 1 0.89443 //TX1
    ( -637.50000 1436 1338 ) ( -637.50000 1436 1466 ) ( -637.50000 1308 1338 ) column01_3 1436 1338 0 -1 1 //TX1
    ( -637 1438.91806 1338.87292 ) ( -637 1367.91644 1445.37534 ) ( -509 1438.91806 1338.87292 ) column01_3 637 1609 0 1 0.83205 //TX1
    ( -637 1440.50000 1338 ) ( -637 1440.50000 1466 ) ( -637 1568.50000 1338 ) column01_3 -1440 1338 0 1 1 //TX1
    ( -638 1435.27452 1340.35014 ) ( -638 1312.19946 1375.51444 ) ( -510 1435.27452 1340.35014 ) column01_3 638 -1493 0 1 -0.96152 //TX1
    })";
    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1207")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1207
    const auto data = R"(
      {
      ( -635.50000 1442.50000 1353.50012 ) ( -763.50000 1442.50000 1353.50012 ) ( -635.50000 1314.50000 1353.50012 ) column01_3 1442 635 -90 1 -1 //TX1
      ( -635.50000 1442.50000 1355 ) ( -507.50000 1442.50000 1355 ) ( -635.50000 1314.50000 1355 ) column01_3 1442 -635 -90 1 1 //TX1
      ( -636 1442.50000 1354 ) ( -636 1442.50000 1482 ) ( -764 1442.50000 1354 ) column01_3 -636 1354 0 -1 1 //TX1
      ( -636 1438 1354 ) ( -636 1438 1482 ) ( -636 1310 1354 ) column01_3 1438 1354 0 -1 1 //TX1
      ( -635.50000 1438 1354 ) ( -635.50000 1438 1482 ) ( -507.50000 1438 1354 ) column01_3 636 1354 0 1 1 //TX1
      ( -635.50000 1442.50000 1354 ) ( -635.50000 1442.50000 1482 ) ( -635.50000 1570.50000 1354 ) column01_3 -1442 1354 0 1 1 //TX1
      })";
    assertCannotSnap(data, taskManager);
  }

  SECTION("snapIssue1232")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1232
    const auto data = R"(
    {
      ( 2152.22540 381.27455 2072 ) ( 2152.22540 381.27455 2200 ) ( 2020.34268 513.15633 2072 ) wbord05 2089 2072 0 -1.03033 1 //TX1
      ( 2042 335.61771 2072 ) ( 2042 335.61771 2200 ) ( 2042 522.12738 2072 ) wbord05 -230 2072 0 1.45711 1 //TX1
      ( 1948.74515 374.24515 2072 ) ( 1948.74515 374.24515 2200 ) ( 2080.62741 506.12741 2072 ) wbord05 -363 2072 0 1.03033 1 //TX1
      ( 1916.74515 451.50000 2072 ) ( 1916.74515 451.50000 2200 ) ( 2103.25482 451.50000 2072 ) wbord05 -1315 2072 0 1.45711 1 //TX1
      ( 2043.56919 493.06919 2026.43074 ) ( 1969.66841 419.16841 2100.33167 ) ( 2134.07889 402.55957 2026.43079 ) kjwall2 -1096 -2197 -44.99997 1 -0.81650 //TX1
      ( 2028.72645 441.39868 2036.31307 ) ( 2140.35950 385.25273 2064.05640 ) ( 2063.24398 543.87358 2104.80712 ) kjwall2 -1262 1843 71.38448 0.84478 -0.96653 //TX1
      ( 1980.74480 497.22377 2022.51040 ) ( 2011.04246 392.71223 2089.91507 ) ( 2093.59579 549.47972 2052.80842 ) kjwall2 -2065 453 24.84662 0.97158 -0.84038 //TX1
      ( 2026.09563 451.97825 2028.19126 ) ( 1995.79798 556.48977 2095.59597 ) ( 1913.24475 399.72220 2058.48949 ) kjwall2 2088 -525 204.84669 0.97158 -0.84038 //TX1
      ( 1994 515.89878 2035.80067 ) ( 1994 401.41210 2093.04401 ) ( 2122 515.89859 2035.80028 ) kjwall2 -1994 -577 -0.00009 1 -0.89443 //TX1
      ( 2010 443.10126 2035.80060 ) ( 2010 557.58793 2093.04394 ) ( 1881.99999 443.10145 2035.80021 ) kjwall2 2010 495 179.99991 1 -0.89443 //TX1
      ( 2018.70638 436.61696 2056.35332 ) ( 2119.11026 375.11218 2106.55513 ) ( 2073.71821 548.87185 2083.85853 ) kjwall2 -1311 1770 63.89229 0.97664 -0.91582 //TX1
      ( 2034 453.83437 2044 ) ( 1982.79994 568.32105 2069.59989 ) ( 1931.59947 396.59103 2095.19895 ) kjwall2 2179 -611 209.20580 0.91652 -0.97590 //TX1
      ( 2018 507.50000 2072 ) ( 2018 507.50000 2200 ) ( 1831.49033 507.50000 2072 ) wbord05 1385 2072 0 -1.45711 1 //TX1
      ( 1986 530.12743 2072 ) ( 1986 530.12743 2200 ) ( 1986 343.61775 2072 ) wbord05 364 2072 0 -1.45711 1 //TX1
      ( 2010 479.50000 2072 ) ( 2010 607.50000 2072 ) ( 2138 479.50000 2072 ) kjwall2 -2010 480 0 1 1 //TX1
      ( 2010 479.50000 2060 ) ( 2010 351.50000 2060 ) ( 2138 479.50000 2060 ) kjwall2 -2010 -480 0 1 -1 //TX1
      ( 2013.31371 518.81371 2072 ) ( 2013.31371 518.81371 2200 ) ( 1881.43146 386.93146 2072 ) wbord05 504 2072 0 -1.03033 1 //TX1
      ( 1941.71572 511.78427 2072 ) ( 1941.71572 511.78427 2200 ) ( 2073.59785 379.90191 2072 ) wbord05 497 2072 0 -1.03033 1 //TX1
     })";

    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1395_24202")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1395 brush at line 24202
    const auto data = R"(
    {
    ( -4 -325 952 ) ( -16 -356 1032 ) ( -44 -309 1016 ) rock3_8 -1.28601 -6.46194 113.395 0.943603 1.06043
    ( -17.57635498046875 -263.510009765625 988.9852294921875 ) ( -137.5655517578125 -375.941162109375 743.296875 ) ( 34.708740234375 -300.228759765625 1073.855712890625 ) rock3_8 -1.28595 -6.46191 113.395 0.943603 1.06043
    ( -135.7427978515625 -370.1265869140625 739.753173828125 ) ( -15.768181800842285 -257.6954345703125 985.42547607421875 ) ( -449.98324584960937 -364.254638671875 589.064697265625 ) rock3_8 -26.8653 -10.137 25.6205 1.15394 -1
    ( -399.50726318359375 -406.7877197265625 677.47894287109375 ) ( -137.5655517578125 -375.941162109375 743.296875 ) ( -451.79229736328125 -370.0692138671875 592.6083984375 ) rock3_8 26.1202 -7.68527 81.5004 0.875611 -1
    ( -280.1622314453125 -291.92608642578125 924.623779296875 ) ( -18.227519989013672 -261.07952880859375 990.43829345703125 ) ( -227.88420104980469 -328.64483642578125 1009.49853515625 ) rock3_8 -28.9783 0.638519 81.5019 0.875609 -1
    ( -195.9036865234375 -282.3568115234375 876.8590087890625 ) ( -143.6192626953125 -319.08740234375 961.7213134765625 ) ( -368.19818115234375 -358.08740234375 546.27716064453125 ) rock3_8 -25.9692 -19.1265 113.395 0.943603 1.06043
    ( -276.88287353515625 -332.21014404296875 930.47674560546875 ) ( -449.17929077148437 -407.92318725585937 599.90850830078125 ) ( -14.952971458435059 -301.37832641601562 996.28533935546875 ) rock3_8 -20.4888 -8.56413 -87.0938 1.30373 1.02112
    ( 37.161830902099609 -335.35406494140625 1080.605712890625 ) ( -135.12174987792969 -411.084716796875 750.062744140625 ) ( -224.79318237304687 -366.23345947265625 1014.8262329101562 ) rock3_8 8.91101 4.43578 -87.0938 1.30373 1.02112
    ( -290.354736328125 -397.304931640625 703.53790283203125 ) ( -470.618896484375 -265.4686279296875 632.53790283203125 ) ( -400.5767822265625 -391.6395263671875 703.53790283203125 ) rock3_8 8.25781 -11.1122 -165 0.865994 1
    ( -96 -299 1019 ) ( -96 -171 1019 ) ( 50 -400 1017 ) rock3_8 -28.9783 0.638519 81.5019 0.875609 -1
    })";

    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapIssue1395_18995")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1395 brush at line 24202
    const auto data = R"(
      {
      ( 335 891 680 ) ( 314 881 665 ) ( 451 826 680 ) wswamp1_2 2 0 0 1 1
      ( 450 813 671 ) ( 451 826 680 ) ( 446 807 665 ) wswamp1_2 2 0 0 1 1
      ( 451 826 680 ) ( 314 881 665 ) ( 446 807 665 ) wswamp1_2 2 0 0 1 1
      ( 446 807 665 ) ( 446 754 665 ) ( 450 813 671 ) wswamp1_2 2 0 0 1 1
      ( 446 754 680 ) ( 451 826 680 ) ( 446 754 665 ) wswamp1_2 2 0 0 1 1
      ( 313 880 680 ) ( 310 879 677 ) ( 335 891 680 ) wswamp1_2 -16 0 0 1 1
      ( 304 876 670 ) ( 312 880 665 ) ( 310 879 677 ) wswamp1_2 -16 0 0 1 1
      ( 314 881 665 ) ( 335 891 680 ) ( 310 879 677 ) wswamp1_2 -16 0 0 1 1
      ( 330 754 667 ) ( 328 754 665 ) ( 342 757 680 ) wswamp1_2 2 0 0 1 1
      ( 342 757 680 ) ( 328 754 665 ) ( 310 879 677 ) wswamp1_2 2 0 0 1 1
      ( 304 876 670 ) ( 310 879 677 ) ( 328 754 665 ) wswamp1_2 2 0 0 1 1
      ( 312 823 665 ) ( 304 876 670 ) ( 328 754 665 ) wswamp1_2 2 0 0 1 1
      ( 310.50375366210937 879.1187744140625 676.45660400390625 ) ( 313.50375366210937 880.1187744140625 679.45660400390625 ) ( 342.50375366210937 757.1187744140625 679.45660400390625 ) wswamp1_2 2 0 0 1 1
      ( 308.35256958007812 876 676.95867919921875 ) ( 316.35256958007813 823 671.95867919921875 ) ( 316.35256958007813 880 671.95867919921875 ) wswamp1_2 2 0 0 1 1
      ( 342 757 680 ) ( 446 754 680 ) ( 330 754 667 ) wswamp1_2 -16 0 0 1 1
      ( 446 754 665 ) ( 328 754 665 ) ( 446 754 680 ) wswamp1_2 -16 0 0 1 1
      ( 446 754 680 ) ( 342 757 680 ) ( 451 826 680 ) wswamp1_2 -16 -2 0 1 1
      ( 446 754 665 ) ( 446 807 665 ) ( 328 754 665 ) wswamp1_2 -16 -2 0 1 1
      })";

    assertSnapToInteger(data, taskManager);
  }

  SECTION("snapToGrid64")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1415
    const auto data = R"(
      {
      ( 400 224 272 ) ( 416 272 224 ) ( 304 224 224 ) techrock 128 -0 -0 1 1
      ( 416 448 224 ) ( 416 272 224 ) ( 400 448 272 ) techrock 64 -0 -0 1 1
      ( 304 272 32 ) ( 304 832 48 ) ( 304 272 48 ) techrock 64 -0 -0 1 1
      ( 304 448 224 ) ( 416 448 224 ) ( 304 448 272 ) techrock 128 0 0 1 1
      ( 400 224 224 ) ( 304 224 224 ) ( 400 224 272 ) techrock 128 -0 -0 1 1
      ( 352 272 272 ) ( 400 832 272 ) ( 400 272 272 ) techrock 128 -64 -0 1 1
      ( 304 448 224 ) ( 304 224 224 ) ( 416 448 224 ) techrock 128 -64 0 1 1
      })";

    // Seems reasonable for this to fail to snap to grid 64; it's only 48 units tall.
    // If it was able to snap, that would be OK too.
    assertCannotSnapTo(data, 64, taskManager);
  }

  SECTION("moveEdgesFail_2361")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/2361

    const std::string data = R"(
{
( -5706.7302804991996 648 1090 ) ( -5730.7302769049566 730 1096 ) ( -5706.730280499035 722 1076 ) so_b4b 103.27 45.1201 180 1 1
( -5706.730280499035 722 1076 ) ( -5702.7302804990386 720 1070 ) ( -5706.7302804991996 648 1090 ) so_b4b -0 -0 -0 1 1
( -5706.730280499035 722 1076 ) ( -5712.7302804990295 720 1048 ) ( -5702.7302804990386 720 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5712.7302804990295 720 1048 ) ( -5730.7302769049566 730 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5730.7302769049566 730 1096 ) ( -5712.7302804990295 720 1048 ) ( -5706.730280499035 722 1076 ) so_b4b -1.27002 -0 -0 1 1
( -5748.7302591202761 732 1100 ) ( -5730.7302769049566 730 1096 ) ( -5748.7302877821612 732 1104 ) so_b4b 99.2695 -56 180 1 1
( -5698.730280504652 488 1068 ) ( -5702.7302804975034 490 1062 ) ( -5736.7302805013023 496 1034 ) so_b4b -1.26953 -0 -0 1 1
( -5736.7302805013023 496 1034 ) ( -5702.7302804975034 490 1062 ) ( -5724.7302804992714 496 1042 ) so_b4b -1.26953 -46.4615 -0 1 1
( -5764.7302805002028 494 1030 ) ( -5698.730280504652 488 1068 ) ( -5736.7302805013023 496 1034 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7298890995526 484 1089.9989236411209 ) ( -5698.730280504652 488 1068 ) ( -5706.7298897136052 484 1086 ) so_b4b -21.27 -56 -0 1 -1
( -5730.7302804539777 574 1108 ) ( -5706.7302804991996 648 1090 ) ( -5706.7302804735173 484 1090 ) so_b4b -41.2695 6 -0 1 1
( -5702.7302804990386 720 1070 ) ( -5698.7302805012414 644 1068 ) ( -5706.7302804991996 648 1090 ) so_b4b -0 -0 -0 1 1
( -5748.7302877821612 732 1104 ) ( -5737.7302809186394 649 1108 ) ( -5772.7302805004565 732 1108 ) so_b4b -1.27002 -0 -0 1 1
( -5698.7302805012414 644 1068 ) ( -5698.730280504652 488 1068 ) ( -5706.7298890995526 484 1089.9989236411209 ) so_b4b 88 102 180 1 -1
( -5730.7302769049566 730 1096 ) ( -5737.7302809186394 649 1108 ) ( -5748.7302877821612 732 1104 ) so_b4b -1.27002 -0 -0 1 1
( -5706.7302804991996 648 1090 ) ( -5737.7302809186394 649 1108 ) ( -5730.7302769049566 730 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5730.7302804539777 574 1108 ) ( -5737.7302809186394 649 1108 ) ( -5706.7302804991996 648 1090 ) so_b4b -41.27 2 -0 1 -1
( -5712.7302804990295 720 1048 ) ( -5698.7302805012414 644 1068 ) ( -5702.7302804990386 720 1070 ) so_b4b -0 -0 -0 1 1
( -5736.7302805013023 496 1034 ) ( -5734.7302804990195 638 1030 ) ( -5764.7302805002028 494 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5710.7302804925348 636 1048 ) ( -5734.7302804990195 638 1030 ) ( -5724.7302804992714 496 1042 ) so_b4b -37.2695 6 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5734.7302804990195 638 1030 ) ( -5710.7302804925348 636 1048 ) so_b4b -37.2695 6 -0 1 1
( -5724.7302804992714 496 1042 ) ( -5734.7302804990195 638 1030 ) ( -5736.7302805013023 496 1034 ) so_b4b -1.27002 -0 -0 1 1
( -5698.7302805012414 644 1068 ) ( -5710.7302804925348 636 1048 ) ( -5698.730280504652 488 1068 ) so_b4b -0 -0 -0 1 1
( -5698.730280504652 488 1068 ) ( -5710.7302804925348 636 1048 ) ( -5702.7302804975034 490 1062 ) so_b4b -0 -0 -0 1 1
( -5702.7302804975034 490 1062 ) ( -5710.7302804925348 636 1048 ) ( -5724.7302804992714 496 1042 ) so_b4b 103.232 -3.37415 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5710.7302804925348 636 1048 ) ( -5712.7302804990295 720 1048 ) so_b4b 123.169 60.1836 -0 1 -1
( -5712.7302804990295 720 1048 ) ( -5710.7302804925348 636 1048 ) ( -5698.7302805012414 644 1068 ) so_b4b -0 -0 -0 1 1
( -5798.7302805036807 726 1034 ) ( -5842.7302804987194 726 1064 ) ( -5816.730280497547 724 1042 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805003801 728 1108 ) ( -5834.7302796346403 726 1090 ) ( -5844.7302535491081 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5832.7303385250107 490 1048 ) ( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) ( -5808.7308828738051 492 1030 ) so_b4b 67.2695 -25.828 180 1 1
( -5814.730293347111 490 1096 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5840.7302607871034 494 1072 ) so_b4b -1.26953 -0 -0 1 1
( -5840.7302607871034 494 1072 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5832.7303385250107 490 1048 ) so_b4b 87.2695 34 180 1 -1
( -5814.730293347111 490 1096 ) ( -5836.7302804990259 642 1090 ) ( -5812.7302804995788 644 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302804995788 644 1108 ) ( -5836.7302804990259 642 1090 ) ( -5812.7302805003801 728 1108 ) so_b4b 63.2695 12 180 1 -1
( -5812.7302805003801 728 1108 ) ( -5836.7302804990259 642 1090 ) ( -5834.7302796346403 726 1090 ) so_b4b 119.763 -82.8022 -0 1 1
( -5834.7302796346403 726 1090 ) ( -5836.7302804990259 642 1090 ) ( -5844.7302535491081 726 1070 ) so_b4b -50 102 -0 1 1
( -5844.7302535491081 726 1070 ) ( -5836.7302804990259 642 1090 ) ( -5844.7303163465958 646 1070 ) so_b4b -50 102 -0 1 1
( -5844.7303163465958 646 1070 ) ( -5836.7302804990259 642 1090 ) ( -5840.7302607871034 494 1072 ) so_b4b -0 -0 -0 1 1
( -5840.7302607871034 494 1072 ) ( -5836.7302804990259 642 1090 ) ( -5814.730293347111 490 1096 ) so_b4b 111.887 -21.6224 -0 1 1
( -5812.7302804995788 644 1108 ) ( -5802.7302869960968 490 1104 ) ( -5814.730293347111 490 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5774.7302805949284 488 1108 ) ( -5802.7302869960968 490 1104 ) ( -5812.7302804995788 644 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5832.7301202365989 642 1048 ) ( -5832.7303385250107 490 1048 ) ( -5808.7308828738051 492 1030 ) so_b4b 67.2695 -120 180 1 1
( -5832.7301202365989 642 1048 ) ( -5808.7308828738051 492 1030 ) ( -5808.7302827027843 640 1030 ) so_b4b 67.2695 -120 180 1 1
( -5832.7301202365989 642 1048 ) ( -5842.7302804987194 726 1064 ) ( -5844.73018187052 726 1066 ) so_b4b -85.6646 31.4945 -0 1 1
( -5816.730280497547 724 1042 ) ( -5842.7302804987194 726 1064 ) ( -5832.7301202365989 642 1048 ) so_b4b -1.26953 -0 -0 1 1
( -5844.73018187052 726 1066 ) ( -5832.7303385250107 490 1048 ) ( -5832.7301202365989 642 1048 ) so_b4b -0 -0 -0 1 1
( -5816.730280497547 724 1042 ) ( -5808.7302827027843 640 1030 ) ( -5798.7302805036807 726 1034 ) so_b4b -1.27002 -0 -0 1 1
( -5840.7302231706126 494 1068 ) ( -5832.7303385250107 490 1048 ) ( -5844.7302185478356 645.99772419238377 1066 ) so_b4b -0 -0 -0 1 1
( -5808.7302827027843 640 1030 ) ( -5774.7302970963183 726 1030 ) ( -5798.7302805036807 726 1034 ) so_b4b -1.27002 -0 -0 1 1
( -5832.7301202365989 642 1048 ) ( -5808.7302827027843 640 1030 ) ( -5816.730280497547 724 1042 ) so_b4b -1.26953 -0 -0 1 1
( -5844.7302185478356 645.99772419238377 1066 ) ( -5844.73018187052 726 1066 ) ( -5844.7302535491081 726 1070 ) so_b4b 56 12 270 1 1
( -5844.7302185478356 645.99772419238377 1066 ) ( -5844.7303163465958 646 1070 ) ( -5840.7302607871034 494 1072 ) so_b4b -0 -0 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5730.7302769049566 730 1096 ) ( -5774.7302970963183 726 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5774.7302970963183 726 1030 ) ( -5730.7302769049566 730 1096 ) ( -5748.7302591202761 732 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5748.7302877821612 732 1104 ) ( -5772.7302805004565 732 1108 ) ( -5772.7302088121833 732 1104 ) so_b4b 95.2695 -56 180 1 1
( -5772.7302088121833 732 1104 ) ( -5772.7302805004565 732 1108 ) ( -5844.7302535491081 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5798.7302805036807 726 1034 ) ( -5774.7302970963183 726 1030 ) ( -5748.7302591202761 732 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5844.73018187052 726 1066 ) ( -5842.7302804987194 726 1064 ) ( -5772.7302088121833 732 1104 ) so_b4b -1.27002 -0 -0 1 1
( -5772.7302088121833 732 1104 ) ( -5842.7302804987194 726 1064 ) ( -5798.7302805036807 726 1034 ) so_b4b -1.26953 -0 -0 1 1
( -5772.7302805004565 732 1108 ) ( -5812.7302805003801 728 1108 ) ( -5844.7302535491081 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5814.730293347111 490 1096 ) ( -5802.7302869960968 490 1104 ) ( -5774.7302805949284 488 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) ( -5698.730280504652 488 1068 ) ( -5808.7308828738051 492 1030 ) so_b4b -1.27002 -0 -0 1 1
( -5808.7308828738051 492 1030 ) ( -5698.730280504652 488 1068 ) ( -5764.7302805002028 494 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7298897136052 484 1086 ) ( -5698.730280504652 488 1068 ) ( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) so_b4b -1.27002 -0 -0 1 1
( -5832.7303385250107 490 1048 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) so_b4b -1.26953 -0 -0 1 1
( -5774.7302805949284 488 1108 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5814.730293347111 490 1096 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7362814612115 484 1090.0045006603091 ) ( -5774.7302805949284 488 1108 ) ( -5730.7302804977789 486 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7362814612115 484 1090.0045006603091 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5774.7302805949284 488 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5772.7302805004565 732 1108 ) ( -5737.7302809186394 649 1108 ) ( -5730.7302804539777 574 1108 ) so_b4b -37.27 6 -0 1 1
( -5764.7302805002028 494 1030 ) ( -5734.7302804990195 638 1030 ) ( -5734.7302804990386 722 1030 ) so_b4b -33.27 6 -0 1 1
}
)";

    auto status = TestParserStatus{};

    auto nodes =
      NodeReader::read({}, data, MapFormat::Standard, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    REQUIRE(nodes.value().size() == 1u);

    auto brush = static_cast<BrushNode*>(nodes.value().front())->brush();

    const auto vertex1 =
      brush.findClosestVertexPosition(vm::vec3d{-5774.7302805949275, 488, 1108});
    const auto vertex2 =
      brush.findClosestVertexPosition(vm::vec3d{-5730.730280440197, 486, 1108});
    const auto segment = vm::segment3d(vertex1, vertex2);

    const auto transform = vm::translation_matrix(vm::vec3d{0, -4, 0});
    CHECK(brush.canTransformEdges(worldBounds, {segment}, transform));
    CHECK_NOTHROW(brush.transformEdges(worldBounds, {segment}, transform));

    kdl::col_delete_all(nodes.value());
  }

  SECTION("moveFaceFailure_1499")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1499

    const auto p1 = vm::vec3d{-4408, 16, 288};
    const auto p2 = vm::vec3d{-4384, 40, 288};
    const auto p3 = vm::vec3d{-4384, 64, 288};
    const auto p4 = vm::vec3d{-4416, 64, 288};
    const auto p5 = vm::vec3d{-4424, 48, 288}; // left back  top
    const auto p6 = vm::vec3d{-4424, 16, 288}; // left front top
    const auto p7 = vm::vec3d{-4416, 64, 224};
    const auto p8 = vm::vec3d{-4384, 64, 224};
    const auto p9 = vm::vec3d{-4384, 40, 224};
    const auto p10 = vm::vec3d{-4408, 16, 224};
    const auto p11 = vm::vec3d{-4424, 16, 224};
    const auto p12 = vm::vec3d{-4424, 48, 224};

    const auto points = std::vector{p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12};

    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
    auto brush = builder.createBrush(points, "asdf") | kdl::value();

    const auto topFace = vm::polygon3d{std::vector{p1, p2, p3, p4, p5, p6}};

    CHECK(brush.canTransformFaces(
      worldBounds, {topFace}, vm::translation_matrix(vm::vec3d{+16, 0, 0})));
    CHECK(brush.canTransformFaces(
      worldBounds, {topFace}, vm::translation_matrix(vm::vec3d{-16, 0, 0})));
    CHECK(brush.canTransformFaces(
      worldBounds, {topFace}, vm::translation_matrix(vm::vec3d{0, +16, 0})));
    CHECK(brush.canTransformFaces(
      worldBounds, {topFace}, vm::translation_matrix(vm::vec3d{0, -16, 0})));
    CHECK(brush.canTransformFaces(
      worldBounds, {topFace}, vm::translation_matrix(vm::vec3d{0, 0, +16})));
    CHECK(brush.canTransformFaces(
      worldBounds, {topFace}, vm::translation_matrix(vm::vec3d{0, 0, -16})));
  }

  SECTION("convexMergeCrash_2789")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/2789
    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Brush/curvetut-crash.map";
    const auto data = fs::readTextFile(path);
    REQUIRE(!data.empty());

    auto status = TestParserStatus{};

    auto nodes =
      NodeReader::read({}, data, MapFormat::Valve, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    REQUIRE(!nodes.value().empty());

    auto points = std::vector<vm::vec3d>{};
    for (const auto* node : nodes.value())
    {
      if (const auto* brushNode = dynamic_cast<const BrushNode*>(node))
      {
        for (const auto* vertex : brushNode->brush().vertices())
        {
          points.push_back(vertex->position());
        }
      }
    }

    const auto polyhedron = Polyhedron3{std::move(points)};
    const auto expectedPositions = std::vector<vm::vec3d>{
      {40.000000, -144.000031, 180.999969},   {40.000000, -144.000000, -0.000023},
      {55.996799, -111.999001, -0.000018},    {55.996799, -111.999031, 178.999985},
      {16.000000, -168.000000, -0.000027},    {16.000000, -168.000031, 183.999969},
      {16.000000, 39.999969, 184.000000},     {16.000000, 40.000000, 0.000007},
      {-48.000000, 63.996498, 0.000010},      {-80.000000, 64.000000, 0.000010},
      {-48.000000, -192.000031, 191.999969},  {-80.000000, -192.000031, 195.999969},
      {-80.000000, -192.000000, -0.000031},   {-48.000000, -192.000000, -0.000031},
      {-112.000000, 55.999966, 200.000015},   {-112.000000, 56.000000, 0.000009},
      {-144.000000, 40.000000, 0.000007},     {-144.000000, 39.999966, 204.000000},
      {-192.000000, -80.000031, 209.999985},  {-192.000000, -48.000034, 209.999985},
      {-192.000000, -48.000000, -0.000008},   {-192.000000, -80.000000, -0.000013},
      {-184.000000, -112.000031, 208.999985}, {-184.000000, -112.000000, -0.000018},
      {-184.000000, -16.000034, 209.000000},  {-184.000000, -16.000000, -0.000003},
      {-168.000000, -144.000031, 206.999969}, {-168.000000, -144.000000, -0.000023},
      {-168.000000, 15.999967, 207.000000},   {-168.000000, 16.000000, 0.000003},
      {-144.000000, -168.000031, 203.999969}, {-144.000000, -168.000000, -0.000027},
      {-112.000000, -184.000031, 199.999969}, {-112.000000, -184.000000, -0.000030},
      {-80.000000, 63.999969, 196.000015},    {-48.000000, 63.996468, 192.000015},
      {-16.000000, -184.000031, 187.999969},  {-16.000000, -184.000000, -0.000030},
      {-16.001301, 55.996799, 0.000009},      {-16.001301, 55.996769, 188.000015},
      {40.000000, 15.999970, 181.000000},     {40.000000, 16.000000, 0.000003},
      {56.000000, -16.000029, 179.000000},    {56.000000, -16.000000, -0.000003},
      {63.996498, -80.000031, 177.999985},    {63.996498, -80.000000, -0.000013},
      {64.000000, -48.000000, -0.000008},     {64.000000, -48.000031, 177.999985},
    };
    // NOTE: The above was generated by manually cleaning up the output
    // in Blender. It's a 24-sided cylinder.
    // We currently generate some extra vertices/faces, so just check
    // that all vertices in the cleaned-up expected output exist in the
    // computed output.
    for (const auto& position : expectedPositions)
    {
      CHECK(polyhedron.hasVertex(position, 01));
    }

    kdl::col_delete_all(nodes.value());
  }

  SECTION("convexMergeIncorrectResult_2789")
  {
    // weirdcurvemerge.map from https://github.com/TrenchBroom/TrenchBroom/issues/2789

    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Brush/weirdcurvemerge.map";
    const auto data = fs::readTextFile(path);
    REQUIRE(!data.empty());

    auto status = TestParserStatus{};

    const auto nodes =
      NodeReader::read({}, data, MapFormat::Valve, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    REQUIRE(nodes.value().size() == 28);

    auto points = std::vector<vm::vec3d>{};
    for (const auto* node : nodes.value())
    {
      const auto* brushNode = dynamic_cast<const BrushNode*>(node);
      REQUIRE(brushNode != nullptr);
      for (const auto* vertex : brushNode->brush().vertices())
      {
        points.push_back(vertex->position());
      }
    }

    const auto polyhedron = Polyhedron3{std::move(points)};

    // The result should be a 24-sided cylinder
    CHECK(polyhedron.faceCount() == 26);
    CHECK(polyhedron.edgeCount() == 72);
    CHECK(polyhedron.vertexCount() == 48);
    const auto expectedPositions = std::vector<vm::vec3d>{
      {383.997, -959.993, 875}, {383.997, 959.993, 592},  {383.997, 959.993, 875},
      {128, -1024, 624},        {128, -1024, 907},        {128, 1023.99, 624},
      {-1024, -128, 768},       {-1024, -128, 1051},      {-1024, 128, 768},
      {-1024, 128, 1051},       {-960, -384, 760},        {-960, -384, 1043},
      {-960, 384, 760},         {-960, 384, 1043},        {-832, -640, 744},
      {-832, -640, 1027},       {-832, 640, 744},         {-832, 640, 1027},
      {-640, -832, 720},        {-640, -832, 1003},       {-640, 832, 720},
      {-640, 832, 1003},        {-384, -960, 688},        {-384, -960, 971},
      {-384, 960, 688},         {-384, 960, 971},         {-128, -1024, 656},
      {-128, -1024, 939},       {-128, 1023.99, 656},     {-128, 1023.99, 939},
      {128, 1023.99, 907},      {383.997, -959.993, 592}, {640, -832, 560},
      {640, -832, 843},         {640, 832, 560},          {640, 832, 843},
      {832, -640, 536},         {832, -640, 819},         {832, 640, 536},
      {832, 640, 819},          {960, -384, 520},         {960, -384, 803},
      {960, 384, 520},          {960, 384, 803},          {1024, -128, 512},
      {1024, -128, 795},        {1024, 128, 512},         {1024, 128, 795}};
    CHECK(polyhedron.hasAllVertices(expectedPositions, 01));

    kdl::col_delete_all(nodes.value());
  }

  SECTION("subtractTruncatedCones")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1469

    const auto minuendStr = R"({
        ( 29.393876913416079 -16.970562748463635 32 ) ( 16.970562748495468 29.393876913411077 32 ) ( 11.313708499003496 19.595917942278447 -16 ) __TB_empty [ -0.258819 0.965926 0 -0.507559 ] [ -0.158797 -0425496 -0.986394 -0.257094 ] -0 1 1
        ( 32.784609690844263 -8.784609690813113 32 ) ( 8.7846096908451727 32.784609690839488 32 ) ( 5.856406460569815 21.856406460564131 -16 ) __TB_empty [ -0.5 0.866025 0 -0.77533 ] [ -0.142374 -0821995 -0.986394 -0887003 ] -0 1 1
        ( 33.94112549697229 -0 32 ) ( -0 33.941125496967288 32 ) ( -0 22.627416997982664 -16 ) __TB_empty [ -0.707107 0.707107 0 -0.176551 ] [ -0.116248 -0.116248 -0.986394 -0.46579 ] -0 1 1
        ( 32.784609690844718 8.7846096908399431 32 ) ( -8.7846096908083382 32.784609690839488 32 ) ( -5.8564064605325257 21.856406460564131 -16 ) __TB_empty [ -0.866025 0.5 0 -0124664 ] [ -0821995 -0.142374 -0.986394 -0.870919 ] -0 1 1
        ( 29.393876913416534 16.970562748490465 32 ) ( -16.970562748458633 29.393876913411304 32 ) ( -11.313708498966207 19.595917942278675 -16 ) __TB_empty [ -0.965926 0.258819 0 -0.373029 ] [ -0425496 -0.158797 -0.986394 -0.805874 ] -0 1 1
        ( -11.313708498966662 -19.595917942252527 -16 ) ( -16.970562748458633 -29.393876913384929 32 ) ( 29.393876913416079 -16.970562748463635 32 ) __TB_empty [ -0425496 0.158797 -0.986394 -0.30125 ] [ -0.965926 -0.258819 0 -00242329 ] -0 1 1
        ( -5.8564064605325257 -21.85640646053821 -16 ) ( -8.7846096908078835 -32.784609690813113 32 ) ( 32.784609690844263 -8.784609690813113 32 ) __TB_empty [ -0821995 0.142374 -0.986394 -0.474954 ] [ -0.866025 -0.5 0 -0709991 ] -0 1 1
        ( -0 -22.627416997956516 -16 ) ( -0 -33.941125496940913 32 ) ( 33.94112549697229 -0 32 ) __TB_empty [ -0.116248 0.116248 -0.986394 -0.298004 ] [ -0.707107 -0.707107 0 -0.689445 ] -0 1 1
        ( 5.856406460569815 -21.856406460537755 -16 ) ( 8.7846096908451727 -32.784609690813113 32 ) ( 32.784609690844718 8.7846096908399431 32 ) __TB_empty [ -0.142374 0821995 -0.986394 -0.219636 ] [ -0.5 -0.866025 0 -0.872314 ] -0 1 1
        ( 11.313708499003496 -19.595917942252072 -16 ) ( 16.970562748495922 -29.393876913384702 32 ) ( 29.393876913416534 16.970562748490465 32 ) __TB_empty [ -0.158797 0425496 -0.986394 -0.818881 ] [ -0.258819 -0.965926 0 -0.590811 ] -0 1 1
        ( 16 -16 -16 ) ( 24 -24 32 ) ( 24 24 32 ) __TB_empty [ -0.164399 0 -0.986394 -0.283475 ] [ 0 -1 0 -0 ] -0 1 1
        ( 16.970562748495468 29.393876913411077 32 ) ( -29.3938769133797 16.970562748490465 32 ) ( -19.595917942246615 11.313708498997812 -16 ) __TB_empty [ -0425496 0.158797 0.986394 0475388 ] [ -0.965926 -0.258819 0 -0.238751 ] -0 1 1
        ( 8.7846096908451727 32.784609690839488 32 ) ( -32.784609690807883 8.7846096908399431 32 ) ( -21.856406460532071 5.8564064605641306 -16 ) __TB_empty [ -0821995 0.142374 0.986394 -0.902102 ] [ -0.866025 -0.5 0 -0.660111 ] -0 1 1
        ( -0 33.941125496967288 32 ) ( -33.941125496935911 -0 32 ) ( -22.627416997950604 -0 -16 ) __TB_empty [ -0.116248 0.116248 0.986394 -0.50108 ] [ -0.707107 -0.707107 0 -0.631095 ] -0 1 1
        ( -8.7846096908083382 32.784609690839488 32 ) ( -32.784609690807883 -8.7846096908135678 32 ) ( -21.856406460532071 -5.8564064605377553 -16 ) __TB_empty [ -0.142374 0821995 0.986394 -0.198669 ] [ -0.5 -0.866025 0 -0.166748 ] -0 1 1
        ( -16.970562748458633 29.393876913411304 32 ) ( -29.393876913379245 -16.970562748463863 32 ) ( -19.595917942246615 -11.313708498971437 -16 ) __TB_empty [ -0.158797 0425496 0.986394 -0.573831 ] [ -0.258819 -0.965926 0 -0.238028 ] -0 1 1
        ( -29.3938769133797 16.970562748490465 32 ) ( -16.970562748458633 -29.393876913384929 32 ) ( -11.313708498966662 -19.595917942252527 -16 ) __TB_empty [ -0.258819 0.965926 0 -0.271353 ] [ -0.158797 -0425496 0.986394 -0.908333 ] -0 1 1
        ( -32.784609690807883 8.7846096908399431 32 ) ( -8.7846096908078835 -32.784609690813113 32 ) ( -5.8564064605325257 -21.85640646053821 -16 ) __TB_empty [ -0.5 0.866025 0 -0.18634 ] [ -0.142374 -0821995 0.986394 -0.51593 ] -0 1 1
        ( -33.941125496935911 -0 32 ) ( -0 -33.941125496940913 32 ) ( -0 -22.627416997956516 -16 ) __TB_empty [ -0.707107 0.707107 0 -0.234839 ] [ -0.116248 -0.116248 0.986394 -0.668957 ] -0 1 1
        ( -32.784609690807883 -8.7846096908135678 32 ) ( 8.7846096908451727 -32.784609690813113 32 ) ( 5.856406460569815 -21.856406460537755 -16 ) __TB_empty [ -0.866025 0.5 0 -0.717973 ] [ -0821995 -0.142374 0.986394 -0.849948 ] -0 1 1
        ( -29.393876913379245 -16.970562748463863 32 ) ( 16.970562748495922 -29.393876913384702 32 ) ( 11.313708499003496 -19.595917942252072 -16 ) __TB_empty [ -0.965926 0.258819 0 -0.72569 ] [ -0425496 -0.158797 0.986394 -0.560825 ] -0 1 1
        ( -24 24 32 ) ( -24 -24 32 ) ( -16 -16 -16 ) __TB_empty [ -0.164399 0 0.986394 -0.81431 ] [ 0 -1 0 -0 ] -0 1 1
        ( 24 24 32 ) ( -24 24 32 ) ( -16 16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 -0.986394 -0.827715 ] -0 1 1
        ( -24 -24 32 ) ( 24 -24 32 ) ( 16 -16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 0.986394 0.641451 ] -0 1 1
        ( 24 24 32 ) ( 24 -24 32 ) ( -24 -24 32 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
        ( -16 -16 -16 ) ( 16 16 -16 ) ( -16 16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
    })";

    const auto subtrahendStr = R"({
        ( 29.393876913416079 -16.970562748463635 48 ) ( 16.970562748495468 29.393876913411077 48 ) ( 11.313708499003496 19.595917942278447 -0 ) __TB_empty [ -0.258819 0.965926 0 -0.507559 ] [ -0.158797 -0425496 -0.986394 -0.474791 ] -0 1 1
        ( 32.784609690844263 -8.784609690813113 48 ) ( 8.7846096908451727 32.784609690839488 48 ) ( 5.856406460569815 21.856406460564131 -0 ) __TB_empty [ -0.5 0.866025 0 -0.77533 ] [ -0.142374 -0821995 -0.986394 -0.306396 ] -0 1 1
        ( 33.94112549697229 -0 48 ) ( -0 33.941125496967288 48 ) ( -0 22.627416997982664 -0 ) __TB_empty [ -0.707107 0.707107 0 -0.176551 ] [ -0.116248 -0.116248 -0.986394 -0.683485 ] -0 1 1
        ( 32.784609690844718 8.7846096908399431 48 ) ( -8.7846096908083382 32.784609690839488 48 ) ( -5.8564064605325257 21.856406460564131 -0 ) __TB_empty [ -0.866025 0.5 0 -0124664 ] [ -0821995 -0.142374 -0.986394 -0886002 ] -0 1 1
        ( 29.393876913416534 16.970562748490465 48 ) ( -16.970562748458633 29.393876913411304 48 ) ( -11.313708498966207 19.595917942278675 -0 ) __TB_empty [ -0.965926 0.258819 0 -0.373029 ] [ -0425496 -0.158797 -0.986394 -0235691 ] -0 1 1
        ( -11.313708498966662 -19.595917942252527 -0 ) ( -16.970562748458633 -29.393876913384929 48 ) ( 29.393876913416079 -16.970562748463635 48 ) __TB_empty [ -0425496 0.158797 -0.986394 -0.5189 ] [ -0.965926 -0.258819 0 -00242329 ] -0 1 1
        ( -5.8564064605325257 -21.85640646053821 -0 ) ( -8.7846096908078835 -32.784609690813113 48 ) ( 32.784609690844263 -8.784609690813113 48 ) __TB_empty [ -0821995 0.142374 -0.986394 -0.692604 ] [ -0.866025 -0.5 0 -0709991 ] -0 1 1
        ( -0 -22.627416997956516 -0 ) ( -0 -33.941125496940913 48 ) ( 33.94112549697229 -0 48 ) __TB_empty [ -0.116248 0.116248 -0.986394 -0.515699 ] [ -0.707107 -0.707107 0 -0.689445 ] -0 1 1
        ( 5.856406460569815 -21.856406460537755 -0 ) ( 8.7846096908451727 -32.784609690813113 48 ) ( 32.784609690844718 8.7846096908399431 48 ) __TB_empty [ -0.142374 0821995 -0.986394 -0.437332 ] [ -0.5 -0.866025 0 -0.872314 ] -0 1 1
        ( 11.313708499003496 -19.595917942252072 -0 ) ( 16.970562748495922 -29.393876913384702 48 ) ( 29.393876913416534 16.970562748490465 48 ) __TB_empty [ -0.158797 0425496 -0.986394 -0365772 ] [ -0.258819 -0.965926 0 -0.590811 ] -0 1 1
        ( 16 -16 -0 ) ( 24 -24 48 ) ( 24 24 48 ) __TB_empty [ -0.164399 0 -0.986394 -0.501169 ] [ 0 -1 0 -0 ] -0 1 1
        ( 16.970562748495468 29.393876913411077 48 ) ( -29.3938769133797 16.970562748490465 48 ) ( -19.595917942246615 11.313708498997812 -0 ) __TB_empty [ -0425496 0.158797 0.986394 0.265238 ] [ -0.965926 -0.258819 0 -0.238751 ] -0 1 1
        ( 8.7846096908451727 32.784609690839488 48 ) ( -32.784609690807883 8.7846096908399431 48 ) ( -21.856406460532071 5.8564064605641306 -0 ) __TB_empty [ -0821995 0.142374 0.986394 -0.684406 ] [ -0.866025 -0.5 0 -0.660111 ] -0 1 1
        ( -0 33.941125496967288 48 ) ( -33.941125496935911 -0 48 ) ( -22.627416997950604 -0 -0 ) __TB_empty [ -0.116248 0.116248 0.986394 -0.283369 ] [ -0.707107 -0.707107 0 -0.631095 ] -0 1 1
        ( -8.7846096908083382 32.784609690839488 48 ) ( -32.784609690807883 -8.7846096908135678 48 ) ( -21.856406460532071 -5.8564064605377553 -0 ) __TB_empty [ -0.142374 0821995 0.986394 -0.980953 ] [ -0.5 -0.866025 0 -0.166748 ] -0 1 1
        ( -16.970562748458633 29.393876913411304 48 ) ( -29.393876913379245 -16.970562748463863 48 ) ( -19.595917942246615 -11.313708498971437 -0 ) __TB_empty [ -0.158797 0425496 0.986394 -0.35615 ] [ -0.258819 -0.965926 0 -0.238028 ] -0 1 1
        ( -29.3938769133797 16.970562748490465 48 ) ( -16.970562748458633 -29.393876913384929 48 ) ( -11.313708498966662 -19.595917942252527 -0 ) __TB_empty [ -0.258819 0.965926 0 -0.271353 ] [ -0.158797 -0425496 0.986394 -0.690683 ] -0 1 1
        ( -32.784609690807883 8.7846096908399431 48 ) ( -8.7846096908078835 -32.784609690813113 48 ) ( -5.8564064605325257 -21.85640646053821 -0 ) __TB_empty [ -0.5 0.866025 0 -0.18634 ] [ -0.142374 -0821995 0.986394 -0.298214 ] -0 1 1
        ( -33.941125496935911 -0 48 ) ( -0 -33.941125496940913 48 ) ( -0 -22.627416997956516 -0 ) __TB_empty [ -0.707107 0.707107 0 -0.234839 ] [ -0.116248 -0.116248 0.986394 -0.451246 ] -0 1 1
        ( -32.784609690807883 -8.7846096908135678 48 ) ( 8.7846096908451727 -32.784609690813113 48 ) ( 5.856406460569815 -21.856406460537755 -0 ) __TB_empty [ -0.866025 0.5 0 -0.717973 ] [ -0821995 -0.142374 0.986394 -0.632298 ] -0 1 1
        ( -29.393876913379245 -16.970562748463863 48 ) ( 16.970562748495922 -29.393876913384702 48 ) ( 11.313708499003496 -19.595917942252072 -0 ) __TB_empty [ -0.965926 0.258819 0 -0.72569 ] [ -0425496 -0.158797 0.986394 -0.343115 ] -0 1 1
        ( -24 24 48 ) ( -24 -24 48 ) ( -16 -16 -0 ) __TB_empty [ -0.164399 0 0.986394 -0.596628 ] [ 0 -1 0 -0 ] -0 1 1
        ( 24 24 48 ) ( -24 24 48 ) ( -16 16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 -0.986394 -0454121 ] -0 1 1
        ( -24 -24 48 ) ( 24 -24 48 ) ( 16 -16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 0.986394 0.859102 ] -0 1 1
        ( 24 24 48 ) ( 24 -24 48 ) ( -24 -24 48 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
        ( -16 -16 -0 ) ( 16 16 -0 ) ( -16 16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
    })";

    auto status = TestParserStatus{};
    const auto minuendNodes = NodeReader::read({}, 
      minuendStr, MapFormat::Valve, worldBounds, {}, status, taskManager);
    const auto subtrahendNodes = NodeReader::read({}, 
      subtrahendStr, MapFormat::Valve, worldBounds, {}, status, taskManager);

    REQUIRE(minuendNodes);
    REQUIRE(subtrahendNodes);

    const auto& minuend = static_cast<BrushNode*>(minuendNodes.value().front())->brush();
    const auto& subtrahend =
      static_cast<BrushNode*>(subtrahendNodes.value().front())->brush();

    const auto result =
      minuend.subtract(MapFormat::Valve, worldBounds, "some_material", subtrahend)
      | kdl::fold;
    CHECK_FALSE(result.is_error());

    kdl::col_delete_all(minuendNodes.value());
    kdl::col_delete_all(subtrahendNodes.value());
  }

  SECTION("subtractDome")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/2707

    const auto minuendStr = R"({
        ( -1598.09391534391647838 -277.57717407067275417 -20 ) ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1598.09391534391647838 -277.57717407067275417 -12 ) 128_gold_2 -14.94120025634765625 -108 -0 0.72087001800537109 1
        ( -1178.96031746031826515 -277.57717407067275417 -20 ) ( -1598.09391534391647838 -277.57717407067275417 -20 ) ( -1178.96031746031826515 -277.57717407067275417 -12 ) 128_gold_2 28.92790031433105469 -108 -0 0.8250659704208374 1
        ( -1178.96031746031826515 54.02274375211438695 -20 ) ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1178.96031746031826515 -277.57717407067275417 -20 ) 128_gold_2 -28.98690032958984375 -4.01778984069824219 -0 0.77968800067901611 0.65970498323440552
        ( -1178.96031746031826515 -277.57717407067275417 -12 ) ( -1598.09391534391647838 -277.57717407067275417 -12 ) ( -1178.96031746031826515 54.02274375211438695 -12 ) 128_gold_2 -28.98690032958984375 -4.01778984069824219 -0 0.77968800067901611 0.65970498323440552
        ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1178.96031746031826515 54.02274375211438695 -20 ) ( -1598.09391534391647838 54.02274375211438695 -12 ) 128_gold_2 28.92790031433105469 -108 -0 0.8250659704208374 1
        ( -1178 54.02274375211438695 -20 ) ( -1178 -277.57717407067275417 -20 ) ( -1178 54.02274375211438695 -12 ) 128_gold_2 -14.94120025634765625 -108 -0 0.72087001800537109 1
    })";

    const auto subtrahendPath =
      std::filesystem::current_path() / "fixture/test/mdl/Brush/subtrahend.map";
    const auto subtrahendStr = fs::readTextFile(subtrahendPath);

    auto status = TestParserStatus{};
    const auto minuendNodes = NodeReader::read({}, 
      minuendStr, MapFormat::Standard, worldBounds, {}, status, taskManager);
    const auto subtrahendNodes = NodeReader::read({}, 
      subtrahendStr, MapFormat::Standard, worldBounds, {}, status, taskManager);

    REQUIRE(minuendNodes);
    REQUIRE(subtrahendNodes);

    const auto& minuend = static_cast<BrushNode*>(minuendNodes.value().front())->brush();
    const auto& subtrahend =
      static_cast<BrushNode*>(subtrahendNodes.value().front())->brush();

    const auto result =
      minuend.subtract(MapFormat::Standard, worldBounds, "some_material", subtrahend);

    kdl::col_delete_all(minuendNodes.value());
    kdl::col_delete_all(subtrahendNodes.value());
  }

  SECTION("subtractPipeFromCubeWithMissingFragments")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/pull/1764#issuecomment-296341588
    // subtract creates missing fragments

    const auto minuendStr = R"(
    {
    ( -64 -64 -48 ) ( -64 -63 -48 ) ( -64 -64 -47 ) __TB_empty -0 -0 -0 1 1
    ( 64 64 -16 ) ( 64 64 -15 ) ( 64 65 -16 ) __TB_empty -0 -0 -0 1 1
    ( -64 -64 -48 ) ( -64 -64 -47 ) ( -63 -64 -48 ) __TB_empty -0 -0 -0 1 1
    ( 64 64 -16 ) ( 65 64 -16 ) ( 64 64 -15 ) __TB_empty -0 -0 -0 1 1
    ( 64 64 48 ) ( 64 65 48 ) ( 65 64 48 ) __TB_empty -0 -0 -0 1 1
    ( -64 -64 -48 ) ( -63 -64 -48 ) ( -64 -63 -48 ) __TB_empty -0 -0 -0 1 1
    })";

    const auto subtrahendStr = R"(
      {
      ( 174.71990352490863074 -62.14359353944905706 75.16563707012221585 ) ( 175.1529162268008406 -62.39359353944905706 76.03166247390666399 ) ( 175.60378700139182229 -61.83740732160116238 74.81208367952893923 ) __TB_empty 0.78229904174804688 -0.29628753662109375 338.198577880859375 0.95197159051895142 0.96824586391448975
      ( 36.41270357552525638 -34.54767559718354875 115.33507514292870155 ) ( 36.84571627741747335 -34.79767559718354875 116.2011005467131497 ) ( 36.58948027082188759 -35.46623425072723279 114.98152175233542494 ) __TB_empty -0.04352569580078125 0.71729850769042969 201.0517425537109375 0.98425096273422241 -0.90138787031173706
      ( 199.8900184844443686 -128.93134736624534753 80.25103299325476769 ) ( 200.77390196092756014 -128.62516114839746706 79.89747960266149107 ) ( 200.0667951797410069 -129.84990601978904579 79.89747960266149107 ) __TB_empty -0.59069061279296875 -0.1404876708984375 280.89337158203125 0.93541437387466431 0.93541431427001953
      ( -116.00776749053582648 53.45232440281647257 -189.5058669891937484 ) ( -115.83099079523915975 52.53376574927277431 -189.85942037978702501 ) ( -115.12388401405260652 53.75851062066436725 -189.85942037978702501 ) __TB_empty -0.02112197875976562 -0.22997283935546875 280.89337158203125 0.93541437387466431 0.93541431427001953
      ( 72.6107978708658095 -94.6384909672807737 153.79013823665565042 ) ( 145.00698646154697258 -136.4364499384135172 253.32768142207908113 ) ( 89.58136061934294503 -104.43644993841348878 142.47642973767091235 ) __TB_empty 0.93064975738525391 -0.637969970703125 326.3099365234375 1.27475488185882568 0.96824580430984497
      ( 69.78237074611962498 -79.94155251058168687 159.44699248614801945 ) ( 81.0960792451044199 -60.34563456831627803 159.44699248614801945 ) ( 136.52170508730841902 -92.34563456831628514 270.29824417055618824 ) __TB_empty 0.81418228149414062 0.05062103271484375 -0 1.22474479675292969 0.90138781070709229
      ( 81.0960792451044199 -60.34563456831627803 159.44699248614801945 ) ( 95.23821486883537091 -55.4466550827499276 153.79013823665565042 ) ( 150.66384071103937003 -87.44665508274994181 264.6413899210638192 ) __TB_empty 0.67885684967041016 -0.27746772766113281 338.198577880859375 0.95197159051895142 0.96824586391448975
      ( 95.23821486883537091 -55.4466550827499276 153.79013823665565042 ) ( 112.20877761731250644 -65.24461405388265689 142.47642973767091235 ) ( 167.63440345951653399 -97.2446140538826711 253.32768142207908113 ) __TB_empty 0.16141700744628906 -0.67490577697753906 326.3099365234375 1.27475488185882568 0.96824580430984497
      ( 112.20877761731250644 -65.24461405388265689 142.47642973767091235 ) ( 115.03720474205866253 -79.9415525105817153 136.81957548817854331 ) ( 170.46283058426269008 -111.94155251058172951 247.67082717258671209 ) __TB_empty -0.30159759521484375 0.28987884521484375 201.0517425537109375 0.98425096273422241 -0.90138787031173706
      ( 115.03720474205866253 -79.9415525105817153 136.81957548817854331 ) ( 103.72349624307389604 -99.53747045284714545 136.81957548817854331 ) ( 159.14912208527792359 -131.53747045284714545 247.67082717258671209 ) __TB_empty 0.81418418884277344 0.94775390625 -0 1.22474479675292969 0.90138781070709229
      })";

    auto status = TestParserStatus{};
    const auto minuendNodes = NodeReader::read({}, 
      minuendStr, MapFormat::Standard, worldBounds, {}, status, taskManager);
    const auto subtrahendNodes = NodeReader::read({}, 
      subtrahendStr, MapFormat::Standard, worldBounds, {}, status, taskManager);

    REQUIRE(minuendNodes);
    REQUIRE(subtrahendNodes);

    const auto& minuend = static_cast<BrushNode*>(minuendNodes.value().front())->brush();
    const auto& subtrahend =
      static_cast<BrushNode*>(subtrahendNodes.value().front())->brush();

    const auto fragments =
      minuend.subtract(MapFormat::Standard, worldBounds, "some_material", subtrahend)
      | kdl::fold | kdl::value();
    CHECK(fragments.size() == 8u);

    kdl::col_delete_all(minuendNodes.value());
    kdl::col_delete_all(subtrahendNodes.value());
  }

  SECTION("healEdgesCrash")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/3711

    const auto brushString = R"({
      ( -0 1568 0 ) ( -1 1568 0 ) ( 0 1568 1 ) skip [ 1 0 0 226 ] [ 0 0 -1 65 ] 0 1 1
      ( 0 -0 -768 ) ( 0 -1 -768 ) ( 1 -0 -768 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
      ( -239.52705331405377 141.82523989891524 -302.56049871478172 ) ( -239.52705331405377 141.08932137703414 -302.90546053681464 ) ( -238.79113479217267 141.82523989891524 -303.14310085806937 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
      ( 1153.3138507030453 0 419.38684815140005 ) ( 1153.3138507030453 -0.93979340791702271 419.38684815140005 ) ( 1152.9721076510396 0 420.32664155931707 ) skip [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
      ( -0 323.96740674776811 -1151.884136654764 ) ( 0 323.00475579304475 -1152.154882230192 ) ( 0.96265095472335815 323.96740674776811 -1151.884136654764 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
      ( 214.57570493941603 -0 -1029.9634087926825 ) ( 214.57570493941603 -0.9789804220199585 -1029.9634087926825 ) ( 215.55468536143599 -0 -1029.759454543062 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
      ( -0 0 -457 ) ( -1 0 -457 ) ( -0 1 -457 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
      ( -374.14483546845076 374.14483546845076 -472.60400714822026 ) ( -374.14483546845076 373.47868263355849 -473.13137813754292 ) ( -373.47868263355849 374.14483546845076 -473.13137813754292 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
      ( 310.55043564704465 -112.4406700048321 171.33816909724737 ) ( 310.24823828009175 -113.2753103885525 171.33816909724737 ) ( 310.08994440702008 -112.4406700048321 172.17280948096777 ) skip [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
      ( 0 1395.1726195011288 -977.63914778921753 ) ( -0.81895101070404053 1395.1726195011288 -977.63914778921753 ) ( 0 1395.7464829478413 -976.82019677851349 ) skip [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
    })";

    auto status = TestParserStatus{};
    const auto nodes = NodeReader::read({}, 
      brushString, MapFormat::Valve, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);

    const auto* brushNode = dynamic_cast<BrushNode*>(nodes.value().front());
    REQUIRE(brushNode != nullptr);
    const auto brush = brushNode->brush();

    const auto expectedVertexPositions = std::vector<vm::vec3d>{
      {1146.1054242763166, 1568, -731},
      {992, 1760, -457},
      {1472, 1688.8888096909686, -768},
      {1472, 2240, -457},
      {1472, 2137.9047619137045, -457},
      {1550.6615597858411, 2025.5310596540555, -673.31936915200095},
      {1547.939117410865, 2052.1504971808499, -665.83265250210889},
      {1192.8423120886671, 1568, -768},
      {1472, 1664, -768},
      {1482.6424552678279, 1696.7720374398473, -765.78284224138793},
      {1335.1724026344866, 1760, -457},
      {1416.8275977671274, 1568, -731},
      {1437.2413583822708, 1568, -768},
      {1313.7309227127589, 1688.8888096909686, -768}};

    CHECK(brush.vertexCount() == expectedVertexPositions.size());
    for (const auto& position : expectedVertexPositions)
    {
      CHECK(brush.hasVertex(position, 0.01));
    }

    kdl::col_delete_all(nodes.value());
  }

  SECTION("healEdgesCrash2")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/3655

    const auto brushString = R"({
      ( -2146.248291 -32 32 ) ( -2146.248291 0 0 ) ( -2146.248291 32 32 ) clip 0.0 0.0 0.00 1 1
      ( -1752.348022 -32 32 ) ( -1752.348022 32 32 ) ( -1752.348022 0 0 ) clip 0.0 0.0 0.00 1 1
      ( 32 394.013489 -32 ) ( 0 394.013489 0 ) ( 32 394.013489 32 ) clip 0.0 0.0 0.00 1 1
      ( 32 702 -32 ) ( 32 702 32 ) ( 0 702 0 ) clip 0.0 0.0 0.00 1 1
      ( -32 32 56 ) ( 0 0 56 ) ( 32 32 56 ) clip 0.0 0.0 0.00 1 1
      ( -32 32 152 ) ( 32 32 152 ) ( 0 0 152 ) clip 0.0 0.0 0.00 1 1
      ( -32 1024 -572.760068 ) ( 32 1024 -610.781694 ) ( 0 0 -1421.267185 ) clip 0.0 0.0 0.00 1 1
      ( -32 1024 1919.992436 ) ( 32 1024 1981.037917 ) ( 0 0 2272.431727 ) clip 0.0 0.0 0.00 1 1
      ( -32 1024 1705.511944 ) ( 32 1024 1743.537057 ) ( 0 0 895.028135 ) clip 0.0 0.0 0.00 1 1
      ( -32 1024 -1737.678132 ) ( 32 1024 -1798.728547 ) ( 0 0 -1446.307377 ) clip 0.0 0.0 0.00 1 1
      ( -32 1024 -265.999989 ) ( 32 1024 -265.999989 ) ( 0 0 758.000011 ) clip 0.0 0.0 0.00 1 1
      ( 1024 2574.655757 -32 ) ( 0 1823.662467 0 ) ( 1024 2574.655757 32 ) clip 0.0 0.0 0.00 1 1
      ( -32 1024 566.324133 ) ( 32 1024 566.324133 ) ( 0 0 -263.172866 ) clip 0.0 0.0 0.00 1 1
      ( -2886.746494 -32 1024 ) ( -2886.746494 32 1024 ) ( -1686.721723 0 0 ) clip 0.0 0.0 0.00 1 1
      ( 1024 10157.281306 -32 ) ( 1024 10157.281306 32 ) ( 0 7050.358836 0 ) clip 0.0 0.0 0.00 1 1
      ( -1011.790655 -32 1024 ) ( -2211.877898 0 0 ) ( -1011.790655 32 1024 ) clip 0.0 0.0 0.00 1 1
      ( 1024 -1786.82831 -32 ) ( 0 -1035.766249 0 ) ( 1024 -1786.82831 32 ) clip 0.0 0.0 0.00 1 1
      ( 1024 -7886.438924 -32 ) ( 1024 -7886.438924 32 ) ( 0 -4779.068455 0 ) clip 0.0 0.0 0.00 1 1
    })";

    auto status = TestParserStatus{};
    const auto nodes = NodeReader::read({}, 
      brushString, MapFormat::Standard, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);

    const auto* brushNode = dynamic_cast<BrushNode*>(nodes.value().front());
    REQUIRE(brushNode != nullptr);
    const auto brush = brushNode->brush();

    const auto expectedVertexPositions = std::vector<vm::vec3d>{
      {-2092.334017394151, 702, 56},
      {-2023.3281670401159, 606, 152},
      {-2146.2481304697585, 538.41885284376349, 56},
      {-2033.7399478671873, 574.40914471936856, 152},
      {-1806.2261718297916, 702, 56},
      {-1875.2285583593102, 606, 152},
      {-1949.3656333140334, 512.52402046398004, 152},
      {-1949.3656232649191, 394.01359733974687, 56},
      {-1864.8503999453123, 574.5068228746004, 152},
      {-1752.3480981016994, 538.5046672528224, 56}};

    CHECK(brush.vertexCount() == expectedVertexPositions.size());
    for (const auto& position : expectedVertexPositions)
    {
      CHECK(brush.hasVertex(position, 01));
    }

    kdl::col_delete_all(nodes.value());
  }

  SECTION("healEdgesCrash3")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/3655

    const auto brushString = R"({
      ( 1432 0 0 ) ( 1432 -1 0 ) ( 1432 0 1 ) __TB_empty 0 0 0 1 1
      ( -4112 0 0 ) ( -4112 0 -1 ) ( -4112 1 0 ) __TB_empty 0 0 0 1 1
      ( 0 4072 0 ) ( 0 4072 -1 ) ( 1 4072 0 ) __TB_empty 0 0 0 1 1
      ( -0 296 -0 ) ( -1 296 0 ) ( -0 296 1 ) __TB_empty 0 0 0 1 1
      ( 0 0 824 ) ( -1 0 824 ) ( 0 1 824 ) __TB_empty 0 0 0 1 1
      ( 0 0 -728 ) ( 0 -1 -728 ) ( 1 0 -728 ) __TB_empty 0 0 0 1 1
      ( -0 102.65346284921361 -1026.5346438649867 ) ( 0 101.6584256511469 -1026.6341475833033 ) ( 0.99503719806671143 102.65346284921361 -1026.5346438649867 ) __TB_empty 0 0 0 1 1
      ( 1408 0 0 ) ( 1408 -1 0 ) ( 1408 0 1 ) __TB_empty 0 0 0 1 1
      ( -4064 0 0 ) ( -4064 0 -1 ) ( -4064 1 0 ) __TB_empty 0 0 0 1 1
      ( 0 400 0 ) ( -1 400 0 ) ( 0 400 1 ) __TB_empty 0 0 0 1 1
      ( -0 4048 0 ) ( 0 4048 -1 ) ( 1 4048 0 ) __TB_empty 0 0 0 1 1
      ( 0 -0 768 ) ( -1 0 768 ) ( 0 1 768 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -672 ) ( 0 -1 -672 ) ( 1 0 -672 ) __TB_empty 0 0 0 1 1
      ( -894.72015274048317 -1192.9601740264916 0 ) ( -895.5201527524041 -1192.3601740026497 0 ) ( -894.72015274048317 -1192.9601740264916 0.80000001192092896 ) __TB_empty 0 0 0 1 1
      ( -184.61537878392846 923.076907946961 0 ) ( -185.59595947145135 922.88079181243666 0 ) ( -184.61537878392846 923.076907946961 0.98058068752288818 ) __TB_empty 0 0 0 1 1
      ( -36.923078749280421 -184.61539655186607 0 ) ( -37.903659436803309 -184.41928041734172 0 ) ( -36.923078749280421 -184.61539655186607 0.98058068752288818 ) __TB_empty 0 0 0 1 1
      ( 1376 -0 -0 ) ( 1376 -1 0 ) ( 1376 -0 1 ) __TB_empty 0 0 0 1 1
      ( 0 4016 0 ) ( 0 4016 -1 ) ( 1 4016 0 ) __TB_empty 0 0 0 1 1
      ( 960 0 0 ) ( 960 -1 0 ) ( 960 0 1 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -640 ) ( 0 -1 -640 ) ( 1 0 -640 ) __TB_empty 0 0 0 1 1
      ( 1734.3999917319743 3468.7999834639486 0 ) ( 1734.3999917319743 3468.7999834639486 -0.89442718029022217 ) ( 1735.2944189122645 3468.3527698738035 0 ) __TB_empty 0 0 0 1 1
      ( 2227.199888660427 1113.5999443302135 0 ) ( 2227.6471022505721 1112.7055171499233 0 ) ( 2227.199888660427 1113.5999443302135 0.89442718029022217 ) __TB_empty 0 0 0 1 1
      ( 768 -0 -0 ) ( 768 -1 0 ) ( 768 -0 1 ) __TB_empty 0 0 0 1 1
      ( -0 96.31683601305258 -963.16837455443601 ) ( 0 95.321798814985868 -963.26787827275257 ) ( 0.99503719806671143 96.31683601305258 -963.16837455443601 ) __TB_empty 0 0 0 1 1
      ( 0 3968 0 ) ( 0 3968 -1 ) ( 1 3968 0 ) __TB_empty 0 0 0 1 1
      ( 720 0 0 ) ( 720 -1 0 ) ( 720 0 1 ) __TB_empty 0 0 0 1 1
      ( -0 1280 0 ) ( -1 1280 0 ) ( 0 1280 1 ) __TB_empty 0 0 0 1 1
      ( -0 -35.446153644202241 -283.56922915361793 ) ( 0 -36.43843150484372 -283.44519442103774 ) ( 0.99227786064147949 -35.446153644202241 -283.56922915361793 ) __TB_empty 0 0 0 1 1
      ( -0 3648 0 ) ( 0 3648 -1 ) ( 1 3648 0 ) __TB_empty 0 0 0 1 1
      ( -4048 -0 -0 ) ( -4048 0 -1 ) ( -4048 1 0 ) __TB_empty 0 0 0 1 1
      ( -3808.9754353211611 -423.21950867664054 -0 ) ( -3808.9754353211611 -423.21950867664054 -0.99388372898101807 ) ( -3809.0858668507426 -422.22562494765953 0 ) __TB_empty 0 0 0 1 1
      ( -4032 -0 -0 ) ( -4032 0 -1 ) ( -4032 1 0 ) __TB_empty 0 0 0 1 1
      ( -3770.3467325877136 -452.44161809593606 0 ) ( -3770.3467325877136 -452.44161809593606 -0.99287682771682739 ) ( -3770.4658778097219 -451.44874126821924 0 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -512 ) ( 0 -1 -512 ) ( 1 0 -512 ) __TB_empty 0 0 0 1 1
      ( 512 0 0 ) ( 512 -1 0 ) ( 512 0 1 ) __TB_empty 0 0 0 1 1
      ( 304 0 0 ) ( 304 -1 0 ) ( 304 0 1 ) __TB_empty 0 0 0 1 1
      ( 272 0 0 ) ( 272 -1 0 ) ( 272 0 1 ) __TB_empty 0 0 0 1 1
      ( -4336.9409703233396 1084.2352425808349 -0 ) ( -4336.9409703233396 1084.2352425808349 -0.97014248371124268 ) ( -4336.6984347024118 1085.2053850645461 -0 ) __TB_empty 0 0 0 1 1
      ( -3648 0 0 ) ( -3648 0 -1 ) ( -3648 1 0 ) __TB_empty 0 0 0 1 1
      ( 0 1616 0 ) ( -1 1616 0 ) ( 0 1616 1 ) __TB_empty 0 0 0 1 1
      ( 0 1648 0 ) ( -1 1648 0 ) ( 0 1648 1 ) __TB_empty 0 0 0 1 1
      ( -4018.4468234666565 892.98821707048774 -0 ) ( -4018.4468234666565 892.98821707048774 -0.97618705034255981 ) ( -4018.2298930027464 893.9644041208303 -0 ) __TB_empty 0 0 0 1 1
      ( -776.29989216505055 543.40994157358364 109.97581710904615 ) ( -776.8695310133553 542.59617181583599 109.97581710904615 ) ( -776.18460811702971 543.40994157358364 110.7895868667938 ) __TB_empty 0 0 0 1 1
      ( -665.59996358866192 332.79998179433096 -0 ) ( -666.04717717880703 331.90555461404074 0 ) ( -665.59996358866192 332.79998179433096 0.89442718029022217 ) __TB_empty 0 0 0 1 1
      ( -3040 -0 -0 ) ( -3040 0 -1 ) ( -3040 1 0 ) __TB_empty 0 0 0 1 1
      ( -979.20003890991211 1305.6000194549561 0 ) ( -980.00003892183304 1305.0000194311142 0 ) ( -979.20003890991211 1305.6000194549561 0.80000001192092896 ) __TB_empty 0 0 0 1 1
      ( -431.05877816235079 1724.2351126494032 0 ) ( -432.02892064606203 1723.9925770284754 0 ) ( -431.05877816235079 1724.2351126494032 0.97014248371124268 ) __TB_empty 0 0 0 1 1
      ( -628.41628297374336 1759.5655032334689 0 ) ( -629.35802485749809 1759.2291668293838 0 ) ( -628.41628297374336 1759.5655032334689 0.94174188375473022 ) __TB_empty 0 0 0 1 1
      ( -694.6206937102761 1736.551762145682 0 ) ( -695.54917040152213 1736.1803714751441 0 ) ( -694.6206937102761 1736.551762145682 0.92847669124603271 ) __TB_empty 0 0 0 1 1
      ( 8 0 -0 ) ( 8 -1 0 ) ( 8 0 1 ) __TB_empty 0 0 0 1 1
      ( 1496.4705447024317 897.88228521337442 0 ) ( 1496.9850404328317 897.02479228963784 0 ) ( 1496.4705447024317 897.88228521337442 0.85749292373657227 ) __TB_empty 0 0 0 1 1
      ( -723.26997414682774 263.00725915593284 -0 ) ( -723.6117171988335 262.06746574801582 0 ) ( -723.26997414682774 263.00725915593284 0.93979340791702271 ) __TB_empty 0 0 0 1 1
      ( -982.79995713222888 561.59995622729184 -0 ) ( -983.29609606254962 560.73171306942822 0 ) ( -982.79995713222888 561.59995622729184 0.86824315786361694 ) __TB_empty 0 0 0 1 1
      ( 1548.176962892845 1769.3451404871012 0 ) ( 1548.176962892845 1769.3451404871012 -0.75257670879364014 ) ( 1548.9295396016387 1768.6866358818079 0 ) __TB_empty 0 0 0 1 1
      ( 1055.3103500987709 452.27584477527853 0 ) ( 1055.704269387883 451.35669972761389 0 ) ( 1055.3103500987709 452.27584477527853 0.91914504766464233 ) __TB_empty 0 0 0 1 1
      ( 868.80000520859539 289.59999263858663 -0 ) ( 869.11623297248661 288.65130931711064 0 ) ( 868.80000520859539 289.59999263858663 0.94868332147598267 ) __TB_empty 0 0 0 1 1
      ( 0 0 0 ) ( 0 -1 0 ) ( 0 0 1 ) __TB_empty 0 0 0 1 1
      ( 296.78048166697045 32.975610310104685 0 ) ( 296.89091319655199 31.981726581123667 0 ) ( 296.78048166697045 32.975610310104685 0.99388372898101807 ) __TB_empty 0 0 0 1 1
      ( -938.11758473912778 1563.5293803528766 0 ) ( -938.97507766286435 1563.0148846224765 0 ) ( -938.11758473912778 1563.5293803528766 0.85749292373657227 ) __TB_empty 0 0 0 1 1
      ( -1074.461562958546 716.30768298236944 -0 ) ( -1075.0162631543353 715.47563265888311 0 ) ( -1074.461562958546 716.30768298236944 0.83205032348632813 ) __TB_empty 0 0 0 1 1
      ( 1396.2351468302659 2327.0586858869065 0 ) ( 1396.2351468302659 2327.0586858869065 -0.85749292373657227 ) ( 1397.0926397540024 2326.5441901565064 0 ) __TB_empty 0 0 0 1 1
      ( -2720 0 -0 ) ( -2720 0 -1 ) ( -2720 1 -0 ) __TB_empty 0 0 0 1 1
      ( 0 1664 0 ) ( -1 1664 0 ) ( 0 1664 1 ) __TB_empty 0 0 0 1 1
      ( 722.71694159971958 206.49054514255113 0 ) ( 722.99166271554714 205.52902119245118 0 ) ( 722.71694159971958 206.49054514255113 0.96152395009994507 ) __TB_empty 0 0 0 1 1
      ( -1024 -0 -0 ) ( -1024 -1 0 ) ( -1024 0 1 ) __TB_empty 0 0 0 1 1
      ( 1309.53855152693 1964.3078976478428 0 ) ( 1309.53855152693 1964.3078976478428 -0.83205032348632813 ) ( 1310.3706018504163 1963.7531974520534 0 ) __TB_empty 0 0 0 1 1
      ( 0 3104 0 ) ( 0 3104 -1 ) ( 1 3104 0 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -240 ) ( 0 -1 -240 ) ( 1 0 -240 ) __TB_empty 0 0 0 1 1
      ( -1072 -0 -0 ) ( -1072 -1 0 ) ( -1072 0 1 ) __TB_empty 0 0 0 1 1
      ( -1088 -0 -0 ) ( -1088 -1 0 ) ( -1088 0 1 ) __TB_empty 0 0 0 1 1
      ( -1294.7691393810965 161.84614242263706 -0 ) ( -1294.8931741136767 160.85386456199558 0 ) ( -1294.7691393810965 161.84614242263706 0.99227786064147949 ) __TB_empty 0 0 0 1 1
      ( -1270.2440509069129 1587.8050636336411 0 ) ( -1271.0249197352096 1587.1803685710038 0 ) ( -1270.2440509069129 1587.8050636336411 0.78086882829666138 ) __TB_empty 0 0 0 1 1
      ( 254.5618343744045 159.10114872060331 0 ) ( 255.09183333251531 158.25315039954694 0 ) ( 254.5618343744045 159.10114872060331 0.84799832105636597 ) __TB_empty 0 0 0 1 1
      ( 742.10161543439608 773.02252672266332 0 ) ( 742.10161543439608 773.02252672266332 -0.72138732671737671 ) ( 742.82300276111346 772.32999489855138 0 ) __TB_empty 0 0 0 1 1
      ( 876.79996620785823 1753.5999324157165 0 ) ( 876.79996620785823 1753.5999324157165 -0.89442718029022217 ) ( 877.69439338814846 1753.1527188255714 0 ) __TB_empty 0 0 0 1 1
      ( -0 3040 0 ) ( 0 3040 -1 ) ( 1 3040 0 ) __TB_empty 0 0 0 1 1
      ( 346.58459658669017 2772.6767726935213 0 ) ( 346.58459658669017 2772.6767726935213 -0.99227786064147949 ) ( 347.57687444733165 2772.5527379609412 0 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -224 ) ( 0 -1 -224 ) ( 1 0 -224 ) __TB_empty 0 0 0 1 1
      ( -1296 0 -0 ) ( -1296 -1 0 ) ( -1296 0 1 ) __TB_empty 0 0 0 1 1
      ( -205.77604811122001 -84.420941795069666 -0 ) ( -205.39649175335944 -85.346110428530665 0 ) ( -205.77604811122001 -84.420941795069666 0.92516863346099854 ) __TB_empty 0 0 0 1 1
      ( -980.98099382767759 -118.90678958475246 -0 ) ( -980.86066245833717 -119.89952336132183 0 ) ( -980.98099382767759 -118.90678958475246 0.99273377656936646 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -192 ) ( 0 -1 -192 ) ( 1 0 -192 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -176 ) ( 0 -1 -176 ) ( 1 0 -176 ) __TB_empty 0 0 0 1 1
      ( -0 -0 -160 ) ( 0 -1 -160 ) ( 1 0 -160 ) __TB_empty 0 0 0 1 1
      ( 0 2977.4767698443611 -372.18459623054514 ) ( 0 2977.3527351117809 -373.17687409118662 ) ( 0.99227786064147949 2977.4767698443611 -372.18459623054514 ) __TB_empty 0 0 0 1 1
      ( -2480 -0 -0 ) ( -2480 0 -1 ) ( -2480 1 0 ) __TB_empty 0 0 0 1 1
      ( 0 1816 -0 ) ( -1 1816 0 ) ( 0 1816 1 ) __TB_empty 0 0 0 1 1
      ( -2432 -0 -0 ) ( -2432 0 -1 ) ( -2432 1 0 ) __TB_empty 0 0 0 1 1
      ( 0 2736 0 ) ( 0 2736 -1 ) ( 1 2736 0 ) __TB_empty 0 0 0 1 1
      ( 691.72605933243176 1844.6027857453737 -0 ) ( 691.72605933243176 1844.6027857453737 -0.936329185962677 ) ( 692.66238851839444 1844.2516622931871 0 ) __TB_empty 0 0 0 1 1
      ( -2368 -0 -0 ) ( -2368 0 -1 ) ( -2368 1 0 ) __TB_empty 0 0 0 1 1
      ( -2128 -0 -0 ) ( -2128 0 -1 ) ( -2128 1 0 ) __TB_empty 0 0 0 1 1
      ( -2327.9998818780296 2327.9998818780296 -0 ) ( -2327.9998818780296 2327.9998818780296 -0.70710676908493042 ) ( -2327.2927751089446 2328.7069886471145 -0 ) __TB_empty 0 0 0 1 1
      ( 0 1848 0 ) ( -1 1848 0 ) ( 0 1848 1 ) __TB_empty 0 0 0 1 1
      ( -2470.4002321243315 1852.800220108038 -0 ) ( -2470.4002321243315 1852.800220108038 -0.80000001192092896 ) ( -2469.8002321004897 1853.600220119959 -0 ) __TB_empty 0 0 0 1 1
      ( -2518.5879441080033 629.64698602700082 -0 ) ( -2518.5879441080033 629.64698602700082 -0.97014248371124268 ) ( -2518.3454084870755 630.61712851071206 -0 ) __TB_empty 0 0 0 1 1
      ( 0 0 48 ) ( 0 -1 48 ) ( 1 0 48 ) __TB_empty 0 0 0 1 1
      ( 0 2192 0 ) ( 0 2192 -1 ) ( 1 2192 0 ) __TB_empty 0 0 0 1 1
      ( -0 -0 448 ) ( -1 0 448 ) ( -0 1 448 ) __TB_empty 0 0 0 1 1
      ( -1360 0 -0 ) ( -1360 -1 0 ) ( -1360 0 1 ) __TB_empty 0 0 0 1 1
      ( 0 0 192 ) ( 0 -1 192 ) ( 1 0 192 ) __TB_empty 0 0 0 1 1
      ( 0 2160 -0 ) ( -1 2160 0 ) ( 0 2160 1 ) __TB_empty 0 0 0 1 1
      ( -1890.4613258113386 -0 -236.30766572641733 ) ( -1890.4613258113386 -0.99227786064147949 -236.30766572641733 ) ( -1890.5853605439188 0 -235.31538786577585 ) __TB_empty 0 0 0 1 1
      ( -236.67923339392109 0 828.37735539191272 ) ( -236.67923339392109 -0.96152395009994507 828.37735539191272 ) ( -235.71770944382115 0 828.65207650774028 ) __TB_empty 0 0 0 1 1
    })";

    auto status = TestParserStatus{};
    const auto nodes = NodeReader::read({}, 
      brushString, MapFormat::Standard, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);

    const auto* brushNode = dynamic_cast<BrushNode*>(nodes.value().front());
    REQUIRE(brushNode != nullptr);
    const auto brush = brushNode->brush();

    const auto expectedVertexPositions = std::vector<vm::vec3d>{
      {-1976, 2192, 448},
      {-1976, 2160, 448},
      {-1961.9308279391898, 2160, 335.44835129639506},
      {-1961.9308279391903, 2192, 335.44835129639489},
      {-2128, 2160, 288},
      {-2128, 2192, 288},
      {-2128, 2191.9987624590117, 448},
      {-2128, 2160, 448}};

    CHECK(brush.vertexCount() == expectedVertexPositions.size());
    for (const auto& position : expectedVertexPositions)
    {
      CHECK(brush.hasVertex(position, 01));
    }

    kdl::col_delete_all(nodes.value());
  }

  SECTION("findInitialEdgeFail")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/3898

    const auto brushString = R"({
      ( 832 -0 -0 ) ( 832 0 -1 ) ( 832 1 -0 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 877.8181762695312 0 0 ) ( 877.8181762695312 -1 0 ) ( 877.8181762695312 0 1 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 0 -272 0 ) ( -1 -272 0 ) ( 0 -272 1 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( -0 -256 -0 ) ( 0 -256 -1 ) ( 1 -256 0 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( -0 -0 48 ) ( 0 -1 48 ) ( 1 -0 48 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 0 0 106.90908813476562 ) ( -1 0 106.90908813476562 ) ( 0 1 106.90908813476562 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 779.4157104492188 0 -179.8651580810547 ) ( 779.190850943327 0 -180.8395493030548 ) ( 779.4157104492188 0.9743912220001221 -179.8651580810547 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 132.8000030517578 -0 -265.6000061035156 ) ( 131.9055758714676 0 -266.04721969366074 ) ( 132.8000030517578 0.8944271802902222 -265.6000061035156 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 702.8965454101562 0 -301.2413635253906 ) ( 702.8965454101562 -0.9191450476646423 -301.2413635253906 ) ( 703.2904646992683 0 -300.322218477726 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
      ( 68.79999542236328 0 -206.39999389648438 ) ( 68.79999542236328 -0.9486833214759827 -206.39999389648438 ) ( 69.74867874383926 0 -206.08376613259315 ) e1u1/temp [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1
    })";

    auto status = TestParserStatus{};
    const auto nodes = NodeReader::read({}, 
      brushString, MapFormat::Standard, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    REQUIRE(nodes.value().size() == 1u);

    const auto* brushNode = dynamic_cast<BrushNode*>(nodes.value().front());
    REQUIRE(brushNode != nullptr);
    const auto& brush = brushNode->brush();

    CHECK_THAT(
      brush.vertexPositions(),
      UnorderedApproxVecMatches(
        std::vector<vm::vec3d>{
          {832, -256, 48},
          {841.391, -256, 88.6956},
          {841.391, -272, 88.6956},
          {877.818, -272, 106.909},
          {852.571, -272, 48},
          {852.571, -256, 48},
          {832, -272, 48},
          {877.818, -256, 106.909},
        },
        0.001));
  }

  SECTION("healEdgesFail")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/3886
    // this test would previously fail due on an assertion error

    const auto brushString = R"({
      ( 2176 -16 16 ) ( 2176 0 0 ) ( 2176 16 16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 2304 16 16 ) ( 2304 0 0 ) ( 2304 -16 16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 16 672 -16 ) ( 0 672 0 ) ( 16 672 16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 16 800 16 ) ( 0 800 0 ) ( 16 800 -16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( -16 16 -64 ) ( 0 0 -64 ) ( 16 16 -64 ) custom/graphtallica/brick2 0 0 0 1 1 0 0 0
      ( 16 16 128 ) ( 0 0 128 ) ( -16 16 128 ) custom/graphtallica/brick2 0 0 0 1 1 0 0 0
      ( 256 -1312 -16 ) ( 0 -1568 0 ) ( 256 -1312 16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 256 -1184 16 ) ( 0 -1440 0 ) ( 256 -1184 -16 ) custom/graphtallica/brick2 -720 0 0 1.414214 1 0 0 0
      ( 16 256 8239.985 ) ( 0 0 9055.983 ) ( -16 256 8335.985 ) custom/graphtallica/brick2 1429.894 -476.6323 0 1.378405 1.027402 0 0 0
      ( -16 256 8143.985 ) ( 0 0 8863.983 ) ( 16 256 8047.985 ) custom/graphtallica/brick2 1399.579 -466.5271 0 1.378405 1.027402 0 0 0
      ( 2250.667 16 256 ) ( 2293.333 0 0 ) ( 2250.667 -16 256 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( -16 256 2623.994 ) ( 0 0 4159.991 ) ( 16 256 2623.994 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 2154.667 -16 256 ) ( 2197.333 0 0 ) ( 2154.667 16 256 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 16 256 3199.994 ) ( 0 0 4735.992 ) ( -16 256 3199.994 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 256 2784 16 ) ( 0 3040 0 ) ( 256 2784 -16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
      ( 256 2656.005 -16 ) ( 0 2912.006 0 ) ( 256 2656.005 16 ) custom/graphtallica/brick2 -784 0 0 1.414214 1 0 0 0
    })";

    auto status = TestParserStatus{};
    const auto nodes = NodeReader::read({}, 
      brushString, MapFormat::Quake2, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);
    CHECK(nodes.value().size() == 1u);
  }
}

} // namespace tb::mdl
