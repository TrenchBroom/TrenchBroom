/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

TEST(PolyhedronTest, moveSingleVertex) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(32.0, -16.0, 8.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p2 - p1, true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p2, result.newVertexPositions.front());
    
    ASSERT_TRUE(p.point());
}

TEST(PolyhedronTest, moveEdgeVertexWithoutMerge) {
    const Vec3d p1( 0.0, 0.0, 0.0);
    const Vec3d p2(32.0, 0.0, 0.0);
    const Vec3d p3(32.0, 32.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p3 - p1, true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p3, result.newVertexPositions.front());
    
    ASSERT_TRUE(p.edge());
}

TEST(PolyhedronTest, moveEdgeVertexWithMerge) {
    const Vec3d p1( 0.0, 0.0, 0.0);
    const Vec3d p2(32.0, 0.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p2 - p1, true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p2, result.newVertexPositions.front());
    
    ASSERT_TRUE(p.point());
}

TEST(PolyhedronTest, movePolygonVertexToNonCoplanarPosition) {
    const Vec3d p1( 0.0,  0.0,  0.0);
    const Vec3d p2(32.0,  0.0,  0.0);
    const Vec3d p3(32.0, 32.0,  0.0);
    const Vec3d p4( 0.0,  0.0, 16.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p4 - p1, true);
    ASSERT_TRUE(!result.allVerticesMoved());
    ASSERT_EQ(1u, result.unchangedVertices.size());
    ASSERT_VEC_EQ(p1, result.unchangedVertices.front());
    
    ASSERT_TRUE(p.polygon());
}

TEST(PolyhedronTest, movePolygonVertexWithoutMerge) {
    const Vec3d p1(  0.0,  0.0,  0.0);
    const Vec3d p2( 32.0,  0.0,  0.0);
    const Vec3d p3( 32.0, 32.0,  0.0);
    const Vec3d p4(-32.0,  0.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p4 - p1, true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p4, result.newVertexPositions.front());
    
    ASSERT_TRUE(p.polygon());
}

TEST(PolyhedronTest, movePolygonVertexToNonIncidentVertex) {
    const Vec3d p1(  0.0,  0.0,  0.0);
    const Vec3d p2( 32.0,  0.0,  0.0);
    const Vec3d p3( 32.0, 32.0,  0.0);
    const Vec3d p4(  0.0, 32.0,  0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p3 - p1, true);
    ASSERT_FALSE(result.allVerticesMoved());
    ASSERT_TRUE(result.newVertexPositions.empty());
    ASSERT_EQ(1u, result.unchangedVertices.size());
    ASSERT_VEC_EQ(p1, result.unchangedVertices.front());
    
    ASSERT_TRUE(p.polygon());
}

TEST(PolyhedronTest, movePolygonVertexToIncidentVertex) {
    const Vec3d p1(  0.0,  0.0,  0.0);
    const Vec3d p2( 32.0,  0.0,  0.0);
    const Vec3d p3( 32.0, 32.0,  0.0);
    const Vec3d p4(  0.0, 32.0, 0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    p.addPoint(p4);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p2 - p1, true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p2, result.newVertexPositions.front());

    ASSERT_TRUE(p.polygon());
    ASSERT_EQ(3u, p.vertexCount());
}

TEST(PolyhedronTest, movePolygonVertex) {
    const Vec3d p1(  0.0,  0.0,  0.0);
    const Vec3d p2( 32.0,  0.0,  0.0);
    const Vec3d p3( 32.0, 32.0,  0.0);
    const Vec3d p4( 64.0,  0.0,  0.0);
    
    Polyhedron3d p;
    p.addPoint(p1);
    p.addPoint(p2);
    p.addPoint(p3);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p1), p4 - p1, true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p4, result.newVertexPositions.front());
    
    ASSERT_TRUE(p.polygon());
    ASSERT_EQ(3u, p.vertexCount());
}

TEST(PolyhedronTest, moveVertexDownWithoutMerges) {
    Polyhedron3d p(BBox3d(-64.0, 64.0));
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, Vec3d(64.0, 64.0, 64.0)), Vec3d(-8.0, -8.0, -8.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(Vec3d(56.0, 56.0, 56.0), result.newVertexPositions.front());
    
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+56.0, +56.0, +56.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p6));
    edgeInfos.push_back(std::make_pair(p4, p7));
    edgeInfos.push_back(std::make_pair(p4, p8));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p7));
    edgeInfos.push_back(std::make_pair(p6, p8));
    edgeInfos.push_back(std::make_pair(p7, p8));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(9u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p8, p6, p7));
    ASSERT_TRUE(hasTriangleOf(p, p8, p4, p6));
    ASSERT_TRUE(hasTriangleOf(p, p8, p7, p4));
}

