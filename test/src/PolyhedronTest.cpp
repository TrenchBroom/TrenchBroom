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

#include <gtest/gtest.h>

#include "CollectionUtils.h"

#include "Polyhedron.h"
#include "Polyhedron_DefaultPayload.h"
#include "MathUtils.h"
#include "TestUtils.h"

typedef Polyhedron<double, DefaultPolyhedronPayload, DefaultPolyhedronPayload> Polyhedron3d;
typedef Polyhedron3d::Vertex Vertex;
typedef Polyhedron3d::VertexList VertexList;
typedef Polyhedron3d::Edge Edge;
typedef Polyhedron3d::HalfEdge HalfEdge;
typedef Polyhedron3d::EdgeList EdgeList;
typedef Polyhedron3d::Face Face;
typedef Polyhedron3d::FaceList FaceList;

typedef std::pair<Vec3d, Vec3d> EdgeInfo;
typedef std::vector<EdgeInfo> EdgeInfoList;

bool hasVertex(const Polyhedron3d& p, const Vec3d& point);
bool hasVertices(const Polyhedron3d& p, const Vec3d::List& points);
bool hasEdge(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2);
bool hasEdges(const Polyhedron3d& p, const EdgeInfoList& edgeInfos);
bool hasTriangleOf(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3);
bool hasQuadOf(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3, const Vec3d& p4);
bool hasPolygonOf(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3, const Vec3d& p4, const Vec3d& p5);

void assertIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs);
void assertNotIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs);


TEST(PolyhedronTest, initWith4Points) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    
    const Polyhedron3d p(p1, p2, p3, p4);
    ASSERT_TRUE(p.closed());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    ASSERT_TRUE(hasVertices(p, points));

    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p2, p3));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p4, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p4, p1));
    
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_TRUE(hasTriangleOf(p, p2, p3, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p2));
    ASSERT_TRUE(hasTriangleOf(p, p1, p2, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p4, p3));
}

TEST(PolyhedronTest, copy) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);

    Polyhedron3d original;
    Polyhedron3d copy;
    
    copy = original;
    ASSERT_EQ(original, copy);
    
    original.addPoint(p1);
    copy = original;
    ASSERT_EQ(original, copy);
    
    original.addPoint(p2);
    copy = original;
    ASSERT_EQ(original, copy);
    
    original.addPoint(p3);
    copy = original;
    ASSERT_EQ(original, copy);

    original.addPoint(p4);
    copy = original;
    ASSERT_EQ(original, copy);
}

TEST(PolyhedronTest, swap) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    
    Polyhedron3d original;
    original.addPoint(p1);
    original.addPoint(p2);
    original.addPoint(p3);
    original.addPoint(p4);

    Polyhedron3d other;
    other.addPoint(p2);
    other.addPoint(p3);
    other.addPoint(p4);
    
    Polyhedron3d lhs = original;
    Polyhedron3d rhs = other;
    
    // Just to be sure...
    assert(lhs == original);
    assert(rhs == other);
    
    using std::swap;
    swap(lhs, rhs);
    
    ASSERT_EQ(other, lhs);
    ASSERT_EQ(original, rhs);
    
    ASSERT_EQ(other.bounds(), lhs.bounds());
    ASSERT_EQ(original.bounds(), rhs.bounds());
}

TEST(PolyhedronTest, convexHullWithFailingPoints) {
    const Vec3d p1(-64.0,    -45.5049, -34.4752);
    const Vec3d p2(-64.0,    -43.6929, -48.0);
    const Vec3d p3(-64.0,     20.753,  -34.4752);
    const Vec3d p4(-64.0,     64.0,    -48.0);
    const Vec3d p5(-63.7297,  22.6264, -48.0);
    const Vec3d p6(-57.9411,  22.6274, -37.9733);
    const Vec3d p7(-44.6031, -39.1918, -48.0);
    const Vec3d p8(-43.5959, -39.1918, -46.2555);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4); // so far, all were on the same plane at x=-64
    p.addPoint(p5); // something is going wrong here, the result is not a proper polyhedron
    p.addPoint(p6); // assertion failure here
    p.addPoint(p7);
    p.addPoint(p8);
}

TEST(PolyhedronTest, convexHullWithFailingPoints2) {
    const Vec3d p1(-64.0,    48.7375, -34.4752);
    const Vec3d p2(-64.0,    64.0,    -48.0);
    const Vec3d p3(-64.0,    64.0,    -34.4752);
    const Vec3d p4(-63.7297, 22.6264, -48.0);
    const Vec3d p5(-57.9411, 22.6274, -37.9733);
    const Vec3d p6(-40.5744, 28.0,    -48.0);
    const Vec3d p7(-40.5744, 64.0,    -48.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3); // so far, all were on the same plane at x=-64
    p.addPoint(p4); // wasn't added due to being too close
    p.addPoint(p5); // assertion failure here
    p.addPoint(p6);
    p.addPoint(p7);
}

TEST(PolyhedronTest, convexHullWithFailingPoints3) {
    const Vec3d p1(-64,      -64,      -48);
    const Vec3d p2(-64,       22.5637, -48);
    const Vec3d p3(-64,       64,      -48);
    const Vec3d p4(-63.7297,  22.6264, -48);
    const Vec3d p5(-57.9411,  22.6274, -37.9733);
    const Vec3d p6(-44.6031, -39.1918, -48);
    const Vec3d p7(-43.5959, -39.1918, -46.2555);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6); // assertion failure here
    p.addPoint(p7);
}

TEST(PolyhedronTest, convexHullWithFailingPoints4) {
    const Vec3d p01(-64, 64, -48);
    const Vec3d p02(-43.5959, -39.1918, -46.2555);
    const Vec3d p03(-40.5744, -38.257, -48);
    const Vec3d p04(-36.9274, -64, -48);
    const Vec3d p05(1.58492, -39.1918, 32);
    const Vec3d p06(9.2606, -64, 32);
    const Vec3d p07(12.8616, -64, 32);
    const Vec3d p08(12.8616, -36.5751, 32);
    const Vec3d p09(26.7796, -22.6274, -48);
    const Vec3d p10(39.5803, -64, -48);
    const Vec3d p11(57.9411, -22.6274, 5.9733);
    const Vec3d p12(64, -64, -5.70392);
    const Vec3d p13(64, -64, 2.47521);
    const Vec3d p14(64, -48.7375, 2.47521);
    
    Polyhedron3d p;
    p.addPoint(p01);
    p.addPoint(p02);
    p.addPoint(p03);
    p.addPoint(p04);
    p.addPoint(p05); // assertion failure here
    p.addPoint(p06);
    p.addPoint(p07);
    p.addPoint(p08);
    p.addPoint(p09);
    p.addPoint(p10);
    p.addPoint(p11);
    p.addPoint(p12);
    p.addPoint(p13);
    p.addPoint(p14);
}

TEST(PolyhedronTest, convexHullWithFailingPoints5) {
    const Vec3d p01(-64, -64, -64);
    const Vec3d p02(-64, -64, 64);
    const Vec3d p03(-64, -32, 64);
    const Vec3d p04(-32, -64, -64);
    const Vec3d p05(-32, -64, 64);
    const Vec3d p06(-32, -0, -64);
    const Vec3d p07(-32, -0, 64);
    const Vec3d p08(-0, -32, -64);
    const Vec3d p09(-0, -32, 64);
    const Vec3d p10(64, -64, -64);
    
    Polyhedron3d p;
    p.addPoint(p01);
    p.addPoint(p02);
    p.addPoint(p03);
    p.addPoint(p04);
    p.addPoint(p05);
    p.addPoint(p06);
    p.addPoint(p07);
    p.addPoint(p08);
    p.addPoint(p09);
    p.addPoint(p10); // assertion failure here
}

TEST(PolyhedronTest, convexHullWithFailingPoints6) {
    const Vec3d p1(-32, -16, -32);
    const Vec3d p2(-32, 16, -32);
    const Vec3d p3(-32, 16, -0);
    const Vec3d p4(-16, -16, -32);
    const Vec3d p5(-16, -16, -0);
    const Vec3d p6(-16, 16, -32);
    const Vec3d p7(-16, 16, -0);
    const Vec3d p8(32, -16, -32);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6);
    p.addPoint(p7);
    p.addPoint(p8);
}

TEST(PolyhedronTest, convexHullWithFailingPoints7) {
    const Vec3d p1(12.8616, -36.5751, 32);
    const Vec3d p2(57.9411, -22.6274, 5.9733);
    const Vec3d p3(64, -64, 2.47521);
    const Vec3d p4(64, -64, 32);
    const Vec3d p5(64, -48.7375, 2.47521);
    const Vec3d p6(64, -24.7084, 32);
    const Vec3d p7(64, -22.6274, 16.4676);
    const Vec3d p8(64, 64, 32);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6);
    p.addPoint(p7);
    p.addPoint(p8);
}

TEST(PolyhedronTest, convexHullWithFailingPoints8) {
    // Cause of https://github.com/kduske/TrenchBroom/issues/1469
    // See also BrushTest.subtractTruncatedCones
    
    const Vec3d  p1(-22.364439661516872, 9.2636542228362799, 32);
    const Vec3d  p2(-21.333333333333332, 11.049582771255995, 32);
    const Vec3d  p3(-20.235886048009661, 12.95041722806517, 32);
    const Vec3d  p4(-19.126943405596094, 11.042945924655637, 32);
    const Vec3d  p5(-18.31934864142023, 14.056930615671543, 32);
    const Vec3d  p6(-17.237604305873624, 9.9521354859295226, 7.4256258352417603);
    const Vec3d  p7(-16, 6.6274169975893429, -0);
    const Vec3d  p8(-15.999999999999998, 9.2376043067828455, -0);
    const Vec3d  p9(-14.345207554102323, 8.2822094434885454, -0);
    const Vec3d p10(-13.739511480972288, 10.542697961743528, -0);
    
    Polyhedron3d p;
    p.addPoint( p1);
    p.addPoint( p2);
    p.addPoint( p3);
    p.addPoint( p4); // assertion failure here, fixed by using an epsilon value in method linearlyDependent
    p.addPoint( p5);
    p.addPoint( p6);
    p.addPoint( p7);
    p.addPoint( p8);
    p.addPoint( p9);
    p.addPoint(p10);
}

/*
TEST(PolyhedronTest, testImpossibleSplit) {
    const Vec3d p1( 0.0, 4.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 4.0, 4.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    Polyhedron3d::Seam seam = p.split(Polyhedron3d::SplitByVisibilityCriterion(p5));
    ASSERT_TRUE(seam.empty());
}

TEST(PolyhedronTest, testSimpleSplit) {
    const Vec3d p1( 0.0, 4.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 4.0, 12.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    Polyhedron3d::Seam seam = p.split(Polyhedron3d::SplitByVisibilityCriterion(p5));
    ASSERT_EQ(3u, seam.size());
    
    ASSERT_FALSE(p.closed());
    ASSERT_EQ(3u, p.vertexCount());
    ASSERT_EQ(3u, p.edgeCount());
    ASSERT_EQ(1u, p.faceCount());

    ASSERT_TRUE(hasTriangleOf(p, p2, p3, p4));
}

TEST(PolyhedronTest, testWeaveSimpleCap) {
    const Vec3d p1( 0.0, 4.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 4.0, 12.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    Polyhedron3d::Seam seam = p.split(Polyhedron3d::SplitByVisibilityCriterion(p5));
    
    p.weaveCap(seam, p5);
    ASSERT_TRUE(p.closed());
    ASSERT_EQ(4u, p.vertexCount());
    ASSERT_EQ(6u, p.edgeCount());
    ASSERT_EQ(4u, p.faceCount());
}
*/
TEST(PolyhedronTest, testSimpleConvexHull) {
    const Vec3d p1( 0.0, 4.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 4.0, 12.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    p.addPoint(p5);
    
    ASSERT_TRUE(p.closed());

    Vec3d::List points;
    points.push_back(p5);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    ASSERT_TRUE(hasVertices(p, points));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p2, p3));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p4, p2));
    edgeInfos.push_back(std::make_pair(p5, p3));
    edgeInfos.push_back(std::make_pair(p5, p2));
    edgeInfos.push_back(std::make_pair(p4, p5));
    
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_TRUE(hasTriangleOf(p, p2, p3, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p3, p2));
    ASSERT_TRUE(hasTriangleOf(p, p5, p2, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p4, p3));
}

