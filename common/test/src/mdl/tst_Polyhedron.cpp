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

#include "mdl/Polyhedron.h"
#include "mdl/Polyhedron_DefaultPayload.h"
#include "mdl/Polyhedron_IO.h" // IWYU pragma: keep
#include "mdl/Polyhedron_Instantiation.h"

#include "vm/vec.h"
#include "vm/vec_io.h"

#include <algorithm>
#include <iterator>
#include <set>

#include "Catch2.h"

namespace tb::mdl
{
namespace
{
using Polyhedron3d =
  Polyhedron<double, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
using PVertex = Polyhedron3d::Vertex;
using VertexList = Polyhedron3d::VertexList;
using PEdge = Polyhedron3d::Edge;
using PHalfEdge = Polyhedron3d::HalfEdge;
using PFace = Polyhedron3d::Face;

using EdgeInfo = std::pair<vm::vec3d, vm::vec3d>;
using EdgeInfoList = std::vector<EdgeInfo>;

bool hasEdges(
  const Polyhedron3d& p, const EdgeInfoList& edgeInfos, const double epsilon = 0.0)
{
  return p.edgeCount() == edgeInfos.size()
         && std::ranges::all_of(edgeInfos, [&](const auto& edgeInfo) {
              return p.hasEdge(edgeInfo.first, edgeInfo.second, epsilon);
            });
}

bool hasFaces(
  const Polyhedron3d& p,
  const std::vector<std::vector<vm::vec3d>> faceInfos,
  const double epsilon = 0.0)
{
  return p.faceCount() == faceInfos.size()
         && std::ranges::all_of(
           faceInfos, [&](const auto& faceInfo) { return p.hasFace(faceInfo, epsilon); });
}

bool mutuallyIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs)
{
  return lhs.intersects(rhs) && rhs.intersects(lhs);
}

bool mutuallyNotIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs)
{
  return !lhs.intersects(rhs) && !rhs.intersects(lhs);
}

} // namespace

TEST_CASE("PolyhedronTest.constructEmpty")
{
  auto p = Polyhedron3d{};
  CHECK(p.empty());
}

TEST_CASE("PolyhedronTest.constructWithOnePoint")
{
  const auto p1 = vm::vec3d{-8, -8, -8};

  auto p = Polyhedron3d{p1};

  CHECK_FALSE(p.empty());
  CHECK(p.point());
  CHECK_FALSE(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1}));
}

TEST_CASE("PolyhedronTest.constructWithTwoIdenticalPoints")
{
  const auto p1 = vm::vec3d{-8, -8, -8};

  auto p = Polyhedron3d{p1, p1};

  CHECK_FALSE(p.empty());
  CHECK(p.point());
  CHECK_FALSE(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1}));
}

TEST_CASE("PolyhedronTest.constructWithTwoPoints")
{
  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{3, 0, 0};

  auto p = Polyhedron3d{p1, p2};

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1, p2}));
}

TEST_CASE("PolyhedronTest.constructWithThreeColinearPoints")
{
  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{3, 0, 0};
  const auto p3 = vm::vec3d{6, 0, 0};

  auto p = Polyhedron3d{p1, p2, p3};

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1, p3}));
}

TEST_CASE("PolyhedronTest.constructWithThreePoints")
{
  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{3, 0, 0};
  const auto p3 = vm::vec3d{6, 5, 0};

  auto p = Polyhedron3d{p1, p2, p3};

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK_FALSE(p.edge());
  CHECK(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1, p2, p3}));
}

TEST_CASE("PolyhedronTest.constructTriangleWithContainedPoint")
{
  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{6, 0, 0};
  const auto p3 = vm::vec3d{3, 6, 0};
  const auto p4 = vm::vec3d{3, 3, 0};

  auto p = Polyhedron3d{p1, p2, p3, p4};

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK_FALSE(p.edge());
  CHECK(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1, p2, p3}));
}

TEST_CASE("PolyhedronTest.constructWithFourCoplanarPoints")
{
  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{6, 0, 0};
  const auto p3 = vm::vec3d{3, 3, 0};
  const auto p4 = vm::vec3d{3, 6, 0};

  auto p = Polyhedron3d{p1, p2, p3, p4};

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK_FALSE(p.edge());
  CHECK(p.polygon());
  CHECK_FALSE(p.polyhedron());

  CHECK(p.hasAllVertices({p1, p2, p4}));
}

TEST_CASE("PolyhedronTest.constructWith4Points")
{
  const auto p1 = vm::vec3d{0, 0, 8};
  const auto p2 = vm::vec3d{8, 0, 0};
  const auto p3 = vm::vec3d{-8, 0, 0};
  const auto p4 = vm::vec3d{0, 8, 0};

  const auto p = Polyhedron3d{p1, p2, p3, p4};
  CHECK(p.closed());

  CHECK(p.hasAllVertices({p1, p2, p3, p4}));
  CHECK(hasEdges(p, {{p2, p3}, {p3, p4}, {p4, p2}, {p1, p3}, {p1, p2}, {p4, p1}}));
  CHECK(hasFaces(p, {{p2, p3, p4}, {p1, p3, p2}, {p1, p2, p4}, {p1, p4, p3}}));
}

TEST_CASE("PolyhedronTest.constructRectangleWithRedundantPoint")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/1659
  /*
   p4 p5 p3
   *--+--*
   |     |
   |     |
   *-----*
   p1    p2
   */

  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{+32, 0, 0};
  const auto p3 = vm::vec3d{+32, +32, 0};
  const auto p4 = vm::vec3d{0, +32, 0};
  const auto p5 = vm::vec3d{+16, +32, 0};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5};

  CHECK(p.hasAllVertices({p1, p2, p3, p4}));
}

