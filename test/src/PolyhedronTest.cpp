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
#include "MathUtils.h"
#include "TestUtils.h"


typedef Polyhedron<double> Polyhedron3d;
typedef Polyhedron3d::Vertex Vertex;
typedef Polyhedron3d::Edge Edge;
typedef Polyhedron3d::Face Face;

typedef std::pair<const Vertex*, const Vertex*> EdgeInfo;
typedef std::vector<EdgeInfo> EdgeInfoList;

bool hasVertices(const Vertex* vertices, Vec3d::List points);
bool hasEdges(const Edge* edges, EdgeInfoList edgeInfos);
bool hasTriangleOf(const Face* faces, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3);

TEST(PolyhedronTest, initWith4Points) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    
    const Polyhedron3d p(p1, p2, p3, p4);
    
    const Vertex* v1 = p.vertices();
    const Vertex* v2 = v1->next();
    const Vertex* v3 = v2->next();
    const Vertex* v4 = v3->next();
    ASSERT_EQ(v1, v4->next());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    ASSERT_TRUE(hasVertices(v1, points));
    
    const Edge* e1 = p.edges();
    const Edge* e2 = e1->next();
    const Edge* e3 = e2->next();
    const Edge* e4 = e3->next();
    const Edge* e5 = e4->next();
    const Edge* e6 = e5->next();
    ASSERT_EQ(e1, e6->next());
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(v2, v3));
    edgeInfos.push_back(std::make_pair(v3, v4));
    edgeInfos.push_back(std::make_pair(v4, v2));
    edgeInfos.push_back(std::make_pair(v1, v3));
    edgeInfos.push_back(std::make_pair(v1, v2));
    edgeInfos.push_back(std::make_pair(v4, v1));
    
    const Face* f1 = p.faces();
    const Face* f2 = f1->next();
    const Face* f3 = f2->next();
    const Face* f4 = f3->next();
    ASSERT_EQ(f1, f4->next());
    
    ASSERT_TRUE(hasTriangleOf(f1, p2, p3, p4));
    ASSERT_TRUE(hasTriangleOf(f1, p1, p3, p2));
    ASSERT_TRUE(hasTriangleOf(f1, p1, p2, p4));
    ASSERT_TRUE(hasTriangleOf(f1, p1, p4, p3));

    const Edge* f1e1 = f1->edges();
    const Edge* f1e2 = f1e1->nextBoundaryEdge();
    const Edge* f1e3 = f1e2->nextBoundaryEdge();
    ASSERT_EQ(f1e1, f1e3->nextBoundaryEdge());
    
    
    const Edge* f2e1 = f2->edges();
    const Edge* f2e2 = f2e1->nextBoundaryEdge();
    const Edge* f2e3 = f2e2->nextBoundaryEdge();
    ASSERT_EQ(f2e1, f2e3->nextBoundaryEdge());

    
    const Edge* f3e1 = f3->edges();
    const Edge* f3e2 = f3e1->nextBoundaryEdge();
    const Edge* f3e3 = f3e2->nextBoundaryEdge();
    ASSERT_EQ(f3e1, f3e3->nextBoundaryEdge());
    
    
    const Edge* f4e1 = f4->edges();
    const Edge* f4e2 = f4e1->nextBoundaryEdge();
    const Edge* f4e3 = f4e2->nextBoundaryEdge();
    ASSERT_EQ(f4e1, f4e3->nextBoundaryEdge());
}

TEST(PolyhedronTest, convexHullWithContainedPoint) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 0.0, 4.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    p.addPoints(Vec3d::List(1, p5));

    const Vertex* v1 = p.vertices();
    const Vertex* v2 = v1->next();
    const Vertex* v3 = v2->next();
    const Vertex* v4 = v3->next();
    ASSERT_EQ(v1, v4->next());
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    ASSERT_TRUE(hasVertices(v1, points));
    
    const Edge* e1 = p.edges();
    const Edge* e2 = e1->next();
    const Edge* e3 = e2->next();
    const Edge* e4 = e3->next();
    const Edge* e5 = e4->next();
    const Edge* e6 = e5->next();
    ASSERT_EQ(e1, e6->next());
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(v2, v3));
    edgeInfos.push_back(std::make_pair(v3, v4));
    edgeInfos.push_back(std::make_pair(v4, v2));
    edgeInfos.push_back(std::make_pair(v1, v3));
    edgeInfos.push_back(std::make_pair(v1, v2));
    edgeInfos.push_back(std::make_pair(v4, v1));
    
    const Face* f1 = p.faces();
    const Face* f2 = f1->next();
    const Face* f3 = f2->next();
    const Face* f4 = f3->next();
    ASSERT_EQ(f1, f4->next());
    
    ASSERT_TRUE(hasTriangleOf(f1, p2, p3, p4));
    ASSERT_TRUE(hasTriangleOf(f1, p1, p3, p2));
    ASSERT_TRUE(hasTriangleOf(f1, p1, p2, p4));
    ASSERT_TRUE(hasTriangleOf(f1, p1, p4, p3));
    
    const Edge* f1e1 = f1->edges();
    const Edge* f1e2 = f1e1->nextBoundaryEdge();
    const Edge* f1e3 = f1e2->nextBoundaryEdge();
    ASSERT_EQ(f1e1, f1e3->nextBoundaryEdge());
    
    
    const Edge* f2e1 = f2->edges();
    const Edge* f2e2 = f2e1->nextBoundaryEdge();
    const Edge* f2e3 = f2e2->nextBoundaryEdge();
    ASSERT_EQ(f2e1, f2e3->nextBoundaryEdge());
    
    
    const Edge* f3e1 = f3->edges();
    const Edge* f3e2 = f3e1->nextBoundaryEdge();
    const Edge* f3e3 = f3e2->nextBoundaryEdge();
    ASSERT_EQ(f3e1, f3e3->nextBoundaryEdge());
    
    
    const Edge* f4e1 = f4->edges();
    const Edge* f4e2 = f4e1->nextBoundaryEdge();
    const Edge* f4e3 = f4e2->nextBoundaryEdge();
    ASSERT_EQ(f4e1, f4e3->nextBoundaryEdge());
}