TEST(PolyhedronTest, testSimpleConvexHullWithCoplanarFaces) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 0.0, 12.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    p.addPoint(p5);
    
    ASSERT_TRUE(p.closed());
    
    Vec3d::List points;
    points.push_back(p5);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    ASSERT_TRUE(hasVertices(p, points));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p2, p3));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p4, p2));
    edgeInfos.push_back(std::make_pair(p5, p3));
    edgeInfos.push_back(std::make_pair(p5, p2));
    edgeInfos.push_back(std::make_pair(p4, p5));
    
    ASSERT_TRUE(hasTriangleOf(p, p2, p3, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p3, p2));
    ASSERT_TRUE(hasTriangleOf(p, p5, p2, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p4, p3));
}

TEST(PolyhedronTest, testSimpleConvexHullOfCube) {
    const Vec3d p1( -8.0, -8.0, -8.0);
    const Vec3d p2( -8.0, -8.0, +8.0);
    const Vec3d p3( -8.0, +8.0, -8.0);
    const Vec3d p4( -8.0, +8.0, +8.0);
    const Vec3d p5( +8.0, -8.0, -8.0);
    const Vec3d p6( +8.0, -8.0, +8.0);
    const Vec3d p7( +8.0, +8.0, -8.0);
    const Vec3d p8( +8.0, +8.0, +8.0);

    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    points.push_back(p6);
    points.push_back(p7);
    points.push_back(p8);
    
    Polyhedron3d p(points);
    
    ASSERT_TRUE(p.closed());
    
    ASSERT_TRUE(hasVertices(p, points));
    
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
    
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p3, p1, p2, p4));
    ASSERT_TRUE(hasQuadOf(p, p7, p3, p4, p8));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p8, p6));
    ASSERT_TRUE(hasQuadOf(p, p3, p7, p5, p1));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p8, p4));
}

TEST(PolyhedronTest, initEmpty) {
    Polyhedron3d p;
    ASSERT_TRUE(p.empty());
}

TEST(PolyhedronTest, initEmptyAndAddOnePoint) {
    const Vec3d p1( -8.0, -8.0, -8.0);

    Polyhedron3d p;
    p.addPoint(p1);
    
    ASSERT_FALSE(p.empty());
    ASSERT_TRUE(p.point());
    ASSERT_FALSE(p.edge());
    ASSERT_FALSE(p.polygon());
    ASSERT_FALSE(p.polyhedron());

    Vec3d::List points;
    points.push_back(p1);
    
    ASSERT_TRUE(hasVertices(p, points));
}


TEST(PolyhedronTest, initEmptyAndAddTwoIdenticalPoints) {
    const Vec3d p1( -8.0, -8.0, -8.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p1);
    
    ASSERT_FALSE(p.empty());
    ASSERT_TRUE(p.point());
    ASSERT_FALSE(p.edge());
    ASSERT_FALSE(p.polygon());
    ASSERT_FALSE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, initEmptyAndAddTwoPoints) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(3.0, 0.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    
    ASSERT_FALSE(p.empty());
    ASSERT_FALSE(p.point());
    ASSERT_TRUE(p.edge());
    ASSERT_FALSE(p.polygon());
    ASSERT_FALSE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, initEmptyAndAddThreeColinearPoints) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(3.0, 0.0, 0.0);
    const Vec3d p3(6.0, 0.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    
    ASSERT_FALSE(p.empty());
    ASSERT_FALSE(p.point());
    ASSERT_TRUE(p.edge());
    ASSERT_FALSE(p.polygon());
    ASSERT_FALSE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p3);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, initEmptyAndAddThreePoints) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(3.0, 0.0, 0.0);
    const Vec3d p3(6.0, 5.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    
    ASSERT_FALSE(p.empty());
    ASSERT_FALSE(p.point());
    ASSERT_FALSE(p.edge());
    ASSERT_TRUE(p.polygon());
    ASSERT_FALSE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, initEmptyAndAddThreePointsAndOneInnerPoint) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(6.0, 0.0, 0.0);
    const Vec3d p3(3.0, 6.0, 0.0);
    const Vec3d p4(3.0, 3.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    ASSERT_FALSE(p.empty());
    ASSERT_FALSE(p.point());
    ASSERT_FALSE(p.edge());
    ASSERT_TRUE(p.polygon());
    ASSERT_FALSE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, initEmptyAndAddFourCoplanarPoints) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(6.0, 0.0, 0.0);
    const Vec3d p3(3.0, 3.0, 0.0);
    const Vec3d p4(3.0, 6.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    ASSERT_FALSE(p.empty());
    ASSERT_FALSE(p.point());
    ASSERT_FALSE(p.edge());
    ASSERT_TRUE(p.polygon());
    ASSERT_FALSE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p4);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, initEmptyAndAddFourPoints) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(6.0, 0.0, 0.0);
    const Vec3d p3(3.0, 6.0, 0.0);
    const Vec3d p4(3.0, 3.0, 6.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    ASSERT_FALSE(p.empty());
    ASSERT_FALSE(p.point());
    ASSERT_FALSE(p.edge());
    ASSERT_FALSE(p.polygon());
    ASSERT_TRUE(p.polyhedron());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    
    ASSERT_TRUE(hasVertices(p, points));
}

TEST(PolyhedronTest, testAddColinearPointToRectangleOnEdge) {
    // https://github.com/kduske/TrenchBroom/issues/1659
    /*
     p4 p5 p3
     *--+--*
     |     |
     |     |
     *-----*
     p1    p2
     */
    
    const Vec3d p1(  0.0,   0.0, 0.0);
    const Vec3d p2(+32.0,   0.0, 0.0);
    const Vec3d p3(+32.0, +32.0, 0.0);
    const Vec3d p4(  0.0, +32.0, 0.0);
    const Vec3d p5(+16.0, +32.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    
    ASSERT_TRUE(p.addPoint(p5) == nullptr);
    ASSERT_FALSE(p.hasVertex(p5));
}

TEST(PolyhedronTest, testAddPointToRectangleMakingOneColinear) {
    /*
     p4    p3  p5
     *-----*   +
     |     |
     |     |
     *-----*
     p1    p2
     */
    
    const Vec3d p1(  0.0,   0.0, 0.0);
    const Vec3d p2(+32.0,   0.0, 0.0);
    const Vec3d p3(+32.0, +32.0, 0.0);
    const Vec3d p4(  0.0, +32.0, 0.0);
    const Vec3d p5(+40.0, +32.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    
    ASSERT_TRUE(p.addPoint(p5) != nullptr);
    ASSERT_TRUE(p.hasVertex(p5));
    ASSERT_FALSE(p.hasVertex(p3));
}

TEST(PolyhedronTest, testAddExistingPoints) {
    /*
     p4    p3
     *-----*
     |     |
     |     |
     *-----*
     p1    p2
     */
    
    const Vec3d p1(  0.0,   0.0, 0.0);
    const Vec3d p2(+32.0,   0.0, 0.0);
    const Vec3d p3(+32.0, +32.0, 0.0);
    const Vec3d p4(  0.0, +32.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    
    ASSERT_TRUE(p.addPoint(p1) == nullptr);
    ASSERT_TRUE(p.addPoint(p2) == nullptr);
    ASSERT_TRUE(p.addPoint(p3) == nullptr);
    ASSERT_TRUE(p.addPoint(p4) == nullptr);
}

TEST(PolyhedronTest, testAddManyPointsCrash) {
    const Vec3d p1( 8, 10, 0);
    const Vec3d p2( 0, 24, 0);
    const Vec3d p3( 8, 10, 8);
    const Vec3d p4(10, 11, 8);
    const Vec3d p5(12, 24, 8);
    const Vec3d p6( 0,  6, 8);
    const Vec3d p7(10,  0, 8);

    Polyhedron3d p;
    
    p.addPoint(p1);
    
    ASSERT_TRUE(p.point());
    ASSERT_EQ(1u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    
    p.addPoint(p2);
    
    ASSERT_TRUE(p.edge());
    ASSERT_EQ(2u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_EQ(1u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));

    p.addPoint(p3);
    
    ASSERT_TRUE(p.polygon());
    ASSERT_EQ(3u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_EQ(3u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));
    ASSERT_TRUE(p.hasEdge(p1, p3));
    ASSERT_TRUE(p.hasEdge(p2, p3));
    ASSERT_EQ(1u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p2, p3));
    
    p.addPoint(p4);
    
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(4u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_EQ(6u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));
    ASSERT_TRUE(p.hasEdge(p1, p3));
    ASSERT_TRUE(p.hasEdge(p2, p3));
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p2, p4));
    ASSERT_TRUE(p.hasEdge(p3, p4));
    ASSERT_EQ(4u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p2));
    ASSERT_TRUE(hasTriangleOf(p, p1, p2, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p4, p3));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p2));

    p.addPoint(p5);
    
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(5u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p5));
    ASSERT_EQ(9u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));
    ASSERT_TRUE(p.hasEdge(p1, p3));
    ASSERT_TRUE(p.hasEdge(p2, p3));
    ASSERT_TRUE(p.hasEdge(p1, p4));
    // ASSERT_TRUE(p.hasEdge(p2, p4));
    ASSERT_TRUE(p.hasEdge(p3, p4));
    ASSERT_TRUE(p.hasEdge(p5, p1));
    ASSERT_TRUE(p.hasEdge(p5, p2));
    ASSERT_TRUE(p.hasEdge(p5, p3));
    ASSERT_TRUE(p.hasEdge(p5, p4));
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p2));
    // ASSERT_TRUE(hasTriangleOf(p, p1, p2, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p4, p3));
    // ASSERT_TRUE(hasTriangleOf(p, p3, p4, p2));
    ASSERT_TRUE(hasTriangleOf(p, p5, p4, p1));
    ASSERT_TRUE(hasTriangleOf(p, p5, p3, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p2, p3));
    ASSERT_TRUE(hasTriangleOf(p, p5, p1, p2));
    
    p.addPoint(p6);
    ASSERT_EQ(5u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    // ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p5));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_EQ(9u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));
    // ASSERT_TRUE(p.hasEdge(p1, p3));
    // ASSERT_TRUE(p.hasEdge(p2, p3));
    ASSERT_TRUE(p.hasEdge(p1, p4));
    // ASSERT_TRUE(p.hasEdge(p2, p4));
    // ASSERT_TRUE(p.hasEdge(p3, p4));
    ASSERT_TRUE(p.hasEdge(p5, p1));
    ASSERT_TRUE(p.hasEdge(p5, p2));
    // ASSERT_TRUE(p.hasEdge(p5, p3));
    ASSERT_TRUE(p.hasEdge(p5, p4));
    ASSERT_TRUE(p.hasEdge(p6, p2));
    ASSERT_TRUE(p.hasEdge(p6, p5));
    ASSERT_TRUE(p.hasEdge(p6, p4));
    ASSERT_TRUE(p.hasEdge(p6, p1));
    ASSERT_EQ(6u, p.faceCount());
    // ASSERT_TRUE(hasTriangleOf(p, p1, p3, p2));
    // ASSERT_TRUE(hasTriangleOf(p, p1, p2, p4));
    // ASSERT_TRUE(hasTriangleOf(p, p1, p4, p3));
    // ASSERT_TRUE(hasTriangleOf(p, p3, p4, p2));
    ASSERT_TRUE(hasTriangleOf(p, p5, p4, p1));
    // ASSERT_TRUE(hasTriangleOf(p, p5, p3, p4));
    // ASSERT_TRUE(hasTriangleOf(p, p5, p2, p3));
    ASSERT_TRUE(hasTriangleOf(p, p5, p1, p2));
    ASSERT_TRUE(hasTriangleOf(p, p6, p2, p1));
    ASSERT_TRUE(hasTriangleOf(p, p6, p5, p2));
    ASSERT_TRUE(hasTriangleOf(p, p6, p4, p5));
    ASSERT_TRUE(hasTriangleOf(p, p6, p1, p4));
    
    p.addPoint(p7);
    ASSERT_EQ(5u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    // ASSERT_TRUE(p.hasVertex(p3));
    // ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p5));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p7));
    ASSERT_EQ(9u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));
    // ASSERT_TRUE(p.hasEdge(p1, p3));
    // ASSERT_TRUE(p.hasEdge(p2, p3));
    // ASSERT_TRUE(p.hasEdge(p1, p4));
    // ASSERT_TRUE(p.hasEdge(p2, p4));
    // ASSERT_TRUE(p.hasEdge(p3, p4));
    ASSERT_TRUE(p.hasEdge(p5, p1));
    ASSERT_TRUE(p.hasEdge(p5, p2));
    // ASSERT_TRUE(p.hasEdge(p5, p3));
    // ASSERT_TRUE(p.hasEdge(p5, p4));
    ASSERT_TRUE(p.hasEdge(p6, p2));
    ASSERT_TRUE(p.hasEdge(p6, p5));
    // ASSERT_TRUE(p.hasEdge(p6, p4));
    ASSERT_TRUE(p.hasEdge(p6, p1));
    ASSERT_EQ(6u, p.faceCount());
    // ASSERT_TRUE(hasTriangleOf(p, p1, p3, p2));
    // ASSERT_TRUE(hasTriangleOf(p, p1, p2, p4));
    // ASSERT_TRUE(hasTriangleOf(p, p1, p4, p3));
    // ASSERT_TRUE(hasTriangleOf(p, p3, p4, p2));
    // ASSERT_TRUE(hasTriangleOf(p, p5, p4, p1));
    // ASSERT_TRUE(hasTriangleOf(p, p5, p3, p4));
    // ASSERT_TRUE(hasTriangleOf(p, p5, p2, p3));
    ASSERT_TRUE(hasTriangleOf(p, p5, p1, p2));
    ASSERT_TRUE(hasTriangleOf(p, p6, p2, p1));
    ASSERT_TRUE(hasTriangleOf(p, p6, p5, p2));
    // ASSERT_TRUE(hasTriangleOf(p, p6, p4, p5));
    // ASSERT_TRUE(hasTriangleOf(p, p6, p1, p4));
    ASSERT_TRUE(hasTriangleOf(p, p7, p1, p5));
    ASSERT_TRUE(hasTriangleOf(p, p7, p6, p1));
    ASSERT_TRUE(hasTriangleOf(p, p7, p5, p6));
}