TEST_CASE("PolyhedronTest.constructTrapezoidWithRedundantPoint")
{
  /*
   p4    p3 p5
   *-----*--+
   |       /
   |      /
   *-----*
   p1    p2
   */

  const auto p1 = vm::vec3d{0, 0, 0};
  const auto p2 = vm::vec3d{+32, 0, 0};
  const auto p3 = vm::vec3d{+32, +32, 0};
  const auto p4 = vm::vec3d{0, +32, 0};
  const auto p5 = vm::vec3d{+40, +32, 0};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5};

  CHECK(p.hasAllVertices({p1, p2, p4, p5}));
}

TEST_CASE("PolyhedronTest.constructPolygonWithRedundantPoint")
{
  auto p = Polyhedron3d{
    {-64.0, 64.0, -16.0},
    {64.0, 64.0, -16.0},
    {22288.0, 18208.0, 16.0},
    // does not get added due to all incident faces being coplanar:
    {22288.0, 18336.0, 16.0},
    {22416.0, 18336.0, 16.0},
  };

  CHECK(p.hasAllVertices(
    {
      {-64.0, 64.0, -16.0},
      {64.0, 64.0, -16.0},
      {22288.0, 18208.0, 16.0},
      {22416.0, 18336.0, 16.0},
    },
    0.0));
}

TEST_CASE("PolyhedronTest.constructTetrahedonWithRedundantPoint")
{
  const auto p1 = vm::vec3d{0, 4, 8};
  const auto p2 = vm::vec3d{8, 0, 0};
  const auto p3 = vm::vec3d{-8, 0, 0};
  const auto p4 = vm::vec3d{0, 8, 0};
  const auto p5 = vm::vec3d{0, 4, 12};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5};
  CHECK(p.closed());

  CHECK(p.hasAllVertices({p5, p2, p3, p4}));
  CHECK(hasEdges(p, {{p2, p3}, {p3, p4}, {p4, p2}, {p5, p3}, {p5, p2}, {p4, p5}}));
  CHECK(hasFaces(p, {{p2, p3, p4}, {p5, p3, p2}, {p5, p2, p4}, {p5, p4, p3}}));
}

TEST_CASE("PolyhedronTest.constructTetrahedonWithCoplanarFaces")
{
  const auto p1 = vm::vec3d{0, 0, 8};
  const auto p2 = vm::vec3d{8, 0, 0};
  const auto p3 = vm::vec3d{-8, 0, 0};
  const auto p4 = vm::vec3d{0, 8, 0};
  const auto p5 = vm::vec3d{0, 0, 12};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5};
  CHECK(p.closed());

  CHECK(p.hasAllVertices({p5, p2, p3, p4}));
  CHECK(hasEdges(p, {{p2, p3}, {p3, p4}, {p4, p2}, {p5, p3}, {p5, p2}, {p4, p5}}));
  CHECK(hasFaces(p, {{p2, p3, p4}, {p5, p3, p2}, {p5, p2, p4}, {p5, p4, p3}}));
}

TEST_CASE("PolyhedronTest.constructCube")
{
  const auto p1 = vm::vec3d{-8, -8, -8};
  const auto p2 = vm::vec3d{-8, -8, +8};
  const auto p3 = vm::vec3d{-8, +8, -8};
  const auto p4 = vm::vec3d{-8, +8, +8};
  const auto p5 = vm::vec3d{+8, -8, -8};
  const auto p6 = vm::vec3d{+8, -8, +8};
  const auto p7 = vm::vec3d{+8, +8, -8};
  const auto p8 = vm::vec3d{+8, +8, +8};

  const auto p = Polyhedron3d{p1, p2, p3, p4, p5, p6, p7, p8};

  CHECK(p.closed());
  CHECK(p.hasAllVertices({p1, p2, p3, p4, p5, p6, p7, p8}));
  CHECK(hasEdges(
    p,
    {{p1, p2},
     {p1, p3},
     {p1, p5},
     {p2, p4},
     {p2, p6},
     {p3, p4},
     {p3, p7},
     {p4, p8},
     {p5, p6},
     {p5, p7},
     {p6, p8},
     {p7, p8}}));
  CHECK(hasFaces(
    p,
    {{p1, p5, p6, p2},
     {p3, p1, p2, p4},
     {p7, p3, p4, p8},
     {p5, p7, p8, p6},
     {p3, p7, p5, p1},
     {p2, p6, p8, p4}}));
}

TEST_CASE("PolyhedronTest.copy")
{
  const auto p1 = vm::vec3d{0, 0, 8};
  const auto p2 = vm::vec3d{8, 0, 0};
  const auto p3 = vm::vec3d{-8, 0, 0};
  const auto p4 = vm::vec3d{0, 8, 0};

  CHECK(Polyhedron3d{} == (Polyhedron3d{} = Polyhedron3d{}));
  CHECK(Polyhedron3d{p1} == (Polyhedron3d{} = Polyhedron3d{p1}));
  CHECK(Polyhedron3d{p1, p2} == (Polyhedron3d{} = Polyhedron3d{p1, p2}));
  CHECK(Polyhedron3d{p1, p2, p3} == (Polyhedron3d{} = Polyhedron3d{p1, p2, p3}));
  CHECK(Polyhedron3d{p1, p2, p3, p4} == (Polyhedron3d{} = Polyhedron3d{p1, p2, p3, p4}));
}

