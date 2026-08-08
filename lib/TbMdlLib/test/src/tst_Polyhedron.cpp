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

#include "mdl/CatchConfig.h"
#include "mdl/Polyhedron.h"
#include "mdl/Polyhedron_DefaultPayload.h"
#include "mdl/Polyhedron_IO.h" // IWYU pragma: keep
#include "mdl/Polyhedron_Instantiation.h"

#include "vm/approx.h"
#include "vm/ray.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <algorithm>
#include <iterator>
#include <set>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

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

} // namespace

TEST_CASE("Polyhedron")
{
  SECTION("copy")
  {
    const auto p1 = vm::vec3d{0, 0, 8};
    const auto p2 = vm::vec3d{8, 0, 0};
    const auto p3 = vm::vec3d{-8, 0, 0};
    const auto p4 = vm::vec3d{0, 8, 0};

    CHECK(Polyhedron3d{} == (Polyhedron3d{} = Polyhedron3d{}));
    CHECK(Polyhedron3d{p1} == (Polyhedron3d{} = Polyhedron3d{p1}));
    CHECK(Polyhedron3d{p1, p2} == (Polyhedron3d{} = Polyhedron3d{p1, p2}));
    CHECK(Polyhedron3d{p1, p2, p3} == (Polyhedron3d{} = Polyhedron3d{p1, p2, p3}));
    CHECK(
      Polyhedron3d{p1, p2, p3, p4} == (Polyhedron3d{} = Polyhedron3d{p1, p2, p3, p4}));
  }

  SECTION("swap")
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
    REQUIRE(lhs == original);
    REQUIRE(rhs == other);

    using std::swap;
    swap(lhs, rhs);

    CHECK(lhs == other);
    CHECK(rhs == original);

    CHECK(lhs.bounds() == other.bounds());
    CHECK(rhs.bounds() == original.bounds());
  }

  SECTION("constructor")
  {
    SECTION("With no points")
    {
      auto p = Polyhedron3d{};
      CHECK(p.empty());
    }

    SECTION("With one point")
    {
      const auto p1 = vm::vec3d{-8, -8, -8};

      auto p = Polyhedron3d{p1};

      CHECK(!p.empty());
      CHECK(p.point());
      CHECK(!p.edge());
      CHECK(!p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1}));
    }

    SECTION("With two identical points")
    {
      const auto p1 = vm::vec3d{-8, -8, -8};

      auto p = Polyhedron3d{p1, p1};

      CHECK(!p.empty());
      CHECK(p.point());
      CHECK(!p.edge());
      CHECK(!p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1}));
    }

    SECTION("With two points")
    {
      const auto p1 = vm::vec3d{0, 0, 0};
      const auto p2 = vm::vec3d{3, 0, 0};

      auto p = Polyhedron3d{p1, p2};

      CHECK(!p.empty());
      CHECK(!p.point());
      CHECK(p.edge());
      CHECK(!p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1, p2}));
    }

    SECTION("With three colinear points")
    {
      const auto p1 = vm::vec3d{0, 0, 0};
      const auto p2 = vm::vec3d{3, 0, 0};
      const auto p3 = vm::vec3d{6, 0, 0};

      auto p = Polyhedron3d{p1, p2, p3};

      CHECK(!p.empty());
      CHECK(!p.point());
      CHECK(p.edge());
      CHECK(!p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1, p3}));
    }

    SECTION("With three points")
    {
      const auto p1 = vm::vec3d{0, 0, 0};
      const auto p2 = vm::vec3d{3, 0, 0};
      const auto p3 = vm::vec3d{6, 5, 0};

      auto p = Polyhedron3d{p1, p2, p3};

      CHECK(!p.empty());
      CHECK(!p.point());
      CHECK(!p.edge());
      CHECK(p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1, p2, p3}));
    }

    SECTION("With a triangle and a point contained in it")
    {
      const auto p1 = vm::vec3d{0, 0, 0};
      const auto p2 = vm::vec3d{6, 0, 0};
      const auto p3 = vm::vec3d{3, 6, 0};
      const auto p4 = vm::vec3d{3, 3, 0};

      auto p = Polyhedron3d{p1, p2, p3, p4};

      CHECK(!p.empty());
      CHECK(!p.point());
      CHECK(!p.edge());
      CHECK(p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1, p2, p3}));
    }

    SECTION("With four coplanar points")
    {
      const auto p1 = vm::vec3d{0, 0, 0};
      const auto p2 = vm::vec3d{6, 0, 0};
      const auto p3 = vm::vec3d{3, 3, 0};
      const auto p4 = vm::vec3d{3, 6, 0};

      auto p = Polyhedron3d{p1, p2, p3, p4};

      CHECK(!p.empty());
      CHECK(!p.point());
      CHECK(!p.edge());
      CHECK(p.polygon());
      CHECK(!p.polyhedron());

      CHECK(p.hasAllVertices({p1, p2, p4}));
    }

    SECTION("With four points forming a tetrahedron")
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

    SECTION("With a rectangle and a redundant point")
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

    SECTION("With a trapezoid and a redundant point")
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

    SECTION("With a polygon and a redundant point")
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

    SECTION("With a tetrahedron and a redundant point")
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

    SECTION("With a tetrahedron that has coplanar faces")
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

    SECTION("With a cube")
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

    SECTION("With a point set that previously failed to build the correct hull (1)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-64.0, -45.5049, -34.4752),
        vm::vec3d(-64.0, -43.6929, -48.0),
        vm::vec3d(-64.0, 20.753, -34.4752),
        vm::vec3d(-64.0, 64.0, -48.0),
        vm::vec3d(-63.7297, 22.6264, -48.0),
        vm::vec3d(-57.9411, 22.6274, -37.9733),
        vm::vec3d(-44.6031, -39.1918, -48.0),
        vm::vec3d(-43.5959, -39.1918, -46.2555),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 7u);
    }

    SECTION("With a point set that previously failed to build the correct hull (2)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-64.0, 48.7375, -34.4752),
        vm::vec3d(-64.0, 64.0, -48.0),
        vm::vec3d(-64.0, 64.0, -34.4752),
        vm::vec3d(-63.7297, 22.6264, -48.0),
        vm::vec3d(-57.9411, 22.6274, -37.9733),
        vm::vec3d(-40.5744, 28.0, -48.0),
        vm::vec3d(-40.5744, 64.0, -48.0),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == vertices.size());

      for (const auto& v : vertices)
      {
        CHECK(p.hasVertex(v));
      }
    }

    SECTION("With a point set that previously failed to build the correct hull (3)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-64, -64, -48),
        vm::vec3d(-64, 22.5637, -48),
        vm::vec3d(-64, 64, -48),
        vm::vec3d(-63.7297, 22.6264, -48),
        vm::vec3d(-57.9411, 22.6274, -37.9733),
        vm::vec3d(-44.6031, -39.1918, -48),
        vm::vec3d(-43.5959, -39.1918, -46.2555),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 5u);
    }

    SECTION("With a point set that previously failed to build the correct hull (4)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-64, 64, -48),
        vm::vec3d(-43.5959, -39.1918, -46.2555),
        vm::vec3d(-40.5744, -38.257, -48),
        vm::vec3d(-36.9274, -64, -48),
        vm::vec3d(1.58492, -39.1918, 32),
        vm::vec3d(9.2606, -64, 32),
        vm::vec3d(12.8616, -64, 32),
        vm::vec3d(12.8616, -36.5751, 32),
        vm::vec3d(26.7796, -22.6274, -48),
        vm::vec3d(39.5803, -64, -48),
        vm::vec3d(57.9411, -22.6274, 5.9733),
        vm::vec3d(64, -64, -5.70392),
        vm::vec3d(64, -64, 2.47521),
        vm::vec3d(64, -48.7375, 2.47521),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 13);
    }

    SECTION("With a point set that previously failed to build the correct hull (5)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-64, -64, -64),
        vm::vec3d(-64, -64, 64),
        vm::vec3d(-64, -32, 64),
        vm::vec3d(-32, -64, -64),
        vm::vec3d(-32, -64, 64),
        vm::vec3d(-32, -0, -64),
        vm::vec3d(-32, -0, 64),
        vm::vec3d(-0, -32, -64),
        vm::vec3d(-0, -32, 64),
        vm::vec3d(64, -64, -64),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 8u);
    }

    SECTION("With a point set that previously failed to build the correct hull (6)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-32, -16, -32),
        vm::vec3d(-32, 16, -32),
        vm::vec3d(-32, 16, -0),
        vm::vec3d(-16, -16, -32),
        vm::vec3d(-16, -16, -0),
        vm::vec3d(-16, 16, -32),
        vm::vec3d(-16, 16, -0),
        vm::vec3d(32, -16, -32),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 7u);
    }

    SECTION("With a point set that previously failed to build the correct hull (7)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(12.8616, -36.5751, 32),
        vm::vec3d(57.9411, -22.6274, 5.9733),
        vm::vec3d(64, -64, 2.47521),
        vm::vec3d(64, -64, 32),
        vm::vec3d(64, -48.7375, 2.47521),
        vm::vec3d(64, -24.7084, 32),
        vm::vec3d(64, -22.6274, 16.4676),
        vm::vec3d(64, 64, 32),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 6u);
    }

    SECTION("With a point set that previously failed to build the correct hull (8)")
    {
      // Cause of https://github.com/TrenchBroom/TrenchBroom/issues/1469
      // See also BrushTest.subtractTruncatedCones

      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(-22.364439661516872, 9.2636542228362799, 32),
        vm::vec3d(-21.333333333333332, 11.049582771255995, 32),
        vm::vec3d(-20.235886048009661, 12.95041722806517, 32),
        vm::vec3d(-19.126943405596094, 11.042945924655637, 32),
        vm::vec3d(-18.31934864142023, 14.056930615671543, 32),
        vm::vec3d(-17.237604305873624, 9.9521354859295226, 7.4256258352417603),
        vm::vec3d(-16, 6.6274169975893429, -0),
        vm::vec3d(-15.999999999999998, 9.2376043067828455, -0),
        vm::vec3d(-14.345207554102323, 8.2822094434885454, -0),
        vm::vec3d(-13.739511480972288, 10.542697961743528, -0),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 9u);
    }

    SECTION("With points added incrementally, one at a time")
    {
      const vm::vec3d p1(8, 10, 0);
      const vm::vec3d p2(0, 24, 0);
      const vm::vec3d p3(8, 10, 8);
      const vm::vec3d p4(10, 11, 8);
      const vm::vec3d p5(12, 24, 8);
      const vm::vec3d p6(0, 6, 8);
      const vm::vec3d p7(10, 0, 8);

      Polyhedron3d p;

      p = Polyhedron3d({p1});
      CHECK(p.point());
      CHECK(p.vertexCount() == 1u);
      CHECK(p.hasVertex(p1));

      p = Polyhedron3d({p1, p2});
      CHECK(p.edge());
      CHECK(p.vertexCount() == 2u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p2));
      CHECK(p.edgeCount() == 1u);
      CHECK(p.hasEdge(p1, p2));

      p = Polyhedron3d({p1, p2, p3});
      CHECK(p.polygon());
      CHECK(p.vertexCount() == 3u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p2));
      CHECK(p.hasVertex(p3));
      CHECK(p.edgeCount() == 3u);
      CHECK(p.hasEdge(p1, p2));
      CHECK(p.hasEdge(p1, p3));
      CHECK(p.hasEdge(p2, p3));
      CHECK(p.faceCount() == 1u);
      CHECK(p.hasFace({p1, p3, p2}));

      p = Polyhedron3d({p1, p2, p3, p4});
      CHECK(p.polyhedron());
      CHECK(p.vertexCount() == 4u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p2));
      CHECK(p.hasVertex(p3));
      CHECK(p.hasVertex(p4));
      CHECK(p.edgeCount() == 6u);
      CHECK(p.hasEdge(p1, p2));
      CHECK(p.hasEdge(p1, p3));
      CHECK(p.hasEdge(p2, p3));
      CHECK(p.hasEdge(p1, p4));
      CHECK(p.hasEdge(p2, p4));
      CHECK(p.hasEdge(p3, p4));
      CHECK(p.faceCount() == 4u);
      CHECK(p.hasFace({p1, p3, p2}));
      CHECK(p.hasFace({p1, p2, p4}));
      CHECK(p.hasFace({p1, p4, p3}));
      CHECK(p.hasFace({p3, p4, p2}));

      p = Polyhedron3d({p1, p2, p3, p4, p5});
      CHECK(p.polyhedron());
      CHECK(p.vertexCount() == 5u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p2));
      CHECK(p.hasVertex(p3));
      CHECK(p.hasVertex(p4));
      CHECK(p.hasVertex(p5));
      CHECK(p.edgeCount() == 9u);
      CHECK(p.hasEdge(p1, p2));
      CHECK(p.hasEdge(p1, p3));
      CHECK(p.hasEdge(p2, p3));
      CHECK(p.hasEdge(p1, p4));
      // CHECK(p.hasEdge(p2, p4));
      CHECK(p.hasEdge(p3, p4));
      CHECK(p.hasEdge(p5, p1));
      CHECK(p.hasEdge(p5, p2));
      CHECK(p.hasEdge(p5, p3));
      CHECK(p.hasEdge(p5, p4));
      CHECK(p.faceCount() == 6u);
      CHECK(p.hasFace({p1, p3, p2}));
      // CHECK(p.hasFace({ p1, p2, p4 }));
      CHECK(p.hasFace({p1, p4, p3}));
      // CHECK(p.hasFace({ p3, p4, p2 }));
      CHECK(p.hasFace({p5, p4, p1}));
      CHECK(p.hasFace({p5, p3, p4}));
      CHECK(p.hasFace({p5, p2, p3}));
      CHECK(p.hasFace({p5, p1, p2}));

      p = Polyhedron3d({p1, p2, p3, p4, p5, p6});
      CHECK(p.vertexCount() == 5u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p2));
      // CHECK(p.hasVertex(p3));
      CHECK(p.hasVertex(p4));
      CHECK(p.hasVertex(p5));
      CHECK(p.hasVertex(p6));
      CHECK(p.edgeCount() == 9u);
      CHECK(p.hasEdge(p1, p2));
      // CHECK(p.hasEdge(p1, p3));
      // CHECK(p.hasEdge(p2, p3));
      CHECK(p.hasEdge(p1, p4));
      // CHECK(p.hasEdge(p2, p4));
      // CHECK(p.hasEdge(p3, p4));
      CHECK(p.hasEdge(p5, p1));
      CHECK(p.hasEdge(p5, p2));
      // CHECK(p.hasEdge(p5, p3));
      CHECK(p.hasEdge(p5, p4));
      CHECK(p.hasEdge(p6, p2));
      CHECK(p.hasEdge(p6, p5));
      CHECK(p.hasEdge(p6, p4));
      CHECK(p.hasEdge(p6, p1));
      CHECK(p.faceCount() == 6u);
      // CHECK(p.hasFace({ p1, p3, p2 }));
      // CHECK(p.hasFace({ p1, p2, p4 }));
      // CHECK(p.hasFace({ p1, p4, p3 }));
      // CHECK(p.hasFace({ p3, p4, p2 }));
      CHECK(p.hasFace({p5, p4, p1}));
      // CHECK(p.hasFace({ p5, p3, p4 }));
      // CHECK(p.hasFace({ p5, p2, p3 }));
      CHECK(p.hasFace({p5, p1, p2}));
      CHECK(p.hasFace({p6, p2, p1}));
      CHECK(p.hasFace({p6, p5, p2}));
      CHECK(p.hasFace({p6, p4, p5}));
      CHECK(p.hasFace({p6, p1, p4}));

      p = Polyhedron3d({p1, p2, p3, p4, p5, p6, p7});
      CHECK(p.vertexCount() == 5u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p2));
      // CHECK(p.hasVertex(p3));
      // CHECK(p.hasVertex(p4));
      CHECK(p.hasVertex(p5));
      CHECK(p.hasVertex(p6));
      CHECK(p.hasVertex(p7));
      CHECK(p.edgeCount() == 9u);
      CHECK(p.hasEdge(p1, p2));
      // CHECK(p.hasEdge(p1, p3));
      // CHECK(p.hasEdge(p2, p3));
      // CHECK(p.hasEdge(p1, p4));
      // CHECK(p.hasEdge(p2, p4));
      // CHECK(p.hasEdge(p3, p4));
      CHECK(p.hasEdge(p5, p1));
      CHECK(p.hasEdge(p5, p2));
      // CHECK(p.hasEdge(p5, p3));
      // CHECK(p.hasEdge(p5, p4));
      CHECK(p.hasEdge(p6, p2));
      CHECK(p.hasEdge(p6, p5));
      // CHECK(p.hasEdge(p6, p4));
      CHECK(p.hasEdge(p6, p1));
      CHECK(p.faceCount() == 6u);
      // CHECK(p.hasFace({ p1, p3, p2 }));
      // CHECK(p.hasFace({ p1, p2, p4 }));
      // CHECK(p.hasFace({ p1, p4, p3 }));
      // CHECK(p.hasFace({ p3, p4, p2 }));
      // CHECK(p.hasFace({ p5, p4, p1 }));
      // CHECK(p.hasFace({ p5, p3, p4 }));
      // CHECK(p.hasFace({ p5, p2, p3 }));
      CHECK(p.hasFace({p5, p1, p2}));
      CHECK(p.hasFace({p6, p2, p1}));
      CHECK(p.hasFace({p6, p5, p2}));
      // CHECK(p.hasFace({ p6, p4, p5 }));
      // CHECK(p.hasFace({ p6, p1, p4 }));
      CHECK(p.hasFace({p7, p1, p5}));
      CHECK(p.hasFace({p7, p6, p1}));
      CHECK(p.hasFace({p7, p5, p6}));
    }

    SECTION("With a horizontal and a vertical rectangle sharing an edge")
    {
      const auto vertices = std::vector<vm::vec3d>({
        // a horizontal rectangle
        vm::vec3d(0, 0, 0),
        vm::vec3d(0, 32, 0),
        vm::vec3d(32, 32, 0),
        vm::vec3d(32, 0, 0),

        // a vertical rectangle
        vm::vec3d(32, 16, 16),
        vm::vec3d(32, 16, 32),
        vm::vec3d(32, 32, 32),
        vm::vec3d(32, 32, 16),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 6u);
    }

    SECTION("With another point set that previously crashed while adding points (1)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(224, 336, 0),
        vm::vec3d(272, 320, 0),
        vm::vec3d(-96, 352, 128),
        vm::vec3d(192, 192, 128),
        vm::vec3d(256, 256, 128),
        vm::vec3d(320, 480, 128),
        vm::vec3d(320, 256, 128),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 6u);
    }

    SECTION("With another point set that previously crashed while adding points (2)")
    {
      const vm::vec3d p1(256, 39, 160);
      const vm::vec3d p4(256, 39, 64);
      const vm::vec3d p6(0, 32, 160);
      const vm::vec3d p9(0, 0, 0);
      const vm::vec3d p10(0, 32, 0);
      const vm::vec3d p13(0, 39, 64);
      const vm::vec3d p14(0, 39, 160);
      const vm::vec3d p15(0, 39, 0);

      const Polyhedron3d p({p1, p4, p6, p9, p10, p13, p14, p15});
      CHECK(p.polyhedron());
      CHECK(p.vertexCount() == 6u);
      CHECK(p.hasVertex(p1));
      CHECK(p.hasVertex(p4));
      CHECK(p.hasVertex(p6));
      CHECK(p.hasVertex(p9));
      CHECK(p.hasVertex(p14));
      CHECK(p.hasVertex(p15));
      CHECK(p.edgeCount() == 10u);
      CHECK(p.hasEdge(p1, p4));
      CHECK(p.hasEdge(p1, p6));
      CHECK(p.hasEdge(p1, p9));
      CHECK(p.hasEdge(p1, p14));
      CHECK(p.hasEdge(p4, p9));
      CHECK(p.hasEdge(p4, p15));
      CHECK(p.hasEdge(p6, p9));
      CHECK(p.hasEdge(p6, p14));
      CHECK(p.hasEdge(p9, p15));
      CHECK(p.hasEdge(p14, p15));
      CHECK(p.faceCount() == 6u);
      CHECK(p.hasFace({p1, p14, p6}));
      CHECK(p.hasFace({p1, p4, p15, p14}));
      CHECK(p.hasFace({p1, p6, p9}));
      CHECK(p.hasFace({p1, p9, p4}));
      CHECK(p.hasFace({p4, p9, p15}));
      CHECK(p.hasFace({p6, p14, p15, p9}));
    }

    SECTION("With another point set that previously crashed while adding points (3)")
    {
      const auto vertices = std::vector<vm::vec3d>({
        vm::vec3d(256, 39, 160),
        vm::vec3d(256, 0, 160),
        vm::vec3d(256, 0, 64),
        vm::vec3d(256, 39, 64),
        vm::vec3d(0, 0, 160),
        vm::vec3d(0, 32, 160),
        vm::vec3d(0, 0, 64),
        vm::vec3d(0, 32, 64),
        vm::vec3d(0, 0, 0),
        vm::vec3d(0, 32, 0),
        vm::vec3d(256, 32, 0),
        vm::vec3d(256, 0, 0),
        vm::vec3d(0, 39, 64),
        vm::vec3d(0, 39, 160),
        vm::vec3d(0, 39, 0),
      });

      const Polyhedron3d p(vertices);
      CHECK(p.vertexCount() == 9u);
    }

    SECTION("With a redundant point at the center of a square")
    {
      //
      // p2 .  |  . p3
      //       |
      //    -------
      //       |
      // p1 .  |  . p4
      //
      const vm::vec3d p1(-1, -1, 0);
      const vm::vec3d p2(-1, +1, 0);
      const vm::vec3d p3(+1, +1, 0);
      const vm::vec3d p4(+1, -1, 0);
      const vm::vec3d p5(0, 0, 0);

      const Polyhedron3d p({p1, p2, p3, p4, p5});
      CHECK(p.hasFace({p1, p2, p3, p4}));
    }

    SECTION("With a point that is not added because all incident faces are coplanar")
    {
      auto p = Polyhedron3d{
        vm::vec3d{-64.0, 64.0, -16.0},
        vm::vec3d{64.0, 64.0, -16.0},
        vm::vec3d{22288.0, 18208.0, 16.0},
        vm::vec3d{
          22288.0,
          18336.0,
          16.0}, // does not get added due to all incident faces being coplanar
        vm::vec3d{22416.0, 18336.0, 16.0},
      };

      CHECK(p.hasAllVertices(
        {
          vm::vec3d{-64.0, 64.0, -16.0},
          vm::vec3d{64.0, 64.0, -16.0},
          vm::vec3d{22288.0, 18208.0, 16.0},
          vm::vec3d{22416.0, 18336.0, 16.0},
        },
        0.0));
    }
  }

  SECTION("clip")
  {
    SECTION("With a horizontal plane through the center of a cube")
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

    SECTION("With a horizontal plane at the top face of a cube")
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

    SECTION("With a horizontal plane above a cube")
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

    SECTION("With a horizontal plane at the bottom face of a cube")
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

    SECTION("With a slanted plane through a cube")
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

    SECTION("With a plane that cuts a cube diagonally")
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
        {{p1, p2, p4, p3},
         {p1, p5, p6, p2},
         {p3, p4, p6, p5},
         {p1, p3, p5},
         {p2, p6, p4}}));
    }

    SECTION("With a vertical slanted plane through a cube")
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

    SECTION("With a point set that previously triggered a crash")
    {
      std::vector<vm::vec3d> polyVertices;
      vm::parse_all<double, 3>(
        R"(
        (42.343111906757798 -24.90770936530231 48)
        (-5.6569680341747599 2.8051472462014218 -48)
        (-5.6567586128027614 -49.450466294904317 -48)
        (19.543884272280891 -64 2.4012022379983975)
        (64 -37.411190147253905 48)
        (64 -37.411184396581227 46.058241521600749)
        (16.970735645328752 -10.25882837570019 -48)
        (-15.996232760046849 -43.48119425295382 -48)
        (19.543373293787141 -64 32.936432269212482)
        (8.4017750903182601 -31.43996828352385 48)
        (-39.598145767921849 -3.7271836202911599 -48)
        (-28.284087977216849 -36.386647152659414 -48)
        (19.543509018008759 -64 47.655300195644266)
        (19.681387204653735 -64 48)
        (11.313359105885354 -46.184610213813635 -48)
        (42.170501479615339 -64 13.71441369506833)
        (64 -64 46.458506734897242)
        (64 -64 48)
        (64 -40.963243586214006 42.982066058285824)
        (64 -50.475344214694601 34.745773336493968)
        (22.627205203363062 -26.588725604065875 -48)
        (19.915358366079595 -18.759196710165369 -48)
        (16.82318198217952 -36.641571668509357 -48)
        (30.54114372047146 -27.178907257955132 48)
        (-13.006693391918915 1.3907491999939996 -48)
      )",
        std::back_inserter(polyVertices));

      Polyhedron3d poly(polyVertices);
      const vm::plane3d plane(
        -19.170582845718307,
        vm::vec3d(0.88388309419256438, 0.30618844562885328, -0.35355241699635576));

      CHECK_NOTHROW(poly.clip(plane));
    }

    SECTION("With a sequence of clips that previously produced an invalid seam")
    {
      // see https://github.com/TrenchBroom/TrenchBroom/issues/1801
      // see BrushTest::invalidBrush1801

      Polyhedron3d poly{
        // create a huge cube
        8192.0 * vm::vec3d(-1.0, -1.0, -1.0),
        8192.0 * vm::vec3d(-1.0, -1.0, +1.0),
        8192.0 * vm::vec3d(-1.0, +1.0, -1.0),
        8192.0 * vm::vec3d(-1.0, +1.0, +1.0),
        8192.0 * vm::vec3d(+1.0, -1.0, -1.0),
        8192.0 * vm::vec3d(+1.0, -1.0, +1.0),
        8192.0 * vm::vec3d(+1.0, +1.0, -1.0),
        8192.0 * vm::vec3d(+1.0, +1.0, +1.0),
      };

      poly.clip(*vm::from_points(
        vm::vec3d(-459.0, 1579.0, -115.0),
        vm::vec3d(-483.0, 1371.0, 131.0),
        vm::vec3d(-184.0, 1428.0, 237.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-184.0, 1428.0, 237.0),
        vm::vec3d(-184.0, 1513.0, 396.0),
        vm::vec3d(-184.0, 1777.0, 254.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-484.0, 1513.0, 395.0),
        vm::vec3d(-483.0, 1371.0, 131.0),
        vm::vec3d(-483.0, 1777.0, 253.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-483.0, 1371.0, 131.0),
        vm::vec3d(-459.0, 1579.0, -115.0),
        vm::vec3d(-483.0, 1777.0, 253.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-184.0, 1513.0, 396.0),
        vm::vec3d(-484.0, 1513.0, 395.0),
        vm::vec3d(-184.0, 1777.0, 254.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-184.0, 1777.0, 254.0),
        vm::vec3d(-483.0, 1777.0, 253.0),
        vm::vec3d(-183.0, 1692.0, 95.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-483.0, 1777.0, 253.0),
        vm::vec3d(-459.0, 1579.0, -115.0),
        vm::vec3d(-183.0, 1692.0, 95.0))); //  Assertion failure here!
      poly.clip(*vm::from_points(
        vm::vec3d(-483.0, 1371.0, 131.0),
        vm::vec3d(-484.0, 1513.0, 395.0),
        vm::vec3d(-184.0, 1513.0, 396.0)));
      poly.clip(*vm::from_points(
        vm::vec3d(-483.0, 1371.0, 131.0),
        vm::vec3d(-184.0, 1513.0, 396.0),
        vm::vec3d(-184.0, 1428.0, 237.0)));
    }
  }

  SECTION("subtract")
  {
    SECTION("With a smaller cuboid centered inside a larger one")
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
        UnorderedEquals(std::vector<Polyhedron3d>{
          Polyhedron3d{p1, p3, p2, p4, p9, p12, p11, p10},
          Polyhedron3d{p6, p8, p13, p14, p16, p15, p7, p5},
          Polyhedron3d{p14, p13, p10, p9, p18, p22, p21, p17},
          Polyhedron3d{p15, p16, p19, p23, p24, p20, p12, p11},
          Polyhedron3d{p20, p24, p22, p18, p26, p28, p32, p30},
          Polyhedron3d{p17, p21, p19, p23, p25, p29, p31, p27},
        }));
    }

    SECTION("With a disjoint cuboid")
    {
      const auto minuend = Polyhedron3d{vm::bbox3d{64.0}};
      const auto subtrahend = Polyhedron3d{vm::bbox3d{{96, 96, 96}, {128, 128, 128}}};

      CHECK(minuend.subtract(subtrahend) == std::vector<Polyhedron3d>{minuend});
    }

    SECTION("With a minuend entirely contained in the subtrahend")
    {
      const auto minuend = Polyhedron3d{vm::bbox3d{32.0}};
      const auto subtrahend = Polyhedron3d{vm::bbox3d{64.0}};

      CHECK(minuend.subtract(subtrahend) == std::vector<Polyhedron3d>{});
    }

    SECTION("With an identical cuboid")
    {
      const auto minuend = Polyhedron3d{vm::bbox3d{64.0}};
      const auto subtrahend = Polyhedron3d{vm::bbox3d{64.0}};

      CHECK(minuend.subtract(subtrahend) == std::vector<Polyhedron3d>{});
    }

    SECTION("With a cuboid protruding all the way through a cuboid")
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
        UnorderedEquals(std::vector<Polyhedron3d>{
          {p1, p2, p3, p4, p9, p10, p11, p12},
          {p5, p6, p7, p8, p13, p14, p15, p16},
          {p9, p10, p13, p14, p17, p18, p21, p22},
          {p11, p12, p15, p16, p19, p20, p23, p24},
        }));
    }

    SECTION("With a cuboid protruding from one side of a cuboid (1)")
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

    SECTION("With a cuboid protruding from one side of a cuboid (2)")
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

    SECTION("With a cuboid that has its corners cut off")
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

      const auto minuend =
        Polyhedron3d{p2, p3, p5, p6, p7, p8, p9, p10, p11, p12, p13, p16};
      const auto subtrahend = Polyhedron3d{vm::bbox3d{{-16, -8, 0}, {16, 8, 32}}};

      auto result = minuend.subtract(subtrahend);

      CHECK_THAT(
        minuend.subtract(subtrahend),
        UnorderedEquals(std::vector<Polyhedron3d>{
          {p1, p2, p3, p4, p5, p6, p7, p8},
          {p9, p10, p11, p12, p13, p14, p15, p16},
          {p17, p18, p19, p20, p3, p2, p13, p16},
        }));
    }

    SECTION("With a rhombus")
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

    SECTION("With a subtrahend that previously produced missing fragments")
    {
      const std::vector<vm::vec3d> minuendVertices{
        vm::vec3d(-1056, 864, -192),
        vm::vec3d(-1024, 896, -192),
        vm::vec3d(-1024, 1073, -192),
        vm::vec3d(-1056, 1080, -192),
        vm::vec3d(-1024, 1073, -416),
        vm::vec3d(-1024, 896, -416),
        vm::vec3d(-1056, 864, -416),
        vm::vec3d(-1056, 1080, -416)};

      const std::vector<vm::vec3d> subtrahendVertices{
        vm::vec3d(-1088, 960, -288),
        vm::vec3d(-1008, 960, -288),
        vm::vec3d(-1008, 1024, -288),
        vm::vec3d(-1088, 1024, -288),
        vm::vec3d(-1008, 1024, -400),
        vm::vec3d(-1008, 960, -400),
        vm::vec3d(-1088, 960, -400),
        vm::vec3d(-1088, 1024, -400)};

      const Polyhedron3d minuend(minuendVertices);
      const Polyhedron3d subtrahend(subtrahendVertices);

      auto result = minuend.subtract(subtrahend);
      CHECK(result.size() == 4u);
    }

    SECTION("With a tetrahedron that previously produced overlapping fragments")
    {
      // see https://github.com/TrenchBroom/TrenchBroom/pull/1764#issuecomment-296342133
      // merge creates overlapping fragments

      std::vector<vm::vec3d> minuendVertices, subtrahendVertices;
      vm::parse_all<double, 3>(
        R"((-32 -32 32) (32 -32 32) (32 32 32) (-32 32 32) (32 32 -32) (32 -32 -32) (-32 -32 -32) (-32 32 -32))",
        std::back_inserter(minuendVertices));
      vm::parse_all<double, 3>(
        "(-0 -16 -32) (-0 16 -32) (32 16 -32) (16 16 -0)",
        std::back_inserter(subtrahendVertices));

      const Polyhedron3d minuend(minuendVertices);
      const Polyhedron3d subtrahend(subtrahendVertices);

      auto result = minuend.subtract(subtrahend);
      CHECK(result.size() == 3u);
    }
  }

  SECTION("intersects")
  {
    SECTION("With an empty polyhedron")
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

    SECTION("With two points")
    {
      const auto point = Polyhedron3d{{0, 0, 0}};

      CHECK(mutuallyIntersects(point, point));
      CHECK(mutuallyNotIntersects(point, Polyhedron3d{{0, 0, 1}}));
    }

    SECTION("With a point and an edge")
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

    SECTION("With a point and a polygon")
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
        Polyhedron3d{
          {-1, -1, 1}, {1, -1, 1}, {0, 1, 1}})); // point / triangle above point
    }

    SECTION("With a point and a polyhedron")
    {
      const auto pointPos = vm::vec3d{0, 0, 0};
      const auto point = Polyhedron3d{pointPos};

      CHECK(mutuallyIntersects(
        point,
        Polyhedron3d{pointPos, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}})); // point / tetrahedron
                                                                   // with point as vertex
      CHECK(mutuallyIntersects(
        point,
        Polyhedron3d{
          {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}})); // point / tetrahedron
                                                          // with point on edge
      CHECK(mutuallyIntersects(
        point,
        Polyhedron3d{
          {-1, -1, 0}, {1, -1, 0}, {0, 1, 0}, {0, 0, 1}})); // point / tetrahedron
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
          {-1, -1, 1},
          {1, -1, 1},
          {0, 1, 1},
          {0, 0, 2}})); // point / tetrahedron above point
    }

    SECTION("With two edges")
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

    SECTION("With an edge and a polygon on the same plane")
    {
      const auto point1 = vm::vec3d{-1, 0, 0};
      const auto point2 = vm::vec3d{+1, 0, 0};
      const auto edge = Polyhedron3d{point1, point2};

      CHECK(mutuallyIntersects(
        edge,
        Polyhedron3d{{1, 0, 0}, {1, -1, 0}, {2, -1, 0}, {2, 0, 0}})); // one shared point
      CHECK(mutuallyIntersects(
        edge,
        Polyhedron3d{
          {-1, 0, 0}, {0, -1, 0}, {2, 0, 0}, {0, +1, 0}})); // two shared points
      CHECK(mutuallyIntersects(
        edge, Polyhedron3d{{-1, 0, 0}, {1, 0, 0}, {1, 1, 0}, {-1, 1, 0}})); // shared edge
      CHECK(mutuallyIntersects(
        edge,
        Polyhedron3d{
          {0, 1, 0}, {0, -1, 0}, {2, -1, 0}, {2, 1, 0}})); // polygon contains one point
      CHECK(mutuallyIntersects(
        edge,
        Polyhedron3d{
          {-2, 1, 0},
          {-2, -1, 0},
          {2, -1, 0},
          {2, 1, 0}})); // polygon contains both points
      CHECK(mutuallyIntersects(
        edge,
        Polyhedron3d{
          {-0.5, 1.0, 0.0},
          {-0.5, -1.0, 0.0},
          {0.5, -1.0, 0.0},
          {0.5, 1.0, 0.0}})); // edge intersects polygon completely

      CHECK(mutuallyNotIntersects(
        edge,
        Polyhedron3d{
          {+2, 1, 0}, {+2, -1, 0}, {+3, -1, 0}, {+3, 1, 0}})); // no intersection
    }

    SECTION("With an edge and a polygon on different planes")
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

    SECTION("With an edge and a polyhedron")
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

    SECTION("With two polygons on the same plane")
    {
      const Polyhedron3d square{{-1, -1, 0}, {+1, -1, 0}, {+1, +1, 0}, {-1, +1, 0}};

      // shared vertex:
      CHECK(
        mutuallyIntersects(Polyhedron3d{{+1, +1, 0}, {+2, +1, 0}, {+1, +2, 0}}, square));

      // shared edge
      CHECK(
        mutuallyIntersects(Polyhedron3d{{-1, +1, 0}, {+1, +1, 0}, {0, +2, 0}}, square));

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
        Polyhedron3d{{-2, -0.5, 0}, {+2, -0.5, 0}, {+2, +0.5, 0}, {-2, +0.5, 0}},
        square));

      // no intersection
      CHECK(mutuallyNotIntersects(
        Polyhedron3d{{+2, +2, 0}, {+3, +2, 0}, {+3, +3, 0}, {+3, +3, 0}}, square));
    }

    SECTION("With two polygons on different planes")
    {
      const Polyhedron3d square{{-1, -1, 0}, {+1, -1, 0}, {+1, +1, 0}, {-1, +1, 0}};

      // shared vertex
      CHECK(
        mutuallyIntersects(Polyhedron3d{{-1, -1, 0}, {-2, -1, 0}, {-2, -1, 1}}, square));

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

    SECTION("With a polygon on the same plane as a polyhedron face")
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
      CHECK(
        mutuallyIntersects(Polyhedron3d{{+1, +1, +1}, {-1, +1, +1}, {+1, +2, +1}}, cube));

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

    SECTION("With a polygon and a polyhedron in an arbitrary orientation")
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
      CHECK(
        mutuallyIntersects(Polyhedron3d{{+1, +1, +1}, {+2, +1, +2}, {+2, +2, +2}}, cube));

      // polygon vertex on polyhedron edge
      CHECK(
        mutuallyIntersects(Polyhedron3d{{+0, +1, +1}, {+2, +1, +2}, {+2, +2, +2}}, cube));

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
  }

  SECTION("pickFace")
  {
    const auto p1 = vm::vec3d{-64, -64, -64};
    const auto p2 = vm::vec3d{-64, -64, +64};
    const auto p3 = vm::vec3d{-64, +64, -64};
    const auto p4 = vm::vec3d{-64, +64, +64};
    const auto p5 = vm::vec3d{+64, -64, -64};
    const auto p6 = vm::vec3d{+64, -64, +64};
    const auto p7 = vm::vec3d{+64, +64, -64};
    const auto p8 = vm::vec3d{+64, +64, +64};

    const auto cube = Polyhedron3d{p1, p2, p3, p4, p5, p6, p7, p8};

    SECTION("With a ray that hits the top face")
    {
      const auto ray = vm::ray3d{{0, 0, 200}, {0, 0, -1}};

      const auto hit = cube.pickFace(ray);
      REQUIRE(hit.has_value());
      CHECK(hit->distance == vm::approx{136.0});
      CHECK(hit->face.plane().normal == vm::vec3d{0, 0, 1});
    }

    SECTION("With a ray that misses the cube")
    {
      const auto ray = vm::ray3d{{200, 200, 200}, {0, 0, -1}};

      CHECK(cube.pickFace(ray) == std::nullopt);
    }

    SECTION("With a ray that starts inside the cube, facing away from every face")
    {
      // pickFace only considers the front side of each face, so a ray cast from the
      // interior towards a face's back side does not hit it
      const auto ray = vm::ray3d{{0, 0, 0}, {0, 0, 1}};

      CHECK(cube.pickFace(ray) == std::nullopt);
    }
  }
}

} // namespace tb::mdl