TEST(PolyhedronTest, testAdd8PointsCrash) {
    // a horizontal rectangle
    const Vec3d p1( 0,  0,  0);
    const Vec3d p2( 0, 32,  0);
    const Vec3d p3(32, 32,  0);
    const Vec3d p4(32,  0,  0);
    
    // a vertical rectangle
    const Vec3d p5(32, 16, 16);
    const Vec3d p6(32, 16, 32);
    const Vec3d p7(32, 32, 32);
    const Vec3d p8(32, 32, 16);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6);
    p.addPoint(p7);
    p.addPoint(p8); // assertion failure here
}

TEST(PolyhedronTest, testMergeManyFacesAfterAddingPoint) {
    const Vec3d  p1(0.0,  0.0, 0.0);
    const Vec3d  p2(0.0,  0.0, 4.0);
    const Vec3d  p3(0.0, 10.0, 0.0);
    const Vec3d  p4(0.0, 10.0, 4.0);
    const Vec3d  p5(4.0,  0.0, 0.0);
    const Vec3d  p6(4.0,  0.0, 4.0);
    const Vec3d  p7(4.0, 10.0, 0.0);
    const Vec3d  p8(4.0, 10.0, 4.0);
    const Vec3d  p9(6.0,  4.0, 4.0);
    const Vec3d p10(6.0,  6.0, 4.0);
    const Vec3d p11(8.0,  5.0, 2.0);
    const Vec3d p12(8.0,  5.0, 4.0);
    
    Vec3d::List topOld;
    topOld.push_back(p2);
    topOld.push_back(p6);
    topOld.push_back(p9);
    topOld.push_back(p10);
    topOld.push_back(p8);
    topOld.push_back(p4);
    
    Vec3d::List topNew;
    topNew.push_back(p2);
    topNew.push_back(p6);
    topNew.push_back(p12);
    topNew.push_back(p8);
    topNew.push_back(p4);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6);
    p.addPoint(p7);
    p.addPoint(p8);
    p.addPoint(p9);
    p.addPoint(p10);
    p.addPoint(p11);
    
    ASSERT_EQ(11u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p5));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p7));
    ASSERT_TRUE(p.hasVertex(p8));
    ASSERT_TRUE(p.hasVertex(p9));
    ASSERT_TRUE(p.hasVertex(p10));
    ASSERT_TRUE(p.hasVertex(p11));

    ASSERT_EQ(20u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p2));
    ASSERT_TRUE(p.hasEdge(p1, p3));
    ASSERT_TRUE(p.hasEdge(p1, p5));
    ASSERT_TRUE(p.hasEdge(p2, p4));
    ASSERT_TRUE(p.hasEdge(p2, p6));
    ASSERT_TRUE(p.hasEdge(p3, p4));
    ASSERT_TRUE(p.hasEdge(p3, p7));
    ASSERT_TRUE(p.hasEdge(p4, p8));
    ASSERT_TRUE(p.hasEdge(p5, p6));
    ASSERT_TRUE(p.hasEdge(p5, p7));
    ASSERT_TRUE(p.hasEdge(p5, p11));
    ASSERT_TRUE(p.hasEdge(p6, p9));
    ASSERT_TRUE(p.hasEdge(p6, p11));
    ASSERT_TRUE(p.hasEdge(p7, p8));
    ASSERT_TRUE(p.hasEdge(p7, p11));
    ASSERT_TRUE(p.hasEdge(p8, p10));
    ASSERT_TRUE(p.hasEdge(p8, p11));
    ASSERT_TRUE(p.hasEdge(p9, p10));
    ASSERT_TRUE(p.hasEdge(p9, p11));
    ASSERT_TRUE(p.hasEdge(p10, p11));
    
    ASSERT_EQ(11u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p8, p7));
    ASSERT_TRUE(hasTriangleOf(p, p11,  p6,  p5));
    ASSERT_TRUE(hasTriangleOf(p, p11,  p9,  p6));
    ASSERT_TRUE(hasTriangleOf(p, p11, p10,  p9));
    ASSERT_TRUE(hasTriangleOf(p, p11,  p8, p10));
    ASSERT_TRUE(hasTriangleOf(p, p11,  p7,  p8));
    ASSERT_TRUE(hasTriangleOf(p, p11,  p5,  p7));
    ASSERT_TRUE(p.hasFace(topOld));
    
    p.addPoint(p12);

    ASSERT_EQ(10u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_TRUE(p.hasVertex(p3));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p5));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p7));
    ASSERT_TRUE(p.hasVertex(p8));
    ASSERT_TRUE(p.hasVertex(p11));

    ASSERT_EQ(16u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge( p1, p2));
    ASSERT_TRUE(p.hasEdge( p1, p3));
    ASSERT_TRUE(p.hasEdge( p1, p5));
    ASSERT_TRUE(p.hasEdge( p2, p4));
    ASSERT_TRUE(p.hasEdge( p2, p6));
    ASSERT_TRUE(p.hasEdge( p3, p4));
    ASSERT_TRUE(p.hasEdge( p3, p7));
    ASSERT_TRUE(p.hasEdge( p4, p8));
    ASSERT_TRUE(p.hasEdge( p5, p6));
    ASSERT_TRUE(p.hasEdge( p5, p7));
    ASSERT_TRUE(p.hasEdge( p5, p11));
    ASSERT_TRUE(p.hasEdge( p7, p8));
    ASSERT_TRUE(p.hasEdge( p7, p11));
    ASSERT_TRUE(p.hasEdge( p6, p12));
    ASSERT_TRUE(p.hasEdge( p8, p12));
    ASSERT_TRUE(p.hasEdge(p11, p12));

    ASSERT_EQ(8u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p8, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p11, p12, p6));
    ASSERT_TRUE(hasQuadOf(p, p7, p8, p12, p11));
    ASSERT_TRUE(hasTriangleOf(p, p11,  p5,  p7));
    ASSERT_TRUE(p.hasFace(topNew));
}

TEST(PolyhedronTest, crashWhileAddingPoints1) {
    const Vec3d p1(224, 336, 0);
    const Vec3d p2(272, 320, 0);
    const Vec3d p3(-96, 352, 128);
    const Vec3d p4(192, 192, 128);
    const Vec3d p5(256, 256, 128);
    const Vec3d p6(320, 480, 128);
    const Vec3d p7(320, 256, 128);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6);
    p.addPoint(p7); // Assertion failure here.
}