TEST_CASE("PolyhedronTest.swap")
{
  const auto p1 = vm::vec3d{0, 0, 8};
  const auto p2 = vm::vec3d{8, 0, 0};
  const auto p3 = vm::vec3d{-8, 0, 0};
  const auto p4 = vm::vec3d{0, 8, 0};

  auto original = Polyhedron3d{p1, p2, p3, p4};
  auto other = Polyhedron3d{p2, p3, p4};

  auto lhs = original;
  auto rhs = other;

  // Just to be sure...
  assert(lhs == original);
  assert(rhs == other);

  using std::swap;
  swap(lhs, rhs);

  CHECK(lhs == other);
  CHECK(rhs == original);

  CHECK(lhs.bounds() == other.bounds());
  CHECK(rhs.bounds() == original.bounds());
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlane")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5, p6, p7, p8};

  CHECK(p.clip({vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}).success());

  const auto d = vm::vec3d{0, 0, -64};
  CHECK(hasEdges(
    p,
    {{p1, p2 + d},
     {p1, p3},
     {p1, p5},
     {p2 + d, p4 + d},
     {p2 + d, p6 + d},
     {p3, p4 + d},
     {p3, p7},
     {p4 + d, p8 + d},
     {p5, p6 + d},
     {p5, p7},
     {p6 + d, p8 + d},
     {p7, p8 + d}}));
  CHECK(hasFaces(
    p,
    {{p1, p2 + d, p4 + d, p3},
     {p1, p3, p7, p5},
     {p1, p5, p6 + d, p2 + d},
     {p2 + d, p6 + d, p8 + d, p4 + d},
     {p3, p4 + d, p8 + d, p7},
     {p5, p7, p8 + d, p6 + d}}));
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAtTop")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5, p6, p7, p8};

  CHECK(p.clip({vm::vec3d{0, 0, 64}, vm::vec3d{0, 0, 1}}).unchanged());

  CHECK(hasEdges(
    p,
    {{p1, p2},
     {p1, p3},
     {p1, p5},
     {p2, p4},
     {p2, p6},
     {p3, p4},
     {p3, p7},
     {p4, p8},
     {p5, p6},
     {p5, p7},
     {p6, p8},
     {p7, p8}}));
  CHECK(hasFaces(
    p,
    {{p1, p2, p4, p3},
     {p1, p3, p7, p5},
     {p1, p5, p6, p2},
     {p2, p6, p8, p4},
     {p3, p4, p8, p7},
     {p5, p7, p8, p6}}));
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAboveTop")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5, p6, p7, p8};
  CHECK(p.clip({vm::vec3d{0, 0, 72}, vm::vec3d{0, 0, 1}}).unchanged());

  CHECK(hasEdges(
    p,
    {{p1, p2},
     {p1, p3},
     {p1, p5},
     {p2, p4},
     {p2, p6},
     {p3, p4},
     {p3, p7},
     {p4, p8},
     {p5, p6},
     {p5, p7},
     {p6, p8},
     {p7, p8}}));
  CHECK(hasFaces(
    p,
    {{p1, p2, p4, p3},
     {p1, p3, p7, p5},
     {p1, p5, p6, p2},
     {p2, p6, p8, p4},
     {p3, p4, p8, p7},
     {p5, p7, p8, p6}}));
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAtBottom")
{
  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p8 = vm::vec3d{+64, +64, +64};

  auto p = Polyhedron3d{p1, p2, p3, p4, p5, p6, p7, p8};

  CHECK(p.clip({vm::vec3d{0, 0, -64}, vm::vec3d{0, 0, 1}}).empty());
}

TEST_CASE("PolyhedronTest.clipCubeWithSlantedPlane")
{
  auto p = Polyhedron3d{vm::bbox3d{64.0}};

  CHECK(p.clip({vm::vec3d{64, 64, 0}, vm::normalize(vm::vec3d{1, 1, 1})}).success());

  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};
  const auto p7 = vm::vec3d{+64, +64, -64};
  const auto p9 = vm::vec3d{+64, 0, +64};
  const auto p10 = vm::vec3d{0, +64, +64};
  const auto p11 = vm::vec3d{+64, +64, 0};

  CHECK(p.hasAllVertices({p1, p2, p3, p4, p5, p6, p7, p9, p10, p11}, 0.0001));
  CHECK(hasEdges(
    p,
    {{p1, p2},
     {p1, p3},
     {p1, p5},
     {p2, p4},
     {p2, p6},
     {p3, p4},
     {p3, p7},
     {p4, p10},
     {p5, p6},
     {p5, p7},
     {p6, p9},
     {p7, p11},
     {p9, p10},
     {p9, p11},
     {p10, p11}},
    0.0001));
  CHECK(hasFaces(
    p,
    {{p1, p3, p7, p5},
     {p1, p5, p6, p2},
     {p1, p2, p4, p3},
     {p2, p6, p9, p10, p4},
     {p3, p4, p10, p11, p7},
     {p5, p7, p11, p9, p6},
     {p9, p11, p10}},
    0.0001));
}

TEST_CASE("PolyhedronTest.clipCubeDiagonally")
{
  auto p = Polyhedron3d{vm::bbox3d{64.0}};

  CHECK(p.clip({vm::vec3d{0, 0, 0}, vm::normalize(vm::vec3d{1, 1, 0})}).success());

  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{+64, -64, -64};
  const auto p6 = vm::vec3d{+64, -64, +64};

  CHECK(p.hasAllVertices({p1, p2, p3, p4, p5, p6}));
  CHECK(hasEdges(
    p,
    {{p1, p2},
     {p1, p3},
     {p1, p5},
     {p2, p4},
     {p2, p6},
     {p3, p4},
     {p3, p5},
     {p4, p6},
     {p5, p6}}));
  CHECK(hasFaces(
    p,
    {{p1, p2, p4, p3}, {p1, p5, p6, p2}, {p3, p4, p6, p5}, {p1, p3, p5}, {p2, p6, p4}}));
}

