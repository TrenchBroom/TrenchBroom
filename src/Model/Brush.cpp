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

#include "Brush.h"

#include "CollectionUtils.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"

namespace TrenchBroom {
    namespace Model {
        const Brush::List Brush::EmptyList = Brush::List();
        const Hit::HitType Brush::BrushHit = Hit::freeHitType();

        Brush::Brush(const BBox3& worldBounds, const BrushFace::List& faces) :
        Object(OTBrush),
        m_faces(faces),
        m_geometry(NULL) {
            rebuildGeometry(worldBounds);
        }

        Brush::Ptr Brush::newBrush(const BBox3& worldBounds, const BrushFace::List& faces) {
            return Brush::Ptr(new Brush(worldBounds, faces));
        }
        
        Brush::~Brush() {
            delete m_geometry;
            m_geometry = NULL;
        }

        BBox3 Brush::bounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds();
        }

        void Brush::pick(const Ray3& ray, PickResult& result) {
            BrushFace::List::iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace::Ptr face = *it;
                const FloatType distance = face->intersectWithRay(ray);
                if (!Math<FloatType>::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    Hit hit(BrushHit, distance, hitPoint, face);
                    result.addHit(hit);
                    break;
                }
            }
        }

        const BrushFace::List& Brush::faces() const {
            return m_faces;
        }

        void Brush::addEdges(Vertex::List& vertices) const {
            const BrushEdge::List edges = m_geometry->edges();
            BrushEdge::List::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                vertices.push_back(Vertex(edge->start()->position()));
                vertices.push_back(Vertex(edge->end()->position()));
            }
        }

        Brush::Ptr Brush::sharedFromThis() {
            return shared_from_this();
        }

        void Brush::rebuildGeometry(const BBox3& worldBounds) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds, m_faces);
        }
    }
}
