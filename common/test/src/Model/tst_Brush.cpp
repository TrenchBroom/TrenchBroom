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

#include "Assets/Texture.h"
#include "Error.h"
#include "Exceptions.h"
#include "FloatType.h"
#include "IO/DiskIO.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/Polyhedron.h"
#include "TestUtils.h"

#include <kdl/intrusive_circular_list.h>
#include <kdl/result.h>
#include <kdl/result_fold.h>
#include <kdl/vector_utils.h>

#include <vecmath/approx.h>
#include <vecmath/polygon.h>
#include <vecmath/ray.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>

#include <fstream>
#include <string>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
static bool canMoveBoundary(
  Brush brush,
  const vm::bbox3& worldBounds,
  const size_t faceIndex,
  const vm::vec3& delta)
{
  return brush.moveBoundary(worldBounds, faceIndex, delta, false)
    .transform([&]() { return worldBounds.contains(brush.bounds()); })
    .value_or(false);
}

TEST_CASE("BrushTest.constructBrushWithFaces")
{
  const vm::bbox3 worldBounds(4096.0);

  // build a cube with length 16 at the origin
  const Brush brush =
    Brush::create(
      worldBounds,
      {
        // left
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0), vm::vec3(0.0, 0.0, 1.0)),
        // right
        createParaxial(
          vm::vec3(16.0, 0.0, 0.0), vm::vec3(16.0, 0.0, 1.0), vm::vec3(16.0, 1.0, 0.0)),
        // front
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 0.0, 1.0), vm::vec3(1.0, 0.0, 0.0)),
        // back
        createParaxial(
          vm::vec3(0.0, 16.0, 0.0), vm::vec3(1.0, 16.0, 0.0), vm::vec3(0.0, 16.0, 1.0)),
        // top
        createParaxial(
          vm::vec3(0.0, 0.0, 16.0), vm::vec3(0.0, 1.0, 16.0), vm::vec3(1.0, 0.0, 16.0)),
        // bottom
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
      })
      .value();

  REQUIRE(brush.fullySpecified());
  REQUIRE(brush.faceCount() == 6u);
  CHECK(brush.findFace(vm::vec3::pos_x()));
  CHECK(brush.findFace(vm::vec3::neg_x()));
  CHECK(brush.findFace(vm::vec3::pos_y()));
  CHECK(brush.findFace(vm::vec3::neg_y()));
  CHECK(brush.findFace(vm::vec3::pos_z()));
  CHECK(brush.findFace(vm::vec3::neg_z()));
}

TEST_CASE("BrushTest.constructBrushWithRedundantFaces")
{
  const vm::bbox3 worldBounds(4096.0);

  CHECK(Brush::create(
          worldBounds,
          {
            createParaxial(
              vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
            createParaxial(
              vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
            createParaxial(
              vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
          })
          .is_error());
}

TEST_CASE("BrushTest.clip")
{
  const vm::bbox3 worldBounds(4096.0);

  const auto left = createParaxial(
    vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0), vm::vec3(0.0, 0.0, 1.0));
  const auto right = createParaxial(
    vm::vec3(16.0, 0.0, 0.0), vm::vec3(16.0, 0.0, 1.0), vm::vec3(16.0, 1.0, 0.0));
  const auto front = createParaxial(
    vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 0.0, 1.0), vm::vec3(1.0, 0.0, 0.0));
  const auto back = createParaxial(
    vm::vec3(0.0, 16.0, 0.0), vm::vec3(1.0, 16.0, 0.0), vm::vec3(0.0, 16.0, 1.0));
  const auto top = createParaxial(
    vm::vec3(0.0, 0.0, 16.0), vm::vec3(0.0, 1.0, 16.0), vm::vec3(1.0, 0.0, 16.0));
  const auto bottom = createParaxial(
    vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0));

  // build a cube with length 16 at the origin
  Brush brush =
    Brush::create(worldBounds, {left, right, front, back, top, bottom}).value();

  BrushFace clip = createParaxial(
    vm::vec3(8.0, 0.0, 0.0), vm::vec3(8.0, 0.0, 1.0), vm::vec3(8.0, 1.0, 0.0));
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
  const vm::bbox3 worldBounds(4096.0);
  Brush brush = Brush::create(
                  worldBounds,
                  {
                    createParaxial(
                      vm::vec3(0.0, 0.0, 0.0),
                      vm::vec3(0.0, 1.0, 0.0),
                      vm::vec3(1.0, 0.0, 1.0)), // left
                    createParaxial(
                      vm::vec3(16.0, 0.0, 0.0),
                      vm::vec3(15.0, 0.0, 1.0),
                      vm::vec3(16.0, 1.0, 0.0)), // right
                    createParaxial(
                      vm::vec3(0.0, 0.0, 0.0),
                      vm::vec3(0.0, 0.0, 1.0),
                      vm::vec3(1.0, 0.0, 0.0)), // front
                    createParaxial(
                      vm::vec3(0.0, 16.0, 0.0),
                      vm::vec3(1.0, 16.0, 0.0),
                      vm::vec3(0.0, 16.0, 1.0)), // back
                    createParaxial(
                      vm::vec3(0.0, 0.0, 6.0),
                      vm::vec3(0.0, 1.0, 6.0),
                      vm::vec3(1.0, 0.0, 6.0)), // top
                    createParaxial(
                      vm::vec3(0.0, 0.0, 0.0),
                      vm::vec3(1.0, 0.0, 0.0),
                      vm::vec3(0.0, 1.0, 0.0)), // bottom
                  })
                  .value();

  REQUIRE(brush.faceCount() == 6u);

  const auto topFaceIndex = brush.findFace(vm::vec3::pos_z());
  REQUIRE(topFaceIndex);

  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, +16.0)));
  CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, -16.0)));
  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, +2.0)));
  CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, -6.0)));
  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, +1.0)));
  CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, -5.0)));

  CHECK(brush.moveBoundary(worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, 1.0), false)
          .is_success());
  CHECK(worldBounds.contains(brush.bounds()));

  CHECK(brush.faces().size() == 6u);
  CHECK(brush.bounds().size().z() == 7.0);
}

TEST_CASE("BrushTest.resizePastWorldBounds")
{
  const vm::bbox3 worldBounds(8192.0);
  const BrushBuilder builder(MapFormat::Standard, worldBounds);

  Brush brush1 = builder
                   .createBrush(
                     std::vector<vm::vec3>{
                       vm::vec3(64, -64, 16),
                       vm::vec3(64, 64, 16),
                       vm::vec3(64, -64, -16),
                       vm::vec3(64, 64, -16),
                       vm::vec3(48, 64, 16),
                       vm::vec3(48, 64, -16)},
                     "texture")
                   .value();

  const auto rightFaceIndex = brush1.findFace(vm::vec3::pos_x());
  REQUIRE(rightFaceIndex);

  CHECK(canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3(16, 0, 0)));
  CHECK(!canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3(8000, 0, 0)));
}

TEST_CASE("BrushTest.expand")
{
  const vm::bbox3 worldBounds(8192.0);
  const BrushBuilder builder(MapFormat::Standard, worldBounds);

  Brush brush1 =
    builder
      .createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture")
      .value();
  CHECK(brush1.expand(worldBounds, 6, true).is_success());

  const vm::bbox3 expandedBBox(vm::vec3(-70, -70, -70), vm::vec3(70, 70, 70));
  const auto expectedVerticesArray = expandedBBox.vertices();
  const auto expectedVertices = std::vector<vm::vec3>(
    std::begin(expectedVerticesArray), std::end(expectedVerticesArray));

  CHECK(brush1.bounds() == expandedBBox);
  CHECK_THAT(brush1.vertexPositions(), Catch::UnorderedEquals(expectedVertices));
}