TEST(PolyhedronTest, crashWhileAddingPoints2) {
    const Vec3d  p1(256, 39, 160);
    const Vec3d  p2(256, 0, 160);
    const Vec3d  p3(256,  0, 64);
    const Vec3d  p4(256, 39, 64);
    const Vec3d  p5(  0,  0, 160);
    const Vec3d  p6(  0, 32, 160);
    const Vec3d  p7(  0,  0, 64);
    const Vec3d  p8(  0, 32, 64);
    const Vec3d  p9(  0,  0, 0);
    const Vec3d p10(  0, 32, 0);
    const Vec3d p11(256, 32, 0);
    const Vec3d p12(256,  0, 0);
    const Vec3d p13(  0, 39, 64);
    const Vec3d p14(  0, 39, 160);
    const Vec3d p15(  0, 39, 0);

    Polyhedron3d p;
    
    p.addPoint(p1);
    ASSERT_TRUE(p.point());
    ASSERT_TRUE(p.hasVertex(p1));
    
    //p.addPoint(p2);
    //p.addPoint(p3);
    p.addPoint(p4);
    ASSERT_TRUE(p.edge());
    ASSERT_EQ(2u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_EQ(1u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    
    //p.addPoint(p5);
    p.addPoint(p6);
    ASSERT_TRUE(p.polygon());
    ASSERT_EQ(3u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_EQ(3u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p1, p6));
    ASSERT_TRUE(p.hasEdge(p4, p6));
    
    //p.addPoint(p7);
    //p.addPoint(p8);
    p.addPoint(p9);
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(4u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p9));
    ASSERT_EQ(6u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p1, p6));
    ASSERT_TRUE(p.hasEdge(p1, p9));
    ASSERT_TRUE(p.hasEdge(p4, p6));
    ASSERT_TRUE(p.hasEdge(p4, p9));
    ASSERT_TRUE(p.hasEdge(p6, p9));
    ASSERT_EQ(4u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p4, p6));
    ASSERT_TRUE(hasTriangleOf(p, p1, p6, p9));
    ASSERT_TRUE(hasTriangleOf(p, p4, p9, p6));
    
    p.addPoint(p10);
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(5u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p9));
    ASSERT_TRUE(p.hasVertex(p10));
    ASSERT_EQ(8u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p1, p6));
    ASSERT_TRUE(p.hasEdge(p1, p9));
    ASSERT_TRUE(p.hasEdge(p4, p9));
    ASSERT_TRUE(p.hasEdge(p6, p9));
    ASSERT_TRUE(p.hasEdge(p4, p10));
    ASSERT_TRUE(p.hasEdge(p6, p10));
    ASSERT_TRUE(p.hasEdge(p9, p10));
    ASSERT_EQ(5u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p6, p9));
    ASSERT_TRUE(hasTriangleOf(p, p4, p9, p10));
    ASSERT_TRUE(hasTriangleOf(p, p6, p10, p9));
    ASSERT_TRUE(hasQuadOf(p, p1, p4, p10, p6));

    //p.addPoint(p11);
    //p.addPoint(p12);
    p.addPoint(p13);
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(6u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p9));
    ASSERT_TRUE(p.hasVertex(p10));
    ASSERT_TRUE(p.hasVertex(p13));
    ASSERT_EQ(11u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p1, p6));
    ASSERT_TRUE(p.hasEdge(p1, p9));
    ASSERT_TRUE(p.hasEdge(p1, p13));
    ASSERT_TRUE(p.hasEdge(p4, p9));
    ASSERT_TRUE(p.hasEdge(p4, p10));
    ASSERT_TRUE(p.hasEdge(p4, p13));
    ASSERT_TRUE(p.hasEdge(p6, p9));
    ASSERT_TRUE(p.hasEdge(p6, p13));
    ASSERT_TRUE(p.hasEdge(p9, p10));
    ASSERT_TRUE(p.hasEdge(p10, p13));
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p6, p9));
    ASSERT_TRUE(hasTriangleOf(p, p1, p13, p6));
    ASSERT_TRUE(hasTriangleOf(p, p1, p4, p13));
    ASSERT_TRUE(hasTriangleOf(p, p4, p10, p13));
    ASSERT_TRUE(hasTriangleOf(p, p4, p9, p10));
    ASSERT_TRUE(hasQuadOf(p, p6, p13, p10, p9));

    p.addPoint(p14);
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(7u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p9));
    ASSERT_TRUE(p.hasVertex(p10));
    ASSERT_TRUE(p.hasVertex(p13));
    ASSERT_TRUE(p.hasVertex(p14));
    ASSERT_EQ(12u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p1, p6));
    ASSERT_TRUE(p.hasEdge(p1, p9));
    ASSERT_TRUE(p.hasEdge(p1, p14));
    ASSERT_TRUE(p.hasEdge(p4, p9));
    ASSERT_TRUE(p.hasEdge(p4, p10));
    ASSERT_TRUE(p.hasEdge(p4, p13));
    ASSERT_TRUE(p.hasEdge(p6, p9));
    ASSERT_TRUE(p.hasEdge(p6, p14));
    ASSERT_TRUE(p.hasEdge(p9, p10));
    ASSERT_TRUE(p.hasEdge(p10, p13));
    ASSERT_TRUE(p.hasEdge(p13, p14));
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p14, p6));
    ASSERT_TRUE(hasQuadOf(p, p1, p4, p13, p14));
    ASSERT_TRUE(hasTriangleOf(p, p1, p6, p9));
    ASSERT_TRUE(hasTriangleOf(p, p1, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p4, p10, p13));
    ASSERT_TRUE(hasTriangleOf(p, p4, p9, p10));
    ASSERT_TRUE(hasPolygonOf(p, p6, p14, p13, p10, p9));

    p.addPoint(p15); // Assertion failure here.
    ASSERT_TRUE(p.polyhedron());
    ASSERT_EQ(6u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p4));
    ASSERT_TRUE(p.hasVertex(p6));
    ASSERT_TRUE(p.hasVertex(p9));
    ASSERT_TRUE(p.hasVertex(p14));
    ASSERT_TRUE(p.hasVertex(p15));
    ASSERT_EQ(10u, p.edgeCount());
    ASSERT_TRUE(p.hasEdge(p1, p4));
    ASSERT_TRUE(p.hasEdge(p1, p6));
    ASSERT_TRUE(p.hasEdge(p1, p9));
    ASSERT_TRUE(p.hasEdge(p1, p14));
    ASSERT_TRUE(p.hasEdge(p4, p9));
    ASSERT_TRUE(p.hasEdge(p4, p15));
    ASSERT_TRUE(p.hasEdge(p6, p9));
    ASSERT_TRUE(p.hasEdge(p6, p14));
    ASSERT_TRUE(p.hasEdge(p9, p15));
    ASSERT_TRUE(p.hasEdge(p14, p15));
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasTriangleOf(p, p1, p14, p6));
    ASSERT_TRUE(hasQuadOf(p, p1, p4, p15, p14));
    ASSERT_TRUE(hasTriangleOf(p, p1, p6, p9));
    ASSERT_TRUE(hasTriangleOf(p, p1, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p4, p9, p15));
    ASSERT_TRUE(hasQuadOf(p, p6, p14, p15, p9));
}

TEST(PolyhedronTest, crashWhileAddingPoints3) {
    const Vec3d  p1(256, 39, 160);
    const Vec3d  p2(256, 0, 160);
    const Vec3d  p3(256,  0, 64);
    const Vec3d  p4(256, 39, 64);
    const Vec3d  p5(  0,  0, 160);
    const Vec3d  p6(  0, 32, 160);
    const Vec3d  p7(  0,  0, 64);
    const Vec3d  p8(  0, 32, 64);
    const Vec3d  p9(  0,  0, 0);
    const Vec3d p10(  0, 32, 0);
    const Vec3d p11(256, 32, 0);
    const Vec3d p12(256,  0, 0);
    const Vec3d p13(  0, 39, 64);
    const Vec3d p14(  0, 39, 160);
    const Vec3d p15(  0, 39, 0);
    
    Polyhedron3d p;
    
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    p.addPoint(p5);
    p.addPoint(p6);
    p.addPoint(p7);
    p.addPoint(p8);
    p.addPoint(p9);
    p.addPoint(p10);
    p.addPoint(p11);
    p.addPoint(p12);
    p.addPoint(p13);
    p.addPoint(p14);
    p.addPoint(p15); // Assertion failure here.
}

TEST(PolyhedronTest, crashWhileAddingPoints4) {
    //
    // p2 .  |  . p3
    //       |
    //    -------
    //       |
    // p1 .  |  . p4
    //
    const Vec3d p1(-1, -1, 0);
    const Vec3d p2(-1, +1, 0);
    const Vec3d p3(+1, +1, 0);
    const Vec3d p4(+1, -1, 0);
    const Vec3d p5( 0,  0, 0);
    
    Polyhedron3d p;
    
    p.addPoint(p1);
    p.addPoint(p4);
    p.addPoint(p2);
    ASSERT_TRUE(hasTriangleOf(p, p1, p4, p2));
    
    p.addPoint(p3);
    ASSERT_TRUE(hasQuadOf(p, p1, p4, p3, p2));
    
    p.addPoint(p5); // Assertion failure here.
    ASSERT_TRUE(hasQuadOf(p, p1, p4, p3, p2));
}

TEST(PolyhedronTest, crashWhileAddingPoints5) {
    // https://github.com/kduske/TrenchBroom/issues/1573
    
    const Vec3d p1(2, 0, 0);
    const Vec3d p2(0, 1, 0);
    const Vec3d p3(2, 1, 0); // Triangle (p1, p2, p3)
    const Vec3d p4(1, 1, 0); // Colinear along (p2, p3) edge
    
    Polyhedron3d p;
    
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    p.addPoint(p2); // Assertion failure here - re-adding p2
}

TEST(PolyhedronTest, removeVertexFromPoint) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p1);
    p.removeVertex(v);
    
    ASSERT_TRUE(p.empty());
}

TEST(PolyhedronTest, removeVertexFromEdge) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    const Vec3d p2(+64.0,   0.0,   0.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p2);
    p.removeVertex(v);
    
    ASSERT_TRUE(p.point());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_FALSE(p.hasVertex(p2));
}

TEST(PolyhedronTest, removeVertexFromTriangle) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    const Vec3d p2(+64.0,   0.0,   0.0);
    const Vec3d p3(+64.0, +64.0,   0.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p3);
    p.removeVertex(v);
    
    ASSERT_TRUE(p.edge());
    ASSERT_TRUE(p.hasVertex(p1));
    ASSERT_TRUE(p.hasVertex(p2));
    ASSERT_FALSE(p.hasVertex(p3));
}

TEST(PolyhedronTest, removeVertexFromSquare) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    const Vec3d p2(+64.0,   0.0,   0.0);
    const Vec3d p3(+64.0, +64.0,   0.0);
    const Vec3d p4(  0.0, +64.0,   0.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p3);
    p.removeVertex(v);
    
    ASSERT_TRUE(p.polygon());
    ASSERT_TRUE(hasTriangleOf(p, p1, p2, p4) || hasTriangleOf(p, p1, p4, p2));
    ASSERT_FALSE(p.hasVertex(p3));
}

TEST(PolyhedronTest, removeVertexFromTetrahedron) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    const Vec3d p2(+64.0,   0.0,   0.0);
    const Vec3d p3(  0.0, +64.0,   0.0);
    const Vec3d p4(  0.0,   0.0, +64.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p4);
    p.removeVertex(v);
    
    ASSERT_TRUE(p.polygon());
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p2) || hasTriangleOf(p, p1, p2, p3));
}

TEST(PolyhedronTest, removeVertexFromCube) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    const Vec3d p2(  0.0,   0.0, +64.0);
    const Vec3d p3(  0.0, +64.0,   0.0);
    const Vec3d p4(  0.0, +64.0, +64.0);
    const Vec3d p5(+64.0,   0.0,   0.0);
    const Vec3d p6(+64.0,   0.0, +64.0);
    const Vec3d p7(+64.0, +64.0,   0.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p8);
    p.removeVertex(v);
    
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p4, p6, p7));
}

TEST(PolyhedronTest, removeVertexFromCubeWithRoof) {
    const Vec3d p1(  0.0,   0.0,   0.0);
    const Vec3d p2(  0.0,   0.0, +64.0);
    const Vec3d p3(  0.0, +64.0,   0.0);
    const Vec3d p4(  0.0, +64.0, +64.0);
    const Vec3d p5(+64.0,   0.0,   0.0);
    const Vec3d p6(+64.0,   0.0, +64.0);
    const Vec3d p7(+64.0, +64.0,   0.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    const Vec3d p9(+32.0, +32.0, +96.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    positions.push_back(p9);
    
    Polyhedron3d p(positions);
    
    Vertex* v = p.findVertexByPosition(p9);
    p.removeVertex(v);
    
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2)); // front
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5)); // bottom
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3)); // left
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p8, p4)); // top
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p8, p6)); // right
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p8, p7)); // back
}

TEST(PolyhedronTest, removeVertexFromClippedCube) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +00.0); // note this vertex
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    
    Polyhedron3d p(positions);

    Vertex* v = p.findVertexByPosition(p8);
    p.removeVertex(v);
    
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2)); // front
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5)); // bottom
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3)); // left
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6)); // right
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4)); // top
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7)); // back
    ASSERT_TRUE(hasTriangleOf(p, p4, p6, p7)); // slanted
}

class ClipCallback : public Polyhedron3d::Callback {
private:
    typedef std::set<Face*> FaceSet;
    FaceSet m_originals;
public:
    void faceWillBeDeleted(Face* face) {
        ASSERT_TRUE(m_originals.find(face) == std::end(m_originals));
    }

    void faceWasSplit(Face* original, Face* clone) {
        m_originals.insert(original);
    }
};