TEST_CASE("PolyhedronTest.clipCubeWithVerticalSlantedPlane")
{
  auto p = Polyhedron3d{vm::bbox3d{64.0}};

  CHECK(p.clip(vm::plane3d{vm::vec3d{0, -64, 0}, vm::normalize(vm::vec3d{2, 1, 0})})
          .success());

  const auto p1 = vm::vec3d{-64, -64, -64};
  const auto p2 = vm::vec3d{-64, -64, +64};
  const auto p3 = vm::vec3d{-64, +64, -64};
  const auto p4 = vm::vec3d{-64, +64, +64};
  const auto p5 = vm::vec3d{0, -64, -64};
  const auto p6 = vm::vec3d{0, -64, +64};

  CHECK(p.hasAllVertices({p1, p2, p3, p4, p5, p6}));
  CHECK(hasEdges(
    p,
    {{p1, p2},
     {p1, p3},
     {p1, p5},
     {p2, p4},
     {p2, p6},
     {p3, p4},
     {p3, p5},
     {p4, p6},
     {p5, p6}}));
  CHECK(hasFaces(
    p,
    {
      {p1, p2, p4, p3},
      {p1, p5, p6, p2},
      {p3, p4, p6, p5},
      {p1, p3, p5},
      {p2, p6, p4},
    }));
}

bool findAndRemove(
  std::vector<Polyhedron3d>& result, const std::vector<vm::vec3d>& vertices);
bool findAndRemove(
  std::vector<Polyhedron3d>& result, const std::vector<vm::vec3d>& vertices)
{
  for (auto it = std::begin(result), end = std::end(result); it != end; ++it)
  {
    const Polyhedron3d& polyhedron = *it;
    if (polyhedron.hasAllVertices(vertices, vm::Cd::almost_zero()))
    {
      result.erase(it);
      return true;
    }
  }

  return false;
}

TEST_CASE("PolyhedronTest.subtractInnerCuboidFromCuboid")
{
  const auto minuend = Polyhedron3d{vm::bbox3d{32.0}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{16.0}};

  const auto p1 = vm::vec3d{-32, -32, -32};
  const auto p2 = vm::vec3d{-32, -32, +32};
  const auto p3 = vm::vec3d{-32, +32, -32};
  const auto p4 = vm::vec3d{-32, +32, +32};
  const auto p5 = vm::vec3d{+32, -32, -32};
  const auto p6 = vm::vec3d{+32, -32, +32};
  const auto p7 = vm::vec3d{+32, +32, -32};
  const auto p8 = vm::vec3d{+32, +32, +32};

  const auto p9 = vm::vec3d{-16, -32, -32};
  const auto p10 = vm::vec3d{-16, -32, +32};
  const auto p11 = vm::vec3d{-16, +32, -32};
  const auto p12 = vm::vec3d{-16, +32, +32};
  const auto p13 = vm::vec3d{+16, -32, -32};
  const auto p14 = vm::vec3d{+16, -32, +32};
  const auto p15 = vm::vec3d{+16, +32, -32};
  const auto p16 = vm::vec3d{+16, +32, +32};

  const auto p17 = vm::vec3d{-16, -16, -32};
  const auto p18 = vm::vec3d{-16, -16, +32};
  const auto p19 = vm::vec3d{-16, +16, -32};
  const auto p20 = vm::vec3d{-16, +16, +32};
  const auto p21 = vm::vec3d{+16, -16, -32};
  const auto p22 = vm::vec3d{+16, -16, +32};
  const auto p23 = vm::vec3d{+16, +16, -32};
  const auto p24 = vm::vec3d{+16, +16, +32};

  const auto p25 = vm::vec3d{-16, -16, -16};
  const auto p26 = vm::vec3d{-16, -16, +16};
  const auto p27 = vm::vec3d{-16, +16, -16};
  const auto p28 = vm::vec3d{-16, +16, +16};
  const auto p29 = vm::vec3d{+16, -16, -16};
  const auto p30 = vm::vec3d{+16, -16, +16};
  const auto p31 = vm::vec3d{+16, +16, -16};
  const auto p32 = vm::vec3d{+16, +16, +16};

  CHECK_THAT(
    minuend.subtract(subtrahend),
    Catch::UnorderedEquals(std::vector<Polyhedron3d>{
      Polyhedron3d{p1, p3, p2, p4, p9, p12, p11, p10},
      Polyhedron3d{p6, p8, p13, p14, p16, p15, p7, p5},
      Polyhedron3d{p14, p13, p10, p9, p18, p22, p21, p17},
      Polyhedron3d{p15, p16, p19, p23, p24, p20, p12, p11},
      Polyhedron3d{p20, p24, p22, p18, p26, p28, p32, p30},
      Polyhedron3d{p17, p21, p19, p23, p25, p29, p31, p27},
    }));
}

TEST_CASE("PolyhedronTest.subtractDisjointCuboidFromCuboid")
{
  const auto minuend = Polyhedron3d{vm::bbox3d{64.0}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{{96, 96, 96}, {128, 128, 128}}};

  CHECK(minuend.subtract(subtrahend) == std::vector<Polyhedron3d>{minuend});
}

TEST_CASE("PolyhedronTest.subtractCuboidFromInnerCuboid")
{
  const auto minuend = Polyhedron3d{vm::bbox3d{32.0}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{64.0}};

  CHECK(minuend.subtract(subtrahend) == std::vector<Polyhedron3d>{});
}

TEST_CASE("PolyhedronTest.subtractCuboidFromIdenticalCuboid")
{
  const auto minuend = Polyhedron3d{vm::bbox3d{64.0}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{64.0}};

  CHECK(minuend.subtract(subtrahend) == std::vector<Polyhedron3d>{});
}