TEST_CASE("BrushTest.contract")
{
  const vm::bbox3 worldBounds(8192.0);
  const BrushBuilder builder(MapFormat::Standard, worldBounds);

  Brush brush1 =
    builder
      .createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture")
      .value();
  CHECK(brush1.expand(worldBounds, -32, true).is_success());

  const vm::bbox3 expandedBBox(vm::vec3(-32, -32, -32), vm::vec3(32, 32, 32));
  const auto expectedVerticesArray = expandedBBox.vertices();
  const auto expectedVertices = std::vector<vm::vec3>(
    std::begin(expectedVerticesArray), std::end(expectedVerticesArray));

  CHECK(brush1.bounds() == expandedBBox);
  CHECK_THAT(brush1.vertexPositions(), Catch::UnorderedEquals(expectedVertices));
}

TEST_CASE("BrushTest.contractToZero")
{
  const vm::bbox3 worldBounds(8192.0);
  const BrushBuilder builder(MapFormat::Standard, worldBounds);

  Brush brush1 =
    builder
      .createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture")
      .value();
  CHECK(brush1.expand(worldBounds, -64, true).is_error());
}

TEST_CASE("BrushTest.moveVertex")
{
  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom").value();

  const vm::vec3 p1(-32.0, -32.0, -32.0);
  const vm::vec3 p2(-32.0, -32.0, +32.0);
  const vm::vec3 p3(-32.0, +32.0, -32.0);
  const vm::vec3 p4(-32.0, +32.0, +32.0);
  const vm::vec3 p5(+32.0, -32.0, -32.0);
  const vm::vec3 p6(+32.0, -32.0, +32.0);
  const vm::vec3 p7(+32.0, +32.0, -32.0);
  const vm::vec3 p8(+32.0, +32.0, +32.0);
  const vm::vec3 p9(+16.0, +16.0, +32.0);

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, p9 - p8).is_success());
  auto newVertexPositions =
    brush.findClosestVertexPositions(oldVertexPositions + (p9 - p8));

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

  assertTexture("left", brush, p1, p2, p4, p3);
  assertTexture("right", brush, p5, p7, p6);
  assertTexture("right", brush, p6, p7, p9);
  assertTexture("front", brush, p1, p5, p6, p2);
  assertTexture("back", brush, p3, p4, p7);
  assertTexture("back", brush, p4, p9, p7);
  assertTexture("top", brush, p2, p6, p9, p4);
  assertTexture("bottom", brush, p1, p3, p7, p5);

  oldVertexPositions = std::move(newVertexPositions);
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, p8 - p9).is_success());
  newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + (p8 - p9));

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p8));

  assertTexture("left", brush, p1, p2, p4, p3);
  assertTexture("right", brush, p5, p7, p8, p6);
  assertTexture("front", brush, p1, p5, p6, p2);
  assertTexture("back", brush, p3, p4, p8, p7);
  assertTexture("top", brush, p2, p6, p8, p4);
  assertTexture("bottom", brush, p1, p3, p7, p5);
}

TEST_CASE("BrushTest.moveTetrahedronVertexToOpposideSide")
{
  const vm::bbox3 worldBounds(4096.0);

  const vm::vec3 top(0.0, 0.0, +16.0);

  std::vector<vm::vec3> points;
  points.push_back(vm::vec3(-16.0, -16.0, 0.0));
  points.push_back(vm::vec3(+16.0, -16.0, 0.0));
  points.push_back(vm::vec3(0.0, +16.0, 0.0));
  points.push_back(top);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(points, "some_texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({top});
  auto delta = vm::vec3(0.0, 0.0, -32.0);
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(vm::vec3(0.0, 0.0, -16.0)));
  CHECK(brush.fullySpecified());
}

TEST_CASE("BrushTest.moveVertexInwardWithoutMerges")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);
  const vm::vec3d p9(+56.0, +56.0, +56.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

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
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);
  const vm::vec3d p9(+72.0, +72.0, +72.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p9})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p9, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p9})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p9, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p9, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p9})));
}

TEST_CASE("BrushTest.moveVertexWithOneOuterNeighbourMerge")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+56.0, +56.0, +56.0);
  const vm::vec3d p9(+56.0, +56.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p9, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p9, p6, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p9, p7, p4})));
}

TEST_CASE("BrushTest.moveVertexWithTwoOuterNeighbourMerges")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+56.0, +56.0, +56.0);
  const vm::vec3d p9(+64.0, +64.0, +56.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p9, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p9, p4, p6})));
}

TEST_CASE("BrushTest.moveVertexWithAllOuterNeighbourMerges")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+56.0, +56.0, +56.0);
  const vm::vec3d p9(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

  CHECK(newVertexPositions.size() == 1u);
  CHECK(newVertexPositions[0] == vm::approx(p9));

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p9, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p9, p6})));
}

TEST_CASE("BrushTest.moveVertexWithAllInnerNeighbourMerge")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);
  const vm::vec3d p9(0.0, 0.0, 0.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p7)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p4, p6, p7})));
}

TEST_CASE("BrushTest.moveVertexUpThroughPlane")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +56.0);
  const vm::vec3d p9(+64.0, +64.0, +72.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p9, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p9, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p9})));
}

TEST_CASE("BrushTest.moveVertexOntoEdge")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, 0.0);
  const vm::vec3d p9(0.0, 0.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p7)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p4, p6, p7})));
}

TEST_CASE("BrushTest.moveVertexOntoIncidentVertex")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p7 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p7)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p4, p6, p7})));
}

TEST_CASE("BrushTest.moveVertexOntoIncidentVertexInOppositeDirection")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p7});
  auto delta = p8 - p7;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p8)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p8)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p8)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p8)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p8, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p8})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p8, p6})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p8, p5})));
}

TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);
  const vm::vec3d p9(+80.0, +64.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p6});
  auto delta = p9 - p6;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p5, p9})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p9, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p9})));
}

TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex2")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);
  const vm::vec3d p9(+80.0, -64.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p8});
  auto delta = p9 - p8;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p9)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p9)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p9, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p9, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p4, p9, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p9})));
}

TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithDeletingVertex")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);
  const vm::vec3d p9(+80.0, 0.0, +64.0);
  const vm::vec3d p10(+64.0, 0.0, +64.0);

  std::vector<vm::vec3d> oldPositions;
  oldPositions.push_back(p1);
  oldPositions.push_back(p2);
  oldPositions.push_back(p3);
  oldPositions.push_back(p4);
  oldPositions.push_back(p5);
  oldPositions.push_back(p6);
  oldPositions.push_back(p7);
  oldPositions.push_back(p8);
  oldPositions.push_back(p9);

  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createBrush(oldPositions, "texture").value();

  auto oldVertexPositions = std::vector<vm::vec3>({p9});
  auto delta = p10 - p9;
  CHECK(brush.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
  auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

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

  CHECK(brush.hasEdge(vm::segment3d(p1, p2)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p3)));
  CHECK(brush.hasEdge(vm::segment3d(p1, p5)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p2, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p4)));
  CHECK(brush.hasEdge(vm::segment3d(p3, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p4, p8)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p6)));
  CHECK(brush.hasEdge(vm::segment3d(p5, p7)));
  CHECK(brush.hasEdge(vm::segment3d(p6, p8)));
  CHECK(brush.hasEdge(vm::segment3d(p7, p8)));

  CHECK(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
  CHECK(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
  CHECK(brush.hasFace(vm::polygon3d({p2, p6, p8, p4})));
  CHECK(brush.hasFace(vm::polygon3d({p3, p4, p8, p7})));
  CHECK(brush.hasFace(vm::polygon3d({p5, p7, p8, p6})));
}

TEST_CASE("BrushTest.moveVerticesPastWorldBounds")
{
  const vm::bbox3 worldBounds(8192.0);
  const BrushBuilder builder(MapFormat::Standard, worldBounds);

  Model::Brush brush = builder.createCube(128.0, "texture").value();

  std::vector<vm::vec3> allVertexPositions;
  for (const auto* vertex : brush.vertices())
  {
    allVertexPositions.push_back(vertex->position());
  }

  CHECK(brush.canMoveVertices(worldBounds, allVertexPositions, vm::vec3(16, 0, 0)));
  CHECK_FALSE(
    brush.canMoveVertices(worldBounds, allVertexPositions, vm::vec3(8192, 0, 0)));
}

static void assertCanMoveVertices(
  Brush brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);

  CHECK(brush.canMoveVertices(worldBounds, vertexPositions, delta));

  REQUIRE(brush.moveVertices(worldBounds, vertexPositions, delta).is_success());

  auto movedVertexPositions = brush.findClosestVertexPositions(vertexPositions + delta);
  movedVertexPositions =
    kdl::vec_sort_and_remove_duplicates(std::move(movedVertexPositions));

  auto expectedVertexPositions = vertexPositions + delta;
  expectedVertexPositions =
    kdl::vec_sort_and_remove_duplicates(std::move(expectedVertexPositions));

  CHECK(movedVertexPositions == expectedVertexPositions);
}

// "Move point" tests

static void assertMovingVerticesDeletes(
  Brush brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);

  CHECK(brush.canMoveVertices(worldBounds, vertexPositions, delta));

  REQUIRE(brush.moveVertices(worldBounds, vertexPositions, delta).is_success());
  const std::vector<vm::vec3> movedVertexPositions =
    brush.findClosestVertexPositions(vertexPositions + delta);
  CHECK(movedVertexPositions.empty());
}

static void assertCanNotMoveVertices(
  const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);
  CHECK_FALSE(brush.canMoveVertices(worldBounds, vertexPositions, delta));
}

static void assertCanMoveVertex(
  const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta)
{
  assertCanMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
}

static void assertMovingVertexDeletes(
  const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta)
{
  assertMovingVerticesDeletes(brush, std::vector<vm::vec3>{vertexPosition}, delta);
}

static void assertCanNotMoveVertex(
  const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta)
{
  assertCanNotMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
}

// NOTE: Different than movePolygonRemainingPoint, because in this case we allow
// point moves that flip the normal of the remaining polygon
TEST_CASE("BrushTest.movePointRemainingPolygon")
{
  const vm::bbox3 worldBounds(4096.0);

  const vm::vec3 peakPosition(0.0, 0.0, +64.0);
  const std::vector<vm::vec3> baseQuadVertexPositions{
    vm::vec3(-64.0, -64.0, -64.0), // base quad
    vm::vec3(-64.0, +64.0, -64.0),
    vm::vec3(+64.0, +64.0, -64.0),
    vm::vec3(+64.0, -64.0, -64.0)};
  const std::vector<vm::vec3> vertexPositions =
    kdl::vec_concat(std::vector<vm::vec3>{peakPosition}, baseQuadVertexPositions);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -127.0));
  assertCanNotMoveVertex(
    brush, peakPosition, vm::vec3(0.0, 0.0, -128.0)); // Onto the base quad plane
  assertCanMoveVertex(
    brush,
    peakPosition,
    vm::vec3(0.0, 0.0, -129.0)); // Through the other side of the base quad

  // More detailed testing of the last assertion
  {
    auto brushCopy = brush;
    std::vector<vm::vec3> temp(baseQuadVertexPositions);
    std::reverse(temp.begin(), temp.end());
    const std::vector<vm::vec3> flippedBaseQuadVertexPositions(temp);

    const vm::vec3 delta(0.0, 0.0, -129.0);

    CHECK(brushCopy.faceCount() == 5u);
    CHECK(brushCopy.findFace(vm::polygon3(baseQuadVertexPositions)));
    CHECK_FALSE(brushCopy.findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
    CHECK(brushCopy.findFace(vm::vec3::neg_z()));
    CHECK_FALSE(brushCopy.findFace(vm::vec3::pos_z()));

    const auto oldVertexPositions = std::vector<vm::vec3>({peakPosition});
    CHECK(brushCopy.canMoveVertices(worldBounds, oldVertexPositions, delta));
    REQUIRE(brushCopy.moveVertices(worldBounds, oldVertexPositions, delta).is_success());
    const auto newVertexPositions =
      brushCopy.findClosestVertexPositions(oldVertexPositions + delta);
    CHECK(newVertexPositions == oldVertexPositions + delta);

    CHECK(brushCopy.faceCount() == 5u);
    CHECK_FALSE(brushCopy.findFace(vm::polygon3(baseQuadVertexPositions)));
    CHECK(brushCopy.findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
    CHECK_FALSE(brushCopy.findFace(vm::vec3::neg_z()));
    CHECK(brushCopy.findFace(vm::vec3::pos_z()));
  }

  assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -127.0));
  assertCanNotMoveVertex(
    brush, peakPosition, vm::vec3(256.0, 0.0, -128.0)); // Onto the base quad plane
  assertCanMoveVertex(
    brush, peakPosition, vm::vec3(256.0, 0.0, -129.0)); // Flips the normal of the base
                                                        // quad, without moving through it
}

TEST_CASE("BrushTest.movePointRemainingPolyhedron")
{
  const vm::bbox3 worldBounds(4096.0);

  const vm::vec3 peakPosition(0.0, 0.0, 128.0);
  const std::vector<vm::vec3> vertexPositions{
    vm::vec3(-64.0, -64.0, 0.0), // base quad
    vm::vec3(-64.0, +64.0, 0.0),
    vm::vec3(+64.0, +64.0, 0.0),
    vm::vec3(+64.0, -64.0, 0.0),
    vm::vec3(-64.0, -64.0, 64.0), // upper quad
    vm::vec3(-64.0, +64.0, 64.0),
    vm::vec3(+64.0, +64.0, 64.0),
    vm::vec3(+64.0, -64.0, 64.0),
    peakPosition};

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  assertMovingVertexDeletes(
    brush, peakPosition, vm::vec3(0.0, 0.0, -65.0)); // Move inside the remaining cuboid
  assertCanMoveVertex(
    brush,
    peakPosition,
    vm::vec3(0.0, 0.0, -63.0)); // Slightly above the top of the cuboid is OK
  assertCanNotMoveVertex(
    brush,
    peakPosition,
    vm::vec3(0.0, 0.0, -129.0)); // Through and out the other side is disallowed
}