TEST(PolyhedronTest, moveVertexUpWithoutMerges) {
    Polyhedron3d p(BBox3d(-64.0, 64.0));
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, Vec3d(64.0, 64.0, 64.0)), Vec3d(+8.0, +8.0, +8.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(Vec3d(72.0, 72.0, 72.0), result.newVertexPositions.front());
    
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+72.0, +72.0, +72.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    positions.push_back(p6);
    positions.push_back(p7);
    positions.push_back(p8);
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p2, p8));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p3, p8));
    edgeInfos.push_back(std::make_pair(p4, p8));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p5, p8));
    edgeInfos.push_back(std::make_pair(p6, p8));
    edgeInfos.push_back(std::make_pair(p7, p8));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(9u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p8));
    ASSERT_TRUE(hasTriangleOf(p, p2, p8, p4));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p8));
    ASSERT_TRUE(hasTriangleOf(p, p3, p8, p7));
    ASSERT_TRUE(hasTriangleOf(p, p5, p8, p6));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p8));
}

TEST(PolyhedronTest, moveVertexWithOneOuterNeighbourMerge) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+56.0, +56.0, +56.0);
    const Vec3d p9(+56.0, +56.0, +64.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(0.0, 0.0, 8.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.back() = p9;
    ASSERT_TRUE(hasVertices(p, positions));

    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p7));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p7));
    edgeInfos.push_back(std::make_pair(p6, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));

    ASSERT_EQ(8u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p9, p6, p7));
    ASSERT_TRUE(hasTriangleOf(p, p9, p7, p4));
}

TEST(PolyhedronTest, moveVertexWithTwoOuterNeighbourMerges) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+56.0, +56.0, +56.0);
    const Vec3d p9(+64.0, +64.0, +56.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(8.0, 8.0, 0.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.back() = p9;
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p6));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p9, p6));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p9, p7));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
    ASSERT_TRUE(hasTriangleOf(p, p9, p4, p6));
}

TEST(PolyhedronTest, moveVertexWithAllOuterNeighbourMerges) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+56.0, +56.0, +56.0);
    const Vec3d p9(+64.0, +64.0, +64.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(8.0, 8.0, 8.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.back() = p9;
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p9, p4));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p9, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p9, p6));
}

TEST(PolyhedronTest, moveVertexWithInnerNeighbourMerges) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +72.0);
    const Vec3d p9(+64.0, +64.0, +64.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(0.0, 0.0, -8.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.back() = p9;
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p9, p4));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p9, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p9, p6));
}

TEST(PolyhedronTest, moveVertexWithAllInnerNeighbourMerge) {
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(-64.0, -64.0, -64.0), true);
    ASSERT_TRUE(result.hasDeletedVertices());
    ASSERT_EQ(1u, result.deletedVertices.size());
    ASSERT_VEC_EQ(p8, result.deletedVertices.front());

    positions.pop_back();
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p6));
    edgeInfos.push_back(std::make_pair(p4, p7));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p7));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6));
    ASSERT_TRUE(hasTriangleOf(p, p4, p6, p7));
}

TEST(PolyhedronTest, moveVertexUpThroughPlane) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +56.0);
    const Vec3d p9(+64.0, +64.0, +72.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(0.0, 0.0, 16.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.back() = p9;
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p2, p9));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p9, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p9, p6));
    ASSERT_TRUE(hasTriangleOf(p, p2, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p9));
}

TEST(PolyhedronTest, moveVertexOntoEdge) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +00.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(-64.0, -64.0, 64.0), true);
    ASSERT_TRUE(result.hasDeletedVertices());
    ASSERT_EQ(1u, result.deletedVertices.size());
    ASSERT_VEC_EQ(p8, result.deletedVertices.front());
    
    positions.pop_back();
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p6));
    edgeInfos.push_back(std::make_pair(p4, p7));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p7));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6));
    ASSERT_TRUE(hasTriangleOf(p, p4, p6, p7));
}

TEST(PolyhedronTest, moveVertexOntoIncidentVertex) {
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, Vec3d(64.0, 64.0, 64.0)), Vec3d(0.0, 0.0, -128.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p7, result.newVertexPositions.front());
    
    positions.pop_back();
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p6));
    edgeInfos.push_back(std::make_pair(p4, p7));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p6, p7));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasTriangleOf(p, p2, p6, p4));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p6));
    ASSERT_TRUE(hasTriangleOf(p, p4, p6, p7));
}