TEST_CASE("PolyhedronTest.subtractCuboidProtrudingThroughCuboid")
{
  const auto minuend = Polyhedron3d{vm::bbox3d{{-32, -32, -16}, {32, 32, 16}}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{{-16, -16, -32}, {16, 16, 32}}};

  const auto p1 = vm::vec3d{-32, -32, -16};
  const auto p2 = vm::vec3d{-32, -32, 16};
  const auto p3 = vm::vec3d{-32, 32, -16};
  const auto p4 = vm::vec3d{-32, 32, 16};
  const auto p5 = vm::vec3d{32, -32, -16};
  const auto p6 = vm::vec3d{32, -32, 16};
  const auto p7 = vm::vec3d{32, 32, -16};
  const auto p8 = vm::vec3d{32, 32, 16};

  const auto p9 = vm::vec3d{-16, -32, -16};
  const auto p10 = vm::vec3d{-16, -32, 16};
  const auto p11 = vm::vec3d{-16, 32, -16};
  const auto p12 = vm::vec3d{-16, 32, 16};
  const auto p13 = vm::vec3d{16, -32, -16};
  const auto p14 = vm::vec3d{16, -32, 16};
  const auto p15 = vm::vec3d{16, 32, -16};
  const auto p16 = vm::vec3d{16, 32, 16};

  const auto p17 = vm::vec3d{-16, -16, -16};
  const auto p18 = vm::vec3d{-16, -16, 16};
  const auto p19 = vm::vec3d{-16, 16, -16};
  const auto p20 = vm::vec3d{-16, 16, 16};
  const auto p21 = vm::vec3d{16, -16, -16};
  const auto p22 = vm::vec3d{16, -16, 16};
  const auto p23 = vm::vec3d{16, 16, -16};
  const auto p24 = vm::vec3d{16, 16, 16};

  CHECK_THAT(
    minuend.subtract(subtrahend),
    Catch::UnorderedEquals(std::vector<Polyhedron3d>{
      {p1, p2, p3, p4, p9, p10, p11, p12},
      {p5, p6, p7, p8, p13, p14, p15, p16},
      {p9, p10, p13, p14, p17, p18, p21, p22},
      {p11, p12, p15, p16, p19, p20, p23, p24},
    }));
}

TEST_CASE("PolyhedronTest.subtractCuboidProtrudingFromCuboid")
{
  /*
   ____________
   |          |
   |  ______  |
   |  |    |  |
   |__|    |__|
      |    |
      |____|
   */

  const auto minuend = Polyhedron3d{vm::bbox3d{{-32, -16, -32}, {32, 16, 32}}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{{-16, -32, -64}, {16, 32, 0}}};

  auto result = minuend.subtract(subtrahend);
  CHECK(result.size() == 3u);
}

TEST_CASE("PolyhedronTest.subtractCuboidProtrudingFromCuboid2")
{
  /*
   ____________
   |          |
   |  ______  |
   |  |    |  |
   |__|____|__|
   */

  const auto minuend = Polyhedron3d{vm::bbox3d{{-64, -64, -16}, {64, 64, 16}}};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{{-32, -64, -32}, {32, 0, 32}}};

  auto result = minuend.subtract(subtrahend);
  CHECK(result.size() == 3u);
}

TEST_CASE("PolyhedronTest.subtractCuboidFromCuboidWithCutCorners")
{

  /*
     ____
    /    \
   / ____ \
   | |  | |
   | |  | |
   | |  | |
   |_|__|_|

   */


  const auto p1 = vm::vec3d{-16, 8, 0};
  const auto p2 = vm::vec3d{-16, 8, 48};
  const auto p3 = vm::vec3d{-16, -8, 48};
  const auto p4 = vm::vec3d{-16, -8, 0};
  const auto p5 = vm::vec3d{-32, -8, 0};
  const auto p6 = vm::vec3d{-32, -8, 32};
  const auto p7 = vm::vec3d{-32, 8, 0};
  const auto p8 = vm::vec3d{-32, 8, 32};
  const auto p9 = vm::vec3d{32, -8, 32};
  const auto p10 = vm::vec3d{32, 8, 32};
  const auto p11 = vm::vec3d{32, 8, 0};
  const auto p12 = vm::vec3d{32, -8, 0};
  const auto p13 = vm::vec3d{16, 8, 48};
  const auto p14 = vm::vec3d{16, 8, 0};
  const auto p15 = vm::vec3d{16, -8, 0};
  const auto p16 = vm::vec3d{16, -8, 48};
  const auto p17 = vm::vec3d{16, 8, 32};
  const auto p18 = vm::vec3d{16, -8, 32};
  const auto p19 = vm::vec3d{-16, -8, 32};
  const auto p20 = vm::vec3d{-16, 8, 32};

  const auto minuend = Polyhedron3d{p2, p3, p5, p6, p7, p8, p9, p10, p11, p12, p13, p16};
  const auto subtrahend = Polyhedron3d{vm::bbox3d{{-16, -8, 0}, {16, 8, 32}}};

  auto result = minuend.subtract(subtrahend);

  CHECK_THAT(
    minuend.subtract(subtrahend),
    Catch::UnorderedEquals(std::vector<Polyhedron3d>{
      {p1, p2, p3, p4, p5, p6, p7, p8},
      {p9, p10, p11, p12, p13, p14, p15, p16},
      {p17, p18, p19, p20, p3, p2, p13, p16},
    }));
}

