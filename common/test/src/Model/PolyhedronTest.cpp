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

#include "Model/Polyhedron.h"
#include "FloatType.h"
#include "Model/Polyhedron_BrushGeometryPayload.h"
#include "Model/Polyhedron_DefaultPayload.h"
#include "Model/Polyhedron_Instantiation.h"

#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <iterator>
#include <set>
#include <tuple>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
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

static bool hasVertices(
  const Polyhedron3d& p, const std::vector<vm::vec3d>& points, const double epsilon = 0.0)
{
  if (p.vertexCount() != points.size())
    return false;

  for (size_t i = 0; i < points.size(); ++i)
  {
    if (!p.hasVertex(points[i], epsilon))
      return false;
  }
  return true;
}

static bool hasEdges(
  const Polyhedron3d& p, const EdgeInfoList& edgeInfos, const double epsilon = 0.0)
{
  if (p.edgeCount() != edgeInfos.size())
    return false;

  for (size_t i = 0; i < edgeInfos.size(); ++i)
  {
    if (!p.hasEdge(edgeInfos[i].first, edgeInfos[i].second, epsilon))
      return false;
  }
  return true;
}

static bool mutuallyIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs)
{
  return lhs.intersects(rhs) && rhs.intersects(lhs);
}

static bool mutuallyNotIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs)
{
  return !lhs.intersects(rhs) && !rhs.intersects(lhs);
}

TEST_CASE("PolyhedronTest.constructEmpty")
{
  Polyhedron3d p;
  CHECK(p.empty());
}

TEST_CASE("PolyhedronTest.constructWithOnePoint")
{
  const vm::vec3d p1(-8.0, -8.0, -8.0);

  Polyhedron3d p({p1});

  CHECK_FALSE(p.empty());
  CHECK(p.point());
  CHECK_FALSE(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructWithTwoIdenticalPoints")
{
  const vm::vec3d p1(-8.0, -8.0, -8.0);

  Polyhedron3d p({p1, p1});

  CHECK_FALSE(p.empty());
  CHECK(p.point());
  CHECK_FALSE(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructWithTwoPoints")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(3.0, 0.0, 0.0);

  Polyhedron3d p({p1, p2});

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructWithThreeColinearPoints")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(3.0, 0.0, 0.0);
  const vm::vec3d p3(6.0, 0.0, 0.0);

  Polyhedron3d p({p1, p2, p3});

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK(p.edge());
  CHECK_FALSE(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p3);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructWithThreePoints")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(3.0, 0.0, 0.0);
  const vm::vec3d p3(6.0, 5.0, 0.0);

  Polyhedron3d p({p1, p2, p3});

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK_FALSE(p.edge());
  CHECK(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructTriangleWithContainedPoint")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(6.0, 0.0, 0.0);
  const vm::vec3d p3(3.0, 6.0, 0.0);
  const vm::vec3d p4(3.0, 3.0, 0.0);

  Polyhedron3d p({p1, p2, p3, p4});

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK_FALSE(p.edge());
  CHECK(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructWithFourCoplanarPoints")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(6.0, 0.0, 0.0);
  const vm::vec3d p3(3.0, 3.0, 0.0);
  const vm::vec3d p4(3.0, 6.0, 0.0);

  Polyhedron3d p({p1, p2, p3, p4});

  CHECK_FALSE(p.empty());
  CHECK_FALSE(p.point());
  CHECK_FALSE(p.edge());
  CHECK(p.polygon());
  CHECK_FALSE(p.polyhedron());

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p4);

  CHECK(hasVertices(p, points));
}

TEST_CASE("PolyhedronTest.constructWith4Points")
{
  const vm::vec3d p1(0.0, 0.0, 8.0);
  const vm::vec3d p2(8.0, 0.0, 0.0);
  const vm::vec3d p3(-8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);

  const Polyhedron3d p({p1, p2, p3, p4});
  CHECK(p.closed());

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);
  CHECK(hasVertices(p, points));

  EdgeInfoList edgeInfos;
  edgeInfos.push_back(std::make_pair(p2, p3));
  edgeInfos.push_back(std::make_pair(p3, p4));
  edgeInfos.push_back(std::make_pair(p4, p2));
  edgeInfos.push_back(std::make_pair(p1, p3));
  edgeInfos.push_back(std::make_pair(p1, p2));
  edgeInfos.push_back(std::make_pair(p4, p1));

  CHECK(hasEdges(p, edgeInfos));

  CHECK(p.hasFace({p2, p3, p4}));
  CHECK(p.hasFace({p1, p3, p2}));
  CHECK(p.hasFace({p1, p2, p4}));
  CHECK(p.hasFace({p1, p4, p3}));
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

  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(+32.0, 0.0, 0.0);
  const vm::vec3d p3(+32.0, +32.0, 0.0);
  const vm::vec3d p4(0.0, +32.0, 0.0);
  const vm::vec3d p5(+16.0, +32.0, 0.0);

  Polyhedron3d p({p1, p2, p3, p4, p5});

  CHECK(p.hasVertex(p1));
  CHECK(p.hasVertex(p2));
  CHECK(p.hasVertex(p3));
  CHECK(p.hasVertex(p4));
  CHECK_FALSE(p.hasVertex(p5));
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

  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(+32.0, 0.0, 0.0);
  const vm::vec3d p3(+32.0, +32.0, 0.0);
  const vm::vec3d p4(0.0, +32.0, 0.0);
  const vm::vec3d p5(+40.0, +32.0, 0.0);

  Polyhedron3d p({p1, p2, p3, p4, p5});

  CHECK(p.hasVertex(p1));
  CHECK(p.hasVertex(p2));
  CHECK(p.hasVertex(p4));
  CHECK(p.hasVertex(p5));
  CHECK_FALSE(p.hasVertex(p3));
}

TEST_CASE("PolyhedronTest.constructPolygonWithRedundantPoint")
{
  auto p = Polyhedron3d{
    vm::vec3{-64.0, 64.0, -16.0},
    vm::vec3{64.0, 64.0, -16.0},
    vm::vec3{22288.0, 18208.0, 16.0},
    vm::vec3{
      22288.0,
      18336.0,
      16.0}, // does not get added due to all incident faces being coplanar
    vm::vec3{22416.0, 18336.0, 16.0},
  };

  CHECK(p.hasAllVertices(
    {
      vm::vec3{-64.0, 64.0, -16.0},
      vm::vec3{64.0, 64.0, -16.0},
      vm::vec3{22288.0, 18208.0, 16.0},
      vm::vec3{22416.0, 18336.0, 16.0},
    },
    0.0));
}

TEST_CASE("PolyhedronTest.constructTetrahedonWithRedundantPoint")
{
  const vm::vec3d p1(0.0, 4.0, 8.0);
  const vm::vec3d p2(8.0, 0.0, 0.0);
  const vm::vec3d p3(-8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);
  const vm::vec3d p5(0.0, 4.0, 12.0);

  Polyhedron3d p({p1, p2, p3, p4, p5});
  CHECK(p.closed());

  std::vector<vm::vec3d> points;
  points.push_back(p5);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);
  CHECK(hasVertices(p, points));

  EdgeInfoList edgeInfos;
  edgeInfos.push_back(std::make_pair(p2, p3));
  edgeInfos.push_back(std::make_pair(p3, p4));
  edgeInfos.push_back(std::make_pair(p4, p2));
  edgeInfos.push_back(std::make_pair(p5, p3));
  edgeInfos.push_back(std::make_pair(p5, p2));
  edgeInfos.push_back(std::make_pair(p4, p5));

  CHECK(hasEdges(p, edgeInfos));

  CHECK(p.hasFace({p2, p3, p4}));
  CHECK(p.hasFace({p5, p3, p2}));
  CHECK(p.hasFace({p5, p2, p4}));
  CHECK(p.hasFace({p5, p4, p3}));
}

TEST_CASE("PolyhedronTest.constructTetrahedonWithCoplanarFaces")
{
  const vm::vec3d p1(0.0, 0.0, 8.0);
  const vm::vec3d p2(8.0, 0.0, 0.0);
  const vm::vec3d p3(-8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);
  const vm::vec3d p5(0.0, 0.0, 12.0);

  Polyhedron3d p({p1, p2, p3, p4, p5});
  CHECK(p.closed());

  std::vector<vm::vec3d> points;
  points.push_back(p5);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);
  CHECK(hasVertices(p, points));

  EdgeInfoList edgeInfos;
  edgeInfos.push_back(std::make_pair(p2, p3));
  edgeInfos.push_back(std::make_pair(p3, p4));
  edgeInfos.push_back(std::make_pair(p4, p2));
  edgeInfos.push_back(std::make_pair(p5, p3));
  edgeInfos.push_back(std::make_pair(p5, p2));
  edgeInfos.push_back(std::make_pair(p4, p5));

  CHECK(p.hasFace({p2, p3, p4}));
  CHECK(p.hasFace({p5, p3, p2}));
  CHECK(p.hasFace({p5, p2, p4}));
  CHECK(p.hasFace({p5, p4, p3}));
}

TEST_CASE("PolyhedronTest.constructCube")
{
  const vm::vec3d p1(-8.0, -8.0, -8.0);
  const vm::vec3d p2(-8.0, -8.0, +8.0);
  const vm::vec3d p3(-8.0, +8.0, -8.0);
  const vm::vec3d p4(-8.0, +8.0, +8.0);
  const vm::vec3d p5(+8.0, -8.0, -8.0);
  const vm::vec3d p6(+8.0, -8.0, +8.0);
  const vm::vec3d p7(+8.0, +8.0, -8.0);
  const vm::vec3d p8(+8.0, +8.0, +8.0);

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);
  points.push_back(p5);
  points.push_back(p6);
  points.push_back(p7);
  points.push_back(p8);

  Polyhedron3d p(points);

  CHECK(p.closed());

  CHECK(hasVertices(p, points));

  EdgeInfoList edgeInfos;
  edgeInfos.push_back(std::make_pair(p1, p2));
  edgeInfos.push_back(std::make_pair(p1, p3));
  edgeInfos.push_back(std::make_pair(p1, p5));
  edgeInfos.push_back(std::make_pair(p2, p4));
  edgeInfos.push_back(std::make_pair(p2, p6));
  edgeInfos.push_back(std::make_pair(p3, p4));
  edgeInfos.push_back(std::make_pair(p3, p7));
  edgeInfos.push_back(std::make_pair(p4, p8));
  edgeInfos.push_back(std::make_pair(p5, p6));
  edgeInfos.push_back(std::make_pair(p5, p7));
  edgeInfos.push_back(std::make_pair(p6, p8));
  edgeInfos.push_back(std::make_pair(p7, p8));

  CHECK(hasEdges(p, edgeInfos));

  CHECK(p.hasFace({p1, p5, p6, p2}));
  CHECK(p.hasFace({p3, p1, p2, p4}));
  CHECK(p.hasFace({p7, p3, p4, p8}));
  CHECK(p.hasFace({p5, p7, p8, p6}));
  CHECK(p.hasFace({p3, p7, p5, p1}));
  CHECK(p.hasFace({p2, p6, p8, p4}));
}