// add vertex tests

// TODO: add tests for Brush::addVertex

// remove vertex tests

TEST_CASE("BrushTest.removeSingleVertex")
{
  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createCube(64.0, "asdf").value();

  CHECK(brush
          .removeVertices(
            worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, +32.0)))
          .is_success());

  CHECK(brush.vertexCount() == 7u);
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));

  CHECK(brush
          .removeVertices(
            worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, -32.0)))
          .is_success());

  CHECK(brush.vertexCount() == 6u);
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));

  CHECK(brush
          .removeVertices(
            worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, +32.0)))
          .is_success());

  CHECK(brush.vertexCount() == 5u);
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));

  CHECK(brush
          .removeVertices(
            worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, -32.0)))
          .is_success());

  CHECK(brush.vertexCount() == 4u);
  CHECK_FALSE(brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
  CHECK(brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
  CHECK(brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
  CHECK_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));

  CHECK_FALSE(brush.canRemoveVertices(
    worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, +32.0))));
  CHECK_FALSE(brush.canRemoveVertices(
    worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, -32.0))));
  CHECK_FALSE(brush.canRemoveVertices(
    worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, +32.0))));
  CHECK_FALSE(brush.canRemoveVertices(
    worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, -32.0))));
}

TEST_CASE("BrushTest.removeMultipleVertices")
{
  const vm::bbox3 worldBounds(4096.0);
  BrushBuilder builder(MapFormat::Standard, worldBounds);

  std::vector<vm::vec3> vertices;
  vertices.push_back(vm::vec3(-32.0, -32.0, -32.0));
  vertices.push_back(vm::vec3(-32.0, -32.0, +32.0));
  vertices.push_back(vm::vec3(-32.0, +32.0, -32.0));
  vertices.push_back(vm::vec3(-32.0, +32.0, +32.0));
  vertices.push_back(vm::vec3(+32.0, -32.0, -32.0));
  vertices.push_back(vm::vec3(+32.0, -32.0, +32.0));
  vertices.push_back(vm::vec3(+32.0, +32.0, -32.0));
  vertices.push_back(vm::vec3(+32.0, +32.0, +32.0));

  for (size_t i = 0; i < 6; ++i)
  {
    for (size_t j = i + 1; j < 7; ++j)
    {
      for (size_t k = j + 1; k < 8; ++k)
      {
        std::vector<vm::vec3> toRemove;
        toRemove.push_back(vertices[i]);
        toRemove.push_back(vertices[j]);
        toRemove.push_back(vertices[k]);

        Brush brush = builder.createBrush(vertices, "asdf").value();
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
  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom").value();

  const vm::vec3 p1(-32.0, -32.0, -32.0);
  const vm::vec3 p2(-32.0, -32.0, +32.0);
  const vm::vec3 p3(-32.0, +32.0, -32.0);
  const vm::vec3 p4(-32.0, +32.0, +32.0);
  const vm::vec3 p5(+32.0, -32.0, -32.0);
  const vm::vec3 p6(+32.0, -32.0, +32.0);
  const vm::vec3 p7(+32.0, +32.0, -32.0);
  const vm::vec3 p8(+32.0, +32.0, +32.0);
  const vm::vec3 p1_2(-32.0, -32.0, -16.0);
  const vm::vec3 p2_2(-32.0, -32.0, +48.0);

  assertTexture("left", brush, p1, p2, p4, p3);
  assertTexture("right", brush, p5, p7, p8, p6);
  assertTexture("front", brush, p1, p5, p6, p2);
  assertTexture("back", brush, p3, p4, p8, p7);
  assertTexture("top", brush, p2, p6, p8, p4);
  assertTexture("bottom", brush, p1, p3, p7, p5);

  const auto originalEdge = vm::segment(p1, p2);
  auto oldEdgePositions = std::vector<vm::segment3>({originalEdge});
  auto delta = p1_2 - p1;
  CHECK(brush.moveEdges(worldBounds, oldEdgePositions, delta).is_success());
  auto newEdgePositions = brush.findClosestEdgePositions(kdl::vec_transform(
    oldEdgePositions, [&](const auto& s) { return s.translate(delta); }));

  CHECK(newEdgePositions.size() == 1u);
  CHECK(newEdgePositions[0] == vm::segment3(p1_2, p2_2));

  assertTexture("left", brush, p1_2, p2_2, p4, p3);
  assertTexture("right", brush, p5, p7, p8, p6);
  assertTexture("front", brush, p1_2, p5, p6, p2_2);
  assertTexture("back", brush, p3, p4, p8, p7);
  assertTexture("top", brush, p2_2, p6, p8);
  assertTexture("top", brush, p2_2, p8, p4);
  assertTexture("bottom", brush, p1_2, p3, p5);
  assertTexture("bottom", brush, p3, p7, p5);

  CHECK(brush.canMoveEdges(worldBounds, newEdgePositions, p1 - p1_2));

  oldEdgePositions = std::move(newEdgePositions);
  delta = p1 - p1_2;
  CHECK(brush.moveEdges(worldBounds, oldEdgePositions, delta).is_success());
  newEdgePositions = brush.findClosestEdgePositions(kdl::vec_transform(
    oldEdgePositions, [&](const auto& s) { return s.translate(delta); }));

  CHECK(newEdgePositions.size() == 1u);
  CHECK(newEdgePositions[0] == originalEdge);

  assertTexture("left", brush, p1, p2, p4, p3);
  assertTexture("right", brush, p5, p7, p8, p6);
  assertTexture("front", brush, p1, p5, p6, p2);
  assertTexture("back", brush, p3, p4, p8, p7);
  assertTexture("top", brush, p2, p6, p8, p4);
  assertTexture("bottom", brush, p1, p3, p7, p5);
}

static void assertCanMoveEdges(
  Brush brush, const std::vector<vm::segment3> edges, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);

  std::vector<vm::segment3> expectedMovedEdges;
  for (const vm::segment3& edge : edges)
  {
    expectedMovedEdges.push_back(vm::segment3(edge.start() + delta, edge.end() + delta));
  }

  CHECK(brush.canMoveEdges(worldBounds, edges, delta));
  CHECK(brush.moveEdges(worldBounds, edges, delta).is_success());
  const auto movedEdges = brush.findClosestEdgePositions(
    kdl::vec_transform(edges, [&](const auto& s) { return s.translate(delta); }));
  CHECK(movedEdges == expectedMovedEdges);
}

static void assertCanNotMoveEdges(
  const Brush& brush, const std::vector<vm::segment3> edges, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);
  CHECK_FALSE(brush.canMoveEdges(worldBounds, edges, delta));
}

TEST_CASE("BrushTest.moveEdgeRemainingPolyhedron")
{
  const vm::bbox3 worldBounds(4096.0);

  // Taller than the cube, starts to the left of the +-64 unit cube
  const vm::segment3 edge(vm::vec3(-128, 0, -128), vm::vec3(-128, 0, +128));

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createCube(128, Model::BrushFaceAttributes::NoTextureName).value();
  CHECK(brush.addVertex(worldBounds, edge.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge.end()).is_success());

  CHECK(brush.vertexCount() == 10u);

  assertCanMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+63, 0, 0));
  assertCanNotMoveEdges(
    brush,
    std::vector<vm::segment3>{edge},
    vm::vec3(+64, 0, 0)); // On the side of the cube
  assertCanNotMoveEdges(
    brush, std::vector<vm::segment3>{edge}, vm::vec3(+128, 0, 0)); // Center of the cube

  assertCanMoveVertices(
    brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+63, 0, 0));
  assertCanMoveVertices(
    brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+64, 0, 0));
  assertCanMoveVertices(
    brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+128, 0, 0));
}

