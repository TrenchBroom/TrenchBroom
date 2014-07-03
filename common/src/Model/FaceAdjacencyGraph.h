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

#ifndef __TrenchBroom__FaceAdjacencyGraph__
#define __TrenchBroom__FaceAdjacencyGraph__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushEdge;
        
        class FaceAdjacencyGraph {
        private:
            class Node;
            
            class Edge {
            public:
                typedef TrenchBroom::shared_ptr<Edge> Ptr;
                typedef std::vector<Ptr> List;
            private:
                Node* m_node1;
                Node* m_node2;
                const BrushEdge* m_brushEdge1;
                const BrushEdge* m_brushEdge2;
            public:
                Edge(Node* node1, Node* node2, const BrushEdge* brushEdge1, const BrushEdge* brushEdge2);
                
                Node* node1() const;
                Node* node2() const;

                const BrushEdge* brushEdge1() const;
                const BrushEdge* brushEdge2() const;
            };
            
            class Node {
            public:
                typedef std::vector<Node*> List;
            private:
                const BrushFace* m_face;
                Edge::List m_edges;
            public:
                Node(const BrushFace* face);
                
                bool addNeighbour(Node* node);
            private:
                bool isSharedEdgePair(const BrushEdge* edge1, const BrushEdge* edge2) const;
            };
            
            typedef std::map<Vec3, Node::List, Vec3::LexicographicOrder> NodeMap;
            
            Node::List m_nodes;
            NodeMap m_nodeMap;
        public:
            ~FaceAdjacencyGraph();
            
            void addFace(const BrushFace* face);
            Vec3 orderedEdgeVec(const BrushEdge* edge) const;
        };
    }
}

#endif /* defined(__TrenchBroom__FaceAdjacencyGraph__) */
