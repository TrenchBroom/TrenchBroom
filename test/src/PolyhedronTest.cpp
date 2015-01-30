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

#include "Polyhedron.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(PolyhedronTest, initWith4Points) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    
    const Polyhedron<double> p(p1, p2, p3, p4);
    
    ASSERT_EQ(4u, p.vertexCount());
    ASSERT_EQ(6u, p.edgeCount());
    ASSERT_EQ(4u, p.faceCount());
    
    typedef Polyhedron<double>::Vertex Vertex;
    
    const Vertex* v1 = p.vertices();
    const Vertex* v2 = v1->next();
    const Vertex* v3 = v2->next();
    const Vertex* v4 = v3->next();
    ASSERT_EQ(v1, v4->next());
    
    ASSERT_VEC_EQ(p1, v1->position());
    ASSERT_VEC_EQ(p2, v2->position());
    ASSERT_VEC_EQ(p3, v3->position());
    ASSERT_VEC_EQ(p4, v4->position());
    
    typedef Polyhedron<double>::Edge Edge;
    const Edge* e1 = p.edges();
    const Edge* e2 = e1->next();
    const Edge* e3 = e2->next();
    const Edge* e4 = e3->next();
    const Edge* e5 = e4->next();
    const Edge* e6 = e5->next();
    ASSERT_EQ(e1, e6->next());
    
    ASSERT_EQ(v2, e1->origin());
    ASSERT_EQ(v3, e1->destination());
    ASSERT_EQ(v3, e2->origin());
    ASSERT_EQ(v4, e2->destination());
    ASSERT_EQ(v4, e3->origin());
    ASSERT_EQ(v2, e3->destination());
    ASSERT_EQ(v1, e4->origin());
    ASSERT_EQ(v3, e4->destination());
    ASSERT_EQ(v2, e5->origin());
    ASSERT_EQ(v1, e5->destination());
    ASSERT_EQ(v4, e6->origin());
    ASSERT_EQ(v1, e6->destination());
    
    typedef Polyhedron<double>::Face Face;
    const Face* f1 = p.faces();
    const Face* f2 = f1->next();
    const Face* f3 = f2->next();
    const Face* f4 = f3->next();
    ASSERT_EQ(f1, f4->next());
    
    const Edge* f1e1 = f1->edges();
    const Edge* f1e2 = f1->nextEdge(f1e1);
    const Edge* f1e3 = f1->nextEdge(f1e2);
    ASSERT_EQ(f1e1, f1->nextEdge(f1e3));
    
    ASSERT_EQ(v2, f1e1->origin());
    ASSERT_EQ(v3, f1e2->origin());
    ASSERT_EQ(v4, f1e3->origin());
    
    
    const Edge* f2e1 = f2->edges();
    const Edge* f2e2 = f2->nextEdge(f2e1);
    const Edge* f2e3 = f2->nextEdge(f2e2);
    ASSERT_EQ(f2e1, f2->nextEdge(f2e3));
    
    ASSERT_EQ(v1, f2e1->origin());
    ASSERT_EQ(v3, f2e2->origin());
    ASSERT_EQ(v2, f2e3->origin());
    
    
    const Edge* f3e1 = f3->edges();
    const Edge* f3e2 = f3->nextEdge(f3e1);
    const Edge* f3e3 = f3->nextEdge(f3e2);
    ASSERT_EQ(f3e1, f3->nextEdge(f3e3));
    
    ASSERT_EQ(v1, f3e1->origin());
    ASSERT_EQ(v2, f3e2->origin());
    ASSERT_EQ(v4, f3e3->origin());
    
    
    const Edge* f4e1 = f4->edges();
    const Edge* f4e2 = f4->nextEdge(f4e1);
    const Edge* f4e3 = f4->nextEdge(f4e2);
    ASSERT_EQ(f4e1, f4->nextEdge(f4e3));
    
    ASSERT_EQ(v1, f4e1->origin());
    ASSERT_EQ(v4, f4e2->origin());
    ASSERT_EQ(v3, f4e3->origin());
    
    
    ASSERT_TRUE(p.containsPoint(Vec3d(0.0, 0.0, 2.0)));
    ASSERT_TRUE(p.containsPoint(Vec3d(0.0, 0.0, 0.0)));
    ASSERT_FALSE(p.containsPoint(Vec3d(0.0, 0.0, -2.0)));
}