TEST_CASE("PolyhedronTest.copy")
{
  const vm::vec3d p1(0.0, 0.0, 8.0);
  const vm::vec3d p2(8.0, 0.0, 0.0);
  const vm::vec3d p3(-8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);

  CHECK(Polyhedron3d() == (Polyhedron3d() = Polyhedron3d()));
  CHECK(Polyhedron3d({p1}) == (Polyhedron3d() = Polyhedron3d({p1})));
  CHECK(Polyhedron3d({p1, p2}) == (Polyhedron3d() = Polyhedron3d({p1, p2})));
  CHECK(Polyhedron3d({p1, p2, p3}) == (Polyhedron3d() = Polyhedron3d({p1, p2, p3})));
  CHECK(
    Polyhedron3d({p1, p2, p3, p4}) == (Polyhedron3d() = Polyhedron3d({p1, p2, p3, p4})));
}

TEST_CASE("PolyhedronTest.swap")
{
  const vm::vec3d p1(0.0, 0.0, 8.0);
  const vm::vec3d p2(8.0, 0.0, 0.0);
  const vm::vec3d p3(-8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);

  Polyhedron3d original({p1, p2, p3, p4});
  Polyhedron3d other({p2, p3, p4});

  Polyhedron3d lhs = original;
  Polyhedron3d rhs = other;

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
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> positions;
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p3);
  positions.push_back(p4);
  positions.push_back(p5);
  positions.push_back(p6);
  positions.push_back(p7);
  positions.push_back(p8);

  Polyhedron3d p(positions);

  const vm::plane3d plane(vm::vec3d::zero(), vm::vec3d::pos_z());
  CHECK(p.clip(plane).success());

  const vm::vec3d d(0.0, 0.0, -64.0);
  CHECK(p.edgeCount() == 12u);
  CHECK(p.hasEdge(p1, p2 + d));
  CHECK(p.hasEdge(p1, p3));
  CHECK(p.hasEdge(p1, p5));
  CHECK(p.hasEdge(p2 + d, p4 + d));
  CHECK(p.hasEdge(p2 + d, p6 + d));
  CHECK(p.hasEdge(p3, p4 + d));
  CHECK(p.hasEdge(p3, p7));
  CHECK(p.hasEdge(p4 + d, p8 + d));
  CHECK(p.hasEdge(p5, p6 + d));
  CHECK(p.hasEdge(p5, p7));
  CHECK(p.hasEdge(p6 + d, p8 + d));

  CHECK(p.faceCount() == 6u);
  CHECK(p.hasFace({p1, p2 + d, p4 + d, p3}));
  CHECK(p.hasFace({p1, p3, p7, p5}));
  CHECK(p.hasFace({p1, p5, p6 + d, p2 + d}));
  CHECK(p.hasFace({p2 + d, p6 + d, p8 + d, p4 + d}));
  CHECK(p.hasFace({p3, p4 + d, p8 + d, p7}));
  CHECK(p.hasFace({p5, p7, p8 + d, p6 + d}));
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAtTop")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> positions;
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p3);
  positions.push_back(p4);
  positions.push_back(p5);
  positions.push_back(p6);
  positions.push_back(p7);
  positions.push_back(p8);

  Polyhedron3d p(positions);

  const vm::plane3d plane(vm::vec3d(0.0, 0.0, 64.0), vm::vec3d::pos_z());
  CHECK(p.clip(plane).unchanged());

  CHECK(p.edgeCount() == 12u);
  CHECK(p.hasEdge(p1, p2));
  CHECK(p.hasEdge(p1, p3));
  CHECK(p.hasEdge(p1, p5));
  CHECK(p.hasEdge(p2, p4));
  CHECK(p.hasEdge(p2, p6));
  CHECK(p.hasEdge(p3, p4));
  CHECK(p.hasEdge(p3, p7));
  CHECK(p.hasEdge(p4, p8));
  CHECK(p.hasEdge(p5, p6));
  CHECK(p.hasEdge(p5, p7));
  CHECK(p.hasEdge(p6, p8));

  CHECK(p.faceCount() == 6u);
  CHECK(p.hasFace({p1, p2, p4, p3}));
  CHECK(p.hasFace({p1, p3, p7, p5}));
  CHECK(p.hasFace({p1, p5, p6, p2}));
  CHECK(p.hasFace({p2, p6, p8, p4}));
  CHECK(p.hasFace({p3, p4, p8, p7}));
  CHECK(p.hasFace({p5, p7, p8, p6}));
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAboveTop")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> positions;
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p3);
  positions.push_back(p4);
  positions.push_back(p5);
  positions.push_back(p6);
  positions.push_back(p7);
  positions.push_back(p8);

  Polyhedron3d p(positions);

  const vm::plane3d plane(vm::vec3d(0.0, 0.0, 72.0), vm::vec3d::pos_z());
  CHECK(p.clip(plane).unchanged());

  CHECK(p.edgeCount() == 12u);
  CHECK(p.hasEdge(p1, p2));
  CHECK(p.hasEdge(p1, p3));
  CHECK(p.hasEdge(p1, p5));
  CHECK(p.hasEdge(p2, p4));
  CHECK(p.hasEdge(p2, p6));
  CHECK(p.hasEdge(p3, p4));
  CHECK(p.hasEdge(p3, p7));
  CHECK(p.hasEdge(p4, p8));
  CHECK(p.hasEdge(p5, p6));
  CHECK(p.hasEdge(p5, p7));
  CHECK(p.hasEdge(p6, p8));

  CHECK(p.faceCount() == 6u);
  CHECK(p.hasFace({p1, p2, p4, p3}));
  CHECK(p.hasFace({p1, p3, p7, p5}));
  CHECK(p.hasFace({p1, p5, p6, p2}));
  CHECK(p.hasFace({p2, p6, p8, p4}));
  CHECK(p.hasFace({p3, p4, p8, p7}));
  CHECK(p.hasFace({p5, p7, p8, p6}));
}

TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAtBottom")
{
  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p8(+64.0, +64.0, +64.0);

  std::vector<vm::vec3d> positions;
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p3);
  positions.push_back(p4);
  positions.push_back(p5);
  positions.push_back(p6);
  positions.push_back(p7);
  positions.push_back(p8);

  Polyhedron3d p(positions);

  const vm::plane3d plane(vm::vec3d(0.0, 0.0, -64.0), vm::vec3d::pos_z());
  CHECK(p.clip(plane).empty());
}

TEST_CASE("PolyhedronTest.clipCubeWithSlantedPlane")
{
  Polyhedron3d p(vm::bbox3d(64.0));

  const vm::plane3d plane(
    vm::vec3d(64.0, 64.0, 0.0), normalize(vm::vec3d(1.0, 1.0, 1.0)));
  CHECK(p.clip(plane).success());

  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);
  const vm::vec3d p7(+64.0, +64.0, -64.0);
  const vm::vec3d p9(+64.0, 0.0, +64.0);
  const vm::vec3d p10(0.0, +64.0, +64.0);
  const vm::vec3d p11(+64.0, +64.0, 0.0);

  CHECK(p.vertexCount() == 10u);
  CHECK(p.hasVertex(p1));
  CHECK(p.hasVertex(p2));
  CHECK(p.hasVertex(p3));
  CHECK(p.hasVertex(p4));
  CHECK(p.hasVertex(p5));
  CHECK(p.hasVertex(p6));
  CHECK(p.hasVertex(p7));
  CHECK(p.hasVertex(p9));
  CHECK(p.hasVertex(p10));
  CHECK(p.hasVertex(p11, 0.0001));

  CHECK(p.edgeCount() == 15u);
  CHECK(p.hasEdge(p1, p2));
  CHECK(p.hasEdge(p1, p3));
  CHECK(p.hasEdge(p1, p5));
  CHECK(p.hasEdge(p2, p4));
  CHECK(p.hasEdge(p2, p6));
  CHECK(p.hasEdge(p3, p4));
  CHECK(p.hasEdge(p3, p7));
  CHECK(p.hasEdge(p4, p10));
  CHECK(p.hasEdge(p5, p6));
  CHECK(p.hasEdge(p5, p7));
  CHECK(p.hasEdge(p6, p9));
  CHECK(p.hasEdge(p7, p11, 0.0001));
  CHECK(p.hasEdge(p9, p10));
  CHECK(p.hasEdge(p9, p11, 0.0001));
  CHECK(p.hasEdge(p10, p11, 0.0001));

  CHECK(p.faceCount() == 7u);
  CHECK(p.hasFace({p1, p3, p7, p5}));
  CHECK(p.hasFace({p1, p5, p6, p2}));
  CHECK(p.hasFace({p1, p2, p4, p3}));
  CHECK(p.hasFace({p2, p6, p9, p10, p4}));
  CHECK(p.hasFace({p3, p4, p10, p11, p7}, 0.0001));
  CHECK(p.hasFace({p5, p7, p11, p9, p6}, 0.0001));
  CHECK(p.hasFace({p9, p11, p10}, 0.0001));
}