// Same as above, but moving 2 edges
TEST_CASE("BrushTest.moveEdgesRemainingPolyhedron")
{
  const vm::bbox3 worldBounds(4096.0);

  // Taller than the cube, starts to the left of the +-64 unit cube
  const vm::segment3 edge1(vm::vec3(-128, -32, -128), vm::vec3(-128, -32, +128));
  const vm::segment3 edge2(vm::vec3(-128, +32, -128), vm::vec3(-128, +32, +128));
  const std::vector<vm::segment3> movingEdges{edge1, edge2};

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createCube(128, Model::BrushFaceAttributes::NoTextureName).value();
  CHECK(brush.addVertex(worldBounds, edge1.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge1.end()).is_success());
  CHECK(brush.addVertex(worldBounds, edge2.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge2.end()).is_success());

  CHECK(brush.vertexCount() == 12u);

  assertCanMoveEdges(brush, movingEdges, vm::vec3(+63, 0, 0));
  assertCanNotMoveEdges(
    brush, movingEdges, vm::vec3(+64, 0, 0)); // On the side of the cube
  assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+128, 0, 0)); // Center of the cube

  assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+63, 0, 0));
  assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+64, 0, 0));
  assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+128, 0, 0));
}

// "Move face" tests

TEST_CASE("BrushTest.moveFace")
{
  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush = builder.createCube(64.0, "asdf").value();

  std::vector<vm::vec3> vertexPositions(4);
  vertexPositions[0] = vm::vec3(-32.0, -32.0, +32.0);
  vertexPositions[1] = vm::vec3(+32.0, -32.0, +32.0);
  vertexPositions[2] = vm::vec3(+32.0, +32.0, +32.0);
  vertexPositions[3] = vm::vec3(-32.0, +32.0, +32.0);

  const vm::polygon3 face(vertexPositions);

  CHECK(brush.canMoveFaces(
    worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(-16.0, -16.0, 0.0)));

  auto oldFacePositions = std::vector<vm::polygon3>({face});
  auto delta = vm::vec3(-16.0, -16.0, 0.0);
  CHECK(brush.moveFaces(worldBounds, oldFacePositions, delta).is_success());
  auto newFacePositions = brush.findClosestFacePositions(kdl::vec_transform(
    oldFacePositions, [&](const auto& f) { return f.translate(delta); }));

  CHECK(newFacePositions.size() == 1u);
  CHECK(newFacePositions[0].hasVertex(vm::vec3(-48.0, -48.0, +32.0)));
  CHECK(newFacePositions[0].hasVertex(vm::vec3(-48.0, +16.0, +32.0)));
  CHECK(newFacePositions[0].hasVertex(vm::vec3(+16.0, +16.0, +32.0)));
  CHECK(newFacePositions[0].hasVertex(vm::vec3(+16.0, -48.0, +32.0)));

  oldFacePositions = std::move(newFacePositions);
  delta = vm::vec3(16.0, 16.0, 0.0);
  CHECK(brush.moveFaces(worldBounds, oldFacePositions, delta).is_success());
  newFacePositions = brush.findClosestFacePositions(kdl::vec_transform(
    oldFacePositions, [&](const auto& f) { return f.translate(delta); }));

  CHECK(newFacePositions.size() == 1u);
  CHECK(newFacePositions[0].vertices().size() == 4u);
  for (size_t i = 0; i < 4; ++i)
    CHECK(newFacePositions[0].hasVertex(face.vertices()[i]));
}

TEST_CASE("BrushNodeTest.cannotMoveFace")
{
  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder
      .createCuboid(
        vm::vec3(128.0, 128.0, 32.0), Model::BrushFaceAttributes::NoTextureName)
      .value();

  std::vector<vm::vec3> vertexPositions(4);
  vertexPositions[0] = vm::vec3(-64.0, -64.0, -16.0);
  vertexPositions[1] = vm::vec3(+64.0, -64.0, -16.0);
  vertexPositions[2] = vm::vec3(+64.0, -64.0, +16.0);
  vertexPositions[3] = vm::vec3(-64.0, -64.0, +16.0);

  const vm::polygon3 face(vertexPositions);

  CHECK_FALSE(brush.canMoveFaces(
    worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(0.0, 128.0, 0.0)));
}

static void assertCanMoveFaces(
  Brush brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);

  std::vector<vm::polygon3> expectedMovedFaces;
  for (const vm::polygon3& polygon : movingFaces)
  {
    expectedMovedFaces.push_back(vm::polygon3(polygon.vertices() + delta));
  }

  CHECK(brush.canMoveFaces(worldBounds, movingFaces, delta));
  CHECK(brush.moveFaces(worldBounds, movingFaces, delta).is_success());
  const auto movedFaces = brush.findClosestFacePositions(
    kdl::vec_transform(movingFaces, [&](const auto& f) { return f.translate(delta); }));
  CHECK(movedFaces == expectedMovedFaces);
}

static void assertCanNotMoveFaces(
  const Brush& brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);
  CHECK_FALSE(brush.canMoveFaces(worldBounds, movingFaces, delta));
}

static void assertCanMoveFace(
  const Brush& brush, const std::optional<size_t>& topFaceIndex, const vm::vec3 delta)
{
  REQUIRE(topFaceIndex);
  const BrushFace& topFace = brush.face(*topFaceIndex);
  assertCanMoveFaces(brush, std::vector<vm::polygon3>{topFace.polygon()}, delta);
}

static void assertCanNotMoveFace(
  const Brush& brush, const std::optional<size_t>& topFaceIndex, const vm::vec3 delta)
{
  const vm::bbox3 worldBounds(4096.0);

  REQUIRE(topFaceIndex);
  const BrushFace& topFace = brush.face(*topFaceIndex);
  CHECK_FALSE(
    brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>{topFace.polygon()}, delta));
}

static void assertCanMoveTopFace(const Brush& brush, const vm::vec3 delta)
{
  assertCanMoveFace(brush, brush.findFace(vm::vec3::pos_z()), delta);
}

static void assertCanNotMoveTopFace(const Brush& brush, const vm::vec3 delta)
{
  assertCanNotMoveFace(brush, brush.findFace(vm::vec3::pos_z()), delta);
}

static void assertCanNotMoveTopFaceBeyond127UnitsDown(const Brush& brush)
{
  assertCanMoveTopFace(brush, vm::vec3(0, 0, -127));
  assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -128));
  assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -129));

  assertCanMoveTopFace(brush, vm::vec3(256, 0, -127));
  assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -128));
  assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -129));
}