TEST(PolyhedronTest, moveVertexOntoIncidentVertexInOppositeDirection) {
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, Vec3d(+64.0, +64.0, -64.0)), Vec3d(0.0, 0.0, +128.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p8, result.newVertexPositions.front());
    
    positions.pop_back();
    positions.back() = p8;
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p6));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p5));
    edgeInfos.push_back(std::make_pair(p3, p8));
    edgeInfos.push_back(std::make_pair(p4, p8));
    edgeInfos.push_back(std::make_pair(p5, p6));
    edgeInfos.push_back(std::make_pair(p5, p8));
    edgeInfos.push_back(std::make_pair(p6, p8));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p8, p4));
    ASSERT_TRUE(hasTriangleOf(p, p1, p3, p5));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p8));
    ASSERT_TRUE(hasTriangleOf(p, p5, p8, p6));
    ASSERT_TRUE(hasTriangleOf(p, p3, p8, p5));
}

TEST(PolyhedronTest, moveVertexAndMergeColinearEdgesWithoutDeletingVertex) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    const Vec3d p9(+80.0, +64.0, +64.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p6), Vec3d(16.0, 128.0, 0.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.pop_back(); // p8 will be erased due to colinear incident edges
    positions[5] = p9; // p6 was moved to p9
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p5));
    edgeInfos.push_back(std::make_pair(p2, p9));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p5, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p9, p7));
    ASSERT_TRUE(hasTriangleOf(p, p1, p5, p2));
    ASSERT_TRUE(hasTriangleOf(p, p2, p5, p9));
    ASSERT_TRUE(hasTriangleOf(p, p2, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p9));
}

TEST(PolyhedronTest, moveVertexAndMergeColinearEdgesWithoutDeletingVertex2) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    const Vec3d p9(+80.0, -64.0, +64.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p8), Vec3d(16.0, -128.0, 0.0), true);
    ASSERT_TRUE(result.allVerticesMoved());
    ASSERT_EQ(1u, result.newVertexPositions.size());
    ASSERT_VEC_EQ(p9, result.newVertexPositions.front());
    
    positions.erase(positions.begin() + 5); // p6 will be erased due to colinear incident edges
    positions[6] = p9; // p8 was moved to p9
    ASSERT_TRUE(hasVertices(p, positions));
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(p1, p2));
    edgeInfos.push_back(std::make_pair(p1, p3));
    edgeInfos.push_back(std::make_pair(p1, p5));
    edgeInfos.push_back(std::make_pair(p2, p4));
    edgeInfos.push_back(std::make_pair(p2, p9));
    edgeInfos.push_back(std::make_pair(p3, p4));
    edgeInfos.push_back(std::make_pair(p3, p7));
    edgeInfos.push_back(std::make_pair(p4, p7));
    edgeInfos.push_back(std::make_pair(p4, p9));
    edgeInfos.push_back(std::make_pair(p5, p7));
    edgeInfos.push_back(std::make_pair(p5, p9));
    edgeInfos.push_back(std::make_pair(p7, p9));
    ASSERT_TRUE(hasEdges(p, edgeInfos));
    
    ASSERT_EQ(7u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p9, p2));
    ASSERT_TRUE(hasTriangleOf(p, p2, p9, p4));
    ASSERT_TRUE(hasTriangleOf(p, p3, p4, p7));
    ASSERT_TRUE(hasTriangleOf(p, p4, p9, p7));
    ASSERT_TRUE(hasTriangleOf(p, p5, p7, p9));
}