TEST_CASE("PolyhedronTest.clipCubeDiagonally")
{
  Polyhedron3d p(vm::bbox3d(64.0));

  const vm::plane3d plane(vm::vec3d::zero(), normalize(vm::vec3d(1.0, 1.0, 0.0)));
  CHECK(p.clip(plane).success());

  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(+64.0, -64.0, -64.0);
  const vm::vec3d p6(+64.0, -64.0, +64.0);

  CHECK(p.vertexCount() == 6u);
  CHECK(p.hasVertex(p1));
  CHECK(p.hasVertex(p2));
  CHECK(p.hasVertex(p3));
  CHECK(p.hasVertex(p4));
  CHECK(p.hasVertex(p5));
  CHECK(p.hasVertex(p6));

  CHECK(p.edgeCount() == 9u);
  CHECK(p.hasEdge(p1, p2));
  CHECK(p.hasEdge(p1, p3));
  CHECK(p.hasEdge(p1, p5));
  CHECK(p.hasEdge(p2, p4));
  CHECK(p.hasEdge(p2, p6));
  CHECK(p.hasEdge(p3, p4));
  CHECK(p.hasEdge(p3, p5));
  CHECK(p.hasEdge(p4, p6));
  CHECK(p.hasEdge(p5, p6));

  CHECK(p.faceCount() == 5u);
  CHECK(p.hasFace({p1, p2, p4, p3}));
  CHECK(p.hasFace({p1, p5, p6, p2}));
  CHECK(p.hasFace({p3, p4, p6, p5}));
  CHECK(p.hasFace({p1, p3, p5}));
  CHECK(p.hasFace({p2, p6, p4}));
}

TEST_CASE("PolyhedronTest.clipCubeWithVerticalSlantedPlane")
{
  Polyhedron3d p(vm::bbox3d(64.0));

  const vm::plane3d plane(
    vm::vec3d(0.0, -64.0, 0.0), normalize(vm::vec3d(2.0, 1.0, 0.0)));
  CHECK(p.clip(plane).success());

  const vm::vec3d p1(-64.0, -64.0, -64.0);
  const vm::vec3d p2(-64.0, -64.0, +64.0);
  const vm::vec3d p3(-64.0, +64.0, -64.0);
  const vm::vec3d p4(-64.0, +64.0, +64.0);
  const vm::vec3d p5(0.0, -64.0, -64.0);
  const vm::vec3d p6(0.0, -64.0, +64.0);

  CHECK(p.vertexCount() == 6u);
  CHECK(p.hasVertex(p1));
  CHECK(p.hasVertex(p2));
  CHECK(p.hasVertex(p3));
  CHECK(p.hasVertex(p4));
  CHECK(p.hasVertex(p5));
  CHECK(p.hasVertex(p6));

  CHECK(p.edgeCount() == 9u);
  CHECK(p.hasEdge(p1, p2));
  CHECK(p.hasEdge(p1, p3));
  CHECK(p.hasEdge(p1, p5));
  CHECK(p.hasEdge(p2, p4));
  CHECK(p.hasEdge(p2, p6));
  CHECK(p.hasEdge(p3, p4));
  CHECK(p.hasEdge(p3, p5));
  CHECK(p.hasEdge(p4, p6));
  CHECK(p.hasEdge(p5, p6));

  CHECK(p.faceCount() == 5u);
  CHECK(p.hasFace({p1, p2, p4, p3}));
  CHECK(p.hasFace({p1, p5, p6, p2}));
  CHECK(p.hasFace({p3, p4, p6, p5}));
  CHECK(p.hasFace({p1, p3, p5}));
  CHECK(p.hasFace({p2, p6, p4}));
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
  const Polyhedron3d minuend(vm::bbox3d(32.0));
  const Polyhedron3d subtrahend(vm::bbox3d(16.0));

  auto result = minuend.subtract(subtrahend);

  std::vector<vm::vec3d> leftVertices, rightVertices, frontVertices, backVertices,
    topVertices, bottomVertices;

  vm::parse_all<double, 3>(
    "(-32 -32 -32) (-32 32 -32) (-32 -32 32) (-32 32 32) (-16 -32 -32) (-16 32 -32) (-16 "
    "32 32) "
    "(-16 -32 32)",
    std::back_inserter(leftVertices));
  vm::parse_all<double, 3>(
    "(32 -32 32) (32 32 32) (16 -32 -32) (16 -32 32) (16 32 32) (16 32 -32) (32 32 -32) "
    "(32 -32 "
    "-32)",
    std::back_inserter(rightVertices));

  vm::parse_all<double, 3>(
    "(16 -32 32) (16 -32 -32) (-16 -32 32) (-16 -32 -32) (-16 -16 32) (16 -16 32) (16 "
    "-16 -32) "
    "(-16 -16 -32)",
    std::back_inserter(frontVertices));
  vm::parse_all<double, 3>(
    "(16 32 -32) (16 32 32) (-16 16 -32) (16 16 -32) (16 16 32) (-16 16 32) (-16 32 32) "
    "(-16 32 "
    "-32)",
    std::back_inserter(backVertices));

  vm::parse_all<double, 3>(
    "(-16 16 32) (16 16 32) (16 -16 32) (-16 -16 32) (-16 -16 16) (-16 16 16) (16 16 16) "
    "(16 -16 "
    "16)",
    std::back_inserter(topVertices));
  vm::parse_all<double, 3>(
    "(-16 -16 -32) (16 -16 -32) (-16 16 -32) (16 16 -32) (-16 -16 -16) (16 -16 -16) (16 "
    "16 -16) "
    "(-16 16 -16)",
    std::back_inserter(bottomVertices));

  CHECK(findAndRemove(result, leftVertices));
  CHECK(findAndRemove(result, rightVertices));
  CHECK(findAndRemove(result, frontVertices));
  CHECK(findAndRemove(result, backVertices));
  CHECK(findAndRemove(result, topVertices));
  CHECK(findAndRemove(result, bottomVertices));

  CHECK(result.empty());
}

TEST_CASE("PolyhedronTest.subtractDisjointCuboidFromCuboid")
{
  const Polyhedron3d minuend(vm::bbox3d(64.0));
  const Polyhedron3d subtrahend(
    vm::bbox3d(vm::vec3d(96.0, 96.0, 96.0), vm::vec3d(128.0, 128.0, 128.0)));

  auto result = minuend.subtract(subtrahend);
  CHECK(result.size() == 1u);

  const Polyhedron3d resultPolyhedron = result.front();
  CHECK(resultPolyhedron == minuend);
}

TEST_CASE("PolyhedronTest.subtractCuboidFromInnerCuboid")
{
  const Polyhedron3d minuend(vm::bbox3d(32.0));
  const Polyhedron3d subtrahend(vm::bbox3d(64.0));

  auto result = minuend.subtract(subtrahend);
  CHECK(result.empty());
}