TEST_CASE("PolyhedronTest.subtractRhombusFromCuboid")
{

  /*
   ______
   |    |
   | /\ |
   | \/ |
   |____|

   */

  const auto minuend = Polyhedron3d{vm::bbox3d{64.0}};
  const auto subtrahend = Polyhedron3d{
    {-32, 0, -96},
    {-32, 0, 96},
    {0, -32, -96},
    {0, -32, 96},
    {0, 32, -96},
    {0, 32, 96},
    {32, 0, -96},
    {32, 0, 96}};

  auto result = minuend.subtract(subtrahend);

  std::vector<vm::vec3d> f1, f2, f3, f4;
  vm::parse_all<double, 3>(
    R"((64 64 64) (-32 64 -64) (64 -32 -64) (64 -32 64) (-32 64 64) (64 64 -64))",
    std::back_inserter(f1));
  vm::parse_all<double, 3>(
    R"((-64 32 64) (-64 32 -64) (-32 -0 64) (-32 -0 -64) (-0 32 -64) (-0 32 64) (-64 64 64) (-32 64 -64) (-32 64 64) (-64 64 -64))",
    std::back_inserter(f2));
  vm::parse_all<double, 3>(
    R"((64 -32 64) (64 -32 -64) (64 -64 64) (64 -64 -64) (-0 -32 64) (32 -0 64) (32 -0 -64) (-0 -32 -64) (32 -64 -64) (32 -64 64))",
    std::back_inserter(f3));
  vm::parse_all<double, 3>(
    R"((-64 -64 64) (-64 -64 -64) (-64 32 -64) (-64 32 64) (32 -64 64) (32 -64 -64))",
    std::back_inserter(f4));
  CHECK(findAndRemove(result, f1));
  CHECK(findAndRemove(result, f2));
  CHECK(findAndRemove(result, f3));
  CHECK(findAndRemove(result, f4));

  CHECK(result.size() == 0u);
}

TEST_CASE("PolyhedronTest.intersection_empty_polyhedron")
{
  const auto empty = Polyhedron3d{};
  const auto point = Polyhedron3d{{1, 0, 0}};
  const auto edge = Polyhedron3d{{1, 0, 0}, {2, 0, 0}};
  const auto polygon = Polyhedron3d{{1, 0, 0}, {2, 0, 0}, {0, 1, 0}};
  const auto polyhedron = Polyhedron3d{{1, 0, 0}, {2, 0, 0}, {0, 1, 0}, {0, 0, 1}};

  CHECK(mutuallyNotIntersects(empty, empty));
  CHECK(mutuallyNotIntersects(empty, point));
  CHECK(mutuallyNotIntersects(empty, edge));
  CHECK(mutuallyNotIntersects(empty, polygon));
  CHECK(mutuallyNotIntersects(empty, polyhedron));
}

TEST_CASE("PolyhedronTest.intersection_point_point")
{
  const auto point = Polyhedron3d{{0, 0, 0}};

  CHECK(mutuallyIntersects(point, point));
  CHECK(mutuallyNotIntersects(point, Polyhedron3d{{0, 0, 1}}));
}

TEST_CASE("PolyhedronTest.intersection_point_edge")
{
  const auto pointPos = vm::vec3d{0, 0, 0};
  const auto point = Polyhedron3d{pointPos};

  CHECK(mutuallyIntersects(
    point, Polyhedron3d{pointPos, {1, 0, 0}})); // point / edge originating at point
  CHECK(mutuallyIntersects(
    point, Polyhedron3d{{-1, 0, 0}, {1, 0, 0}})); // point / edge containing point
  CHECK(mutuallyNotIntersects(
    point, Polyhedron3d{{-1, 0, 1}, {1, 0, 1}})); // point / unrelated edge
}

TEST_CASE("PolyhedronTest.intersection_point_polygon")
{
  const auto pointPos = vm::vec3d{0, 0, 0};
  const auto point = Polyhedron3d{pointPos};

  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      pointPos, {1, 0, 0}, {0, 1, 0}})); // point / triangle with point as vertex
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}})); // point / triangle with point on edge
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      {-1, -1, 0}, {1, -1, 0}, {0, 1, 0}})); // point / triangle containing point

  CHECK(mutuallyNotIntersects(
    point,
    Polyhedron3d{{-1, -1, 1}, {1, -1, 1}, {0, 1, 1}})); // point / triangle above point
}

TEST_CASE("PolyhedronTest.intersection_point_polyhedron")
{
  const auto pointPos = vm::vec3d{0, 0, 0};
  const auto point = Polyhedron3d{pointPos};

  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{pointPos, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}})); // point / tetrahedron with
                                                               // point as vertex
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{{-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}})); // point / tetrahedron
                                                                 // with point on edge
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{{-1, -1, 0}, {1, -1, 0}, {0, 1, 0}, {0, 0, 1}})); // point / tetrahedron
                                                                   // with point on face
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      {-1, -1, -1},
      {1, -1, -1},
      {0, 1, -1},
      {0, 0, 1}})); // point / tetrahedron with point on face

  CHECK(mutuallyNotIntersects(
    point,
    Polyhedron3d{
      {-1, -1, 1}, {1, -1, 1}, {0, 1, 1}, {0, 0, 2}})); // point / tetrahedron above point
}

TEST_CASE("PolyhedronTest.intersection_edge_edge")
{
  const auto point1 = vm::vec3d{-1, 0, 0};
  const auto point2 = vm::vec3d{+1, 0, 0};
  const auto edge = Polyhedron3d{point1, point2};

  CHECK(mutuallyIntersects(edge, edge));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{point1, {0, 0, 1}}));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{point2, {0, 0, 1}}));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{{0, -1, 0}, {0, 1, 0}}));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{{0, 0, 0}, {2, 0, 0}}));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{{-2, 0, 0}, {2, 0, 0}}));

  CHECK(mutuallyNotIntersects(
    edge, Polyhedron3d{point1 + vm::vec3d{0, 0, 1}, point2 + vm::vec3d{0, 0, 1}}));
}