TEST(PolyhedronTest, moveVertexAndMergeColinearEdgesWithDeletingVertex) {
    const Vec3d p1(-64.0, -64.0, -64.0);
    const Vec3d p2(-64.0, -64.0, +64.0);
    const Vec3d p3(-64.0, +64.0, -64.0);
    const Vec3d p4(-64.0, +64.0, +64.0);
    const Vec3d p5(+64.0, -64.0, -64.0);
    const Vec3d p6(+64.0, -64.0, +64.0);
    const Vec3d p7(+64.0, +64.0, -64.0);
    const Vec3d p8(+64.0, +64.0, +64.0);
    const Vec3d p9(+80.0,   0.0, +64.0);
    
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
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p9), Vec3d(-16.0, 0.0, 0.0), true);
    ASSERT_TRUE(result.hasDeletedVertices());
    ASSERT_EQ(1u, result.deletedVertices.size());
    ASSERT_VEC_EQ(p9, result.deletedVertices.front());

    positions.pop_back();
    ASSERT_TRUE(hasVertices(p, positions));
    
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
    
    ASSERT_EQ(6u, p.faceCount());
    ASSERT_TRUE(hasQuadOf(p, p1, p2, p4, p3));
    ASSERT_TRUE(hasQuadOf(p, p1, p3, p7, p5));
    ASSERT_TRUE(hasQuadOf(p, p1, p5, p6, p2));
    ASSERT_TRUE(hasQuadOf(p, p2, p6, p8, p4));
    ASSERT_TRUE(hasQuadOf(p, p3, p4, p8, p7));
    ASSERT_TRUE(hasQuadOf(p, p5, p7, p8, p6));
}

TEST(PolyhedronTest, moveVertexFailing1) {
    const Vec3d p1(  0.0, +32.0, +32.0);
    const Vec3d p2(  0.0,   0.0,   0.0);
    const Vec3d p3(  0.0, +32.0,   0.0);
    const Vec3d p4(+32.0, +32.0,   0.0);
    const Vec3d p5(+32.0,   0.0,   0.0);
    
    Vec3d::List positions;
    positions.push_back(p1);
    positions.push_back(p2);
    positions.push_back(p3);
    positions.push_back(p4);
    positions.push_back(p5);
    
    Polyhedron3d p(positions);
    
    const Polyhedron3d::MoveVerticesResult result = p.moveVertices(Vec3d::List(1, p2), p1-p2, true);
}