TEST_CASE("PolyhedronTest.subtractCuboidFromIdenticalCuboid")
{
  const Polyhedron3d minuend(vm::bbox3d(64.0));
  const Polyhedron3d subtrahend(vm::bbox3d(64.0));

  auto result = minuend.subtract(subtrahend);
  CHECK(result.empty());
}

TEST_CASE("PolyhedronTest.subtractCuboidProtrudingThroughCuboid")
{
  const Polyhedron3d minuend(
    vm::bbox3d(vm::vec3d(-32.0, -32.0, -16.0), vm::vec3d(32.0, 32.0, 16.0)));
  const Polyhedron3d subtrahend(
    vm::bbox3d(vm::vec3d(-16.0, -16.0, -32.0), vm::vec3d(16.0, 16.0, 32.0)));

  auto result = minuend.subtract(subtrahend);
  CHECK(result.size() == 4u);

  const std::vector<vm::vec3d> leftVertices{
    vm::vec3d(-16, -32, -16),
    vm::vec3d(-16, 32, -16),
    vm::vec3d(-16, 32, 16),
    vm::vec3d(-16, -32, 16),
    vm::vec3d(-32, 32, 16),
    vm::vec3d(-32, -32, 16),
    vm::vec3d(-32, -32, -16),
    vm::vec3d(-32, 32, -16),
  };

  const std::vector<vm::vec3d> rightVertices{
    vm::vec3d(32, -32, 16),
    vm::vec3d(32, 32, 16),
    vm::vec3d(32, -32, -16),
    vm::vec3d(32, 32, -16),
    vm::vec3d(16, -32, -16),
    vm::vec3d(16, -32, 16),
    vm::vec3d(16, 32, 16),
    vm::vec3d(16, 32, -16)};

  const std::vector<vm::vec3d> frontVertices{
    vm::vec3d(-16, -32, -16),
    vm::vec3d(-16, -32, 16),
    vm::vec3d(16, -16, -16),
    vm::vec3d(-16, -16, -16),
    vm::vec3d(-16, -16, 16),
    vm::vec3d(16, -16, 16),
    vm::vec3d(16, -32, 16),
    vm::vec3d(16, -32, -16)};

  const std::vector<vm::vec3d> backVertices{
    vm::vec3d(-16, 32, 16),
    vm::vec3d(-16, 32, -16),
    vm::vec3d(16, 32, 16),
    vm::vec3d(16, 32, -16),
    vm::vec3d(16, 16, 16),
    vm::vec3d(-16, 16, 16),
    vm::vec3d(-16, 16, -16),
    vm::vec3d(16, 16, -16)};

  CHECK(findAndRemove(result, frontVertices));
  CHECK(findAndRemove(result, backVertices));
  CHECK(findAndRemove(result, leftVertices));
  CHECK(findAndRemove(result, rightVertices));

  CHECK(result.empty());
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

  const Polyhedron3d minuend(
    vm::bbox3d(vm::vec3d(-32.0, -16.0, -32.0), vm::vec3d(32.0, 16.0, 32.0)));
  const Polyhedron3d subtrahend(
    vm::bbox3d(vm::vec3d(-16.0, -32.0, -64.0), vm::vec3d(16.0, 32.0, 0.0)));

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

  const Polyhedron3d minuend(
    vm::bbox3d(vm::vec3d(-64.0, -64.0, -16.0), vm::vec3d(64.0, 64.0, 16.0)));
  const Polyhedron3d subtrahend(
    vm::bbox3d(vm::vec3d(-32.0, -64.0, -32.0), vm::vec3d(32.0, 0.0, 32.0)));

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

  const Polyhedron3d minuend{
    vm::vec3d(-32.0, -8.0, 0.0),
    vm::vec3d(+32.0, -8.0, 0.0),
    vm::vec3d(+32.0, -8.0, 32.0),
    vm::vec3d(+16.0, -8.0, 48.0),
    vm::vec3d(-16.0, -8.0, 48.0),
    vm::vec3d(-32.0, -8.0, 32.0),
    vm::vec3d(-32.0, +8.0, 0.0),
    vm::vec3d(+32.0, +8.0, 0.0),
    vm::vec3d(+32.0, +8.0, 32.0),
    vm::vec3d(+16.0, +8.0, 48.0),
    vm::vec3d(-16.0, +8.0, 48.0),
    vm::vec3d(-32.0, +8.0, 32.0)};

  const Polyhedron3d subtrahend(
    vm::bbox3d(vm::vec3d(-16.0, -8.0, 0.0), vm::vec3d(16.0, 8.0, 32.0)));

  auto result = minuend.subtract(subtrahend);

  std::vector<vm::vec3d> left, right, top;
  vm::parse_all<double, 3>(
    "(-16 8 -0) (-16 8 48) (-16 -8 48) (-16 -8 -0) (-32 -8 -0) (-32 -8 32) (-32 8 -0) "
    "(-32 8 32)",
    std::back_inserter(left));
  vm::parse_all<double, 3>(
    "(32 -8 32) (32 8 32) (32 8 -0) (32 -8 -0) (16 8 48) (16 8 -0) (16 -8 -0) (16 -8 48)",
    std::back_inserter(right));
  vm::parse_all<double, 3>(
    "(16 8 32) (16 -8 32) (-16 -8 32) (-16 8 32) (-16 -8 48) (-16 8 48) (16 8 48) (16 -8 "
    "48)",
    std::back_inserter(top));

  CHECK(findAndRemove(result, left));
  CHECK(findAndRemove(result, right));
  CHECK(findAndRemove(result, top));

  CHECK(result.empty());
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

  std::vector<vm::vec3d> subtrahendVertices;
  vm::parse_all<double, 3>(
    "(-32.0 0.0 +96.0) (0.0 -32.0 +96.0) (+32.0 0.0 +96.0) (0.0 +32.0 +96.0) (-32.0 0.0 "
    "-96.0) "
    "(0.0 -32.0 -96.0) (+32.0 0.0 -96.0) (0.0 +32.0 -96.0)",
    std::back_inserter(subtrahendVertices));

  const Polyhedron3d minuend(vm::bbox3d(64.0));
  const Polyhedron3d subtrahend(subtrahendVertices);

  auto result = minuend.subtract(subtrahend);

  std::vector<vm::vec3d> f1, f2, f3, f4;
  vm::parse_all<double, 3>(
    "(64 64 64) (-32 64 -64) (64 -32 -64) (64 -32 64) (-32 64 64) (64 64 -64)",
    std::back_inserter(f1));
  vm::parse_all<double, 3>(
    "(-64 32 64) (-64 32 -64) (-32 -0 64) (-32 -0 -64) (-0 32 -64) (-0 32 64) (-64 64 "
    "64) (-32 64 "
    "-64) (-32 64 64) (-64 64 -64)",
    std::back_inserter(f2));
  vm::parse_all<double, 3>(
    "(64 -32 64) (64 -32 -64) (64 -64 64) (64 -64 -64) (-0 -32 64) (32 -0 64) (32 -0 "
    "-64) (-0 -32 "
    "-64) (32 -64 -64) (32 -64 64)",
    std::back_inserter(f3));
  vm::parse_all<double, 3>(
    "(-64 -64 64) (-64 -64 -64) (-64 32 -64) (-64 32 64) (32 -64 64) (32 -64 -64)",
    std::back_inserter(f4));
  CHECK(findAndRemove(result, f1));
  CHECK(findAndRemove(result, f2));
  CHECK(findAndRemove(result, f3));
  CHECK(findAndRemove(result, f4));

  CHECK(result.size() == 0u);
}