TEST_CASE("PolyhedronTest.intersection_edge_polygon_same_plane")
{
  const auto point1 = vm::vec3d{-1, 0, 0};
  const auto point2 = vm::vec3d{+1, 0, 0};
  const auto edge = Polyhedron3d{point1, point2};

  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{{1, 0, 0}, {1, -1, 0}, {2, -1, 0}, {2, 0, 0}})); // one shared point
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{{-1, 0, 0}, {0, -1, 0}, {2, 0, 0}, {0, +1, 0}})); // two shared points
  CHECK(mutuallyIntersects(
    edge, Polyhedron3d{{-1, 0, 0}, {1, 0, 0}, {1, 1, 0}, {-1, 1, 0}})); // shared edge
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      {0, 1, 0}, {0, -1, 0}, {2, -1, 0}, {2, 1, 0}})); // polygon contains one point
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      {-2, 1, 0}, {-2, -1, 0}, {2, -1, 0}, {2, 1, 0}})); // polygon contains both points
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      {-0.5, 1.0, 0.0},
      {-0.5, -1.0, 0.0},
      {0.5, -1.0, 0.0},
      {0.5, 1.0, 0.0}})); // edge intersects polygon completely

  CHECK(mutuallyNotIntersects(
    edge,
    Polyhedron3d{{+2, 1, 0}, {+2, -1, 0}, {+3, -1, 0}, {+3, 1, 0}})); // no intersection
}

TEST_CASE("PolyhedronTest.intersection_edge_polygon_different_plane")
{
  const auto point1 = vm::vec3d{0, 0, 1};
  const auto point2 = vm::vec3d{0, 0, -1};
  const auto edge = Polyhedron3d{point1, point2};

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, +1}},
    Polyhedron3d{
      vm::vec3d{0, 0, 0},
      vm::vec3d{2, 0, 0},
      vm::vec3d{2, 2, 0},
      vm::vec3d{0, 2, 0}})); // one shared point

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d{1, 0, 0}, vm::vec3d{1, 0, +1}},
    Polyhedron3d{
      vm::vec3d{0, 0, 0},
      vm::vec3d{2, 0, 0},
      vm::vec3d{2, 2, 0},
      vm::vec3d{0, 2, 0}})); // polygon edge contains edge origin

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d{1, 1, 0}, vm::vec3d{1, 1, +1}},
    Polyhedron3d{
      vm::vec3d{0, 0, 0},
      vm::vec3d{2, 0, 0},
      vm::vec3d{2, 2, 0},
      vm::vec3d{0, 2, 0}})); // polygon contains edge origin

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d{0, 0, -1}, vm::vec3d{0, 0, +1}},
    Polyhedron3d{
      vm::vec3d{0, 0, 0},
      vm::vec3d{2, 0, 0},
      vm::vec3d{2, 2, 0},
      vm::vec3d{0, 2, 0}})); // edge intersects polygon vertex

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d{1, 0, -1}, vm::vec3d{1, 0, +1}},
    Polyhedron3d{
      vm::vec3d{0, 0, 0},
      vm::vec3d{2, 0, 0},
      vm::vec3d{2, 2, 0},
      vm::vec3d{0, 2, 0}})); // edge intersects polygon edge

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d{1, 1, -1}, vm::vec3d{1, 1, +1}},
    Polyhedron3d{
      vm::vec3d{0, 0, 0},
      vm::vec3d{2, 0, 0},
      vm::vec3d{2, 2, 0},
      vm::vec3d{0, 2, 0}})); // edge intersects polygon center

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{{3, 1, -1}, {3, 1, +1}},
    Polyhedron3d{{0, 0, 0}, {2, 0, 0}, {2, 2, 0}, {0, 2, 0}}));

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{{1, 1, 1}, {1, 1, 2}},
    Polyhedron3d{{0, 0, 0}, {2, 0, 0}, {2, 2, 0}, {0, 2, 0}}));

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{{0, 0, 1}, {1, 1, 1}},
    Polyhedron3d{{0, 0, 0}, {2, 0, 0}, {2, 2, 0}, {0, 2, 0}}));
}

TEST_CASE("PolyhedronTest.intersection_edge_polyhedron")
{
  const Polyhedron3d tetrahedron{{-1, -1, 0}, {+1, -1, 0}, {0, +1, 0}, {0, 0, 1}};

  CHECK(mutuallyIntersects(
    Polyhedron3d{{0, 0, 1}, {0, 0, 2}},
    tetrahedron)); // one shared point
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0.0, -0.9999, 0.0}, {0, -2, 0}},
    tetrahedron)); // edge point on polyhedron edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0, 0, 0}, {0, 0, -1}},
    tetrahedron)); // edge point on polyhedron face
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-1, -1, 0}, {+1, -1, 0}},
    tetrahedron)); // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0.0, 0.0, 0.5}, {0, 0, 2}},
    tetrahedron)); // polyhedron contains one edge point
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0.0, 0.0, 0.2}, {0.0, 0.0, 0.7}},
    tetrahedron)); // polyhedron contains both edge points
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0, 0, -1}, {0, 0, 2}},
    tetrahedron)); // edge penetrates polyhedron

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{{-2, -2, -1}, {2, 2, -1}},
    tetrahedron)); // no intersection
}

TEST_CASE("PolyhedronTest.intersection_polygon_polygon_same_plane")
{
  const Polyhedron3d square{{-1, -1, 0}, {+1, -1, 0}, {+1, +1, 0}, {-1, +1, 0}};

  // shared vertex:
  CHECK(mutuallyIntersects(Polyhedron3d{{+1, +1, 0}, {+2, +1, 0}, {+1, +2, 0}}, square));

  // shared edge
  CHECK(mutuallyIntersects(Polyhedron3d{{-1, +1, 0}, {+1, +1, 0}, {0, +2, 0}}, square));

  // edge contains other edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {-2, -1, 0},
      {+2, -1, 0},
      {+2, +1, 0},
      {-2, +1, 0},
    },
    square));

  // one contains vertex of another
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0, 0, 0}, {+2, 0, 0}, {+2, +2, 0}, {0, +2, 0}}, square));

  // one contains another entirely
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-2, -2, 0}, {+2, -2, 0}, {+2, +2, 0}, {-2, +2, 0}}, square));

  // one penetrates the other
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-2, -0.5, 0}, {+2, -0.5, 0}, {+2, +0.5, 0}, {-2, +0.5, 0}}, square));

  // no intersection
  CHECK(mutuallyNotIntersects(
    Polyhedron3d{{+2, +2, 0}, {+3, +2, 0}, {+3, +3, 0}, {+3, +3, 0}}, square));
}