class ClipCallback : public Polyhedron3d::Callback {
private:
    typedef std::set<Face*> FaceSet;
    FaceSet m_originals;
public:
    void faceWillBeDeleted(Face* face) {
        ASSERT_TRUE(m_originals.find(face) == m_originals.end());
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

bool findAndRemove(Polyhedron3d::SubtractResult& result, const Vec3d::List& vertices);
bool findAndRemove(Polyhedron3d::SubtractResult& result, const Vec3d::List& vertices) {
    Polyhedron3d::SubtractResult::iterator it, end;
    for (it = result.begin(), end = result.end(); it != end; ++it) {
        const Polyhedron3d& polyhedron = *it;
        if (polyhedron.vertices().size() == vertices.size()) {
            size_t count = 0;
            
            for (size_t j = 0; j < vertices.size(); ++j) {
                if (!polyhedron.hasVertex(vertices[j], 0.0))
                    break;
                ++count;
            }
            
            if (count == vertices.size()) {
                result.erase(it);
                return true;
            }
        }
    }
    
    return false;
}

TEST(PolyhedronTest, subtractInnerCuboidFromCuboid) {
    const Polyhedron3d minuend(BBox3d(64.0));
    const Polyhedron3d subtrahend(BBox3d(32.0));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(6u, result.size());

    Vec3d::List frontVertices;
    frontVertices.push_back(Vec3d(-64.0, -64.0, -64.0));
    frontVertices.push_back(Vec3d(-64.0, -64.0, +64.0));
    frontVertices.push_back(Vec3d(+64.0, -64.0, -64.0));
    frontVertices.push_back(Vec3d(+64.0, -64.0, +64.0));
    frontVertices.push_back(Vec3d(-32.0, -32.0, -32.0));
    frontVertices.push_back(Vec3d(-32.0, -32.0, +32.0));
    frontVertices.push_back(Vec3d(+32.0, -32.0, -32.0));
    frontVertices.push_back(Vec3d(+32.0, -32.0, +32.0));

    Vec3d::List backVertices;
    backVertices.push_back(Vec3d(-64.0, +64.0, -64.0));
    backVertices.push_back(Vec3d(-64.0, +64.0, +64.0));
    backVertices.push_back(Vec3d(+64.0, +64.0, -64.0));
    backVertices.push_back(Vec3d(+64.0, +64.0, +64.0));
    backVertices.push_back(Vec3d(-32.0, +32.0, -32.0));
    backVertices.push_back(Vec3d(-32.0, +32.0, +32.0));
    backVertices.push_back(Vec3d(+32.0, +32.0, -32.0));
    backVertices.push_back(Vec3d(+32.0, +32.0, +32.0));
    
    Vec3d::List leftVertices;
    leftVertices.push_back(Vec3d(-64.0, -64.0, -64.0));
    leftVertices.push_back(Vec3d(-64.0, -64.0, +64.0));
    leftVertices.push_back(Vec3d(-64.0, +64.0, -64.0));
    leftVertices.push_back(Vec3d(-64.0, +64.0, +64.0));
    leftVertices.push_back(Vec3d(-32.0, -32.0, -32.0));
    leftVertices.push_back(Vec3d(-32.0, -32.0, +32.0));
    leftVertices.push_back(Vec3d(-32.0, +32.0, -32.0));
    leftVertices.push_back(Vec3d(-32.0, +32.0, +32.0));
    
    Vec3d::List rightVertices;
    rightVertices.push_back(Vec3d(+64.0, -64.0, -64.0));
    rightVertices.push_back(Vec3d(+64.0, -64.0, +64.0));
    rightVertices.push_back(Vec3d(+64.0, +64.0, -64.0));
    rightVertices.push_back(Vec3d(+64.0, +64.0, +64.0));
    rightVertices.push_back(Vec3d(+32.0, -32.0, -32.0));
    rightVertices.push_back(Vec3d(+32.0, -32.0, +32.0));
    rightVertices.push_back(Vec3d(+32.0, +32.0, -32.0));
    rightVertices.push_back(Vec3d(+32.0, +32.0, +32.0));

    Vec3d::List bottomVertices;
    bottomVertices.push_back(Vec3d(-64.0, -64.0, -64.0));
    bottomVertices.push_back(Vec3d(-64.0, +64.0, -64.0));
    bottomVertices.push_back(Vec3d(+64.0, -64.0, -64.0));
    bottomVertices.push_back(Vec3d(+64.0, +64.0, -64.0));
    bottomVertices.push_back(Vec3d(-32.0, -32.0, -32.0));
    bottomVertices.push_back(Vec3d(-32.0, +32.0, -32.0));
    bottomVertices.push_back(Vec3d(+32.0, -32.0, -32.0));
    bottomVertices.push_back(Vec3d(+32.0, +32.0, -32.0));

    Vec3d::List topVertices;
    topVertices.push_back(Vec3d(-64.0, -64.0, +64.0));
    topVertices.push_back(Vec3d(-64.0, +64.0, +64.0));
    topVertices.push_back(Vec3d(+64.0, -64.0, +64.0));
    topVertices.push_back(Vec3d(+64.0, +64.0, +64.0));
    topVertices.push_back(Vec3d(-32.0, -32.0, +32.0));
    topVertices.push_back(Vec3d(-32.0, +32.0, +32.0));
    topVertices.push_back(Vec3d(+32.0, -32.0, +32.0));
    topVertices.push_back(Vec3d(+32.0, +32.0, +32.0));

    ASSERT_TRUE(findAndRemove(result, frontVertices));
    ASSERT_TRUE(findAndRemove(result, backVertices));
    ASSERT_TRUE(findAndRemove(result, leftVertices));
    ASSERT_TRUE(findAndRemove(result, rightVertices));
    ASSERT_TRUE(findAndRemove(result, bottomVertices));
    ASSERT_TRUE(findAndRemove(result, topVertices));
    
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, subtractDisjunctCuboidFromCuboid) {
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
    
    Vec3d::List frontVertices;
    frontVertices.push_back(Vec3d(-32.0, -32.0, -16.0));
    frontVertices.push_back(Vec3d(+32.0, -32.0, -16.0));
    frontVertices.push_back(Vec3d(+16.0, -16.0, -16.0));
    frontVertices.push_back(Vec3d(-16.0, -16.0, -16.0));
    frontVertices.push_back(Vec3d(-32.0, -32.0, +16.0));
    frontVertices.push_back(Vec3d(+32.0, -32.0, +16.0));
    frontVertices.push_back(Vec3d(+16.0, -16.0, +16.0));
    frontVertices.push_back(Vec3d(-16.0, -16.0, +16.0));

    Vec3d::List backVertices;
    backVertices.push_back(Vec3d(-32.0, +32.0, -16.0));
    backVertices.push_back(Vec3d(+32.0, +32.0, -16.0));
    backVertices.push_back(Vec3d(+16.0, +16.0, -16.0));
    backVertices.push_back(Vec3d(-16.0, +16.0, -16.0));
    backVertices.push_back(Vec3d(-32.0, +32.0, +16.0));
    backVertices.push_back(Vec3d(+32.0, +32.0, +16.0));
    backVertices.push_back(Vec3d(+16.0, +16.0, +16.0));
    backVertices.push_back(Vec3d(-16.0, +16.0, +16.0));

    Vec3d::List leftVertices;
    leftVertices.push_back(Vec3d(-32.0, -32.0, -16.0));
    leftVertices.push_back(Vec3d(-32.0, +32.0, -16.0));
    leftVertices.push_back(Vec3d(-16.0, -16.0, -16.0));
    leftVertices.push_back(Vec3d(-16.0, +16.0, -16.0));
    leftVertices.push_back(Vec3d(-32.0, -32.0, +16.0));
    leftVertices.push_back(Vec3d(-32.0, +32.0, +16.0));
    leftVertices.push_back(Vec3d(-16.0, -16.0, +16.0));
    leftVertices.push_back(Vec3d(-16.0, +16.0, +16.0));

    Vec3d::List rightVertices;
    rightVertices.push_back(Vec3d(+32.0, -32.0, -16.0));
    rightVertices.push_back(Vec3d(+32.0, +32.0, -16.0));
    rightVertices.push_back(Vec3d(+16.0, -16.0, -16.0));
    rightVertices.push_back(Vec3d(+16.0, +16.0, -16.0));
    rightVertices.push_back(Vec3d(+32.0, -32.0, +16.0));
    rightVertices.push_back(Vec3d(+32.0, +32.0, +16.0));
    rightVertices.push_back(Vec3d(+16.0, -16.0, +16.0));
    rightVertices.push_back(Vec3d(+16.0, +16.0, +16.0));

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
    
    Polyhedron3d minuend;
    minuend.addPoint(Vec3d(-32.0, -8.0,  0.0));
    minuend.addPoint(Vec3d(+32.0, -8.0,  0.0));
    minuend.addPoint(Vec3d(+32.0, -8.0, 32.0));
    minuend.addPoint(Vec3d(+16.0, -8.0, 48.0));
    minuend.addPoint(Vec3d(-16.0, -8.0, 48.0));
    minuend.addPoint(Vec3d(-32.0, -8.0, 32.0));
    minuend.addPoint(Vec3d(-32.0, +8.0,  0.0));
    minuend.addPoint(Vec3d(+32.0, +8.0,  0.0));
    minuend.addPoint(Vec3d(+32.0, +8.0, 32.0));
    minuend.addPoint(Vec3d(+16.0, +8.0, 48.0));
    minuend.addPoint(Vec3d(-16.0, +8.0, 48.0));
    minuend.addPoint(Vec3d(-32.0, +8.0, 32.0));
    
    Polyhedron3d subtrahend(BBox3d(Vec3d(-16.0, -8.0, 0.0), Vec3d(16.0, 8.0, 32.0)));
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(3u, result.size());
    
    Vec3d::List leftVertices;
    leftVertices.push_back(Vec3d(-32.0, -8.0, 0.0));
    leftVertices.push_back(Vec3d(-16.0, -8.0, 0.0));
    leftVertices.push_back(Vec3d(-16.0, -8.0, 32.0));
    leftVertices.push_back(Vec3d(-32.0, -8.0, 32.0));
    leftVertices.push_back(Vec3d(-32.0, +8.0, 0.0));
    leftVertices.push_back(Vec3d(-16.0, +8.0, 0.0));
    leftVertices.push_back(Vec3d(-16.0, +8.0, 32.0));
    leftVertices.push_back(Vec3d(-32.0, +8.0, 32.0));
    
    Vec3d::List rightVertices;
    rightVertices.push_back(Vec3d(+32.0, -8.0, 0.0));
    rightVertices.push_back(Vec3d(+16.0, -8.0, 0.0));
    rightVertices.push_back(Vec3d(+16.0, -8.0, 32.0));
    rightVertices.push_back(Vec3d(+32.0, -8.0, 32.0));
    rightVertices.push_back(Vec3d(+32.0, +8.0, 0.0));
    rightVertices.push_back(Vec3d(+16.0, +8.0, 0.0));
    rightVertices.push_back(Vec3d(+16.0, +8.0, 32.0));
    rightVertices.push_back(Vec3d(+32.0, +8.0, 32.0));
    
    Vec3d::List topVertices;
    topVertices.push_back(Vec3d(+32.0, -8.0, 32.0));
    topVertices.push_back(Vec3d(-32.0, -8.0, 32.0));
    topVertices.push_back(Vec3d(+16.0, -8.0, 48.0));
    topVertices.push_back(Vec3d(-16.0, -8.0, 48.0));
    topVertices.push_back(Vec3d(+32.0, +8.0, 32.0));
    topVertices.push_back(Vec3d(-32.0, +8.0, 32.0));
    topVertices.push_back(Vec3d(+16.0, +8.0, 48.0));
    topVertices.push_back(Vec3d(-16.0, +8.0, 48.0));

    ASSERT_TRUE(findAndRemove(result, leftVertices));
    ASSERT_TRUE(findAndRemove(result, rightVertices));
    ASSERT_TRUE(findAndRemove(result, topVertices));
    
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

    Vec3d::List subtrahendVertices;
    subtrahendVertices.push_back(Vec3d(-32.0,   0.0, +96.0));
    subtrahendVertices.push_back(Vec3d(  0.0, -32.0, +96.0));
    subtrahendVertices.push_back(Vec3d(+32.0,   0.0, +96.0));
    subtrahendVertices.push_back(Vec3d(  0.0, +32.0, +96.0));
    subtrahendVertices.push_back(Vec3d(-32.0,   0.0, -96.0));
    subtrahendVertices.push_back(Vec3d(  0.0, -32.0, -96.0));
    subtrahendVertices.push_back(Vec3d(+32.0,   0.0, -96.0));
    subtrahendVertices.push_back(Vec3d(  0.0, +32.0, -96.0));
    
    const Polyhedron3d minuend(BBox3d(64.0));
    const Polyhedron3d subtrahend(subtrahendVertices);
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(8u, result.size());

    Vec3d::List leftWedge;
    leftWedge.push_back(Vec3d(-64.0, -64.0, -64.0));
    leftWedge.push_back(Vec3d(-32.0,   0.0, -64.0));
    leftWedge.push_back(Vec3d(-64.0, +64.0, -64.0));
    leftWedge.push_back(Vec3d(-64.0, -64.0, +64.0));
    leftWedge.push_back(Vec3d(-32.0,   0.0, +64.0));
    leftWedge.push_back(Vec3d(-64.0, +64.0, +64.0));
    
    Vec3d::List bottomWedge;
    bottomWedge.push_back(Vec3d(-64.0, -64.0, -64.0));
    bottomWedge.push_back(Vec3d(+64.0, -64.0, -64.0));
    bottomWedge.push_back(Vec3d(  0.0, -32.0, -64.0));
    bottomWedge.push_back(Vec3d(-64.0, -64.0, +64.0));
    bottomWedge.push_back(Vec3d(+64.0, -64.0, +64.0));
    bottomWedge.push_back(Vec3d(  0.0, -32.0, +64.0));
    
    Vec3d::List rightWedge;
    rightWedge.push_back(Vec3d(+64.0, -64.0, -64.0));
    rightWedge.push_back(Vec3d(+64.0, +64.0, -64.0));
    rightWedge.push_back(Vec3d(+32.0,   0.0, -64.0));
    rightWedge.push_back(Vec3d(+64.0, -64.0, +64.0));
    rightWedge.push_back(Vec3d(+64.0, +64.0, +64.0));
    rightWedge.push_back(Vec3d(+32.0,   0.0, +64.0));
    
    Vec3d::List topWedge;
    topWedge.push_back(Vec3d(+64.0, +64.0, -64.0));
    topWedge.push_back(Vec3d(-64.0, +64.0, -64.0));
    topWedge.push_back(Vec3d(  0.0, +32.0, -64.0));
    topWedge.push_back(Vec3d(+64.0, +64.0, +64.0));
    topWedge.push_back(Vec3d(-64.0, +64.0, +64.0));
    topWedge.push_back(Vec3d(  0.0, +32.0, +64.0));

    Vec3d::List bottomLeftWedge;
    bottomLeftWedge.push_back(Vec3d(-64.0, -64.0, -64.0));
    bottomLeftWedge.push_back(Vec3d(  0.0, -32.0, -64.0));
    bottomLeftWedge.push_back(Vec3d(-32.0,   0.0, -64.0));
    bottomLeftWedge.push_back(Vec3d(-64.0, -64.0, +64.0));
    bottomLeftWedge.push_back(Vec3d(  0.0, -32.0, +64.0));
    bottomLeftWedge.push_back(Vec3d(-32.0,   0.0, +64.0));

    Vec3d::List bottomRightWedge;
    bottomRightWedge.push_back(Vec3d(+64.0, -64.0, -64.0));
    bottomRightWedge.push_back(Vec3d(+32.0,   0.0, -64.0));
    bottomRightWedge.push_back(Vec3d(  0.0, -32.0, -64.0));
    bottomRightWedge.push_back(Vec3d(+64.0, -64.0, +64.0));
    bottomRightWedge.push_back(Vec3d(+32.0,   0.0, +64.0));
    bottomRightWedge.push_back(Vec3d(  0.0, -32.0, +64.0));
    
    Vec3d::List topRightWedge;
    topRightWedge.push_back(Vec3d(+64.0, +64.0, -64.0));
    topRightWedge.push_back(Vec3d(  0.0, +32.0, -64.0));
    topRightWedge.push_back(Vec3d(+32.0,   0.0, -64.0));
    topRightWedge.push_back(Vec3d(+64.0, +64.0, +64.0));
    topRightWedge.push_back(Vec3d(  0.0, +32.0, +64.0));
    topRightWedge.push_back(Vec3d(+32.0,   0.0, +64.0));
    
    Vec3d::List topLeftWedge;
    topLeftWedge.push_back(Vec3d(-64.0, +64.0, -64.0));
    topLeftWedge.push_back(Vec3d(-32.0,   0.0, -64.0));
    topLeftWedge.push_back(Vec3d(  0.0, +32.0, -64.0));
    topLeftWedge.push_back(Vec3d(-64.0, +64.0, +64.0));
    topLeftWedge.push_back(Vec3d(-32.0,   0.0, +64.0));
    topLeftWedge.push_back(Vec3d(  0.0, +32.0, +64.0));
    
    ASSERT_TRUE(findAndRemove(result, bottomLeftWedge));
    ASSERT_TRUE(findAndRemove(result, bottomRightWedge));
    ASSERT_TRUE(findAndRemove(result, topRightWedge));
    ASSERT_TRUE(findAndRemove(result, topLeftWedge));
    ASSERT_TRUE(findAndRemove(result, leftWedge));
    ASSERT_TRUE(findAndRemove(result, bottomWedge));
    ASSERT_TRUE(findAndRemove(result, rightWedge));
    ASSERT_TRUE(findAndRemove(result, topWedge));
    
    ASSERT_TRUE(result.empty());
}

TEST(PolyhedronTest, mergeRemainingFragments) {
    Vec3d::List minuendVertices;
    minuendVertices.push_back(Vec3d(32, -64, 16));
    minuendVertices.push_back(Vec3d(64, -32, 16));
    minuendVertices.push_back(Vec3d(64, 32, 16));
    minuendVertices.push_back(Vec3d(32, 64, 16));
    minuendVertices.push_back(Vec3d(-64, 64, 16));
    minuendVertices.push_back(Vec3d(-64, -64, 16));
    minuendVertices.push_back(Vec3d(64, 32, -16));
    minuendVertices.push_back(Vec3d(64, -32, -16));
    minuendVertices.push_back(Vec3d(32, -64, -16));
    minuendVertices.push_back(Vec3d(-64, -64, -16));
    minuendVertices.push_back(Vec3d(-64, 64, -16));
    minuendVertices.push_back(Vec3d(32, 64, -16));
    
    Vec3d::List subtrahendVertices;
    subtrahendVertices.push_back(Vec3d(16, -32, 32));
    subtrahendVertices.push_back(Vec3d(32, -0, 32));
    subtrahendVertices.push_back(Vec3d(16, 32, 32));
    subtrahendVertices.push_back(Vec3d(-16, 48, 32));
    subtrahendVertices.push_back(Vec3d(-64, 48, 32));
    subtrahendVertices.push_back(Vec3d(-64, -48, 32));
    subtrahendVertices.push_back(Vec3d(-16, -48, 32));
    subtrahendVertices.push_back(Vec3d(-64, -48, -32));
    subtrahendVertices.push_back(Vec3d(-64, 48, -32));
    subtrahendVertices.push_back(Vec3d(-16, 48, -32));
    subtrahendVertices.push_back(Vec3d(16, 32, -32));
    subtrahendVertices.push_back(Vec3d(32, -0, -32));
    subtrahendVertices.push_back(Vec3d(16, -32, -32));
    subtrahendVertices.push_back(Vec3d(-16, -48, -32));

    const Polyhedron3d minuend(minuendVertices);
    const Polyhedron3d subtrahend(subtrahendVertices);
    
    Polyhedron3d::SubtractResult result = minuend.subtract(subtrahend);
    ASSERT_EQ(7u, result.size());
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