TEST_CASE("PolyhedronTest.intersection_empty_polyhedron")
{
  const Polyhedron3d empty;
  const Polyhedron3d point{vm::vec3d(1.0, 0.0, 0.0)};
  const Polyhedron3d edge{vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0)};
  const Polyhedron3d polygon{
    vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0)};
  const Polyhedron3d polyhedron{
    vm::vec3d(1.0, 0.0, 0.0),
    vm::vec3d(2.0, 0.0, 0.0),
    vm::vec3d(0.0, 1.0, 0.0),
    vm::vec3d(0.0, 0.0, 1.0)};

  CHECK(mutuallyNotIntersects(empty, empty));
  CHECK(mutuallyNotIntersects(empty, point));
  CHECK(mutuallyNotIntersects(empty, edge));
  CHECK(mutuallyNotIntersects(empty, polygon));
  CHECK(mutuallyNotIntersects(empty, polyhedron));
}

TEST_CASE("PolyhedronTest.intersection_point_point")
{
  const Polyhedron3d point{vm::vec3d(0.0, 0.0, 0.0)};

  CHECK(mutuallyIntersects(point, point));
  CHECK(mutuallyNotIntersects(point, Polyhedron3d{vm::vec3d(0.0, 0.0, 1.0)}));
}

TEST_CASE("PolyhedronTest.intersection_point_edge")
{
  const vm::vec3d pointPos(0.0, 0.0, 0.0);
  const Polyhedron3d point{pointPos};

  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      pointPos, vm::vec3d(1.0, 0.0, 0.0)})); // point / edge originating at point
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 0.0),
      vm::vec3d(1.0, 0.0, 0.0)})); // point / edge containing point
  CHECK(mutuallyNotIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 1.0), vm::vec3d(1.0, 0.0, 1.0)})); // point / unrelated edge
}

TEST_CASE("PolyhedronTest.intersection_point_polygon")
{
  const vm::vec3d pointPos(0.0, 0.0, 0.0);
  const Polyhedron3d point{pointPos};

  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      pointPos,
      vm::vec3d(1.0, 0.0, 0.0),
      vm::vec3d(0.0, 1.0, 0.0)})); // point / triangle with point as vertex
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 0.0),
      vm::vec3d(1.0, 0.0, 0.0),
      vm::vec3d(0.0, 1.0, 0.0)})); // point / triangle with point on edge
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, 0.0),
      vm::vec3d(1.0, -1.0, 0.0),
      vm::vec3d(0.0, 1.0, 0.0)})); // point / triangle containing point

  CHECK(mutuallyNotIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, 1.0),
      vm::vec3d(1.0, -1.0, 1.0),
      vm::vec3d(0.0, 1.0, 1.0)})); // point / triangle above point
}

TEST_CASE("PolyhedronTest.intersection_point_polyhedron")
{
  const vm::vec3d pointPos(0.0, 0.0, 0.0);
  const Polyhedron3d point{pointPos};

  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      pointPos,
      vm::vec3d(1.0, 0.0, 0.0),
      vm::vec3d(0.0, 1.0, 0.0),
      vm::vec3d(0.0, 0.0, 1.0)})); // point / tetrahedron with point as vertex
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 0.0),
      vm::vec3d(1.0, 0.0, 0.0),
      vm::vec3d(0.0, 1.0, 0.0),
      vm::vec3d(0.0, 0.0, 1.0)})); // point / tetrahedron with point on edge
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, 0.0),
      vm::vec3d(1.0, -1.0, 0.0),
      vm::vec3d(0.0, 1.0, 0.0),
      vm::vec3d(0.0, 0.0, 1.0)})); // point / tetrahedron with point on face
  CHECK(mutuallyIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, -1.0),
      vm::vec3d(1.0, -1.0, -1.0),
      vm::vec3d(0.0, 1.0, -1.0),
      vm::vec3d(0.0, 0.0, 1.0)})); // point / tetrahedron with point on face

  CHECK(mutuallyNotIntersects(
    point,
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, 1.0),
      vm::vec3d(1.0, -1.0, 1.0),
      vm::vec3d(0.0, 1.0, 1.0),
      vm::vec3d(0.0, 0.0, 2.0)})); // point / tetrahedron above point
}

TEST_CASE("PolyhedronTest.intersection_edge_edge")
{
  const vm::vec3d point1(-1.0, 0.0, 0.0);
  const vm::vec3d point2(+1.0, 0.0, 0.0);
  const Polyhedron3d edge{point1, point2};

  CHECK(mutuallyIntersects(edge, edge));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{point1, vm::vec3d(0.0, 0.0, 1.0)}));
  CHECK(mutuallyIntersects(edge, Polyhedron3d{point2, vm::vec3d(0.0, 0.0, 1.0)}));
  CHECK(mutuallyIntersects(
    edge, Polyhedron3d{vm::vec3d(0.0, -1.0, 0.0), vm::vec3d(0.0, 1.0, 0.0)}));
  CHECK(mutuallyIntersects(
    edge, Polyhedron3d{vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0)}));
  CHECK(mutuallyIntersects(
    edge, Polyhedron3d{vm::vec3d(-2.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0)}));

  CHECK(mutuallyNotIntersects(
    edge, Polyhedron3d{point1 + vm::vec3d::pos_z(), point2 + vm::vec3d::pos_z()}));
}