TEST(PolyhedronTest, convexHullWithNewPoint) {
    const Vec3d p1( 0.0, 4.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    const Vec3d p5( 0.0, 4.0, 12.0);
    
    Polyhedron3d p(p1, p2, p3, p4);
    p.addPoints(Vec3d::List(1, p5));

    const Vertex* v1 = p.vertices();
    const Vertex* v2 = v1->next();
    const Vertex* v3 = v2->next();
    const Vertex* v4 = v3->next();
    ASSERT_EQ(v1, v4->next());
    
    Vec3d::List points;
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    ASSERT_TRUE(hasVertices(v1, points));
    
    const Edge* e1 = p.edges();
    const Edge* e2 = e1->next();
    const Edge* e3 = e2->next();
    const Edge* e4 = e3->next();
    const Edge* e5 = e4->next();
    const Edge* e6 = e5->next();
    ASSERT_EQ(e1, e6->next());
    
    EdgeInfoList edgeInfos;
    edgeInfos.push_back(std::make_pair(v2, v3));
    edgeInfos.push_back(std::make_pair(v3, v4));
    edgeInfos.push_back(std::make_pair(v4, v2));
    edgeInfos.push_back(std::make_pair(v1, v3));
    edgeInfos.push_back(std::make_pair(v1, v2));
    edgeInfos.push_back(std::make_pair(v4, v1));
    
    const Face* f1 = p.faces();
    const Face* f2 = f1->next();
    const Face* f3 = f2->next();
    const Face* f4 = f3->next();
    ASSERT_EQ(f1, f4->next());
    
    ASSERT_TRUE(hasTriangleOf(f1, p2, p3, p4));
    ASSERT_TRUE(hasTriangleOf(f1, p5, p3, p2));
    ASSERT_TRUE(hasTriangleOf(f1, p5, p2, p4));
    ASSERT_TRUE(hasTriangleOf(f1, p5, p4, p3));
    
    const Edge* f1e1 = f1->edges();
    const Edge* f1e2 = f1e1->nextBoundaryEdge();
    const Edge* f1e3 = f1e2->nextBoundaryEdge();
    ASSERT_EQ(f1e1, f1e3->nextBoundaryEdge());
    
    
    const Edge* f2e1 = f2->edges();
    const Edge* f2e2 = f2e1->nextBoundaryEdge();
    const Edge* f2e3 = f2e2->nextBoundaryEdge();
    ASSERT_EQ(f2e1, f2e3->nextBoundaryEdge());
    
    
    const Edge* f3e1 = f3->edges();
    const Edge* f3e2 = f3e1->nextBoundaryEdge();
    const Edge* f3e3 = f3e2->nextBoundaryEdge();
    ASSERT_EQ(f3e1, f3e3->nextBoundaryEdge());
    
    
    const Edge* f4e1 = f4->edges();
    const Edge* f4e2 = f4e1->nextBoundaryEdge();
    const Edge* f4e3 = f4e2->nextBoundaryEdge();
    ASSERT_EQ(f4e1, f4e3->nextBoundaryEdge());
}

bool hasVertices(const Vertex* vertices, Vec3d::List points) {
    const Vertex* vertex = vertices;
    do {
        Vec3d::List::iterator it = VectorUtils::find(points, vertex->position());
        if (it == points.end())
            return false;
        points.erase(it);
        vertex = vertex->next();
    } while (vertex != vertices);
    return points.empty();
}

EdgeInfoList::iterator findEdgeInfo(EdgeInfoList& edgeInfos, const Edge* edge);

bool hasEdges(const Edge* edges, EdgeInfoList edgeInfos) {
    const Edge* edge = edges;
    do {
        EdgeInfoList::iterator it = findEdgeInfo(edgeInfos, edge);
        if (it == edgeInfos.end())
            return false;
        edgeInfos.erase(it);
        edge = edge->next();
    } while (edge != edges);
    return edgeInfos.empty();
}

EdgeInfoList::iterator findEdgeInfo(EdgeInfoList& edgeInfos, const Edge* edge) {
    Vertex* v1 = edge->origin();
    Vertex* v2 = edge->destination();
    
    EdgeInfoList::iterator it, end;
    for (it = edgeInfos.begin(), end = edgeInfos.end(); it != end; ++it) {
        const EdgeInfo& info = *it;
        if ((info.first == v1 && info.second == v2) ||
            (info.first == v2 && info.second == v1))
            return it;
    }
    return end;
}

bool isTriangleOf(const Face* face, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3);

bool hasTriangleOf(const Face* faces, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3) {
    const Face* face = faces;
    do {
        if (isTriangleOf(face, p1, p2, p3))
            return true;
        face = face->next();
    } while (face != faces);
    return false;
}

bool isTriangleOf(const Face* face, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3) {
    Edge* faceEdges = face->edges();
    Edge* e1 = faceEdges;
    while (e1->origin()->position() != p1) {
        e1 = e1->nextBoundaryEdge();
        if (e1 == faceEdges)
            return false;
    }
    
    Edge* e2 = e1->nextBoundaryEdge();
    Edge* e3 = e2->nextBoundaryEdge();
    return e2->origin()->position() == p2 && e3->origin()->position() == p3;
}
