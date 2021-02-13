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

#include "FloatType.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron_BrushGeometryPayload.h"
#include "Model/Polyhedron_DefaultPayload.h"
#include "Model/Polyhedron_Instantiation.h"

#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <iterator>
#include <tuple>
#include <set>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {

        using Polyhedron3d = Polyhedron<double, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        using PVertex = Polyhedron3d::Vertex;
        using VertexList = Polyhedron3d::VertexList;
        using PEdge = Polyhedron3d::Edge;
        using PHalfEdge = Polyhedron3d::HalfEdge;
        using PFace = Polyhedron3d::Face;

        using EdgeInfo = std::pair<vm::vec3d, vm::vec3d>;
        using EdgeInfoList = std::vector<EdgeInfo>;

        static bool hasVertices(const Polyhedron3d& p, const std::vector<vm::vec3d>& points, const double epsilon = 0.0) {
            if (p.vertexCount() != points.size())
                return false;

            for (size_t i = 0; i < points.size(); ++i) {
                if (!p.hasVertex(points[i], epsilon))
                    return false;
            }
            return true;
        }

        static bool hasEdges(const Polyhedron3d& p, const EdgeInfoList& edgeInfos, const double epsilon = 0.0) {
            if (p.edgeCount() != edgeInfos.size())
                return false;

            for (size_t i = 0; i < edgeInfos.size(); ++i) {
                if (!p.hasEdge(edgeInfos[i].first, edgeInfos[i].second, epsilon))
                    return false;
            }
            return true;
        }

        static bool mutuallyIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs) {
            return lhs.intersects(rhs) && rhs.intersects(lhs);
        }

        static bool mutuallyNotIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs) {
            return !lhs.intersects(rhs) && !rhs.intersects(lhs);
        }

        TEST_CASE("PolyhedronTest.initWith4Points", "[PolyhedronTest]") {
            const vm::vec3d p1( 0.0, 0.0, 8.0);
            const vm::vec3d p2( 8.0, 0.0, 0.0);
            const vm::vec3d p3(-8.0, 0.0, 0.0);
            const vm::vec3d p4( 0.0, 8.0, 0.0);

            const Polyhedron3d p({ p1, p2, p3, p4 });
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

            CHECK(p.hasFace({ p2, p3, p4 }));
            CHECK(p.hasFace({ p1, p3, p2 }));
            CHECK(p.hasFace({ p1, p2, p4 }));
            CHECK(p.hasFace({ p1, p4, p3 }));
        }

        TEST_CASE("PolyhedronTest.copy", "[PolyhedronTest]") {
            const vm::vec3d p1( 0.0, 0.0, 8.0);
            const vm::vec3d p2( 8.0, 0.0, 0.0);
            const vm::vec3d p3(-8.0, 0.0, 0.0);
            const vm::vec3d p4( 0.0, 8.0, 0.0);

            CHECK(Polyhedron3d()                 == (Polyhedron3d() = Polyhedron3d()));
            CHECK(Polyhedron3d({p1})             == (Polyhedron3d() = Polyhedron3d({p1})));
            CHECK(Polyhedron3d({p1, p2})         == (Polyhedron3d() = Polyhedron3d({p1, p2})));
            CHECK(Polyhedron3d({p1, p2, p3})     == (Polyhedron3d() = Polyhedron3d({p1, p2, p3})));
            CHECK(Polyhedron3d({p1, p2, p3, p4}) == (Polyhedron3d() = Polyhedron3d({p1, p2, p3, p4})));
        }

        TEST_CASE("PolyhedronTest.swap", "[PolyhedronTest]") {
            const vm::vec3d p1( 0.0, 0.0, 8.0);
            const vm::vec3d p2( 8.0, 0.0, 0.0);
            const vm::vec3d p3(-8.0, 0.0, 0.0);
            const vm::vec3d p4( 0.0, 8.0, 0.0);

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

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
                vm::vec3d(-64.0,    -45.5049, -34.4752),
                vm::vec3d(-64.0,    -43.6929, -48.0),
                vm::vec3d(-64.0,     20.753,  -34.4752),
                vm::vec3d(-64.0,     64.0,    -48.0),
                vm::vec3d(-63.7297,  22.6264, -48.0),
                vm::vec3d(-57.9411,  22.6274, -37.9733),
                vm::vec3d(-44.6031, -39.1918, -48.0),
                vm::vec3d(-43.5959, -39.1918, -46.2555),
            });

            const Polyhedron3d p(vertices);
            CHECK(p.vertexCount() == 7u);
        }

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints2", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
                vm::vec3d(-64.0,    48.7375, -34.4752),
                vm::vec3d(-64.0,    64.0,    -48.0),
                vm::vec3d(-64.0,    64.0,    -34.4752),
                vm::vec3d(-63.7297, 22.6264, -48.0),
                vm::vec3d(-57.9411, 22.6274, -37.9733),
                vm::vec3d(-40.5744, 28.0,    -48.0),
                vm::vec3d(-40.5744, 64.0,    -48.0),
            });

            const Polyhedron3d p(vertices);
            CHECK(p.vertexCount() == vertices.size());
            
            for (const auto& v : vertices) {
                CHECK(p.hasVertex(v));
            }
        }

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints3", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
                vm::vec3d(-64,      -64,      -48),
                vm::vec3d(-64,       22.5637, -48),
                vm::vec3d(-64,       64,      -48),
                vm::vec3d(-63.7297,  22.6264, -48),
                vm::vec3d(-57.9411,  22.6274, -37.9733),
                vm::vec3d(-44.6031, -39.1918, -48),
                vm::vec3d(-43.5959, -39.1918, -46.2555),
            });

            const Polyhedron3d p(vertices);
            CHECK(p.vertexCount() == 5u);
        }

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints4", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
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

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints5", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
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

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints6", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
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

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints7", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
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

        TEST_CASE("PolyhedronTest.convexHullWithFailingPoints8", "[PolyhedronTest]") {
            // Cause of https://github.com/TrenchBroom/TrenchBroom/issues/1469
            // See also BrushTest.subtractTruncatedCones

            const auto vertices = std::vector<vm::vec3>({
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

/*
TEST_CASE("PolyhedronTest.testImpossibleSplit", "[PolyhedronTest]") {
    const vm::vec3d p1( 0.0, 4.0, 8.0);
    const vm::vec3d p2( 8.0, 0.0, 0.0);
    const vm::vec3d p3(-8.0, 0.0, 0.0);
    const vm::vec3d p4( 0.0, 8.0, 0.0);
    const vm::vec3d p5( 0.0, 4.0, 4.0);

    Polyhedron3d p(p1, p2, p3, p4);
    Polyhedron3d::Seam seam = p.split(Polyhedron3d::SplitByVisibilityCriterion(p5));
    CHECK(seam.empty());
}

TEST_CASE("PolyhedronTest.testSimpleSplit", "[PolyhedronTest]") {
    const vm::vec3d p1( 0.0, 4.0, 8.0);
    const vm::vec3d p2( 8.0, 0.0, 0.0);
    const vm::vec3d p3(-8.0, 0.0, 0.0);
    const vm::vec3d p4( 0.0, 8.0, 0.0);
    const vm::vec3d p5( 0.0, 4.0, 12.0);

    Polyhedron3d p(p1, p2, p3, p4);
    Polyhedron3d::Seam seam = p.split(Polyhedron3d::SplitByVisibilityCriterion(p5));
    CHECK(seam.size() == 3u);

    CHECK_FALSE(p.closed());
    CHECK(p.vertexCount() == 3u);
    CHECK(p.edgeCount() == 3u);
    CHECK(p.faceCount() == 1u);

    CHECK(p.hasFace({ p2, p3, p4 }));
}

TEST_CASE("PolyhedronTest.testWeaveSimpleCap", "[PolyhedronTest]") {
    const vm::vec3d p1( 0.0, 4.0, 8.0);
    const vm::vec3d p2( 8.0, 0.0, 0.0);
    const vm::vec3d p3(-8.0, 0.0, 0.0);
    const vm::vec3d p4( 0.0, 8.0, 0.0);
    const vm::vec3d p5( 0.0, 4.0, 12.0);

    Polyhedron3d p(p1, p2, p3, p4);
    Polyhedron3d::Seam seam = p.split(Polyhedron3d::SplitByVisibilityCriterion(p5));

    p.weaveCap(seam, p5);
    CHECK(p.closed());
    CHECK(p.vertexCount() == 4u);
    CHECK(p.edgeCount() == 6u);
    CHECK(p.faceCount() == 4u);
}
*/
        TEST_CASE("PolyhedronTest.testSimpleConvexHull", "[PolyhedronTest]") {
            const vm::vec3d p1( 0.0, 4.0, 8.0);
            const vm::vec3d p2( 8.0, 0.0, 0.0);
            const vm::vec3d p3(-8.0, 0.0, 0.0);
            const vm::vec3d p4( 0.0, 8.0, 0.0);
            const vm::vec3d p5( 0.0, 4.0, 12.0);

            Polyhedron3d p({ p1, p2, p3, p4, p5 });
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

            CHECK(p.hasFace({ p2, p3, p4 }));
            CHECK(p.hasFace({ p5, p3, p2 }));
            CHECK(p.hasFace({ p5, p2, p4 }));
            CHECK(p.hasFace({ p5, p4, p3 }));
        }

        TEST_CASE("PolyhedronTest.testSimpleConvexHullWithCoplanarFaces", "[PolyhedronTest]") {
            const vm::vec3d p1( 0.0, 0.0, 8.0);
            const vm::vec3d p2( 8.0, 0.0, 0.0);
            const vm::vec3d p3(-8.0, 0.0, 0.0);
            const vm::vec3d p4( 0.0, 8.0, 0.0);
            const vm::vec3d p5( 0.0, 0.0, 12.0);

            Polyhedron3d p({ p1, p2, p3, p4, p5 });
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

            CHECK(p.hasFace({ p2, p3, p4 }));
            CHECK(p.hasFace({ p5, p3, p2 }));
            CHECK(p.hasFace({ p5, p2, p4 }));
            CHECK(p.hasFace({ p5, p4, p3 }));
        }

        TEST_CASE("PolyhedronTest.testSimpleConvexHullOfCube", "[PolyhedronTest]") {
            const vm::vec3d p1( -8.0, -8.0, -8.0);
            const vm::vec3d p2( -8.0, -8.0, +8.0);
            const vm::vec3d p3( -8.0, +8.0, -8.0);
            const vm::vec3d p4( -8.0, +8.0, +8.0);
            const vm::vec3d p5( +8.0, -8.0, -8.0);
            const vm::vec3d p6( +8.0, -8.0, +8.0);
            const vm::vec3d p7( +8.0, +8.0, -8.0);
            const vm::vec3d p8( +8.0, +8.0, +8.0);

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

            CHECK(p.hasFace({ p1, p5, p6, p2 }));
            CHECK(p.hasFace({ p3, p1, p2, p4 }));
            CHECK(p.hasFace({ p7, p3, p4, p8 }));
            CHECK(p.hasFace({ p5, p7, p8, p6 }));
            CHECK(p.hasFace({ p3, p7, p5, p1 }));
            CHECK(p.hasFace({ p2, p6, p8, p4 }));
        }

        TEST_CASE("PolyhedronTest.initEmpty", "[PolyhedronTest]") {
            Polyhedron3d p;
            CHECK(p.empty());
        }

        TEST_CASE("PolyhedronTest.initEmptyAndAddOnePoint", "[PolyhedronTest]") {
            const vm::vec3d p1( -8.0, -8.0, -8.0);

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


        TEST_CASE("PolyhedronTest.initEmptyAndAddTwoIdenticalPoints", "[PolyhedronTest]") {
            const vm::vec3d p1( -8.0, -8.0, -8.0);

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

        TEST_CASE("PolyhedronTest.initEmptyAndAddTwoPoints", "[PolyhedronTest]") {
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

        TEST_CASE("PolyhedronTest.initEmptyAndAddThreeColinearPoints", "[PolyhedronTest]") {
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

        TEST_CASE("PolyhedronTest.initEmptyAndAddThreePoints", "[PolyhedronTest]") {
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

        TEST_CASE("PolyhedronTest.initEmptyAndAddThreePointsAndOneInnerPoint", "[PolyhedronTest]") {
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

        TEST_CASE("PolyhedronTest.initEmptyAndAddFourCoplanarPoints", "[PolyhedronTest]") {
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

        TEST_CASE("PolyhedronTest.initEmptyAndAddFourPoints", "[PolyhedronTest]") {
            const vm::vec3d p1(0.0, 0.0, 0.0);
            const vm::vec3d p2(6.0, 0.0, 0.0);
            const vm::vec3d p3(3.0, 6.0, 0.0);
            const vm::vec3d p4(3.0, 3.0, 6.0);

            Polyhedron3d p({p1, p2, p3, p4});

            CHECK_FALSE(p.empty());
            CHECK_FALSE(p.point());
            CHECK_FALSE(p.edge());
            CHECK_FALSE(p.polygon());
            CHECK(p.polyhedron());

            std::vector<vm::vec3d> points;
            points.push_back(p1);
            points.push_back(p2);
            points.push_back(p3);
            points.push_back(p4);

            CHECK(hasVertices(p, points));
        }

        TEST_CASE("PolyhedronTest.testAddColinearPointToRectangleOnEdge", "[PolyhedronTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1659
            /*
             p4 p5 p3
             *--+--*
             |     |
             |     |
             *-----*
             p1    p2
             */

            const vm::vec3d p1(  0.0,   0.0, 0.0);
            const vm::vec3d p2(+32.0,   0.0, 0.0);
            const vm::vec3d p3(+32.0, +32.0, 0.0);
            const vm::vec3d p4(  0.0, +32.0, 0.0);
            const vm::vec3d p5(+16.0, +32.0, 0.0);

            Polyhedron3d p({p1, p2, p3, p4, p5});

            CHECK(p.hasVertex(p1));
            CHECK(p.hasVertex(p2));
            CHECK(p.hasVertex(p3));
            CHECK(p.hasVertex(p4));
            CHECK_FALSE(p.hasVertex(p5));
        }

        TEST_CASE("PolyhedronTest.testAddPointToRectangleMakingOneColinear", "[PolyhedronTest]") {
            /*
             p4    p3  p5
             *-----*   +
             |     |
             |     |
             *-----*
             p1    p2
             */

            const vm::vec3d p1(  0.0,   0.0, 0.0);
            const vm::vec3d p2(+32.0,   0.0, 0.0);
            const vm::vec3d p3(+32.0, +32.0, 0.0);
            const vm::vec3d p4(  0.0, +32.0, 0.0);
            const vm::vec3d p5(+40.0, +32.0, 0.0);

            Polyhedron3d p({p1, p2, p3, p4, p5});

            CHECK(p.hasVertex(p1));
            CHECK(p.hasVertex(p2));
            CHECK(p.hasVertex(p4));
            CHECK(p.hasVertex(p5));
            CHECK_FALSE(p.hasVertex(p3));
        }

        TEST_CASE("PolyhedronTest.testAddManyPointsCrash", "[PolyhedronTest]") {
            const vm::vec3d p1( 8, 10, 0);
            const vm::vec3d p2( 0, 24, 0);
            const vm::vec3d p3( 8, 10, 8);
            const vm::vec3d p4(10, 11, 8);
            const vm::vec3d p5(12, 24, 8);
            const vm::vec3d p6( 0,  6, 8);
            const vm::vec3d p7(10,  0, 8);

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
            CHECK(p.hasFace({ p1, p3, p2 }));

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
            CHECK(p.hasFace({ p1, p3, p2 }));
            CHECK(p.hasFace({ p1, p2, p4 }));
            CHECK(p.hasFace({ p1, p4, p3 }));
            CHECK(p.hasFace({ p3, p4, p2 }));

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
            CHECK(p.hasFace({ p1, p3, p2 }));
            // CHECK(p.hasFace({ p1, p2, p4 }));
            CHECK(p.hasFace({ p1, p4, p3 }));
            // CHECK(p.hasFace({ p3, p4, p2 }));
            CHECK(p.hasFace({ p5, p4, p1 }));
            CHECK(p.hasFace({ p5, p3, p4 }));
            CHECK(p.hasFace({ p5, p2, p3 }));
            CHECK(p.hasFace({ p5, p1, p2 }));

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
            CHECK(p.hasFace({ p5, p4, p1 }));
            // CHECK(p.hasFace({ p5, p3, p4 }));
            // CHECK(p.hasFace({ p5, p2, p3 }));
            CHECK(p.hasFace({ p5, p1, p2 }));
            CHECK(p.hasFace({ p6, p2, p1 }));
            CHECK(p.hasFace({ p6, p5, p2 }));
            CHECK(p.hasFace({ p6, p4, p5 }));
            CHECK(p.hasFace({ p6, p1, p4 }));

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
            CHECK(p.hasFace({ p5, p1, p2 }));
            CHECK(p.hasFace({ p6, p2, p1 }));
            CHECK(p.hasFace({ p6, p5, p2 }));
            // CHECK(p.hasFace({ p6, p4, p5 }));
            // CHECK(p.hasFace({ p6, p1, p4 }));
            CHECK(p.hasFace({ p7, p1, p5 }));
            CHECK(p.hasFace({ p7, p6, p1 }));
            CHECK(p.hasFace({ p7, p5, p6 }));
        }

        TEST_CASE("PolyhedronTest.testAdd8PointsCrash", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
                // a horizontal rectangle
                vm::vec3d( 0,  0,  0),
                vm::vec3d( 0, 32,  0),
                vm::vec3d(32, 32,  0),
                vm::vec3d(32,  0,  0),

                // a vertical rectangle
                vm::vec3d(32, 16, 16),
                vm::vec3d(32, 16, 32),
                vm::vec3d(32, 32, 32),
                vm::vec3d(32, 32, 16),
            });

            const Polyhedron3d p(vertices);
            CHECK(p.vertexCount() == 6u);
        }

        TEST_CASE("PolyhedronTest.crashWhileAddingPoints1", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
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

        TEST_CASE("PolyhedronTest.crashWhileAddingPoints2", "[PolyhedronTest]") {
            const vm::vec3d  p1(256, 39, 160);
            const vm::vec3d  p4(256, 39, 64);
            const vm::vec3d  p6(  0, 32, 160);
            const vm::vec3d  p9(  0,  0, 0);
            const vm::vec3d p10(  0, 32, 0);
            const vm::vec3d p13(  0, 39, 64);
            const vm::vec3d p14(  0, 39, 160);
            const vm::vec3d p15(  0, 39, 0);

            Polyhedron3d p({p1, p4, p6, p9, p10, p13, p14, p15});
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
            CHECK(p.hasFace({ p1, p14, p6 }));
            CHECK(p.hasFace({ p1, p4, p15, p14 }));
            CHECK(p.hasFace({ p1, p6, p9 }));
            CHECK(p.hasFace({ p1, p9, p4 }));
            CHECK(p.hasFace({ p4, p9, p15 }));
            CHECK(p.hasFace({ p6, p14, p15, p9 }));
        }

        TEST_CASE("PolyhedronTest.crashWhileAddingPoints3", "[PolyhedronTest]") {
            const auto vertices = std::vector<vm::vec3>({
                vm::vec3d(256, 39, 160),
                vm::vec3d(256, 0, 160),
                vm::vec3d(256,  0, 64),
                vm::vec3d(256, 39, 64),
                vm::vec3d(  0,  0, 160),
                vm::vec3d(  0, 32, 160),
                vm::vec3d(  0,  0, 64),
                vm::vec3d(  0, 32, 64),
                vm::vec3d(  0,  0, 0),
                vm::vec3d(  0, 32, 0),
                vm::vec3d(256, 32, 0),
                vm::vec3d(256,  0, 0),
                vm::vec3d(  0, 39, 64),
                vm::vec3d(  0, 39, 160),
                vm::vec3d(  0, 39, 0),
            });

            const Polyhedron3d p(vertices);
            CHECK(p.vertexCount() == 9u);
        }

        TEST_CASE("PolyhedronTest.crashWhileAddingPoints4", "[PolyhedronTest]") {
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
            const vm::vec3d p5( 0,  0, 0);

            Polyhedron3d p({p1, p2, p3, p4, p5});
            CHECK(p.hasFace({ p1, p2, p3, p4 }));
        }

        TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlane", "[PolyhedronTest]") {
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
            CHECK(p.hasFace({ p1,     p2 + d, p4 + d, p3 }));
            CHECK(p.hasFace({ p1,     p3,     p7,     p5 }));
            CHECK(p.hasFace({ p1,     p5,     p6 + d, p2 + d }));
            CHECK(p.hasFace({ p2 + d, p6 + d, p8 + d, p4 + d }));
            CHECK(p.hasFace({ p3,     p4 + d, p8 + d, p7 }));
            CHECK(p.hasFace({ p5,     p7,     p8 + d, p6 + d }));
        }

        TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAtTop", "[PolyhedronTest]") {
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
            CHECK(p.hasFace({ p1, p2, p4, p3 }));
            CHECK(p.hasFace({ p1, p3, p7, p5 }));
            CHECK(p.hasFace({ p1, p5, p6, p2 }));
            CHECK(p.hasFace({ p2, p6, p8, p4 }));
            CHECK(p.hasFace({ p3, p4, p8, p7 }));
            CHECK(p.hasFace({ p5, p7, p8, p6 }));
        }

        TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAboveTop", "[PolyhedronTest]") {
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
            CHECK(p.hasFace({ p1, p2, p4, p3 }));
            CHECK(p.hasFace({ p1, p3, p7, p5 }));
            CHECK(p.hasFace({ p1, p5, p6, p2 }));
            CHECK(p.hasFace({ p2, p6, p8, p4 }));
            CHECK(p.hasFace({ p3, p4, p8, p7 }));
            CHECK(p.hasFace({ p5, p7, p8, p6 }));
        }

        TEST_CASE("PolyhedronTest.clipCubeWithHorizontalPlaneAtBottom", "[PolyhedronTest]") {
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

        TEST_CASE("PolyhedronTest.clipCubeWithSlantedPlane", "[PolyhedronTest]") {
            Polyhedron3d p(vm::bbox3d(64.0));

            const vm::plane3d plane(vm::vec3d(64.0, 64.0, 0.0), normalize(vm::vec3d(1.0, 1.0, 1.0)));
            CHECK(p.clip(plane).success());

            const vm::vec3d  p1(-64.0, -64.0, -64.0);
            const vm::vec3d  p2(-64.0, -64.0, +64.0);
            const vm::vec3d  p3(-64.0, +64.0, -64.0);
            const vm::vec3d  p4(-64.0, +64.0, +64.0);
            const vm::vec3d  p5(+64.0, -64.0, -64.0);
            const vm::vec3d  p6(+64.0, -64.0, +64.0);
            const vm::vec3d  p7(+64.0, +64.0, -64.0);
            const vm::vec3d  p9(+64.0,   0.0, +64.0);
            const vm::vec3d p10(  0.0, +64.0, +64.0);
            const vm::vec3d p11(+64.0, +64.0,   0.0);

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
            CHECK(p.hasFace({  p1,  p3,  p7,  p5 }));
            CHECK(p.hasFace({  p1,  p5,  p6,  p2 }));
            CHECK(p.hasFace({  p1,  p2,  p4,  p3 }));
            CHECK(p.hasFace({  p2,  p6,  p9, p10,  p4 }));
            CHECK(p.hasFace({  p3,  p4, p10, p11,  p7 }, 0.0001));
            CHECK(p.hasFace({  p5,  p7, p11,  p9,  p6 }, 0.0001));
            CHECK(p.hasFace({ p9, p11, p10 }, 0.0001));
        }

        TEST_CASE("PolyhedronTest.clipCubeDiagonally", "[PolyhedronTest]") {
            Polyhedron3d p(vm::bbox3d(64.0));

            const vm::plane3d plane(vm::vec3d::zero(), normalize(vm::vec3d(1.0, 1.0, 0.0)));
            CHECK(p.clip(plane).success());

            const vm::vec3d  p1(-64.0, -64.0, -64.0);
            const vm::vec3d  p2(-64.0, -64.0, +64.0);
            const vm::vec3d  p3(-64.0, +64.0, -64.0);
            const vm::vec3d  p4(-64.0, +64.0, +64.0);
            const vm::vec3d  p5(+64.0, -64.0, -64.0);
            const vm::vec3d  p6(+64.0, -64.0, +64.0);

            CHECK(p.vertexCount() == 6u);
            CHECK(p.hasVertex( p1));
            CHECK(p.hasVertex( p2));
            CHECK(p.hasVertex( p3));
            CHECK(p.hasVertex( p4));
            CHECK(p.hasVertex( p5));
            CHECK(p.hasVertex( p6));

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
            CHECK(p.hasFace({ p1, p2, p4, p3 }));
            CHECK(p.hasFace({ p1, p5, p6, p2 }));
            CHECK(p.hasFace({ p3, p4, p6, p5 }));
            CHECK(p.hasFace({ p1, p3, p5 }));
            CHECK(p.hasFace({ p2, p6, p4 }));
        }

        TEST_CASE("PolyhedronTest.clipCubeWithVerticalSlantedPlane", "[PolyhedronTest]") {
            Polyhedron3d p(vm::bbox3d(64.0));

            const vm::plane3d plane(vm::vec3d(  0.0, -64.0, 0.0), normalize(vm::vec3d(2.0, 1.0, 0.0)));
            CHECK(p.clip(plane).success());

            const vm::vec3d  p1(-64.0, -64.0, -64.0);
            const vm::vec3d  p2(-64.0, -64.0, +64.0);
            const vm::vec3d  p3(-64.0, +64.0, -64.0);
            const vm::vec3d  p4(-64.0, +64.0, +64.0);
            const vm::vec3d  p5(  0.0, -64.0, -64.0);
            const vm::vec3d  p6(  0.0, -64.0, +64.0);

            CHECK(p.vertexCount() == 6u);
            CHECK(p.hasVertex( p1));
            CHECK(p.hasVertex( p2));
            CHECK(p.hasVertex( p3));
            CHECK(p.hasVertex( p4));
            CHECK(p.hasVertex( p5));
            CHECK(p.hasVertex( p6));

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
            CHECK(p.hasFace({ p1, p2, p4, p3 }));
            CHECK(p.hasFace({ p1, p5, p6, p2 }));
            CHECK(p.hasFace({ p3, p4, p6, p5 }));
            CHECK(p.hasFace({ p1, p3, p5 }));
            CHECK(p.hasFace({ p2, p6, p4 }));
        }

        TEST_CASE("PolyhedronTest.badClip", "[PolyhedronTest]") {
            std::vector<vm::vec3d> polyVertices;
            vm::parse_all<double, 3>("(42.343111906757798 -24.90770936530231 48) (-5.6569680341747599 2.8051472462014218 -48) (-5.6567586128027614 -49.450466294904317 -48) (19.543884272280891 -64 2.4012022379983975) (64 -37.411190147253905 48) (64 -37.411184396581227 46.058241521600749) (16.970735645328752 -10.25882837570019 -48) (-15.996232760046849 -43.48119425295382 -48) (19.543373293787141 -64 32.936432269212482) (8.4017750903182601 -31.43996828352385 48) (-39.598145767921849 -3.7271836202911599 -48) (-28.284087977216849 -36.386647152659414 -48) (19.543509018008759 -64 47.655300195644266) (19.681387204653735 -64 48) (11.313359105885354 -46.184610213813635 -48) (42.170501479615339 -64 13.71441369506833) (64 -64 46.458506734897242) (64 -64 48) (64 -40.963243586214006 42.982066058285824) (64 -50.475344214694601 34.745773336493968) (22.627205203363062 -26.588725604065875 -48) (19.915358366079595 -18.759196710165369 -48) (16.82318198217952 -36.641571668509357 -48) (30.54114372047146 -27.178907257955132 48) (-13.006693391918915 1.3907491999939996 -48)", std::back_inserter(polyVertices));

            Polyhedron3d poly(polyVertices);
            const vm::plane3d plane(-19.170582845718307, vm::vec3d(0.88388309419256438, 0.30618844562885328, -0.35355241699635576));

            CHECK_NOTHROW(poly.clip(plane));
        }

        TEST_CASE("PolyhedronTest.clipWithInvalidSeam", "[PolyhedronTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/1801
            // see BrushTest::invalidBrush1801

            Polyhedron3d poly { // create a huge cube
                8192.0 * vm::vec3d(-1.0, -1.0, -1.0),
                8192.0 * vm::vec3d(-1.0, -1.0, +1.0),
                8192.0 * vm::vec3d(-1.0, +1.0, -1.0),
                8192.0 * vm::vec3d(-1.0, +1.0, +1.0),
                8192.0 * vm::vec3d(+1.0, -1.0, -1.0),
                8192.0 * vm::vec3d(+1.0, -1.0, +1.0),
                8192.0 * vm::vec3d(+1.0, +1.0, -1.0),
                8192.0 * vm::vec3d(+1.0, +1.0, +1.0),
            };

            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-459.0, 1579.0, -115.0), vm::vec3d(-483.0, 1371.0, 131.0),  vm::vec3d(-184.0, 1428.0, 237.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-184.0, 1428.0, 237.0),  vm::vec3d(-184.0, 1513.0, 396.0),  vm::vec3d(-184.0, 1777.0, 254.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-484.0, 1513.0, 395.0),  vm::vec3d(-483.0, 1371.0, 131.0),  vm::vec3d(-483.0, 1777.0, 253.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-483.0, 1371.0, 131.0),  vm::vec3d(-459.0, 1579.0, -115.0), vm::vec3d(-483.0, 1777.0, 253.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-184.0, 1513.0, 396.0),  vm::vec3d(-484.0, 1513.0, 395.0),  vm::vec3d(-184.0, 1777.0, 254.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-184.0, 1777.0, 254.0),  vm::vec3d(-483.0, 1777.0, 253.0),  vm::vec3d(-183.0, 1692.0,  95.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-483.0, 1777.0, 253.0),  vm::vec3d(-459.0, 1579.0, -115.0), vm::vec3d(-183.0, 1692.0,  95.0)))); //  Assertion failure here!
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-483.0, 1371.0, 131.0),  vm::vec3d(-484.0, 1513.0, 395.0),  vm::vec3d(-184.0, 1513.0, 396.0))));
            poly.clip(std::get<1>(vm::from_points(vm::vec3d(-483.0, 1371.0, 131.0),  vm::vec3d(-184.0, 1513.0, 396.0),  vm::vec3d(-184.0, 1428.0, 237.0))));
        }

        bool findAndRemove(std::vector<Polyhedron3d>& result, const std::vector<vm::vec3d>& vertices);
        bool findAndRemove(std::vector<Polyhedron3d>& result, const std::vector<vm::vec3d>& vertices) {
            for (auto it = std::begin(result), end = std::end(result); it != end; ++it) {
                const Polyhedron3d& polyhedron = *it;
                if (polyhedron.hasAllVertices(vertices, vm::Cd::almost_zero())) {
                    result.erase(it);
                    return true;
                }
            }

            return false;
        }

        TEST_CASE("PolyhedronTest.subtractInnerCuboidFromCuboid", "[PolyhedronTest]") {
            const Polyhedron3d minuend(vm::bbox3d(32.0));
            const Polyhedron3d subtrahend(vm::bbox3d(16.0));

            auto result = minuend.subtract(subtrahend);

            std::vector<vm::vec3d> leftVertices, rightVertices, frontVertices, backVertices, topVertices, bottomVertices;

            vm::parse_all<double, 3>("(-32 -32 -32) (-32 32 -32) (-32 -32 32) (-32 32 32) (-16 -32 -32) (-16 32 -32) (-16 32 32) (-16 -32 32)", std::back_inserter(leftVertices));
            vm::parse_all<double, 3>("(32 -32 32) (32 32 32) (16 -32 -32) (16 -32 32) (16 32 32) (16 32 -32) (32 32 -32) (32 -32 -32)", std::back_inserter(rightVertices));

            vm::parse_all<double, 3>("(16 -32 32) (16 -32 -32) (-16 -32 32) (-16 -32 -32) (-16 -16 32) (16 -16 32) (16 -16 -32) (-16 -16 -32)", std::back_inserter(frontVertices));
            vm::parse_all<double, 3>("(16 32 -32) (16 32 32) (-16 16 -32) (16 16 -32) (16 16 32) (-16 16 32) (-16 32 32) (-16 32 -32)", std::back_inserter(backVertices));

            vm::parse_all<double, 3>("(-16 16 32) (16 16 32) (16 -16 32) (-16 -16 32) (-16 -16 16) (-16 16 16) (16 16 16) (16 -16 16)", std::back_inserter(topVertices));
            vm::parse_all<double, 3>("(-16 -16 -32) (16 -16 -32) (-16 16 -32) (16 16 -32) (-16 -16 -16) (16 -16 -16) (16 16 -16) (-16 16 -16)", std::back_inserter(bottomVertices));

            CHECK(findAndRemove(result, leftVertices));
            CHECK(findAndRemove(result, rightVertices));
            CHECK(findAndRemove(result, frontVertices));
            CHECK(findAndRemove(result, backVertices));
            CHECK(findAndRemove(result, topVertices));
            CHECK(findAndRemove(result, bottomVertices));

            CHECK(result.empty());
        }

        TEST_CASE("PolyhedronTest.subtractDisjointCuboidFromCuboid", "[PolyhedronTest]") {
            const Polyhedron3d minuend(vm::bbox3d(64.0));
            const Polyhedron3d subtrahend(vm::bbox3d(vm::vec3d(96.0, 96.0, 96.0), vm::vec3d(128.0, 128.0, 128.0)));

            auto result = minuend.subtract(subtrahend);
            CHECK(result.size() == 1u);

            const Polyhedron3d resultPolyhedron = result.front();
            CHECK(resultPolyhedron == minuend);
        }

        TEST_CASE("PolyhedronTest.subtractCuboidFromInnerCuboid", "[PolyhedronTest]") {
            const Polyhedron3d minuend(vm::bbox3d(32.0));
            const Polyhedron3d subtrahend(vm::bbox3d(64.0));

            auto result = minuend.subtract(subtrahend);
            CHECK(result.empty());
        }

        TEST_CASE("PolyhedronTest.subtractCuboidFromIdenticalCuboid", "[PolyhedronTest]") {
            const Polyhedron3d minuend(vm::bbox3d(64.0));
            const Polyhedron3d subtrahend(vm::bbox3d(64.0));

            auto result = minuend.subtract(subtrahend);
            CHECK(result.empty());
        }

        TEST_CASE("PolyhedronTest.subtractCuboidProtrudingThroughCuboid", "[PolyhedronTest]") {
            const Polyhedron3d    minuend(vm::bbox3d(vm::vec3d(-32.0, -32.0, -16.0), vm::vec3d(32.0, 32.0, 16.0)));
            const Polyhedron3d subtrahend(vm::bbox3d(vm::vec3d(-16.0, -16.0, -32.0), vm::vec3d(16.0, 16.0, 32.0)));

            auto result = minuend.subtract(subtrahend);
            CHECK(result.size() == 4u);

            const std::vector<vm::vec3d> leftVertices {
                vm::vec3d(-16, -32, -16),
                vm::vec3d(-16, 32, -16),
                vm::vec3d(-16, 32, 16),
                vm::vec3d(-16, -32, 16),
                vm::vec3d(-32, 32, 16),
                vm::vec3d(-32, -32, 16),
                vm::vec3d(-32, -32, -16),
                vm::vec3d(-32, 32, -16),
            };

            const std::vector<vm::vec3d> rightVertices {
                vm::vec3d(32, -32, 16),
                vm::vec3d(32, 32, 16),
                vm::vec3d(32, -32, -16),
                vm::vec3d(32, 32, -16),
                vm::vec3d(16, -32, -16),
                vm::vec3d(16, -32, 16),
                vm::vec3d(16, 32, 16),
                vm::vec3d(16, 32, -16)
            };

            const std::vector<vm::vec3d> frontVertices {
                vm::vec3d(-16, -32, -16),
                vm::vec3d(-16, -32, 16),
                vm::vec3d(16, -16, -16),
                vm::vec3d(-16, -16, -16),
                vm::vec3d(-16, -16, 16),
                vm::vec3d(16, -16, 16),
                vm::vec3d(16, -32, 16),
                vm::vec3d(16, -32, -16)
            };

            const std::vector<vm::vec3d> backVertices {
                vm::vec3d(-16, 32, 16),
                vm::vec3d(-16, 32, -16),
                vm::vec3d(16, 32, 16),
                vm::vec3d(16, 32, -16),
                vm::vec3d(16, 16, 16),
                vm::vec3d(-16, 16, 16),
                vm::vec3d(-16, 16, -16),
                vm::vec3d(16, 16, -16)
            };

            CHECK(findAndRemove(result, frontVertices));
            CHECK(findAndRemove(result, backVertices));
            CHECK(findAndRemove(result, leftVertices));
            CHECK(findAndRemove(result, rightVertices));

            CHECK(result.empty());
        }

        TEST_CASE("PolyhedronTest.subtractCuboidProtrudingFromCuboid", "[PolyhedronTest]") {
            /*
             ____________
             |          |
             |  ______  |
             |  |    |  |
             |__|    |__|
                |    |
                |____|
             */

            const Polyhedron3d    minuend(vm::bbox3d(vm::vec3d(-32.0, -16.0, -32.0), vm::vec3d(32.0, 16.0, 32.0)));
            const Polyhedron3d subtrahend(vm::bbox3d(vm::vec3d(-16.0, -32.0, -64.0), vm::vec3d(16.0, 32.0,  0.0)));

            auto result = minuend.subtract(subtrahend);
            CHECK(result.size() == 3u);
        }

        TEST_CASE("PolyhedronTest.subtractCuboidProtrudingFromCuboid2", "[PolyhedronTest]") {
            /*
             ____________
             |          |
             |  ______  |
             |  |    |  |
             |__|____|__|
             */

            const Polyhedron3d    minuend(vm::bbox3d(vm::vec3d(-64.0, -64.0, -16.0), vm::vec3d(64.0, 64.0, 16.0)));
            const Polyhedron3d subtrahend(vm::bbox3d(vm::vec3d(-32.0, -64.0, -32.0), vm::vec3d(32.0,  0.0, 32.0)));

            auto result = minuend.subtract(subtrahend);
            CHECK(result.size() == 3u);
        }

        TEST_CASE("PolyhedronTest.subtractCuboidFromCuboidWithCutCorners", "[PolyhedronTest]") {

            /*
               ____
              /    \
             / ____ \
             | |  | |
             | |  | |
             | |  | |
             |_|__|_|

             */

            const Polyhedron3d minuend {
                vm::vec3d(-32.0, -8.0,  0.0),
                vm::vec3d(+32.0, -8.0,  0.0),
                vm::vec3d(+32.0, -8.0, 32.0),
                vm::vec3d(+16.0, -8.0, 48.0),
                vm::vec3d(-16.0, -8.0, 48.0),
                vm::vec3d(-32.0, -8.0, 32.0),
                vm::vec3d(-32.0, +8.0,  0.0),
                vm::vec3d(+32.0, +8.0,  0.0),
                vm::vec3d(+32.0, +8.0, 32.0),
                vm::vec3d(+16.0, +8.0, 48.0),
                vm::vec3d(-16.0, +8.0, 48.0),
                vm::vec3d(-32.0, +8.0, 32.0)
            };

            const Polyhedron3d subtrahend(vm::bbox3d(vm::vec3d(-16.0, -8.0, 0.0), vm::vec3d(16.0, 8.0, 32.0)));

            auto result = minuend.subtract(subtrahend);

            std::vector<vm::vec3d> left, right, top;
            vm::parse_all<double, 3>("(-16 8 -0) (-16 8 48) (-16 -8 48) (-16 -8 -0) (-32 -8 -0) (-32 -8 32) (-32 8 -0) (-32 8 32)", std::back_inserter(left));
            vm::parse_all<double, 3>("(32 -8 32) (32 8 32) (32 8 -0) (32 -8 -0) (16 8 48) (16 8 -0) (16 -8 -0) (16 -8 48)", std::back_inserter(right));
            vm::parse_all<double, 3>("(16 8 32) (16 -8 32) (-16 -8 32) (-16 8 32) (-16 -8 48) (-16 8 48) (16 8 48) (16 -8 48)", std::back_inserter(top));

            CHECK(findAndRemove(result, left));
            CHECK(findAndRemove(result, right));
            CHECK(findAndRemove(result, top));

            CHECK(result.empty());
        }

        TEST_CASE("PolyhedronTest.subtractRhombusFromCuboid", "[PolyhedronTest]") {

            /*
             ______
             |    |
             | /\ |
             | \/ |
             |____|

             */

            std::vector<vm::vec3d> subtrahendVertices;
            vm::parse_all<double, 3>("(-32.0 0.0 +96.0) (0.0 -32.0 +96.0) (+32.0 0.0 +96.0) (0.0 +32.0 +96.0) (-32.0 0.0 -96.0) (0.0 -32.0 -96.0) (+32.0 0.0 -96.0) (0.0 +32.0 -96.0)", std::back_inserter(subtrahendVertices));

            const Polyhedron3d minuend(vm::bbox3d(64.0));
            const Polyhedron3d subtrahend(subtrahendVertices);

            auto result = minuend.subtract(subtrahend);
            
            std::vector<vm::vec3d> f1, f2, f3, f4;
            vm::parse_all<double, 3>("(64 64 64) (-32 64 -64) (64 -32 -64) (64 -32 64) (-32 64 64) (64 64 -64)", std::back_inserter(f1));
            vm::parse_all<double, 3>("(-64 32 64) (-64 32 -64) (-32 -0 64) (-32 -0 -64) (-0 32 -64) (-0 32 64) (-64 64 64) (-32 64 -64) (-32 64 64) (-64 64 -64)", std::back_inserter(f2));
            vm::parse_all<double, 3>("(64 -32 64) (64 -32 -64) (64 -64 64) (64 -64 -64) (-0 -32 64) (32 -0 64) (32 -0 -64) (-0 -32 -64) (32 -64 -64) (32 -64 64)", std::back_inserter(f3));
            vm::parse_all<double, 3>("(-64 -64 64) (-64 -64 -64) (-64 32 -64) (-64 32 64) (32 -64 64) (32 -64 -64)", std::back_inserter(f4));
            CHECK(findAndRemove(result, f1));
            CHECK(findAndRemove(result, f2));
            CHECK(findAndRemove(result, f3));
            CHECK(findAndRemove(result, f4));

            CHECK(result.size() == 0u);
        }

        TEST_CASE("PolyhedronTest.subtractFailWithMissingFragments", "[PolyhedronTest]") {
            const std::vector<vm::vec3d> minuendVertices {
                vm::vec3d(-1056, 864, -192),
                vm::vec3d(-1024, 896, -192),
                vm::vec3d(-1024, 1073, -192),
                vm::vec3d(-1056, 1080, -192),
                vm::vec3d(-1024, 1073, -416),
                vm::vec3d(-1024, 896, -416),
                vm::vec3d(-1056, 864, -416),
                vm::vec3d(-1056, 1080, -416)
            };

            const std::vector<vm::vec3d> subtrahendVertices {
                vm::vec3d(-1088, 960, -288),
                vm::vec3d(-1008, 960, -288),
                vm::vec3d(-1008, 1024, -288),
                vm::vec3d(-1088, 1024, -288),
                vm::vec3d(-1008, 1024, -400),
                vm::vec3d(-1008, 960, -400),
                vm::vec3d(-1088, 960, -400),
                vm::vec3d(-1088, 1024, -400)
            };

            const Polyhedron3d minuend(minuendVertices);
            const Polyhedron3d subtrahend(subtrahendVertices);

            auto result = minuend.subtract(subtrahend);
            CHECK(result.size() == 4u);
        }

        TEST_CASE("PolyhedronTest.subtractTetrahedronFromCubeWithOverlappingFragments", "[PolyhedronTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/pull/1764#issuecomment-296342133
            // merge creates overlapping fragments

            std::vector<vm::vec3d> minuendVertices, subtrahendVertices;
            vm::parse_all<double, 3>("(-32 -32 32) (32 -32 32) (32 32 32) (-32 32 32) (32 32 -32) (32 -32 -32) (-32 -32 -32) (-32 32 -32)", std::back_inserter(minuendVertices));
            vm::parse_all<double, 3>("(-0 -16 -32) (-0 16 -32) (32 16 -32) (16 16 -0)", std::back_inserter(subtrahendVertices));


            const Polyhedron3d minuend(minuendVertices);
            const Polyhedron3d subtrahend(subtrahendVertices);

            auto result = minuend.subtract(subtrahend);
            CHECK(result.size() == 3u);
        }

        TEST_CASE("PolyhedronTest.intersection_empty_polyhedron", "[PolyhedronTest]") {
            const Polyhedron3d empty;
            const Polyhedron3d point      { vm::vec3d(1.0, 0.0, 0.0) };
            const Polyhedron3d edge       { vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0) };
            const Polyhedron3d polygon    { vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0) };
            const Polyhedron3d polyhedron { vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0), vm::vec3d(0.0, 0.0, 1.0) };

            CHECK(mutuallyNotIntersects(empty, empty));
            CHECK(mutuallyNotIntersects(empty, point));
            CHECK(mutuallyNotIntersects(empty, edge));
            CHECK(mutuallyNotIntersects(empty, polygon));
            CHECK(mutuallyNotIntersects(empty, polyhedron));
        }

        TEST_CASE("PolyhedronTest.intersection_point_point", "[PolyhedronTest]") {
            const Polyhedron3d point { vm::vec3d(0.0, 0.0, 0.0) };

            CHECK(mutuallyIntersects(point, point));
            CHECK(mutuallyNotIntersects(point, Polyhedron3d { vm::vec3d(0.0, 0.0, 1.0) }));
        }

        TEST_CASE("PolyhedronTest.intersection_point_edge", "[PolyhedronTest]") {
            const vm::vec3d pointPos(0.0, 0.0, 0.0);
            const Polyhedron3d point { pointPos };

            CHECK(mutuallyIntersects(point, Polyhedron3d { pointPos, vm::vec3d(1.0, 0.0, 0.0) } )); // point / edge originating at point
            CHECK(mutuallyIntersects(point, Polyhedron3d { vm::vec3d(-1.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, 0.0) } )); // point / edge containing point
            CHECK(mutuallyNotIntersects(point, Polyhedron3d { vm::vec3d(-1.0, 0.0, 1.0), vm::vec3d(1.0, 0.0, 1.0) } )); // point / unrelated edge
        }

        TEST_CASE("PolyhedronTest.intersection_point_polygon", "[PolyhedronTest]") {
            const vm::vec3d pointPos(0.0, 0.0, 0.0);
            const Polyhedron3d point { pointPos };

            CHECK(mutuallyIntersects(point, Polyhedron3d { pointPos, vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0) } )); // point / triangle with point as vertex
            CHECK(mutuallyIntersects(point, Polyhedron3d { vm::vec3d(-1.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0) } )); // point / triangle with point on edge
            CHECK(mutuallyIntersects(point, Polyhedron3d { vm::vec3d(-1.0, -1.0, 0.0), vm::vec3d(1.0, -1.0, 0.0), vm::vec3d(0.0, 1.0, 0.0) } )); // point / triangle containing point

            CHECK(mutuallyNotIntersects(point, Polyhedron3d { vm::vec3d(-1.0, -1.0, 1.0), vm::vec3d(1.0, -1.0, 1.0), vm::vec3d(0.0, 1.0, 1.0) } )); // point / triangle above point
        }

        TEST_CASE("PolyhedronTest.intersection_point_polyhedron", "[PolyhedronTest]") {
            const vm::vec3d pointPos(0.0, 0.0, 0.0);
            const Polyhedron3d point { pointPos };

            CHECK(mutuallyIntersects(point, Polyhedron3d { pointPos, vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0), vm::vec3d(0.0, 0.0, 1.0) } )); // point / tetrahedron with point as vertex
            CHECK(mutuallyIntersects(point, Polyhedron3d { vm::vec3d(-1.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0), vm::vec3d(0.0, 0.0, 1.0) } )); // point / tetrahedron with point on edge
            CHECK(mutuallyIntersects(point, Polyhedron3d { vm::vec3d(-1.0, -1.0, 0.0), vm::vec3d(1.0, -1.0, 0.0), vm::vec3d(0.0, 1.0, 0.0), vm::vec3d(0.0, 0.0, 1.0) } )); // point / tetrahedron with point on face
            CHECK(mutuallyIntersects(point, Polyhedron3d { vm::vec3d(-1.0, -1.0, -1.0), vm::vec3d(1.0, -1.0, -1.0), vm::vec3d(0.0, 1.0, -1.0), vm::vec3d(0.0, 0.0, 1.0) } )); // point / tetrahedron with point on face

            CHECK(mutuallyNotIntersects(point, Polyhedron3d { vm::vec3d(-1.0, -1.0, 1.0), vm::vec3d(1.0, -1.0, 1.0), vm::vec3d(0.0, 1.0, 1.0), vm::vec3d(0.0, 0.0, 2.0) } )); // point / tetrahedron above point
        }

        TEST_CASE("PolyhedronTest.intersection_edge_edge", "[PolyhedronTest]") {
            const vm::vec3d point1(-1.0, 0.0, 0.0);
            const vm::vec3d point2(+1.0, 0.0, 0.0);
            const Polyhedron3d edge { point1, point2 };

            CHECK(mutuallyIntersects(edge, edge));
            CHECK(mutuallyIntersects(edge, Polyhedron3d { point1, vm::vec3d(0.0, 0.0, 1.0) } ));
            CHECK(mutuallyIntersects(edge, Polyhedron3d { point2, vm::vec3d(0.0, 0.0, 1.0) } ));
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(0.0, -1.0, 0.0), vm::vec3d(0.0, 1.0, 0.0) } ));
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d( 0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0) } ));
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(-2.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0) } ));

            CHECK(mutuallyNotIntersects(edge, Polyhedron3d { point1 + vm::vec3d::pos_z(), point2 + vm::vec3d::pos_z() } ));
        }

        TEST_CASE("PolyhedronTest.intersection_edge_polygon_same_plane", "[PolyhedronTest]") {
            const vm::vec3d point1(-1.0, 0.0, 0.0);
            const vm::vec3d point2(+1.0, 0.0, 0.0);
            const Polyhedron3d edge { point1, point2 };

            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(1.0, -1.0, 0.0), vm::vec3d(2.0, -1.0, 0.0), vm::vec3d(2.0, 0.0, 0.0) } )); // one shared point
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(-1.0, 0.0, 0.0), vm::vec3d(0.0, -1.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(0.0, +1.0, 0.0) } )); // two shared points
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(-1.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(1.0, 1.0, 0.0), vm::vec3d(-1.0, 1.0, 0.0) } )); // shared edge
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(0.0, 1.0, 0.0), vm::vec3d(0.0, -1.0, 0.0), vm::vec3d(2.0, -1.0, 0.0), vm::vec3d(2.0, 1.0, 0.0) } )); // polygon contains one point
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(-2.0, 1.0, 0.0), vm::vec3d(-2.0, -1.0, 0.0), vm::vec3d(2.0, -1.0, 0.0), vm::vec3d(2.0, 1.0, 0.0) } ));// polygon contains both points
            CHECK(mutuallyIntersects(edge, Polyhedron3d { vm::vec3d(-0.5, 1.0, 0.0), vm::vec3d(-0.5, -1.0, 0.0), vm::vec3d(0.5, -1.0, 0.0), vm::vec3d(0.5, 1.0, 0.0) } )); // edge intersects polygon completely

            CHECK(mutuallyNotIntersects(edge, Polyhedron3d { vm::vec3d(+2.0, 1.0, 0.0), vm::vec3d(+2.0, -1.0, 0.0), vm::vec3d(+3.0, -1.0, 0.0), vm::vec3d(+3.0, 1.0, 0.0) } )); // no intersection
        }


        TEST_CASE("PolyhedronTest.intersection_edge_polygon_different_plane", "[PolyhedronTest]") {
            const vm::vec3d point1(0.0, 0.0, 1.0);
            const vm::vec3d point2(0.0, 0.0, -1.0);
            const Polyhedron3d edge { point1, point2 };

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(0.0, 0.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } )); // one shared point

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } )); // polygon edge contains edge origin

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(1.0, 1.0, 0.0), vm::vec3d(1.0, 1.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } )); // polygon contains edge origin

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(0.0, 0.0, -1.0), vm::vec3d(0.0, 0.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } )); // edge intersects polygon vertex

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(1.0, 0.0, -1.0), vm::vec3d(1.0, 0.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } )); // edge intersects polygon edge

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(1.0, 1.0, -1.0), vm::vec3d(1.0, 1.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } )); // edge intersects polygon center

            CHECK(mutuallyNotIntersects(Polyhedron3d { vm::vec3d(3.0, 1.0, -1.0), vm::vec3d(3.0, 1.0, +1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } ));

            CHECK(mutuallyNotIntersects(Polyhedron3d { vm::vec3d(1.0, 1.0, 1.0), vm::vec3d(1.0, 1.0, 2.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } ));

            CHECK(mutuallyNotIntersects(Polyhedron3d { vm::vec3d(0.0, 0.0, 1.0), vm::vec3d(1.0, 1.0, 1.0) },
                Polyhedron3d { vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(2.0, 0.0, 0.0), vm::vec3d(2.0, 2.0, 0.0), vm::vec3d(0.0, 2.0, 0.0) } ));
        }

        TEST_CASE("PolyhedronTest.intersection_edge_polyhedron", "[PolyhedronTest]") {
            const Polyhedron3d tetrahedron {
                vm::vec3d(-1.0, -1.0, 0.0),
                vm::vec3d(+1.0, -1.0, 0.0),
                vm::vec3d( 0.0, +1.0, 0.0),
                vm::vec3d( 0.0,  0.0, 1.0)
            };

            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0.0,  0.0,  1.0), vm::vec3d( 0.0,  0.0,  2.0) }, tetrahedron)); // one shared point
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0.0, -0.9999,  0.0), vm::vec3d( 0.0, -2.0,  0.0) }, tetrahedron)); // edge point on polyhedron edge
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0.0,  0.0,  0.0), vm::vec3d( 0.0,  0.0, -1.0) }, tetrahedron)); // edge point on polyhedron face
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(-1.0, -1.0,  0.0), vm::vec3d(+1.0, -1.0,  0.0) }, tetrahedron)); // shared edge
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0.0,  0.0,  0.5), vm::vec3d( 0.0,  0.0,  2.0) }, tetrahedron)); // polyhedron contains one edge point
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0.0,  0.0,  0.2), vm::vec3d( 0.0,  0.0,  0.7) }, tetrahedron)); // polyhedron contains both edge points
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0.0,  0.0, -1.0), vm::vec3d( 0.0,  0.0,  2.0) }, tetrahedron)); // edge penetrates polyhedron

            CHECK(mutuallyNotIntersects(Polyhedron3d { vm::vec3d( -2.0,  -2.0, -1.0), vm::vec3d( 2.0,  2.0,  -1.0) }, tetrahedron)); // no intersection
        }

        TEST_CASE("PolyhedronTest.intersection_polygon_polygon_same_plane", "[PolyhedronTest]") {
            const Polyhedron3d square {
                vm::vec3d(-1.0, -1.0, 0.0),
                vm::vec3d(+1.0, -1.0, 0.0),
                vm::vec3d(+1.0, +1.0, 0.0),
                vm::vec3d(-1.0, +1.0, 0.0)
            };

            // shared vertex:
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(+1, +1, 0), vm::vec3d(+2, +1, 0), vm::vec3d(+1, +2, 0) }, square));

            // shared edge
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(-1, +1, 0), vm::vec3d(+1, +1, 0), vm::vec3d( 0, +2, 0) }, square));

            // edge contains other edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2, -1, 0),
                vm::vec3d(+2, -1, 0),
                vm::vec3d(+2, +1, 0),
                vm::vec3d(-2, +1, 0),
            }, square));

            // one contains vertex of another
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d( 0,  0, 0), vm::vec3d(+2,  0, 0), vm::vec3d(+2, +2, 0), vm::vec3d(0, +2, 0) }, square));

            // one contains another entirely
            CHECK(mutuallyIntersects(Polyhedron3d { vm::vec3d(-2, -2, 0), vm::vec3d(+2, -2, 0), vm::vec3d(+2, +2, 0), vm::vec3d(-2, +2, 0) }, square));

            // one penetrates the other
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2, -0.5, 0),
                vm::vec3d(+2, -0.5, 0),
                vm::vec3d(+2, +0.5, 0),
                vm::vec3d(-2, +0.5, 0)
            }, square));

            // no intersection
            CHECK(mutuallyNotIntersects(Polyhedron3d {
                vm::vec3d(+2, +2, 0),
                vm::vec3d(+3, +2, 0),
                vm::vec3d(+3, +3, 0),
                vm::vec3d(+3, +3, 0)
            }, square));
        }

        TEST_CASE("PolyhedronTest.intersection_polygon_polygon_different_plane", "[PolyhedronTest]") {
            const Polyhedron3d square {
                vm::vec3d(-1.0, -1.0, 0.0),
                vm::vec3d(+1.0, -1.0, 0.0),
                vm::vec3d(+1.0, +1.0, 0.0),
                vm::vec3d(-1.0, +1.0, 0.0)
            };

            // shared vertex
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-1.0, -1.0, 0.0),
                vm::vec3d(-2.0, -1.0, 0.0),
                vm::vec3d(-2.0, -1.0, 1.0)
            }, square));

            // vertex on edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(0.0, -1.0, 0.0),
                vm::vec3d(0.0, -2.0, 0.0),
                vm::vec3d(0.0, -1.0, 1.0),
                vm::vec3d(0.0, -2.0, 1.0),
            }, square));

            // shared edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-1.0, -1.0, 0.0),
                vm::vec3d(+1.0, -1.0, 0.0),
                vm::vec3d(+1.0, -1.0, 1.0),
                vm::vec3d(-1.0, -1.0, 1.0)
            }, square));

            // edges intersect
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(0.0, -1.0, -1.0),
                vm::vec3d(0.0, -1.0, +1.0),
                vm::vec3d(0.0, -2.0, +1.0),
                vm::vec3d(0.0, -2.0, -1.0)
            }, square));

            // partial penetration (one edge penetrates each)
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(0.0, 0.0, -1.0),
                vm::vec3d(0.0, 0.0, +1.0),
                vm::vec3d(2.0, 0.0, +1.0),
                vm::vec3d(2.0, 0.0, -1.0)
            }, square));

            // full penetration (two edges penetrate)
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2.0, 0.0, -2.0),
                vm::vec3d(-2.0, 0.0, +2.0),
                vm::vec3d(+2.0, 0.0, -2.0),
                vm::vec3d(+2.0, 0.0, +2.0)
            }, square));

            // no intersection
            CHECK(mutuallyNotIntersects(Polyhedron3d {
                vm::vec3d(-1.0, 0.0, 5.0),
                vm::vec3d(+1.0, 0.0, 5.0),
                vm::vec3d(-1.0, 0.0, 6.0),
                vm::vec3d(+1.0, 0.0, 6.0)
            }, square));
        }

        TEST_CASE("PolyhedronTest.intersection_polygon_polyhedron_same_plane_as_face", "[PolyhedronTest]") {
            const Polyhedron3d cube {
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
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(+1.0, +1.0, +1.0),
                vm::vec3d(+2.0, +1.0, +1.0),
                vm::vec3d(+2.0, +2.0, +1.0),
            }, cube));

            // shared edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(+1.0, +1.0, +1.0),
                vm::vec3d(-1.0, +1.0, +1.0),
                vm::vec3d(+1.0, +2.0, +1.0)
            }, cube));

            // edge contains other edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-0.5, +1.0, +1.0),
                vm::vec3d(+0.5, +1.0, +1.0),
                vm::vec3d(+0.5, +2.0, +1.0)
            }, cube));

            // one contains vertex of another
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(+0.0, +0.0, +1.0),
                vm::vec3d(+2.0, +0.0, +1.0),
                vm::vec3d(+2.0, +2.0, +1.0),
                vm::vec3d(+0.0, +2.0, +1.0),
            }, cube));

            // one contains another entirely
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-0.5, -0.5, +1.0),
                vm::vec3d(-0.5, +0.5, +1.0),
                vm::vec3d(+0.5, +0.5, +1.0),
                vm::vec3d(+0.5, -0.5, +1.0),
            }, cube));
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2.5, -2.5, +1.0),
                vm::vec3d(-2.5, +2.5, +1.0),
                vm::vec3d(+2.5, +2.5, +1.0),
                vm::vec3d(+2.5, -2.5, +1.0),
            }, cube));

            // one penetrates the other
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2.0, -0.5, +1.0),
                vm::vec3d(+2.0, -0.5, +1.0),
                vm::vec3d(-2.0, +0.5, +1.0),
                vm::vec3d(+2.0, +0.5, +1.0),
            }, cube));

            // no intersection
            CHECK(mutuallyNotIntersects(Polyhedron3d {
                vm::vec3d(+2.0, +2.0, +1.0),
                vm::vec3d(+3.0, +2.0, +1.0),
                vm::vec3d(+3.0, +3.0, +1.0),
                vm::vec3d(+2.0, +3.0, +1.0),
            }, cube));
        }

        TEST_CASE("PolyhedronTest.intersection_polygon_polyhedron_any_orientation", "[PolyhedronTest]") {
            const Polyhedron3d cube {
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
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(+1.0, +1.0, +1.0),
                vm::vec3d(+2.0, +1.0, +2.0),
                vm::vec3d(+2.0, +2.0, +2.0)
            }, cube));

            // polygon vertex on polyhedron edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(+0.0, +1.0, +1.0),
                vm::vec3d(+2.0, +1.0, +2.0),
                vm::vec3d(+2.0, +2.0, +2.0)
            }, cube));

            // polyhedron vertex on polygon edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(0.0, 2.0, 1.0),
                vm::vec3d(2.0, 0.0, 1.0),
                vm::vec3d(0.0, 0.0, 2.0)
            }, cube));

            // shared edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-1.0, 1.0, 1.0),
                vm::vec3d(+1.0, 1.0, 1.0),
                vm::vec3d( 0.0, 2.0, 2.0)
            }, cube));


            // polygon edge inside polyhedron edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-0.5, 1.0, 1.0),
                vm::vec3d(+0.5, 1.0, 1.0),
                vm::vec3d( 0.0, 2.0, 2.0),
            }, cube));


            // polyhedorn edge inside polygon edge
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2.0, 1.0, 1.0),
                vm::vec3d(+2.0, 1.0, 1.0),
                vm::vec3d( 0.0, 2.0, 2.0)
            }, cube));

            // edges intersect
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(0.0, -2.0, 0.0),
                vm::vec3d(0.0,  0.0, 2.0),
                vm::vec3d(0.0, -2.0, 2.0)
            }, cube));

            // penetration (two polygon edges intersect)
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(0.0,  0.0, 0.0),
                vm::vec3d(0.0, -3.0, 0.0),
                vm::vec3d(3.0,  0.0, 2.0),
            }, cube));

            // polyhedron contains polygon
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-0.5, 0.0, 0.0),
                vm::vec3d( 0.0, 0.5, 0.0),
                vm::vec3d( 0.0, 0.0, 0.5)
            }, cube));

            // polygon slices polyhedron (surrounds it)
            CHECK(mutuallyIntersects(Polyhedron3d {
                vm::vec3d(-2.0, -2.0, 0.0),
                vm::vec3d(-2.0, +2.0, 0.0),
                vm::vec3d(+2.0, -2.0, 0.0),
                vm::vec3d(+2.0, +2.0, 0.0),
            }, cube));
        }

        TEST_CASE("PolyhedronTest.addVertexToPolygonAndAllFacesCoplanar", "[PolyhedronTest]") {
            auto p = Polyhedron3d{
                vm::vec3{-64.0, 64.0, -16.0},
                vm::vec3{64.0, 64.0, -16.0},
                vm::vec3{22288.0, 18208.0, 16.0},
                vm::vec3{22288.0, 18336.0, 16.0}, // does not get added due to all incident faces being coplanar
                vm::vec3{22416.0, 18336.0, 16.0},
            };

            CHECK(p.hasAllVertices({
                vm::vec3{-64.0, 64.0, -16.0},
                vm::vec3{64.0, 64.0, -16.0},
                vm::vec3{22288.0, 18208.0, 16.0},
                vm::vec3{22416.0, 18336.0, 16.0},
            }, 0.0));
        }
    }
}