TEST_CASE("PolyhedronTest.intersection_edge_polygon_same_plane")
{
  const vm::vec3d point1(-1.0, 0.0, 0.0);
  const vm::vec3d point2(+1.0, 0.0, 0.0);
  const Polyhedron3d edge{point1, point2};

  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(1.0, 0.0, 0.0),
      vm::vec3d(1.0, -1.0, 0.0),
      vm::vec3d(2.0, -1.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0)})); // one shared point
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 0.0),
      vm::vec3d(0.0, -1.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(0.0, +1.0, 0.0)})); // two shared points
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 0.0),
      vm::vec3d(1.0, 0.0, 0.0),
      vm::vec3d(1.0, 1.0, 0.0),
      vm::vec3d(-1.0, 1.0, 0.0)})); // shared edge
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(0.0, 1.0, 0.0),
      vm::vec3d(0.0, -1.0, 0.0),
      vm::vec3d(2.0, -1.0, 0.0),
      vm::vec3d(2.0, 1.0, 0.0)})); // polygon contains one point
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(-2.0, 1.0, 0.0),
      vm::vec3d(-2.0, -1.0, 0.0),
      vm::vec3d(2.0, -1.0, 0.0),
      vm::vec3d(2.0, 1.0, 0.0)})); // polygon contains both points
  CHECK(mutuallyIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(-0.5, 1.0, 0.0),
      vm::vec3d(-0.5, -1.0, 0.0),
      vm::vec3d(0.5, -1.0, 0.0),
      vm::vec3d(0.5, 1.0, 0.0)})); // edge intersects polygon completely

  CHECK(mutuallyNotIntersects(
    edge,
    Polyhedron3d{
      vm::vec3d(+2.0, 1.0, 0.0),
      vm::vec3d(+2.0, -1.0, 0.0),
      vm::vec3d(+3.0, -1.0, 0.0),
      vm::vec3d(+3.0, 1.0, 0.0)})); // no intersection
}

TEST_CASE("PolyhedronTest.intersection_edge_polygon_different_plane")
{
  const vm::vec3d point1(0.0, 0.0, 1.0);
  const vm::vec3d point2(0.0, 0.0, -1.0);
  const Polyhedron3d edge{point1, point2};

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(0.0, 0.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)})); // one shared point

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)})); // polygon edge contains edge origin

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(1.0, 1.0, 0.0), vm::vec3d(1.0, 1.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)})); // polygon contains edge origin

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, -1.0), vm::vec3d(0.0, 0.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)})); // edge intersects polygon vertex

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(1.0, 0.0, -1.0), vm::vec3d(1.0, 0.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)})); // edge intersects polygon edge

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(1.0, 1.0, -1.0), vm::vec3d(1.0, 1.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)})); // edge intersects polygon center

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{vm::vec3d(3.0, 1.0, -1.0), vm::vec3d(3.0, 1.0, +1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)}));

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{vm::vec3d(1.0, 1.0, 1.0), vm::vec3d(1.0, 1.0, 2.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)}));

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, 1.0), vm::vec3d(1.0, 1.0, 1.0)},
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(2.0, 0.0, 0.0),
      vm::vec3d(2.0, 2.0, 0.0),
      vm::vec3d(0.0, 2.0, 0.0)}));
}

TEST_CASE("PolyhedronTest.intersection_edge_polyhedron")
{
  const Polyhedron3d tetrahedron{
    vm::vec3d(-1.0, -1.0, 0.0),
    vm::vec3d(+1.0, -1.0, 0.0),
    vm::vec3d(0.0, +1.0, 0.0),
    vm::vec3d(0.0, 0.0, 1.0)};

  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, 1.0), vm::vec3d(0.0, 0.0, 2.0)},
    tetrahedron)); // one shared point
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, -0.9999, 0.0), vm::vec3d(0.0, -2.0, 0.0)},
    tetrahedron)); // edge point on polyhedron edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(0.0, 0.0, -1.0)},
    tetrahedron)); // edge point on polyhedron face
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(-1.0, -1.0, 0.0), vm::vec3d(+1.0, -1.0, 0.0)},
    tetrahedron)); // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, 0.5), vm::vec3d(0.0, 0.0, 2.0)},
    tetrahedron)); // polyhedron contains one edge point
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, 0.2), vm::vec3d(0.0, 0.0, 0.7)},
    tetrahedron)); // polyhedron contains both edge points
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(0.0, 0.0, -1.0), vm::vec3d(0.0, 0.0, 2.0)},
    tetrahedron)); // edge penetrates polyhedron

  CHECK(mutuallyNotIntersects(
    Polyhedron3d{vm::vec3d(-2.0, -2.0, -1.0), vm::vec3d(2.0, 2.0, -1.0)},
    tetrahedron)); // no intersection
}

TEST_CASE("PolyhedronTest.intersection_polygon_polygon_same_plane")
{
  const Polyhedron3d square{
    vm::vec3d(-1.0, -1.0, 0.0),
    vm::vec3d(+1.0, -1.0, 0.0),
    vm::vec3d(+1.0, +1.0, 0.0),
    vm::vec3d(-1.0, +1.0, 0.0)};

  // shared vertex:
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(+1, +1, 0), vm::vec3d(+2, +1, 0), vm::vec3d(+1, +2, 0)},
    square));

  // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{vm::vec3d(-1, +1, 0), vm::vec3d(+1, +1, 0), vm::vec3d(0, +2, 0)},
    square));

  // edge contains other edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2, -1, 0),
      vm::vec3d(+2, -1, 0),
      vm::vec3d(+2, +1, 0),
      vm::vec3d(-2, +1, 0),
    },
    square));

  // one contains vertex of another
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0, 0, 0), vm::vec3d(+2, 0, 0), vm::vec3d(+2, +2, 0), vm::vec3d(0, +2, 0)},
    square));

  // one contains another entirely
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2, -2, 0),
      vm::vec3d(+2, -2, 0),
      vm::vec3d(+2, +2, 0),
      vm::vec3d(-2, +2, 0)},
    square));

  // one penetrates the other
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2, -0.5, 0),
      vm::vec3d(+2, -0.5, 0),
      vm::vec3d(+2, +0.5, 0),
      vm::vec3d(-2, +0.5, 0)},
    square));

  // no intersection
  CHECK(mutuallyNotIntersects(
    Polyhedron3d{
      vm::vec3d(+2, +2, 0),
      vm::vec3d(+3, +2, 0),
      vm::vec3d(+3, +3, 0),
      vm::vec3d(+3, +3, 0)},
    square));
}

TEST_CASE(
  "PolyhedronTest.intersection_polygon_polygon_different_plane", "[PolyhedronTest]")
{
  const Polyhedron3d square{
    vm::vec3d(-1.0, -1.0, 0.0),
    vm::vec3d(+1.0, -1.0, 0.0),
    vm::vec3d(+1.0, +1.0, 0.0),
    vm::vec3d(-1.0, +1.0, 0.0)};

  // shared vertex
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, 0.0), vm::vec3d(-2.0, -1.0, 0.0), vm::vec3d(-2.0, -1.0, 1.0)},
    square));

  // vertex on edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0.0, -1.0, 0.0),
      vm::vec3d(0.0, -2.0, 0.0),
      vm::vec3d(0.0, -1.0, 1.0),
      vm::vec3d(0.0, -2.0, 1.0),
    },
    square));

  // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-1.0, -1.0, 0.0),
      vm::vec3d(+1.0, -1.0, 0.0),
      vm::vec3d(+1.0, -1.0, 1.0),
      vm::vec3d(-1.0, -1.0, 1.0)},
    square));

  // edges intersect
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0.0, -1.0, -1.0),
      vm::vec3d(0.0, -1.0, +1.0),
      vm::vec3d(0.0, -2.0, +1.0),
      vm::vec3d(0.0, -2.0, -1.0)},
    square));

  // partial penetration (one edge penetrates each)
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, -1.0),
      vm::vec3d(0.0, 0.0, +1.0),
      vm::vec3d(2.0, 0.0, +1.0),
      vm::vec3d(2.0, 0.0, -1.0)},
    square));

  // full penetration (two edges penetrate)
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2.0, 0.0, -2.0),
      vm::vec3d(-2.0, 0.0, +2.0),
      vm::vec3d(+2.0, 0.0, -2.0),
      vm::vec3d(+2.0, 0.0, +2.0)},
    square));

  // no intersection
  CHECK(mutuallyNotIntersects(
    Polyhedron3d{
      vm::vec3d(-1.0, 0.0, 5.0),
      vm::vec3d(+1.0, 0.0, 5.0),
      vm::vec3d(-1.0, 0.0, 6.0),
      vm::vec3d(+1.0, 0.0, 6.0)},
    square));
}