TEST(PolyhedronTest, clipCubeWithHorizontalPlane) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    
    Polyhedron3d p(positions);
    
    const Plane3d plane(Vec3d::Null, Vec3d::PosZ);
    ClipCallback callback;
    
    ASSERT_TRUE(p.clip(plane, callback).success());
    
    const Vec3d d(0.0, 0.0, -64.0);
    ASSERT_EQ(12u, p.edgeCount());
    ASSERT_TRUE(hasEdge(p, p1,     p2 + d));
    ASSERT_TRUE(hasEdge(p, p1,     p3));
    ASSERT_TRUE(hasEdge(p, p1,     p5));
    ASSERT_TRUE(hasEdge(p, p2 + d, p4 + d));
    ASSERT_TRUE(hasEdge(p, p2 + d, p6 + d));
    ASSERT_TRUE(hasEdge(p, p3,     p4 + d));
    ASSERT_TRUE(hasEdge(p, p3,     p7));
    ASSERT_TRUE(hasEdge(p, p4 + d, p8 + d));
    ASSERT_TRUE(hasEdge(p, p5,     p6 + d));
    ASSERT_TRUE(hasEdge(p, p5,     p7));
    ASSERT_TRUE(hasEdge(p, p6 + d, p8 + d));

    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1,     p2 + d, p4 + d, p3));
    ASSERT_TRUE(hasQuadOf(p, p1,     p3,     p7,     p5));
    ASSERT_TRUE(hasQuadOf(p, p1,     p5,     p6 + d, p2 + d));
    ASSERT_TRUE(hasQuadOf(p, p2 + d, p6 + d, p8 + d, p4 + d));
    ASSERT_TRUE(hasQuadOf(p, p3,     p4 + d, p8 + d, p7));
    ASSERT_TRUE(hasQuadOf(p, p5,     p7,     p8 + d, p6 + d));
}

TEST(PolyhedronTest, clipCubeWithHorizontalPlaneAtTop) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    
    Polyhedron3d p(positions);
    
    const Plane3d plane(Vec3d(0.0, 0.0, 64.0), Vec3d::PosZ);
    ClipCallback callback;
    
    ASSERT_TRUE(p.clip(plane, callback).unchanged());
    
    ASSERT_EQ(12u, p.edgeCount());
    ASSERT_TRUE(hasEdge(p, p1, p2));
    ASSERT_TRUE(hasEdge(p, p1, p3));
    ASSERT_TRUE(hasEdge(p, p1, p5));
    ASSERT_TRUE(hasEdge(p, p2, p4));
    ASSERT_TRUE(hasEdge(p, p2, p6));
    ASSERT_TRUE(hasEdge(p, p3, p4));
    ASSERT_TRUE(hasEdge(p, p3, p7));
    ASSERT_TRUE(hasEdge(p, p4, p8));
    ASSERT_TRUE(hasEdge(p, p5, p6));
    ASSERT_TRUE(hasEdge(p, p5, p7));
    ASSERT_TRUE(hasEdge(p, p6, p8));
    
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p8, p4));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p8, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p8, p6));
}

TEST(PolyhedronTest, clipCubeWithHorizontalPlaneAboveTop) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    
    Polyhedron3d p(positions);
    
    const Plane3d plane(Vec3d(0.0, 0.0, 72.0), Vec3d::PosZ);
    ClipCallback callback;
    
    ASSERT_TRUE(p.clip(plane, callback).unchanged());
    
    ASSERT_EQ(12u, p.edgeCount());
    ASSERT_TRUE(hasEdge(p, p1, p2));
    ASSERT_TRUE(hasEdge(p, p1, p3));
    ASSERT_TRUE(hasEdge(p, p1, p5));
    ASSERT_TRUE(hasEdge(p, p2, p4));
    ASSERT_TRUE(hasEdge(p, p2, p6));
    ASSERT_TRUE(hasEdge(p, p3, p4));
    ASSERT_TRUE(hasEdge(p, p3, p7));
    ASSERT_TRUE(hasEdge(p, p4, p8));
    ASSERT_TRUE(hasEdge(p, p5, p6));
    ASSERT_TRUE(hasEdge(p, p5, p7));
    ASSERT_TRUE(hasEdge(p, p6, p8));
    
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p8, p4));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p8, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p8, p6));
}

TEST(PolyhedronTest, clipCubeWithHorizontalPlaneAtBottom) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    
    Polyhedron3d p(positions);
    
    const Plane3d plane(Vec3d(0.0, 0.0, -64.0), Vec3d::PosZ);
    ClipCallback callback;

    ASSERT_TRUE(p.clip(plane, callback).empty());
}

TEST(PolyhedronTest, clipCubeWithSlantedPlane) {
    Polyhedron3d p(BBox3d(64.0));
    
    const Plane3d plane(Vec3d(64.0, 64.0, 0.0), Vec3d(1.0, 1.0, 1.0).normalized());
    ClipCallback callback;
    
    ASSERT_TRUE(p.clip(plane, callback).success());
    
    const Vec3d  p1(-64.0, -64.0, -64.0);
    const Vec3d  p2(-64.0, -64.0, +64.0);
    const Vec3d  p3(-64.0, +64.0, -64.0);
    const Vec3d  p4(-64.0, +64.0, +64.0);
    const Vec3d  p5(+64.0, -64.0, -64.0);
    const Vec3d  p6(+64.0, -64.0, +64.0);
    const Vec3d  p7(+64.0, +64.0, -64.0);
    const Vec3d  p9(+64.0,   0.0, +64.0);
    const Vec3d p10(  0.0, +64.0, +64.0);
    const Vec3d p11(+64.0, +64.0,   0.0);

    ASSERT_EQ(10u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex( p1));
    ASSERT_TRUE(p.hasVertex( p2));
    ASSERT_TRUE(p.hasVertex( p3));
    ASSERT_TRUE(p.hasVertex( p4));
    ASSERT_TRUE(p.hasVertex( p5));
    ASSERT_TRUE(p.hasVertex( p6));
    ASSERT_TRUE(p.hasVertex( p7));
    ASSERT_TRUE(p.hasVertex( p9));
    ASSERT_TRUE(p.hasVertex(p10));
    ASSERT_TRUE(p.hasVertex(p11));
    
    ASSERT_EQ(15u, p.edgeCount());
    ASSERT_TRUE(hasEdge(p,  p1,  p2));
    ASSERT_TRUE(hasEdge(p,  p1,  p3));
    ASSERT_TRUE(hasEdge(p,  p1,  p5));
    ASSERT_TRUE(hasEdge(p,  p2,  p4));
    ASSERT_TRUE(hasEdge(p,  p2,  p6));
    ASSERT_TRUE(hasEdge(p,  p3,  p4));
    ASSERT_TRUE(hasEdge(p,  p3,  p7));
    ASSERT_TRUE(hasEdge(p,  p4, p10));
    ASSERT_TRUE(hasEdge(p,  p5,  p6));
    ASSERT_TRUE(hasEdge(p,  p5,  p7));
    ASSERT_TRUE(hasEdge(p,  p6,  p9));
    ASSERT_TRUE(hasEdge(p,  p7, p11));
    ASSERT_TRUE(hasEdge(p,  p9, p10));
    ASSERT_TRUE(hasEdge(p,  p9, p11));
    ASSERT_TRUE(hasEdge(p, p10, p11));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p,  p1,  p3,  p7,  p5));
    ASSERT_TRUE(hasQuadOf(p,  p1,  p5,  p6,  p2));
    ASSERT_TRUE(hasQuadOf(p,  p1,  p2,  p4,  p3));
    ASSERT_TRUE(hasPolygonOf(p,  p2,  p6,  p9, p10,  p4));
    ASSERT_TRUE(hasPolygonOf(p,  p3,  p4, p10, p11,  p7));
    ASSERT_TRUE(hasPolygonOf(p,  p5,  p7, p11,  p9,  p6));
    ASSERT_TRUE(hasTriangleOf(p, p9, p11, p10));
}

TEST(PolyhedronTest, clipCubeDiagonally) {
    Polyhedron3d p(BBox3d(64.0));
    
    const Plane3d plane(Vec3d::Null, Vec3d(1.0, 1.0, 0.0).normalized());
    ClipCallback callback;

    ASSERT_TRUE(p.clip(plane, callback).success());
    
    const Vec3d  p1(-64.0, -64.0, -64.0);
    const Vec3d  p2(-64.0, -64.0, +64.0);
    const Vec3d  p3(-64.0, +64.0, -64.0);
    const Vec3d  p4(-64.0, +64.0, +64.0);
    const Vec3d  p5(+64.0, -64.0, -64.0);
    const Vec3d  p6(+64.0, -64.0, +64.0);
    
    ASSERT_EQ(6u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex( p1));
    ASSERT_TRUE(p.hasVertex( p2));
    ASSERT_TRUE(p.hasVertex( p3));
    ASSERT_TRUE(p.hasVertex( p4));
    ASSERT_TRUE(p.hasVertex( p5));
    ASSERT_TRUE(p.hasVertex( p6));
    
    ASSERT_EQ(9u, p.edgeCount());
    ASSERT_TRUE(hasEdge(p, p1, p2));
    ASSERT_TRUE(hasEdge(p, p1, p3));
    ASSERT_TRUE(hasEdge(p, p1, p5));
    ASSERT_TRUE(hasEdge(p, p2, p4));
    ASSERT_TRUE(hasEdge(p, p2, p6));
    ASSERT_TRUE(hasEdge(p, p3, p4));
    ASSERT_TRUE(hasEdge(p, p3, p5));
    ASSERT_TRUE(hasEdge(p, p4, p6));
    ASSERT_TRUE(hasEdge(p, p5, p6));
    
    ASSERT_EQ(5u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p6, p5));
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
}

TEST(PolyhedronTest, clipCubeWithVerticalSlantedPlane) {
    Polyhedron3d p(BBox3d(64.0));
    
    const Plane3d plane(Vec3d(  0.0, -64.0, 0.0), Vec3d(2.0, 1.0, 0.0).normalized());
    ClipCallback callback;

    ASSERT_TRUE(p.clip(plane, callback).success());
    
    const Vec3d  p1(-64.0, -64.0, -64.0);
    const Vec3d  p2(-64.0, -64.0, +64.0);
    const Vec3d  p3(-64.0, +64.0, -64.0);
    const Vec3d  p4(-64.0, +64.0, +64.0);
    const Vec3d  p5(  0.0, -64.0, -64.0);
    const Vec3d  p6(  0.0, -64.0, +64.0);
    
    ASSERT_EQ(6u, p.vertexCount());
    ASSERT_TRUE(p.hasVertex( p1));
    ASSERT_TRUE(p.hasVertex( p2));
    ASSERT_TRUE(p.hasVertex( p3));
    ASSERT_TRUE(p.hasVertex( p4));
    ASSERT_TRUE(p.hasVertex( p5));
    ASSERT_TRUE(p.hasVertex( p6));
    
    ASSERT_EQ(9u, p.edgeCount());
    ASSERT_TRUE(hasEdge(p, p1, p2));
    ASSERT_TRUE(hasEdge(p, p1, p3));
    ASSERT_TRUE(hasEdge(p, p1, p5));
    ASSERT_TRUE(hasEdge(p, p2, p4));
    ASSERT_TRUE(hasEdge(p, p2, p6));
    ASSERT_TRUE(hasEdge(p, p3, p4));
    ASSERT_TRUE(hasEdge(p, p3, p5));
    ASSERT_TRUE(hasEdge(p, p4, p6));
    ASSERT_TRUE(hasEdge(p, p5, p6));
    
    ASSERT_EQ(5u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p6, p5));
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
}

