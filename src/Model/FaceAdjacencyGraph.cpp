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

#include "FaceAdjacencyGraph.h"

#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        FaceAdjacencyGraph::Edge::Edge(Node* node1, Node* node2, const BrushEdge* brushEdge1, const BrushEdge* brushEdge2) :
        m_node1(node1),
        m_node2(node2),
        m_brushEdge1(brushEdge1),
        m_brushEdge2(brushEdge2){
            assert(m_node1 != NULL);
            assert(m_node2 != NULL);
            assert(m_brushEdge1 != NULL);
            assert(m_brushEdge2 != NULL);
        }
        
        FaceAdjacencyGraph::Node* FaceAdjacencyGraph::Edge::node1() const {
            return m_node1;
        }
        
        FaceAdjacencyGraph::Node* FaceAdjacencyGraph::Edge::node2() const {
            return m_node2;
        }
        
        const BrushEdge* FaceAdjacencyGraph::Edge::brushEdge1() const {
            return m_brushEdge1;
        }

        const BrushEdge* FaceAdjacencyGraph::Edge::brushEdge2() const {
            return m_brushEdge2;
        }
        
        FaceAdjacencyGraph::Node::Node(const BrushFace* face) :
        m_face(face) {
            assert(m_face != NULL);
        }

        bool FaceAdjacencyGraph::Node::addNeighbour(Node* node) {
            assert(node != NULL);
            assert(node->m_face != m_face);
            
            const Model::BrushEdgeList& myEdges = m_face->edges();
            const Model::BrushEdgeList& theirEdges = node->m_face->edges();
            
            for (size_t i = 0; i < myEdges.size(); ++i) {
                const Model::BrushEdge* myEdge = myEdges[i];
                for (size_t j = 0; j < theirEdges.size(); ++j) {
                    const Model::BrushEdge* theirEdge = theirEdges[j];
                    
                    if (isSharedEdgePair(myEdge, theirEdge)) {
                        Edge::Ptr edge(new Edge(this, node, myEdge, theirEdge));
                        m_edges.push_back(edge);
                        node->m_edges.push_back(edge);
                        return true;
                    }
                }
            }
            return false;
        }

        bool FaceAdjacencyGraph::Node::isSharedEdgePair(const BrushEdge* edge1, const BrushEdge* edge2) const {
            if (edge1->contains(edge2->start->position))
                return true;
            if (edge1->contains(edge2->end->position))
                return true;
            if (edge2->contains(edge1->start->position))
                return true;
            return false;
        }

        FaceAdjacencyGraph::~FaceAdjacencyGraph() {
            VectorUtils::clearAndDelete(m_nodes);
            m_nodeMap.clear();
        }

        void FaceAdjacencyGraph::addFace(const BrushFace* face) {
            Node* newNode = new Node(face);
            
            // TODO could speed this up even more if we find a canonic representation of the edge's line
            const Model::BrushEdgeList& edges = face->edges();
            for (size_t i = 0; i < edges.size(); ++i) {
                const Model::BrushEdge* edge = edges[i];
                const Vec3 edgeVec = orderedEdgeVec(edge);
                
                Node::List& nodes = m_nodeMap[edgeVec];
                for (size_t j = 0; j < nodes.size(); ++j) {
                    Node* node = nodes[j];
                    node->addNeighbour(newNode);
                }
                nodes.push_back(newNode);
            }
        }

        Vec3 FaceAdjacencyGraph::orderedEdgeVec(const BrushEdge* edge) const {
            const Vec3& start = edge->start->position;
            const Vec3& end = edge->end->position;
            Vec3::LexicographicOrder cmp;
            if (cmp(start, end))
                return (end - start).normalized();
            return (start - end).normalized();
        }
    }
}