TEST_CASE(
  "PolyhedronTest.intersection_polygon_polyhedron_same_plane_as_face", "[PolyhedronTest]")
{
  const Polyhedron3d cube{
    vm::vec3d(-1.0, -1.0, -1.0),
    vm::vec3d(-1.0, -1.0, +1.0),
    vm::vec3d(-1.0, +1.0, -1.0),
    vm::vec3d(-1.0, +1.0, +1.0),
    vm::vec3d(+1.0, -1.0, -1.0),
    vm::vec3d(+1.0, -1.0, +1.0),
    vm::vec3d(+1.0, +1.0, -1.0),
    vm::vec3d(+1.0, +1.0, +1.0),
  };

  // polygon is on the same plane as top face

  // shared vertex
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(+1.0, +1.0, +1.0),
      vm::vec3d(+2.0, +1.0, +1.0),
      vm::vec3d(+2.0, +2.0, +1.0),
    },
    cube));

  // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(+1.0, +1.0, +1.0),
      vm::vec3d(-1.0, +1.0, +1.0),
      vm::vec3d(+1.0, +2.0, +1.0)},
    cube));

  // edge contains other edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-0.5, +1.0, +1.0),
      vm::vec3d(+0.5, +1.0, +1.0),
      vm::vec3d(+0.5, +2.0, +1.0)},
    cube));

  // one contains vertex of another
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(+0.0, +0.0, +1.0),
      vm::vec3d(+2.0, +0.0, +1.0),
      vm::vec3d(+2.0, +2.0, +1.0),
      vm::vec3d(+0.0, +2.0, +1.0),
    },
    cube));

  // one contains another entirely
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-0.5, -0.5, +1.0),
      vm::vec3d(-0.5, +0.5, +1.0),
      vm::vec3d(+0.5, +0.5, +1.0),
      vm::vec3d(+0.5, -0.5, +1.0),
    },
    cube));
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2.5, -2.5, +1.0),
      vm::vec3d(-2.5, +2.5, +1.0),
      vm::vec3d(+2.5, +2.5, +1.0),
      vm::vec3d(+2.5, -2.5, +1.0),
    },
    cube));

  // one penetrates the other
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2.0, -0.5, +1.0),
      vm::vec3d(+2.0, -0.5, +1.0),
      vm::vec3d(-2.0, +0.5, +1.0),
      vm::vec3d(+2.0, +0.5, +1.0),
    },
    cube));

  // no intersection
  CHECK(mutuallyNotIntersects(
    Polyhedron3d{
      vm::vec3d(+2.0, +2.0, +1.0),
      vm::vec3d(+3.0, +2.0, +1.0),
      vm::vec3d(+3.0, +3.0, +1.0),
      vm::vec3d(+2.0, +3.0, +1.0),
    },
    cube));
}

TEST_CASE(
  "PolyhedronTest.intersection_polygon_polyhedron_any_orientation", "[PolyhedronTest]")
{
  const Polyhedron3d cube{
    vm::vec3d(-1.0, -1.0, -1.0),
    vm::vec3d(-1.0, -1.0, +1.0),
    vm::vec3d(-1.0, +1.0, -1.0),
    vm::vec3d(-1.0, +1.0, +1.0),
    vm::vec3d(+1.0, -1.0, -1.0),
    vm::vec3d(+1.0, -1.0, +1.0),
    vm::vec3d(+1.0, +1.0, -1.0),
    vm::vec3d(+1.0, +1.0, +1.0),
  };

  // shared vertex
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(+1.0, +1.0, +1.0),
      vm::vec3d(+2.0, +1.0, +2.0),
      vm::vec3d(+2.0, +2.0, +2.0)},
    cube));

  // polygon vertex on polyhedron edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(+0.0, +1.0, +1.0),
      vm::vec3d(+2.0, +1.0, +2.0),
      vm::vec3d(+2.0, +2.0, +2.0)},
    cube));

  // polyhedron vertex on polygon edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0.0, 2.0, 1.0), vm::vec3d(2.0, 0.0, 1.0), vm::vec3d(0.0, 0.0, 2.0)},
    cube));

  // shared edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-1.0, 1.0, 1.0), vm::vec3d(+1.0, 1.0, 1.0), vm::vec3d(0.0, 2.0, 2.0)},
    cube));

  // polygon edge inside polyhedron edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-0.5, 1.0, 1.0),
      vm::vec3d(+0.5, 1.0, 1.0),
      vm::vec3d(0.0, 2.0, 2.0),
    },
    cube));

  // polyhedorn edge inside polygon edge
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2.0, 1.0, 1.0), vm::vec3d(+2.0, 1.0, 1.0), vm::vec3d(0.0, 2.0, 2.0)},
    cube));

  // edges intersect
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0.0, -2.0, 0.0), vm::vec3d(0.0, 0.0, 2.0), vm::vec3d(0.0, -2.0, 2.0)},
    cube));

  // penetration (two polygon edges intersect)
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(0.0, 0.0, 0.0),
      vm::vec3d(0.0, -3.0, 0.0),
      vm::vec3d(3.0, 0.0, 2.0),
    },
    cube));

  // polyhedron contains polygon
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-0.5, 0.0, 0.0), vm::vec3d(0.0, 0.5, 0.0), vm::vec3d(0.0, 0.0, 0.5)},
    cube));

  // polygon slices polyhedron (surrounds it)
  CHECK(mutuallyIntersects(
    Polyhedron3d{
      vm::vec3d(-2.0, -2.0, 0.0),
      vm::vec3d(-2.0, +2.0, 0.0),
      vm::vec3d(+2.0, -2.0, 0.0),
      vm::vec3d(+2.0, +2.0, 0.0),
    },
    cube));
}
} // namespace Model
} // namespace TrenchBroom