TEST(PolyhedronTest, badClip) {
    const Vec3d::List polyVertices = Vec3d::parseList("(42.343111906757798 -24.90770936530231 48) (-5.6569680341747599 2.8051472462014218 -48) (-5.6567586128027614 -49.450466294904317 -48) (19.543884272280891 -64 2.4012022379983975) (64 -37.411190147253905 48) (64 -37.411184396581227 46.058241521600749) (16.970735645328752 -10.25882837570019 -48) (-15.996232760046849 -43.48119425295382 -48) (19.543373293787141 -64 32.936432269212482) (8.4017750903182601 -31.43996828352385 48) (-39.598145767921849 -3.7271836202911599 -48) (-28.284087977216849 -36.386647152659414 -48) (19.543509018008759 -64 47.655300195644266) (19.681387204653735 -64 48) (11.313359105885354 -46.184610213813635 -48) (42.170501479615339 -64 13.71441369506833) (64 -64 46.458506734897242) (64 -64 48) (64 -40.963243586214006 42.982066058285824) (64 -50.475344214694601 34.745773336493968) (22.627205203363062 -26.588725604065875 -48) (19.915358366079595 -18.759196710165369 -48) (16.82318198217952 -36.641571668509357 -48) (30.54114372047146 -27.178907257955132 48) (-13.006693391918915 1.3907491999939996 -48)");
    
    Polyhedron3d poly(polyVertices);
    const Plane3d plane(-19.170582845718307, Vec3d(0.88388309419256438, 0.30618844562885328, -0.35355241699635576));
    
    ASSERT_NO_THROW(poly.clip(plane));
}

bool findAndRemove(Polyhedron3d::SubtractResult& result, const Vec3d::List& vertices);
bool findAndRemove(Polyhedron3d::SubtractResult& result, const Vec3d::List& vertices) {
    for (auto it = std::begin(result), end = std::end(result); it != end; ++it) {
        const Polyhedron3d& polyhedron = *it;
        if (polyhedron.hasVertices(vertices)) {
            result.erase(it);
            return true;
        }
    }
    
    return false;
}

TEST(PolyhedronTest, subtractInnerCuboidFromCuboid) {
    const Polyhedron3d minuend(BBox3d(32.0));
    const Polyhedron3d subtrahend(BBox3d(16.0));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);

    const Vec3d::List leftVertices = Vec3d::parseList("(-32 -32 -32) (-32 32 -32) (-32 -32 32) (-32 32 32) (-16 -32 -32) (-16 32 -32) (-16 32 32) (-16 -32 32)");
    const Vec3d::List rightVertices = Vec3d::parseList("(32 -32 32) (32 32 32) (16 -32 -32) (16 -32 32) (16 32 32) (16 32 -32) (32 32 -32) (32 -32 -32)");

    const Vec3d::List frontVertices = Vec3d::parseList("(16 -32 32) (16 -32 -32) (-16 -32 32) (-16 -32 -32) (-16 -16 32) (16 -16 32) (16 -16 -32) (-16 -16 -32)");
    const Vec3d::List backVertices = Vec3d::parseList("(16 32 -32) (16 32 32) (-16 16 -32) (16 16 -32) (16 16 32) (-16 16 32) (-16 32 32) (-16 32 -32)");
    
    const Vec3d::List topVertices = Vec3d::parseList("(-16 16 32) (16 16 32) (16 -16 32) (-16 -16 32) (-16 -16 16) (-16 16 16) (16 16 16) (16 -16 16)");
    const Vec3d::List bottomVertices = Vec3d::parseList("(-16 -16 -32) (16 -16 -32) (-16 16 -32) (16 16 -32) (-16 -16 -16) (16 -16 -16) (16 16 -16) (-16 16 -16)");
    
    ASSERT_TRUE(findAndRemove(result, leftVertices));
    ASSERT_TRUE(findAndRemove(result, rightVertices));
    ASSERT_TRUE(findAndRemove(result, frontVertices));
    ASSERT_TRUE(findAndRemove(result, backVertices));
    ASSERT_TRUE(findAndRemove(result, topVertices));
    ASSERT_TRUE(findAndRemove(result, bottomVertices));
    
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractDisjointCuboidFromCuboid) {
    const Polyhedron3d minuend(BBox3d(64.0));
    const Polyhedron3d subtrahend(BBox3d(Vec3d(96.0, 96.0, 96.0), Vec3d(128.0, 128.0, 128.0)));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractCuboidFromInnerCuboid) {
    const Polyhedron3d minuend(BBox3d(32.0));
    const Polyhedron3d subtrahend(BBox3d(64.0));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractCuboidFromIdenticalCuboid) {
    const Polyhedron3d minuend(BBox3d(64.0));
    const Polyhedron3d subtrahend(BBox3d(64.0));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractCuboidProtrudingThroughCuboid) {
    const Polyhedron3d    minuend(BBox3d(Vec3d(-32.0, -32.0, -16.0), Vec3d(32.0, 32.0, 16.0)));
    const Polyhedron3d subtrahend(BBox3d(Vec3d(-16.0, -16.0, -32.0), Vec3d(16.0, 16.0, 32.0)));

    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(4u, result.size());
    
    const Vec3d::List leftVertices {
        Vec3d(-16, -32, -16),
        Vec3d(-16, 32, -16),
        Vec3d(-16, 32, 16),
        Vec3d(-16, -32, 16),
        Vec3d(-32, 32, 16),
        Vec3d(-32, -32, 16),
        Vec3d(-32, -32, -16),
        Vec3d(-32, 32, -16),
    };

    const Vec3d::List rightVertices {
        Vec3d(32, -32, 16),
        Vec3d(32, 32, 16),
        Vec3d(32, -32, -16),
        Vec3d(32, 32, -16),
        Vec3d(16, -32, -16),
        Vec3d(16, -32, 16),
        Vec3d(16, 32, 16),
        Vec3d(16, 32, -16)
    };
    
    const Vec3d::List frontVertices {
        Vec3d(-16, -32, -16),
        Vec3d(-16, -32, 16),
        Vec3d(16, -16, -16),
        Vec3d(-16, -16, -16),
        Vec3d(-16, -16, 16),
        Vec3d(16, -16, 16),
        Vec3d(16, -32, 16),
        Vec3d(16, -32, -16)
    };
    
    const Vec3d::List backVertices {
        Vec3d(-16, 32, 16),
        Vec3d(-16, 32, -16),
        Vec3d(16, 32, 16),
        Vec3d(16, 32, -16),
        Vec3d(16, 16, 16),
        Vec3d(-16, 16, 16),
        Vec3d(-16, 16, -16),
        Vec3d(16, 16, -16)
    };

    ASSERT_TRUE(findAndRemove(result, frontVertices));
    ASSERT_TRUE(findAndRemove(result, backVertices));
    ASSERT_TRUE(findAndRemove(result, leftVertices));
    ASSERT_TRUE(findAndRemove(result, rightVertices));
    
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractCuboidProtrudingFromCuboid) {
    /*
     ____________
     |          |
     |  ______  |
     |  |    |  |
     |__|    |__|
        |    |
        |____|
     */
    
    const Polyhedron3d    minuend(BBox3d(Vec3d(-32.0, -16.0, -32.0), Vec3d(32.0, 16.0, 32.0)));
    const Polyhedron3d subtrahend(BBox3d(Vec3d(-16.0, -32.0, -64.0), Vec3d(16.0, 32.0,  0.0)));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(3u, result.size());
}

TEST(PolyhedronTest, subtractCuboidProtrudingFromCuboid2) {
    /*
     ____________
     |          |
     |  ______  |
     |  |    |  |
     |__|____|__|
     */
    
    const Polyhedron3d    minuend(BBox3d(Vec3d(-64.0, -64.0, -16.0), Vec3d(64.0, 64.0, 16.0)));
    const Polyhedron3d subtrahend(BBox3d(Vec3d(-32.0, -64.0, -32.0), Vec3d(32.0,  0.0, 32.0)));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(3u, result.size());
}

TEST(PolyhedronTest, subtractCuboidFromCuboidWithCutCorners) {
    
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
        Vec3d(-32.0, -8.0,  0.0),
        Vec3d(+32.0, -8.0,  0.0),
        Vec3d(+32.0, -8.0, 32.0),
        Vec3d(+16.0, -8.0, 48.0),
        Vec3d(-16.0, -8.0, 48.0),
        Vec3d(-32.0, -8.0, 32.0),
        Vec3d(-32.0, +8.0,  0.0),
        Vec3d(+32.0, +8.0,  0.0),
        Vec3d(+32.0, +8.0, 32.0),
        Vec3d(+16.0, +8.0, 48.0),
        Vec3d(-16.0, +8.0, 48.0),
        Vec3d(-32.0, +8.0, 32.0)
    };
    
    const Polyhedron3d subtrahend(BBox3d(Vec3d(-16.0, -8.0, 0.0), Vec3d(16.0, 8.0, 32.0)));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    
    const Vec3d::List left  = Vec3d::parseList("(-16 8 -0) (-16 8 48) (-16 -8 48) (-16 -8 -0) (-32 -8 -0) (-32 -8 32) (-32 8 -0) (-32 8 32)");
    const Vec3d::List right = Vec3d::parseList("(32 -8 32) (32 8 32) (32 8 -0) (32 -8 -0) (16 8 48) (16 8 -0) (16 -8 -0) (16 -8 48)");
    const Vec3d::List top   = Vec3d::parseList("(16 8 32) (16 -8 32) (-16 -8 32) (-16 8 32) (-16 -8 48) (-16 8 48) (16 8 48) (16 -8 48)");
    
    ASSERT_TRUE(findAndRemove(result, left));
    ASSERT_TRUE(findAndRemove(result, right));
    ASSERT_TRUE(findAndRemove(result, top));
    
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractRhombusFromCuboid) {
    
    /*
     ______
     |    |
     | /\ |
     | \/ |
     |____|
     
     */

    const Vec3d::List subtrahendVertices = Vec3d::parseList("(-32.0 0.0 +96.0) (0.0 -32.0 +96.0) (+32.0 0.0 +96.0) (0.0 +32.0 +96.0) (-32.0 0.0 -96.0) (0.0 -32.0 -96.0) (+32.0 0.0 -96.0) (0.0 +32.0 -96.0)");
    
    const Polyhedron3d minuend(BBox3d(64.0));
    const Polyhedron3d subtrahend(subtrahendVertices);
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);

    ASSERT_TRUE(findAndRemove(result, Vec3d::parseList("(64 64 64) (-32 64 -64) (64 -32 -64) (64 -32 64) (-32 64 64) (64 64 -64)")));
    ASSERT_TRUE(findAndRemove(result, Vec3d::parseList("(-64 32 64) (-64 32 -64) (-32 -0 64) (-32 -0 -64) (-0 32 -64) (-0 32 64) (-64 64 64) (-32 64 -64) (-32 64 64) (-64 64 -64)")));
    ASSERT_TRUE(findAndRemove(result, Vec3d::parseList("(64 -32 64) (64 -32 -64) (64 -64 64) (64 -64 -64) (-0 -32 64) (32 -0 64) (32 -0 -64) (-0 -32 -64) (32 -64 -64) (32 -64 64)")));
    ASSERT_TRUE(findAndRemove(result, Vec3d::parseList("(-64 -64 64) (-64 -64 -64) (-64 32 -64) (-64 32 64) (32 -64 64) (32 -64 -64)")));
    
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractFailWithMissingFragments) {
    const Vec3d::List minuendVertices {
        Vec3d(-1056, 864, -192),
        Vec3d(-1024, 896, -192),
        Vec3d(-1024, 1073, -192),
        Vec3d(-1056, 1080, -192),
        Vec3d(-1024, 1073, -416),
        Vec3d(-1024, 896, -416),
        Vec3d(-1056, 864, -416),
        Vec3d(-1056, 1080, -416)
    };
    
    const Vec3d::List subtrahendVertices {
        Vec3d(-1088, 960, -288),
        Vec3d(-1008, 960, -288),
        Vec3d(-1008, 1024, -288),
        Vec3d(-1088, 1024, -288),
        Vec3d(-1008, 1024, -400),
        Vec3d(-1008, 960, -400),
        Vec3d(-1088, 960, -400),
        Vec3d(-1088, 1024, -400)
    };
    
    const Polyhedron3d minuend(minuendVertices);
    const Polyhedron3d subtrahend(subtrahendVertices);
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(4u, result.size());
}