TEST_CASE("PolyhedronTest.intersection_polygon_polygon_different_plane")
{
  const Polyhedron3d square{{-1, -1, 0}, {+1, -1, 0}, {+1, +1, 0}, {-1, +1, 0}};

  // shared vertex
  CHECK(mutuallyIntersects(Polyhedron3d{{-1, -1, 0}, {-2, -1, 0}, {-2, -1, 1}}, square));

  // vertex on edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {0, -1, 0},
      {0, -2, 0},
      {0, -1, 1},
      {0, -2, 1},
    },
    square));

  // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-1, -1, 0}, {+1, -1, 0}, {+1, -1, 1}, {-1, -1, 1}}, square));

  // edges intersect
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0, -1, -1}, {0, -1, +1}, {0, -2, +1}, {0, -2, -1}}, square));

  // partial penetration (one edge penetrates each)
  CHECK(mutuallyIntersects(
    Polyhedron3d{{0, 0, -1}, {0, 0, +1}, {2, 0, +1}, {2, 0, -1}}, square));

  // full penetration (two edges penetrate)
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-2, 0, -2}, {-2, 0, +2}, {+2, 0, -2}, {+2, 0, +2}}, square));

  // no intersection
  CHECK(mutuallyNotIntersects(
    Polyhedron3d{{-1, 0, 5}, {+1, 0, 5}, {-1, 0, 6}, {+1, 0, 6}}, square));
}

TEST_CASE("PolyhedronTest.intersection_polygon_polyhedron_same_plane_as_face")
{
  const Polyhedron3d cube{
    {-1, -1, -1},
    {-1, -1, +1},
    {-1, +1, -1},
    {-1, +1, +1},
    {+1, -1, -1},
    {+1, -1, +1},
    {+1, +1, -1},
    {+1, +1, +1},
  };

  // polygon is on the same plane as top face

  // shared vertex
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {+1, +1, +1},
      {+2, +1, +1},
      {+2, +2, +1},
    },
    cube));

  // shared edge
  CHECK(mutuallyIntersects(Polyhedron3d{{+1, +1, +1}, {-1, +1, +1}, {+1, +2, +1}}, cube));

  // edge contains other edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-0.5, +1.0, +1.0}, {+0.5, +1.0, +1.0}, {+0.5, +2.0, +1.0}}, cube));

  // one contains vertex of another
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {+0, +0, +1},
      {+2, +0, +1},
      {+2, +2, +1},
      {+0, +2, +1},
    },
    cube));

  // one contains another entirely
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {-0.5, -0.5, +1.0},
      {-0.5, +0.5, +1.0},
      {+0.5, +0.5, +1.0},
      {+0.5, -0.5, +1.0},
    },
    cube));
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {-2.5, -2.5, +1.0},
      {-2.5, +2.5, +1.0},
      {+2.5, +2.5, +1.0},
      {+2.5, -2.5, +1.0},
    },
    cube));

  // one penetrates the other
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {-2.0, -0.5, +1.0},
      {+2.0, -0.5, +1.0},
      {-2.0, +0.5, +1.0},
      {+2.0, +0.5, +1.0},
    },
    cube));

  // no intersection
  CHECK(mutuallyNotIntersects(
    Polyhedron3d{
      {+2, +2, +1},
      {+3, +2, +1},
      {+3, +3, +1},
      {+2, +3, +1},
    },
    cube));
}

TEST_CASE("PolyhedronTest.intersection_polygon_polyhedron_any_orientation")
{
  const Polyhedron3d cube{
    {-1, -1, -1},
    {-1, -1, +1},
    {-1, +1, -1},
    {-1, +1, +1},
    {+1, -1, -1},
    {+1, -1, +1},
    {+1, +1, -1},
    {+1, +1, +1},
  };

  // shared vertex
  CHECK(mutuallyIntersects(Polyhedron3d{{+1, +1, +1}, {+2, +1, +2}, {+2, +2, +2}}, cube));

  // polygon vertex on polyhedron edge
  CHECK(mutuallyIntersects(Polyhedron3d{{+0, +1, +1}, {+2, +1, +2}, {+2, +2, +2}}, cube));

  // polyhedron vertex on polygon edge
  CHECK(mutuallyIntersects(Polyhedron3d{{0, 2, 1}, {2, 0, 1}, {0, 0, 2}}, cube));

  // shared edge
  CHECK(mutuallyIntersects(Polyhedron3d{{-1, 1, 1}, {+1, 1, 1}, {0, 2, 2}}, cube));

  // polygon edge inside polyhedron edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {-0.5, 1.0, 1.0},
      {+0.5, 1.0, 1.0},
      {0, 2, 2},
    },
    cube));

  // polyhedorn edge inside polygon edge
  CHECK(mutuallyIntersects(Polyhedron3d{{-2, 1, 1}, {+2, 1, 1}, {0, 2, 2}}, cube));

  // edges intersect
  CHECK(mutuallyIntersects(Polyhedron3d{{0, -2, 0}, {0, 0, 2}, {0, -2, 2}}, cube));

  // penetration (two polygon edges intersect)
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {0, 0, 0},
      {0, -3, 0},
      {3, 0, 2},
    },
    cube));

  // polyhedron contains polygon
  CHECK(mutuallyIntersects(
    Polyhedron3d{{-0.5, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}}, cube));

  // polygon slices polyhedron (surrounds it)
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      {-2, -2, 0},
      {-2, +2, 0},
      {+2, -2, 0},
      {+2, +2, 0},
    },
    cube));
}

} // namespace tb::mdl
