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
#include "Renderer/OutlineTracer.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(OutlineTracerTest, insertIntoEmptyLine) {
            const Edge3 edge(Vec3(1.0, 2.0, 3.0), Vec3(2.0, 4.0, 6.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(edge, edges[0]);
        }

        TEST(OutlineTracerTest, insertEdgeToRightToRight) {
            // ----
            //      ---
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(4.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(edge, edges[0]);
            ASSERT_EQ(edge2, edges[1]);
            
            // ---- ----
            //   -----
            // --  -  --
            
            const Edge3 edge3(Vec3(2.0, 0.0, 0.0), Vec3(4.5, 0.0, 0.0));
            tracer.addEdge(edge3);

            edges = tracer.edges();
            ASSERT_EQ(3u, edges.size());
            ASSERT_EQ(Edge3(Vec3(1.0, 0.0, 0.0), Vec3(2.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(3.0, 0.0, 0.0), Vec3(4.0, 0.0, 0.0)), edges[1]);
            ASSERT_EQ(Edge3(Vec3(4.5, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[2]);
            
            //  --  -  --
            // -----------
            // -         -

            /*
            const Edge3 edge4(Vec3(0.0, 0.0, 0.0), Vec3(6.0, 0.0, 0.0));
            tracer.addEdge(edge4);
            
            edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(Edge3(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(5.0, 0.0, 0.0), Vec3(6.0, 0.0, 0.0)), edges[1]);
             */
        }

        TEST(OutlineTracerTest, insertEdgeAtRightToRight) {
            // ----
            //     ---
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(3.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(Edge3(Vec3(1.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[0]);
        }
        
        TEST(OutlineTracerTest, insertEdgeBetweenToRight) {
            // ----
            //    ---
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(2.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(Edge3(Vec3(1.0, 0.0, 0.0), Vec3(2.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(3.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[1]);
        }
        
        TEST(OutlineTracerTest, insertEdgeAtLeftToRight) {
            // ----
            // ------
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(1.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(Edge3(Vec3(3.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[0]);
        }
        
        TEST(OutlineTracerTest, insertEdgeToLeftToRight) {
            //  ----
            // ------
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(0.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(Edge3(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(3.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[1]);
        }
        
        TEST(OutlineTracerTest, insertEdgeBetweenAtRight) {
            //  ----
            //    --
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(2.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(Edge3(Vec3(1.0, 0.0, 0.0), Vec3(2.0, 0.0, 0.0)), edges[0]);
        }
        
        TEST(OutlineTracerTest, insertEdgeAtLeftAtRight) {
            //  ----
            //  ----
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(0u, edges.size());
        }

        TEST(OutlineTracerTest, insertEdgeToLeftAtRight) {
            //  ----
            // -----
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(0.0, 0.0, 0.0), Vec3(3.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(Edge3(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0)), edges[0]);
        }
        
        TEST(OutlineTracerTest, insertEdgeBetweenBetween) {
            //  ----
            //   --
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(2.0, 0.0, 0.0), Vec3(4.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(Edge3(Vec3(1.0, 0.0, 0.0), Vec3(2.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(4.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[1]);
        }
        
        TEST(OutlineTracerTest, insertEdgeAtLeftBetween) {
            //  ----
            //  ---
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(1.0, 0.0, 0.0), Vec3(4.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(Edge3(Vec3(4.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[0]);
        }
        
        TEST(OutlineTracerTest, insertEdgeToLeftBetween) {
            //  ----
            // ---
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(0.0, 0.0, 0.0), Vec3(4.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(Edge3(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(4.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[1]);
        }
        
        TEST(OutlineTracerTest, insertEdgeToLeftAtLeft) {
            //   ----
            // --
            
            const Edge3 edge(Vec3(1.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(1u, edges.size());
            ASSERT_EQ(Edge3(Vec3(0.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[0]);
        }
        
        TEST(OutlineTracerTest, insertEdgeToLeftToLeft) {
            //    ----
            // --
            
            const Edge3 edge(Vec3(3.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0));
            OutlineTracer tracer;
            tracer.addEdge(edge);
            
            const Edge3 edge2(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0));
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_EQ(Edge3(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0)), edges[0]);
            ASSERT_EQ(Edge3(Vec3(3.0, 0.0, 0.0), Vec3(5.0, 0.0, 0.0)), edges[1]);
        }
        
        TEST(OutlineTracerTest, insertEdgesOnParallelLines) {
            const Edge3 edge1(Vec3(1.0, 0.0, 0.0), Vec3(2.0, 0.0, 0.0));
            const Edge3 edge2(Vec3(1.0, 0.0, 1.0), Vec3(2.0, 0.0, 1.0));
            
            OutlineTracer tracer;
            tracer.addEdge(edge1);
            tracer.addEdge(edge2);
            
            Edge3::List edges = tracer.edges();
            ASSERT_EQ(2u, edges.size());
            ASSERT_TRUE(VectorUtils::contains(edges, edge1));
            ASSERT_TRUE(VectorUtils::contains(edges, edge2));
        }
        
        TEST(OutlineTracerTest, insertTwoAdjacentFaces) {
            const Edge3 f1e1(Vec3(142, 256, -0), Vec3(192, 128, -0));
            const Edge3 f1e2(Vec3(192, 128, -0), Vec3(-96, 128, -0));
            const Edge3 f1e3(Vec3(-96, 128, -0), Vec3(-96, 256, -0));
            const Edge3 f1e4(Vec3(-96, 256, -0), Vec3(142, 256, -0));
            
            const Edge3 f2e1(Vec3(-39, 128, 128), Vec3(192, 128, 128));
            const Edge3 f2e2(Vec3(-39, 128, 128), Vec3(-39, 128, -0));
            const Edge3 f2e3(Vec3(192, 128, -0), Vec3(-39, 128, -0));
            const Edge3 f2e4(Vec3(192, 128, -0), Vec3(192, 128, 128));
            
            OutlineTracer tracer;
            tracer.addEdge(f1e1);
            ASSERT_EQ(1u, tracer.edges().size());
            tracer.addEdge(f1e2);
            ASSERT_EQ(2u, tracer.edges().size());
            tracer.addEdge(f1e3);
            ASSERT_EQ(3u, tracer.edges().size());
            tracer.addEdge(f1e4);
            ASSERT_EQ(4u, tracer.edges().size());
            tracer.addEdge(f2e1);
            ASSERT_EQ(5u, tracer.edges().size());
            tracer.addEdge(f2e2);
            ASSERT_EQ(6u, tracer.edges().size());
            tracer.addEdge(f2e3);
            ASSERT_EQ(6u, tracer.edges().size());
            tracer.addEdge(f2e4);
            ASSERT_EQ(7u, tracer.edges().size());
            
            const Edge3::List edges = tracer.edges();
            ASSERT_EQ(7u, edges.size());
        }
    }
}