TEST(PolyhedronTest, subtractTetrahedronFromCubeWithOverlappingFragments) {
    // see https://github.com/kduske/TrenchBroom/pull/1764#issuecomment-296342133
    // merge creates overlapping fragments
    
    const Vec3d::List minuendVertices = Vec3d::parseList("(-32 -32 32) (32 -32 32) (32 32 32) (-32 32 32) (32 32 -32) (32 -32 -32) (-32 -32 -32) (-32 32 -32)");
    const Vec3d::List subtrahendVertices = Vec3d::parseList("(-0 -16 -32) (-0 16 -32) (32 16 -32) (16 16 -0)");
    
    
    const Polyhedron3d minuend(minuendVertices);
    const Polyhedron3d subtrahend(subtrahendVertices);
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(3u, result.size());
}

TEST(PolyhedronTest, intersection_empty_polyhedron) {
    const Polyhedron3d empty;
    const Polyhedron3d point      { Vec3d(1.0, 0.0, 0.0) };
    const Polyhedron3d edge       { Vec3d(1.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0) };
    const Polyhedron3d polygon    { Vec3d(1.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0) };
    const Polyhedron3d polyhedron { Vec3d(1.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0) };
    
    assertNotIntersects(empty, empty);
    assertNotIntersects(empty, point);
    assertNotIntersects(empty, edge);
    assertNotIntersects(empty, polygon);
    assertNotIntersects(empty, polyhedron);
}

TEST(PolyhedronTest, intersection_point_point) {
    const Polyhedron3d point { Vec3d(0.0, 0.0, 0.0) };
    
    assertIntersects(point, point);
    assertNotIntersects(point, Polyhedron3d { Vec3d(0.0, 0.0, 1.0) });
}

TEST(PolyhedronTest, intersection_point_edge) {
    const Vec3d pointPos(0.0, 0.0, 0.0);
    const Polyhedron3d point { pointPos };

    assertIntersects(point, Polyhedron3d { pointPos, Vec3d(1.0, 0.0, 0.0) } ); // point / edge originating at point
    assertIntersects(point, Polyhedron3d { Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0) } ); // point / edge containing point
    assertNotIntersects(point, Polyhedron3d { Vec3d(-1.0, 0.0, 1.0), Vec3d(1.0, 0.0, 1.0) } ); // point / unrelated edge
}

TEST(PolyhedronTest, intersection_point_polygon) {
    const Vec3d pointPos(0.0, 0.0, 0.0);
    const Polyhedron3d point { pointPos };

    assertIntersects(point, Polyhedron3d { pointPos, Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0) } ); // point / triangle with point as vertex
    assertIntersects(point, Polyhedron3d { Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0) } ); // point / triangle with point on edge
    assertIntersects(point, Polyhedron3d { Vec3d(-1.0, -1.0, 0.0), Vec3d(1.0, -1.0, 0.0), Vec3d(0.0, 1.0, 0.0) } ); // point / triangle containing point
    
    assertNotIntersects(point, Polyhedron3d { Vec3d(-1.0, -1.0, 1.0), Vec3d(1.0, -1.0, 1.0), Vec3d(0.0, 1.0, 1.0) } ); // point / triangle above point
}

TEST(PolyhedronTest, intersection_point_polyhedron) {
    const Vec3d pointPos(0.0, 0.0, 0.0);
    const Polyhedron3d point { pointPos };
    
    assertIntersects(point, Polyhedron3d { pointPos, Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0) } ); // point / tetrahedron with point as vertex
    assertIntersects(point, Polyhedron3d { Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0) } ); // point / tetrahedron with point on edge
    assertIntersects(point, Polyhedron3d { Vec3d(-1.0, -1.0, 0.0), Vec3d(1.0, -1.0, 0.0), Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0) } ); // point / tetrahedron with point on face
    assertIntersects(point, Polyhedron3d { Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, -1.0, -1.0), Vec3d(0.0, 1.0, -1.0), Vec3d(0.0, 0.0, 1.0) } ); // point / tetrahedron with point on face
    
    assertNotIntersects(point, Polyhedron3d { Vec3d(-1.0, -1.0, 1.0), Vec3d(1.0, -1.0, 1.0), Vec3d(0.0, 1.0, 1.0), Vec3d(0.0, 0.0, 2.0) } ); // point / tetrahedron above point
}

TEST(PolyhedronTest, intersection_edge_edge) {
    const Vec3d point1(-1.0, 0.0, 0.0);
    const Vec3d point2(+1.0, 0.0, 0.0);
    const Polyhedron3d edge { point1, point2 };
    
    assertIntersects(edge, edge);
    assertIntersects(edge, Polyhedron3d { point1, Vec3d(0.0, 0.0, 1.0) } );
    assertIntersects(edge, Polyhedron3d { point2, Vec3d(0.0, 0.0, 1.0) } );
    assertIntersects(edge, Polyhedron3d { Vec3d(0.0, -1.0, 0.0), Vec3d(0.0, 1.0, 0.0) } );
    assertIntersects(edge, Polyhedron3d { Vec3d( 0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0) } );
    assertIntersects(edge, Polyhedron3d { Vec3d(-2.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0) } );
    
    assertNotIntersects(edge, Polyhedron3d { point1 + Vec3d::PosZ, point2 + Vec3d::PosZ } );
}

TEST(PolyhedronTest, intersection_edge_polygon_same_plane) {
    const Vec3d point1(-1.0, 0.0, 0.0);
    const Vec3d point2(+1.0, 0.0, 0.0);
    const Polyhedron3d edge { point1, point2 };
    
    assertIntersects(edge, Polyhedron3d { Vec3d(1.0, 0.0, 0.0), Vec3d(1.0, -1.0, 0.0), Vec3d(2.0, -1.0, 0.0), Vec3d(2.0, 0.0, 0.0) } ); // one shared point
    assertIntersects(edge, Polyhedron3d { Vec3d(-1.0, 0.0, 0.0), Vec3d(0.0, -1.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(0.0, +1.0, 0.0) } ); // two shared points
    assertIntersects(edge, Polyhedron3d { Vec3d(-1.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0), Vec3d(1.0, 1.0, 0.0), Vec3d(-1.0, 1.0, 0.0) } ); // shared edge
    assertIntersects(edge, Polyhedron3d { Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, -1.0, 0.0), Vec3d(2.0, -1.0, 0.0), Vec3d(2.0, 1.0, 0.0) } ); // polygon contains one point
    assertIntersects(edge, Polyhedron3d { Vec3d(-2.0, 1.0, 0.0), Vec3d(-2.0, -1.0, 0.0), Vec3d(2.0, -1.0, 0.0), Vec3d(2.0, 1.0, 0.0) } );// polygon contains both points
    assertIntersects(edge, Polyhedron3d { Vec3d(-0.5, 1.0, 0.0), Vec3d(-0.5, -1.0, 0.0), Vec3d(0.5, -1.0, 0.0), Vec3d(0.5, 1.0, 0.0) } ); // edge intersects polygon completely
    
    assertNotIntersects(edge, Polyhedron3d { Vec3d(+2.0, 1.0, 0.0), Vec3d(+2.0, -1.0, 0.0), Vec3d(+3.0, -1.0, 0.0), Vec3d(+3.0, 1.0, 0.0) } ); // no intersection
}


TEST(PolyhedronTest, intersection_edge_polygon_different_plane) {
    const Vec3d point1(0.0, 0.0, 1.0);
    const Vec3d point2(0.0, 0.0, -1.0);
    const Polyhedron3d edge { point1, point2 };

    assertIntersects(Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 0.0, +1.0) },
                     Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } ); // one shared point
    
    assertIntersects(Polyhedron3d { Vec3d(1.0, 0.0, 0.0), Vec3d(1.0, 0.0, +1.0) },
                     Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } ); // polygon edge contains edge origin
    
    assertIntersects(Polyhedron3d { Vec3d(1.0, 1.0, 0.0), Vec3d(1.0, 1.0, +1.0) },
                     Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } ); // polygon contains edge origin
    
    assertIntersects(Polyhedron3d { Vec3d(0.0, 0.0, -1.0), Vec3d(0.0, 0.0, +1.0) },
                     Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } ); // edge intersects polygon vertex
    
    assertIntersects(Polyhedron3d { Vec3d(1.0, 0.0, -1.0), Vec3d(1.0, 0.0, +1.0) },
                     Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } ); // edge intersects polygon edge
    
    assertIntersects(Polyhedron3d { Vec3d(1.0, 1.0, -1.0), Vec3d(1.0, 1.0, +1.0) },
                     Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } ); // edge intersects polygon center

    assertNotIntersects(Polyhedron3d { Vec3d(3.0, 1.0, -1.0), Vec3d(3.0, 1.0, +1.0) },
                        Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } );
    
    assertNotIntersects(Polyhedron3d { Vec3d(1.0, 1.0, 1.0), Vec3d(1.0, 1.0, 2.0) },
                        Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } );
    
    assertNotIntersects(Polyhedron3d { Vec3d(0.0, 0.0, 1.0), Vec3d(1.0, 1.0, 1.0) },
                        Polyhedron3d { Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 0.0, 0.0), Vec3d(2.0, 2.0, 0.0), Vec3d(0.0, 2.0, 0.0) } );
}

TEST(PolyhedronTest, intersection_edge_polyhedron) {
    const Polyhedron3d tetrahedron {
        Vec3d(-1.0, -1.0, 0.0),
        Vec3d(+1.0, -1.0, 0.0),
        Vec3d( 0.0, +1.0, 0.0),
        Vec3d( 0.0,  0.0, 1.0)
    };
    
    assertIntersects(Polyhedron3d { Vec3d( 0.0,  0.0,  1.0), Vec3d( 0.0,  0.0,  2.0) }, tetrahedron); // one shared point
    assertIntersects(Polyhedron3d { Vec3d( 0.0, -1.0,  0.0), Vec3d( 0.0, -2.0,  0.0) }, tetrahedron); // edge point on polyhedron edge
    assertIntersects(Polyhedron3d { Vec3d( 0.0,  0.0,  0.0), Vec3d( 0.0,  0.0, -1.0) }, tetrahedron); // edge point on polyhedron face
    assertIntersects(Polyhedron3d { Vec3d(-1.0, -1.0,  0.0), Vec3d(+1.0, -1.0,  0.0) }, tetrahedron); // shared edge
    assertIntersects(Polyhedron3d { Vec3d( 0.0,  0.0,  0.5), Vec3d( 0.0,  0.0,  2.0) }, tetrahedron); // polyhedron contains one edge point
    assertIntersects(Polyhedron3d { Vec3d( 0.0,  0.0,  0.2), Vec3d( 0.0,  0.0,  0.7) }, tetrahedron); // polyhedron contains both edge points
    assertIntersects(Polyhedron3d { Vec3d( 0.0,  0.0, -1.0), Vec3d( 0.0,  0.0,  2.0) }, tetrahedron); // edge penetrates polyhedron

    assertNotIntersects(Polyhedron3d { Vec3d( -2.0,  -2.0, -1.0), Vec3d( 2.0,  2.0,  -1.0) }, tetrahedron); // no intersection
}