TEST_CASE("BrushTest.movePolygonRemainingPoint")
{
  const vm::bbox3 worldBounds(4096.0);

  const std::vector<vm::vec3> vertexPositions{
    vm::vec3(-64.0, -64.0, +64.0), // top quad
    vm::vec3(-64.0, +64.0, +64.0),
    vm::vec3(+64.0, -64.0, +64.0),
    vm::vec3(+64.0, +64.0, +64.0),

    vm::vec3(0.0, 0.0, -64.0), // bottom point
  };

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingEdge")
{
  const vm::bbox3 worldBounds(4096.0);

  const std::vector<vm::vec3> vertexPositions{
    vm::vec3(-64.0, -64.0, +64.0), // top quad
    vm::vec3(-64.0, +64.0, +64.0),
    vm::vec3(+64.0, -64.0, +64.0),
    vm::vec3(+64.0, +64.0, +64.0),

    vm::vec3(-64.0, 0.0, -64.0), // bottom edge, on the z=-64 plane
    vm::vec3(+64.0, 0.0, -64.0)};

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingPolygon")
{
  const vm::bbox3 worldBounds(4096.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createCube(128.0, Model::BrushFaceAttributes::NoTextureName).value();

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingPolygon2")
{
  const vm::bbox3 worldBounds(4096.0);

  // Same brush as movePolygonRemainingPolygon, but this particular order of vertices
  // triggers a failure in Brush::doCanMoveVertices where the polygon inserted into the
  // "remaining" BrushGeometry gets the wrong normal.
  const std::vector<vm::vec3> vertexPositions{
    vm::vec3(+64.0, +64.0, +64.0),
    vm::vec3(+64.0, -64.0, +64.0),
    vm::vec3(+64.0, -64.0, -64.0),
    vm::vec3(+64.0, +64.0, -64.0),
    vm::vec3(-64.0, -64.0, +64.0),
    vm::vec3(-64.0, -64.0, -64.0),
    vm::vec3(-64.0, +64.0, -64.0),
    vm::vec3(-64.0, +64.0, +64.0)};

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();
  CHECK(brush.bounds() == vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)));

  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
}

TEST_CASE("BrushTest.movePolygonRemainingPolygon_DisallowVertexCombining")
{
  const vm::bbox3 worldBounds(4096.0);

  //       z = +192  //
  // |\              //
  // | \             //
  // |  \  z = +64   //
  // |   |           //
  // |___| z = -64   //
  //                 //

  const std::vector<vm::vec3> vertexPositions{
    vm::vec3(-64.0, -64.0, +192.0), // top quad, slanted
    vm::vec3(-64.0, +64.0, +192.0),
    vm::vec3(+64.0, -64.0, +64.0),
    vm::vec3(+64.0, +64.0, +64.0),

    vm::vec3(-64.0, -64.0, -64.0), // bottom quad
    vm::vec3(-64.0, +64.0, -64.0),
    vm::vec3(+64.0, -64.0, -64.0),
    vm::vec3(+64.0, +64.0, -64.0),
  };

  const vm::vec3 topFaceNormal(sqrt(2.0) / 2.0, 0.0, sqrt(2.0) / 2.0);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  const auto topFaceIndex = brush.findFace(topFaceNormal);
  assertCanMoveFace(brush, topFaceIndex, vm::vec3(0, 0, -127));
  assertCanMoveFace(
    brush,
    topFaceIndex,
    vm::vec3(0, 0, -128)); // Merge 2 verts of the moving polygon with 2 in the
                           // remaining polygon, should be allowed
  assertCanNotMoveFace(brush, topFaceIndex, vm::vec3(0, 0, -129));
}

TEST_CASE("BrushTest.movePolygonRemainingPolyhedron")
{
  const vm::bbox3 worldBounds(4096.0);

  //   _   z = +64   //
  //  / \            //
  // /   \           //
  // |   | z = -64   //
  // |   |           //
  // |___| z = -192  //
  //                 //

  const std::vector<vm::vec3> smallerTopPolygon{
    vm::vec3(-32.0, -32.0, +64.0), // smaller top polygon
    vm::vec3(-32.0, +32.0, +64.0),
    vm::vec3(+32.0, -32.0, +64.0),
    vm::vec3(+32.0, +32.0, +64.0)};
  const std::vector<vm::vec3> cubeTopFace{
    vm::vec3(-64.0, -64.0, -64.0), // top face of cube
    vm::vec3(-64.0, +64.0, -64.0),
    vm::vec3(+64.0, -64.0, -64.0),
    vm::vec3(+64.0, +64.0, -64.0),
  };
  const std::vector<vm::vec3> cubeBottomFace{
    vm::vec3(-64.0, -64.0, -192.0), // bottom face of cube
    vm::vec3(-64.0, +64.0, -192.0),
    vm::vec3(+64.0, -64.0, -192.0),
    vm::vec3(+64.0, +64.0, -192.0),
  };

  const std::vector<vm::vec3> vertexPositions =
    kdl::vec_concat(smallerTopPolygon, cubeTopFace, cubeBottomFace);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  // Try to move the top face down along the Z axis
  assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
  assertCanNotMoveTopFace(
    brush,
    vm::vec3(0.0, 0.0, -257.0)); // Move top through the polyhedron and out the bottom

  // Move the smaller top polygon as 4 separate vertices
  assertCanMoveVertices(brush, smallerTopPolygon, vm::vec3(0, 0, -127));
  assertMovingVerticesDeletes(brush, smallerTopPolygon, vm::vec3(0, 0, -128));
  assertMovingVerticesDeletes(brush, smallerTopPolygon, vm::vec3(0, 0, -129));
  assertCanNotMoveVertices(
    brush,
    smallerTopPolygon,
    vm::vec3(0, 0, -257)); // Move through the polyhedron and out the bottom

  // Move top face along the X axis
  assertCanMoveTopFace(brush, vm::vec3(32.0, 0.0, 0.0));
  assertCanMoveTopFace(brush, vm::vec3(256, 0.0, 0.0));
  assertCanMoveTopFace(
    brush,
    vm::vec3(-32.0, -32.0, 0.0)); // Causes face merging and a vert to be deleted at z=-64
}

TEST_CASE("BrushTest.moveTwoFaces")
{
  const vm::bbox3 worldBounds(4096.0);

  //               //
  // |\    z = 64  //
  // | \           //
  // |  \          //
  // A|   \ z = 0   //
  // |   /         //
  // |__/C         //
  //  B    z = -64 //
  //               //

  const std::vector<vm::vec3> leftPolygon{
    // A
    vm::vec3(-32.0, -32.0, +64.0),
    vm::vec3(-32.0, +32.0, +64.0),
    vm::vec3(-32.0, +32.0, -64.0),
    vm::vec3(-32.0, -32.0, -64.0),
  };
  const std::vector<vm::vec3> bottomPolygon{
    // B
    vm::vec3(-32.0, -32.0, -64.0),
    vm::vec3(-32.0, +32.0, -64.0),
    vm::vec3(+0.0, +32.0, -64.0),
    vm::vec3(+0.0, -32.0, -64.0),
  };
  const std::vector<vm::vec3> bottomRightPolygon{
    // C
    vm::vec3(+0.0, -32.0, -64.0),
    vm::vec3(+0.0, +32.0, -64.0),
    vm::vec3(+32.0, +32.0, +0.0),
    vm::vec3(+32.0, -32.0, +0.0),
  };

  const std::vector<vm::vec3> vertexPositions =
    kdl::vec_concat(leftPolygon, bottomPolygon, bottomRightPolygon);

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName)
      .value();

  CHECK(brush.hasFace(vm::polygon3(leftPolygon)));
  CHECK(brush.hasFace(vm::polygon3(bottomPolygon)));
  CHECK(brush.hasFace(vm::polygon3(bottomRightPolygon)));

  assertCanMoveFaces(
    brush,
    std::vector<vm::polygon3>{vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon)},
    vm::vec3(0, 0, 63));
  assertCanNotMoveFaces(
    brush,
    std::vector<vm::polygon3>{vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon)},
    vm::vec3(0, 0, 64)); // Merges B and C
}

