/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushGeometry.h"

#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "Model/IntersectBrushGeometryWithFace.h"

namespace TrenchBroom {
    namespace Model {
        BrushGeometry::BrushGeometry(const BBox3& worldBounds) :
        m_bounds(worldBounds) {
            initializeWithBounds(worldBounds);
        }

        BrushGeometry::~BrushGeometry() {
            VectorUtils::clearAndDelete(m_sides);
            VectorUtils::clearAndDelete(m_edges);
            VectorUtils::clearAndDelete(m_vertices);
        }

        BBox3 BrushGeometry::bounds() const {
            return m_bounds;
        }

        const BrushVertex::List& BrushGeometry::vertices() const {
            return m_vertices;
        }
        
        const BrushEdge::List& BrushGeometry::edges() const {
            return m_edges;
        }
        
        const BrushFaceGeometry::List& BrushGeometry::sides() const {
            return m_sides;
        }
        
        BrushGeometry::AddFaceResult BrushGeometry::addFaces(const BrushFaceList& faces) {
            AddFaceResult totalResult(BrushIsSplit);
            
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                const AddFaceResult result = addFace(face);
                if (result.resultCode == BrushIsNull)
                    return AddFaceResult(BrushIsNull);
                totalResult.append(result);
            }
            
            updateBounds();
            return totalResult;
        }
        
        BrushGeometry::AddFaceResult BrushGeometry::addFace(BrushFace* face) {
            IntersectBrushGeometryWithFace algorithm(*this, face);
            const AddFaceResultCode resultCode = algorithm.execute();
            switch (resultCode) {
                case BrushIsNull:
                    break;
                case FaceIsRedundant:
                    break;
                case BrushIsSplit:
                    m_vertices = algorithm.vertices();
                    m_edges = algorithm.edges();
                    m_sides = algorithm.sides();
                    break;
            }
            
            return AddFaceResult(resultCode, algorithm.addedFaces(), algorithm.removedFaces());
        }

        void BrushGeometry::initializeWithBounds(const BBox3& bounds) {
            BrushVertex* v000 = new BrushVertex(bounds.vertex(BBox3::Min, BBox3::Min, BBox3::Min));
            BrushVertex* v001 = new BrushVertex(bounds.vertex(BBox3::Min, BBox3::Min, BBox3::Max));
            BrushVertex* v010 = new BrushVertex(bounds.vertex(BBox3::Min, BBox3::Max, BBox3::Min));
            BrushVertex* v011 = new BrushVertex(bounds.vertex(BBox3::Min, BBox3::Max, BBox3::Max));
            BrushVertex* v100 = new BrushVertex(bounds.vertex(BBox3::Max, BBox3::Min, BBox3::Min));
            BrushVertex* v101 = new BrushVertex(bounds.vertex(BBox3::Max, BBox3::Min, BBox3::Max));
            BrushVertex* v110 = new BrushVertex(bounds.vertex(BBox3::Max, BBox3::Max, BBox3::Min));
            BrushVertex* v111 = new BrushVertex(bounds.vertex(BBox3::Max, BBox3::Max, BBox3::Max));
            
            BrushEdge* v000v001 = new BrushEdge(v000, v001);
            BrushEdge* v001v101 = new BrushEdge(v001, v101);
            BrushEdge* v101v100 = new BrushEdge(v101, v100);
            BrushEdge* v100v000 = new BrushEdge(v100, v000);
            BrushEdge* v010v110 = new BrushEdge(v010, v110);
            BrushEdge* v110v111 = new BrushEdge(v110, v111);
            BrushEdge* v111v011 = new BrushEdge(v111, v011);
            BrushEdge* v011v010 = new BrushEdge(v011, v010);
            BrushEdge* v000v010 = new BrushEdge(v000, v010);
            BrushEdge* v011v001 = new BrushEdge(v011, v001);
            BrushEdge* v101v111 = new BrushEdge(v101, v111);
            BrushEdge* v110v100 = new BrushEdge(v110, v100);
            
            BrushFaceGeometry* top = new BrushFaceGeometry();
            BrushFaceGeometry* bottom = new BrushFaceGeometry();
            BrushFaceGeometry* front = new BrushFaceGeometry();
            BrushFaceGeometry* back = new BrushFaceGeometry();
            BrushFaceGeometry* left = new BrushFaceGeometry();
            BrushFaceGeometry* right = new BrushFaceGeometry();
            
            top->addBackwardEdge(v011v001);
            top->addBackwardEdge(v111v011);
            top->addBackwardEdge(v101v111);
            top->addBackwardEdge(v001v101);
            
            bottom->addBackwardEdge(v100v000);
            bottom->addBackwardEdge(v110v100);
            bottom->addBackwardEdge(v010v110);
            bottom->addBackwardEdge(v000v010);

            front->addForwardEdge(v000v001);
            front->addForwardEdge(v001v101);
            front->addForwardEdge(v101v100);
            front->addForwardEdge(v100v000);
            
            back->addForwardEdge(v010v110);
            back->addForwardEdge(v110v111);
            back->addForwardEdge(v111v011);
            back->addForwardEdge(v011v010);
            
            left->addBackwardEdge(v000v001);
            left->addForwardEdge(v000v010);
            left->addBackwardEdge(v011v010);
            left->addForwardEdge(v011v001);
            
            right->addBackwardEdge(v101v100);
            right->addForwardEdge(v101v111);
            right->addBackwardEdge(v110v111);
            right->addForwardEdge(v110v100);

            m_vertices.push_back(v000);
            m_vertices.push_back(v001);
            m_vertices.push_back(v010);
            m_vertices.push_back(v011);
            m_vertices.push_back(v100);
            m_vertices.push_back(v101);
            m_vertices.push_back(v110);
            m_vertices.push_back(v111);
            
            m_edges.push_back(v000v001);
            m_edges.push_back(v001v101);
            m_edges.push_back(v101v100);
            m_edges.push_back(v100v000);
            m_edges.push_back(v010v110);
            m_edges.push_back(v110v111);
            m_edges.push_back(v111v011);
            m_edges.push_back(v011v010);
            m_edges.push_back(v000v010);
            m_edges.push_back(v011v001);
            m_edges.push_back(v101v111);
            m_edges.push_back(v110v100);
            
            m_sides.push_back(top);
            m_sides.push_back(bottom);
            m_sides.push_back(front);
            m_sides.push_back(back);
            m_sides.push_back(left);
            m_sides.push_back(right);
            
            assert(m_vertices.size() == 8);
            assert(m_edges.size() == 12);
            assert(m_sides.size() == 6);
            assert(top->isClosed());
            assert(bottom->isClosed());
            assert(front->isClosed());
            assert(back->isClosed());
            assert(left->isClosed());
            assert(right->isClosed());
        }

        void BrushGeometry::updateBounds() {
            assert(m_vertices.size() > 0);
            m_bounds = BBox3(m_vertices[0]->position(), m_vertices[0]->position());
            for (size_t i = 1; i < m_vertices.size(); ++i)
                m_bounds.mergeWith(m_vertices[i]->position());
        }
    }
}