TEST(PolyhedronTest, intersection_polygon_polygon_same_plane) {
    const Polyhedron3d square {
        Vec3d(-1.0, -1.0, 0.0),
        Vec3d(+1.0, -1.0, 0.0),
        Vec3d(+1.0, +1.0, 0.0),
        Vec3d(-1.0, +1.0, 0.0)
    };
    
    // shared vertex:
    assertIntersects(Polyhedron3d { Vec3d(+1, +1, 0), Vec3d(+2, +1, 0), Vec3d(+1, +2, 0) }, square);
    
    // shared edge
    assertIntersects(Polyhedron3d { Vec3d(-1, +1, 0), Vec3d(+1, +1, 0), Vec3d( 0, +2, 0) }, square);

    // edge contains other edge
    assertIntersects(Polyhedron3d {
        Vec3d(-2, -1, 0),
        Vec3d(+2, -1, 0),
        Vec3d(+2, +1, 0),
        Vec3d(-2, +1, 0),
    }, square);
    
    // one contains vertex of another
    assertIntersects(Polyhedron3d { Vec3d( 0,  0, 0), Vec3d(+2,  0, 0), Vec3d(+2, +2, 0), Vec3d(0, +2, 0) }, square);

    // one contains another entirely
    assertIntersects(Polyhedron3d { Vec3d(-2, -2, 0), Vec3d(+2, -2, 0), Vec3d(+2, +2, 0), Vec3d(-2, +2, 0) }, square);

    // one penetrates the other
    assertIntersects(Polyhedron3d {
        Vec3d(-2, -0.5, 0),
        Vec3d(+2, -0.5, 0),
        Vec3d(+2, +0.5, 0),
        Vec3d(-2, +0.5, 0)
    }, square);
    
    // no intersection
    assertNotIntersects(Polyhedron3d {
        Vec3d(+2, +2, 0),
        Vec3d(+3, +2, 0),
        Vec3d(+3, +3, 0),
        Vec3d(+3, +3, 0)
    }, square);
}

TEST(PolyhedronTest, intersection_polygon_polygon_different_plane) {
    const Polyhedron3d square {
        Vec3d(-1.0, -1.0, 0.0),
        Vec3d(+1.0, -1.0, 0.0),
        Vec3d(+1.0, +1.0, 0.0),
        Vec3d(-1.0, +1.0, 0.0)
    };
    
    // shared vertex
    assertIntersects(Polyhedron3d {
        Vec3d(-1.0, -1.0, 0.0),
        Vec3d(-2.0, -1.0, 0.0),
        Vec3d(-2.0, -1.0, 1.0)
    }, square);
    
    // vertex on edge
    assertIntersects(Polyhedron3d {
        Vec3d(0.0, -1.0, 0.0),
        Vec3d(0.0, -2.0, 0.0),
        Vec3d(0.0, -1.0, 1.0),
        Vec3d(0.0, -2.0, 1.0),
    }, square);
    
    // shared edge
    assertIntersects(Polyhedron3d {
        Vec3d(-1.0, -1.0, 0.0),
        Vec3d(+1.0, -1.0, 0.0),
        Vec3d(+1.0, -1.0, 1.0),
        Vec3d(-1.0, -1.0, 1.0)
    }, square);
    
    // edges intersect
    assertIntersects(Polyhedron3d {
        Vec3d(0.0, -1.0, -1.0),
        Vec3d(0.0, -1.0, +1.0),
        Vec3d(0.0, -2.0, +1.0),
        Vec3d(0.0, -2.0, -1.0)
    }, square);
    
    // partial penetration (one edge penetrates each)
    assertIntersects(Polyhedron3d {
        Vec3d(0.0, 0.0, -1.0),
        Vec3d(0.0, 0.0, +1.0),
        Vec3d(2.0, 0.0, +1.0),
        Vec3d(2.0, 0.0, -1.0)
    }, square);
    
    // full penetration (two edges penetrate)
    assertIntersects(Polyhedron3d {
        Vec3d(-2.0, 0.0, -2.0),
        Vec3d(-2.0, 0.0, +2.0),
        Vec3d(+2.0, 0.0, -2.0),
        Vec3d(+2.0, 0.0, +2.0)
    }, square);
    
    // no intersection
    assertNotIntersects(Polyhedron3d {
        Vec3d(-1.0, 0.0, 5.0),
        Vec3d(+1.0, 0.0, 5.0),
        Vec3d(-1.0, 0.0, 6.0),
        Vec3d(+1.0, 0.0, 6.0)
    }, square);
}

TEST(PolyhedronTest, intersection_polygon_polyhedron_same_plane_as_face) {
    const Polyhedron3d cube {
        Vec3d(-1.0, -1.0, -1.0),
        Vec3d(-1.0, -1.0, +1.0),
        Vec3d(-1.0, +1.0, -1.0),
        Vec3d(-1.0, +1.0, +1.0),
        Vec3d(+1.0, -1.0, -1.0),
        Vec3d(+1.0, -1.0, +1.0),
        Vec3d(+1.0, +1.0, -1.0),
        Vec3d(+1.0, +1.0, +1.0),
    };
    
    
    // polygon is on the same plane as top face
    
    // shared vertex
    assertIntersects(Polyhedron3d {
        Vec3d(+1.0, +1.0, +1.0),
        Vec3d(+2.0, +1.0, +1.0),
        Vec3d(+2.0, +2.0, +1.0),
    }, cube);
    
    // shared edge
    assertIntersects(Polyhedron3d {
        Vec3d(+1.0, +1.0, +1.0),
        Vec3d(-1.0, +1.0, +1.0),
        Vec3d(+1.0, +2.0, +1.0)
    }, cube);

    // edge contains other edge
    assertIntersects(Polyhedron3d {
        Vec3d(-0.5, +1.0, +1.0),
        Vec3d(+0.5, +1.0, +1.0),
        Vec3d(+0.5, +2.0, +1.0)
    }, cube);

    // one contains vertex of another
    assertIntersects(Polyhedron3d {
        Vec3d(+0.0, +0.0, +1.0),
        Vec3d(+2.0, +0.0, +1.0),
        Vec3d(+2.0, +2.0, +1.0),
        Vec3d(+0.0, +2.0, +1.0),
    }, cube);

    // one contains another entirely
    assertIntersects(Polyhedron3d {
        Vec3d(-0.5, -0.5, +1.0),
        Vec3d(-0.5, +0.5, +1.0),
        Vec3d(+0.5, +0.5, +1.0),
        Vec3d(+0.5, -0.5, +1.0),
    }, cube);
    assertIntersects(Polyhedron3d {
        Vec3d(-2.5, -2.5, +1.0),
        Vec3d(-2.5, +2.5, +1.0),
        Vec3d(+2.5, +2.5, +1.0),
        Vec3d(+2.5, -2.5, +1.0),
    }, cube);
    
    // one penetrates the other
    assertIntersects(Polyhedron3d {
        Vec3d(-2.0, -0.5, +1.0),
        Vec3d(+2.0, -0.5, +1.0),
        Vec3d(-2.0, +0.5, +1.0),
        Vec3d(+2.0, +0.5, +1.0),
    }, cube);
    
    // no intersection
    assertNotIntersects(Polyhedron3d {
        Vec3d(+2.0, +2.0, +1.0),
        Vec3d(+3.0, +2.0, +1.0),
        Vec3d(+3.0, +3.0, +1.0),
        Vec3d(+2.0, +3.0, +1.0),
    }, cube);
}

TEST(PolyhedronTest, intersection_polygon_polyhedron_any_orientation) {
    const Polyhedron3d cube {
        Vec3d(-1.0, -1.0, -1.0),
        Vec3d(-1.0, -1.0, +1.0),
        Vec3d(-1.0, +1.0, -1.0),
        Vec3d(-1.0, +1.0, +1.0),
        Vec3d(+1.0, -1.0, -1.0),
        Vec3d(+1.0, -1.0, +1.0),
        Vec3d(+1.0, +1.0, -1.0),
        Vec3d(+1.0, +1.0, +1.0),
    };
    
    // shared vertex
    assertIntersects(Polyhedron3d {
        Vec3d(+1.0, +1.0, +1.0),
        Vec3d(+2.0, +1.0, +2.0),
        Vec3d(+2.0, +2.0, +2.0)
    }, cube);
    
    // polygon vertex on polyhedron edge
    assertIntersects(Polyhedron3d {
        Vec3d(+0.0, +1.0, +1.0),
        Vec3d(+2.0, +1.0, +2.0),
        Vec3d(+2.0, +2.0, +2.0)
    }, cube);

    // polyhedron vertex on polygon edge
    assertIntersects(Polyhedron3d {
        Vec3d(0.0, 2.0, 1.0),
        Vec3d(2.0, 0.0, 1.0),
        Vec3d(0.0, 0.0, 2.0)
    }, cube);
    
    // shared edge
    assertIntersects(Polyhedron3d {
        Vec3d(-1.0, 1.0, 1.0),
        Vec3d(+1.0, 1.0, 1.0),
        Vec3d( 0.0, 2.0, 2.0)
    }, cube);

    
    // polygon edge inside polyhedron edge
    assertIntersects(Polyhedron3d {
        Vec3d(-0.5, 1.0, 1.0),
        Vec3d(+0.5, 1.0, 1.0),
        Vec3d( 0.0, 2.0, 2.0),
    }, cube);

    
    // polyhedorn edge inside polygon edge
    assertIntersects(Polyhedron3d {
        Vec3d(-2.0, 1.0, 1.0),
        Vec3d(+2.0, 1.0, 1.0),
        Vec3d( 0.0, 2.0, 2.0)
    }, cube);
    
    // edges intersect
    assertIntersects(Polyhedron3d {
        Vec3d(0.0, -2.0, 0.0),
        Vec3d(0.0,  0.0, 2.0),
        Vec3d(0.0, -2.0, 2.0)
    }, cube);
    
    // penetration (two polygon edges intersect)
    assertIntersects(Polyhedron3d {
        Vec3d(0.0,  0.0, 0.0),
        Vec3d(0.0, -3.0, 0.0),
        Vec3d(3.0,  0.0, 2.0),
    }, cube);
    
    // polyhedron contains polygon
    assertIntersects(Polyhedron3d {
        Vec3d(-0.5, 0.0, 0.0),
        Vec3d( 0.0, 0.5, 0.0),
        Vec3d( 0.0, 0.0, 0.5)
    }, cube);
    
    // polygon slices polyhedron (surrounds it)
    assertIntersects(Polyhedron3d {
        Vec3d(-2.0, -2.0, 0.0),
        Vec3d(-2.0, +2.0, 0.0),
        Vec3d(+2.0, -2.0, 0.0),
        Vec3d(+2.0, +2.0, 0.0),
    }, cube);
}

bool hasVertex(const Polyhedron3d& p, const Vec3d& point) {
    return p.hasVertex(point);
}

bool hasVertices(const Polyhedron3d& p, const Vec3d::List& points) {
    if (p.vertexCount() != points.size())
        return false;
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (!hasVertex(p, points[i]))
            return false;
    }
    return true;
}

bool hasEdge(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2) {
    return p.hasEdge(p1, p2);
}

bool hasEdges(const Polyhedron3d& p, const EdgeInfoList& edgeInfos) {
    if (p.edgeCount() != edgeInfos.size())
        return false;
    
    for (size_t i = 0; i < edgeInfos.size(); ++i) {
        if (!hasEdge(p, edgeInfos[i].first, edgeInfos[i].second))
            return false;
    }
    return true;
}

bool hasTriangleOf(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3) {
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    return p.hasFace(points);
}

bool hasQuadOf(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3, const Vec3d& p4) {
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    return p.hasFace(points);
}

bool hasPolygonOf(const Polyhedron3d& p, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3, const Vec3d& p4, const Vec3d& p5) {
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    return p.hasFace(points);
}


void assertIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs) {
    ASSERT_TRUE(lhs.intersects(rhs));
    ASSERT_TRUE(lhs.intersects(rhs) == rhs.intersects(lhs));
}

void assertNotIntersects(const Polyhedron3d& lhs, const Polyhedron3d& rhs) {
    ASSERT_FALSE(lhs.intersects(rhs));
    ASSERT_TRUE(lhs.intersects(rhs) == rhs.intersects(lhs));
}