// "Move polyhedron" tests

TEST_CASE("BrushNodeTest.movePolyhedronRemainingEdge")
{
  const vm::bbox3 worldBounds(4096.0);

  // Edge to the left of the cube, shorter, extends down to Z=-256
  const vm::segment3 edge(vm::vec3(-128, 0, -256), vm::vec3(-128, 0, 0));

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  Brush brush =
    builder.createCube(128, Model::BrushFaceAttributes::NoTextureName).value();
  CHECK(brush.addVertex(worldBounds, edge.start()).is_success());
  CHECK(brush.addVertex(worldBounds, edge.end()).is_success());

  CHECK(brush.vertexCount() == 10u);

  const auto cubeTopIndex = brush.findFace(vm::vec3::pos_z());
  const auto cubeBottomIndex = brush.findFace(vm::vec3::neg_z());
  const auto cubeRightIndex = brush.findFace(vm::vec3::pos_x());
  const auto cubeLeftIndex = brush.findFace(vm::vec3::neg_x());
  const auto cubeBackIndex = brush.findFace(vm::vec3::pos_y());
  const auto cubeFrontIndex = brush.findFace(vm::vec3::neg_y());

  CHECK(cubeTopIndex);
  CHECK_FALSE(cubeBottomIndex); // no face here, part of the wedge connecting to `edge`
  CHECK(cubeRightIndex);
  CHECK_FALSE(cubeLeftIndex); // no face here, part of the wedge connecting to `edge`
  CHECK(cubeFrontIndex);
  CHECK(cubeBackIndex);

  const BrushFace& cubeTop = brush.face(*cubeTopIndex);
  const BrushFace& cubeRight = brush.face(*cubeRightIndex);
  const BrushFace& cubeFront = brush.face(*cubeFrontIndex);
  const BrushFace& cubeBack = brush.face(*cubeBackIndex);

  const std::vector<vm::polygon3> movingFaces{
    cubeTop.polygon(),
    cubeRight.polygon(),
    cubeFront.polygon(),
    cubeBack.polygon(),
  };

  assertCanMoveFaces(brush, movingFaces, vm::vec3(32, 0, 0)); // away from `edge`
  assertCanMoveFaces(
    brush, movingFaces, vm::vec3(-63, 0, 0)); // towards `edge`, not touching
  assertCanMoveFaces(brush, movingFaces, vm::vec3(-64, 0, 0)); // towards `edge`, touching
  assertCanMoveFaces(brush, movingFaces, vm::vec3(-65, 0, 0)); // towards `edge`, covering

  // Move the cube down 64 units, so the top vertex of `edge` is on the same plane as
  // `cubeTop` This will turn `cubeTop` from a quad into a pentagon
  assertCanNotMoveFaces(brush, movingFaces, vm::vec3(0, 0, -64));
  assertCanMoveVertices(brush, asVertexList(movingFaces), vm::vec3(0, 0, -64));

  // Make edge poke through the top face
  assertCanNotMoveFaces(brush, movingFaces, vm::vec3(-192, 0, -128));
  assertCanNotMoveVertices(brush, asVertexList(movingFaces), vm::vec3(-192, 0, -128));
}

// UV Lock tests

template <MapFormat F>
class UVLockTest
{
  MapFormat param = F;
};

TEST_CASE("moveFaceWithUVLock")
{
  auto format = GENERATE(MapFormat::Valve, MapFormat::Standard);

  const vm::bbox3 worldBounds(4096.0);

  Assets::Texture testTexture("testTexture", 64, 64);

  BrushBuilder builder(format, worldBounds);
  Brush brush = builder.createCube(64.0, "").value();
  for (auto& face : brush.faces())
  {
    face.setTexture(&testTexture);
  }

  const auto delta = vm::vec3(+8.0, 0.0, 0.0);
  const auto polygonToMove =
    vm::polygon3(brush.face(*brush.findFace(vm::vec3::pos_z())).vertexPositions());
  CHECK(brush.canMoveFaces(worldBounds, {polygonToMove}, delta));

  // move top face by x=+8
  auto changed = brush;
  auto changedWithUVLock = brush;

  REQUIRE(changed.moveFaces(worldBounds, {polygonToMove}, delta, false).is_success());
  REQUIRE(
    changedWithUVLock.moveFaces(worldBounds, {polygonToMove}, delta, true).is_success());

  // The move should be equivalent to shearing by this matrix
  const auto M = vm::shear_bbox_matrix(brush.bounds(), vm::vec3::pos_z(), delta);

  for (auto& oldFace : brush.faces())
  {
    const auto oldTexCoords = kdl::vec_transform(
      oldFace.vertexPositions(), [&](auto x) { return oldFace.textureCoords(x); });
    const auto shearedVertexPositions =
      kdl::vec_transform(oldFace.vertexPositions(), [&](auto x) { return M * x; });
    const auto shearedPolygon = vm::polygon3(shearedVertexPositions);

    const auto normal = oldFace.boundary().normal;

    // The brush modified without texture lock is expected to have changed UV's on some
    // faces, but not on others
    {
      const auto newFaceIndex = changed.findFace(shearedPolygon);
      REQUIRE(newFaceIndex);
      const BrushFace& newFace = changed.face(*newFaceIndex);
      const auto newTexCoords = kdl::vec_transform(
        shearedVertexPositions, [&](auto x) { return newFace.textureCoords(x); });
      if (
        normal == vm::vec3::pos_z() || normal == vm::vec3::pos_y()
        || normal == vm::vec3::neg_y())
      {
        CHECK_FALSE(UVListsEqual(oldTexCoords, newTexCoords));
        // TODO: actually check the UV's
      }
      else
      {
        CHECK(UVListsEqual(oldTexCoords, newTexCoords));
      }
    }

    // UV's should all be the same when using texture lock (with Valve format).
    // Standard format can only do UV lock on the top face, which is not sheared.
    {
      const auto newFaceWithUVLockIndex = changedWithUVLock.findFace(shearedPolygon);
      REQUIRE(newFaceWithUVLockIndex);
      const BrushFace& newFaceWithUVLock =
        changedWithUVLock.face(*newFaceWithUVLockIndex);
      const auto newTexCoordsWithUVLock = kdl::vec_transform(
        shearedVertexPositions,
        [&](auto x) { return newFaceWithUVLock.textureCoords(x); });
      if (normal == vm::vec3d::pos_z() || (format == MapFormat::Valve))
      {
        CHECK(UVListsEqual(oldTexCoords, newTexCoordsWithUVLock));
      }
    }
  }
}

TEST_CASE("BrushTest.subtractCuboidFromCuboid")
{
  const vm::bbox3 worldBounds(4096.0);

  const std::string minuendTexture("minuend");
  const std::string subtrahendTexture("subtrahend");
  const std::string defaultTexture("default");

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  const Brush minuend =
    builder
      .createCuboid(
        vm::bbox3(vm::vec3(-32.0, -16.0, -32.0), vm::vec3(32.0, 16.0, 32.0)),
        minuendTexture)
      .value();
  const Brush subtrahend =
    builder
      .createCuboid(
        vm::bbox3(vm::vec3(-16.0, -32.0, -64.0), vm::vec3(16.0, 32.0, 0.0)),
        subtrahendTexture)
      .value();

  const auto fragments =
    kdl::fold_results(
      minuend.subtract(MapFormat::Standard, worldBounds, defaultTexture, subtrahend))
      .value();
  CHECK(fragments.size() == 3u);

  const Brush* left = nullptr;
  const Brush* top = nullptr;
  const Brush* right = nullptr;

  for (const Brush& brush : fragments)
  {
    if (brush.findFace(vm::plane3(32.0, vm::vec3::neg_x())))
    {
      left = &brush;
    }
    else if (brush.findFace(vm::plane3(32.0, vm::vec3::pos_x())))
    {
      right = &brush;
    }
    else if (brush.findFace(vm::plane3(16.0, vm::vec3::neg_x())))
    {
      top = &brush;
    }
  }

  CHECK(left != nullptr);
  CHECK(top != nullptr);
  CHECK(right != nullptr);

  // left brush faces
  CHECK(left->faceCount() == 6u);
  CHECK(left->findFace(vm::plane3(-16.0, vm::vec3::pos_x())));
  CHECK(left->findFace(vm::plane3(+32.0, vm::vec3::neg_x())));
  CHECK(left->findFace(vm::plane3(+16.0, vm::vec3::pos_y())));
  CHECK(left->findFace(vm::plane3(+16.0, vm::vec3::neg_y())));
  CHECK(left->findFace(vm::plane3(+32.0, vm::vec3::pos_z())));
  CHECK(left->findFace(vm::plane3(+32.0, vm::vec3::neg_z())));

  // left brush textures
  CHECK(
    left->face(*left->findFace(vm::vec3::pos_x())).attributes().textureName()
    == subtrahendTexture);
  CHECK(
    left->face(*left->findFace(vm::vec3::neg_x())).attributes().textureName()
    == minuendTexture);
  CHECK(
    left->face(*left->findFace(vm::vec3::pos_y())).attributes().textureName()
    == minuendTexture);
  CHECK(
    left->face(*left->findFace(vm::vec3::neg_y())).attributes().textureName()
    == minuendTexture);
  CHECK(
    left->face(*left->findFace(vm::vec3::pos_z())).attributes().textureName()
    == minuendTexture);
  CHECK(
    left->face(*left->findFace(vm::vec3::neg_z())).attributes().textureName()
    == minuendTexture);

  // top brush faces
  CHECK(top->faceCount() == 6u);
  CHECK(top->findFace(vm::plane3(+16.0, vm::vec3::pos_x())));
  CHECK(top->findFace(vm::plane3(+16.0, vm::vec3::neg_x())));
  CHECK(top->findFace(vm::plane3(+16.0, vm::vec3::pos_y())));
  CHECK(top->findFace(vm::plane3(+16.0, vm::vec3::neg_y())));
  CHECK(top->findFace(vm::plane3(+32.0, vm::vec3::pos_z())));
  CHECK(top->findFace(vm::plane3(0.0, vm::vec3::neg_z())));

  // top brush textures
  CHECK(
    top->face(*top->findFace(vm::vec3::pos_x())).attributes().textureName()
    == subtrahendTexture);
  CHECK(
    top->face(*top->findFace(vm::vec3::neg_x())).attributes().textureName()
    == subtrahendTexture);
  CHECK(
    top->face(*top->findFace(vm::vec3::pos_y())).attributes().textureName()
    == minuendTexture);
  CHECK(
    top->face(*top->findFace(vm::vec3::neg_y())).attributes().textureName()
    == minuendTexture);
  CHECK(
    top->face(*top->findFace(vm::vec3::pos_z())).attributes().textureName()
    == minuendTexture);
  CHECK(
    top->face(*top->findFace(vm::vec3::neg_z())).attributes().textureName()
    == subtrahendTexture);

  // right brush faces
  CHECK(right->faceCount() == 6u);
  CHECK(right->findFace(vm::plane3(+32.0, vm::vec3::pos_x())));
  CHECK(right->findFace(vm::plane3(-16.0, vm::vec3::neg_x())));
  CHECK(right->findFace(vm::plane3(+16.0, vm::vec3::pos_y())));
  CHECK(right->findFace(vm::plane3(+16.0, vm::vec3::neg_y())));
  CHECK(right->findFace(vm::plane3(+32.0, vm::vec3::pos_z())));
  CHECK(right->findFace(vm::plane3(+32.0, vm::vec3::neg_z())));

  // right brush textures
  CHECK(
    right->face(*right->findFace(vm::vec3::pos_x())).attributes().textureName()
    == minuendTexture);
  CHECK(
    right->face(*right->findFace(vm::vec3::neg_x())).attributes().textureName()
    == subtrahendTexture);
  CHECK(
    right->face(*right->findFace(vm::vec3::pos_y())).attributes().textureName()
    == minuendTexture);
  CHECK(
    right->face(*right->findFace(vm::vec3::neg_y())).attributes().textureName()
    == minuendTexture);
  CHECK(
    right->face(*right->findFace(vm::vec3::pos_z())).attributes().textureName()
    == minuendTexture);
  CHECK(
    right->face(*right->findFace(vm::vec3::neg_z())).attributes().textureName()
    == minuendTexture);
}

TEST_CASE("BrushTest.subtractDisjoint")
{
  const vm::bbox3 worldBounds(4096.0);

  const vm::bbox3 brush1Bounds(vm::vec3::fill(-8.0), vm::vec3::fill(+8.0));
  const vm::bbox3 brush2Bounds(
    vm::vec3(124.0, 124.0, -4.0), vm::vec3(132.0, 132.0, +4.0));
  CHECK_FALSE(brush1Bounds.intersects(brush2Bounds));

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  const Brush brush1 = builder.createCuboid(brush1Bounds, "texture").value();
  const Brush brush2 = builder.createCuboid(brush2Bounds, "texture").value();

  const auto fragments =
    kdl::fold_results(
      brush1.subtract(MapFormat::Standard, worldBounds, "texture", brush2))
      .value();
  CHECK(fragments.size() == 1u);

  const Brush& subtraction = fragments.at(0);
  CHECK_THAT(
    subtraction.vertexPositions(), Catch::UnorderedEquals(brush1.vertexPositions()));
}

TEST_CASE("BrushTest.subtractEnclosed")
{
  const vm::bbox3 worldBounds(4096.0);

  const vm::bbox3 brush1Bounds(vm::vec3::fill(-8.0), vm::vec3::fill(+8.0));
  const vm::bbox3 brush2Bounds(vm::vec3::fill(-9.0), vm::vec3::fill(+9.0));
  CHECK(brush1Bounds.intersects(brush2Bounds));

  BrushBuilder builder(MapFormat::Standard, worldBounds);
  const Brush brush1 = builder.createCuboid(brush1Bounds, "texture").value();
  const Brush brush2 = builder.createCuboid(brush2Bounds, "texture").value();

  const auto fragments =
    kdl::fold_results(
      brush1.subtract(MapFormat::Standard, worldBounds, "texture", brush2))
      .value();
  CHECK(fragments.empty());
}
} // namespace Model
} // namespace TrenchBroom
