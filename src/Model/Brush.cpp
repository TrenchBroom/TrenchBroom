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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Brush.h"

#include "CollectionUtils.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Brush::BrushHit = Hit::freeHitType();
        
        Brush::Brush(const BBox3& worldBounds, const BrushFaceList& faces) :
        Object(OTBrush),
        m_parent(NULL),
        m_geometry(NULL) {
            rebuildGeometry(worldBounds, faces);
        }

        Brush::~Brush() {
            m_parent = NULL;
            delete m_geometry;
            m_geometry = NULL;
            VectorUtils::clearAndDelete(m_faces);
        }

        Entity* Brush::parent() const {
            return m_parent;
        }
        
        void Brush::setParent(Entity* parent) {
            if (m_parent == parent)
                return;
            
            if (m_parent != NULL) {
                if (selected())
                    m_parent->decChildSelectionCount();
            }
            m_parent = parent;
            if (m_parent != NULL) {
                if (selected())
                    m_parent->incChildSelectionCount();
            }
        }

        bool Brush::select() {
            if (!Object::select())
                return false;
            if (m_parent != NULL)
                m_parent->incChildSelectionCount();
            return true;
        }
        
        bool Brush::deselect() {
            if (!Object::deselect())
                return false;
            if (m_parent != NULL)
                m_parent->decChildSelectionCount();
            return true;
        }
        
        BBox3 Brush::bounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds();
        }

        void Brush::pick(const Ray3& ray, PickResult& result) {
            if (Math::isnan(bounds().intersectWithRay(ray)))
                return;
            
            BrushFaceList::iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                const FloatType distance = face->intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    Hit hit(BrushHit, distance, hitPoint, face);
                    result.addHit(hit);
                    break;
                }
            }
        }

        const BrushFaceList& Brush::faces() const {
            return m_faces;
        }

        const BrushEdge::List& Brush::edges() const {
            return m_geometry->edges();
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
        
        void Brush::rebuildGeometry(const BBox3& worldBounds, const BrushFaceList& faces) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds);
            BrushGeometry::AddFaceResult result = m_geometry->addFaces(faces);
            
            BrushFaceList deleteFaces = VectorUtils::difference(m_faces, result.addedFaces);
            VectorUtils::clearAndDelete(deleteFaces);
            addFaces(result.addedFaces);
        }

        void Brush::addFaces(const BrushFaceList& faces) {
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                addFace(face);
            }
        }
        
        void Brush::addFace(BrushFace* face) {
            m_faces.push_back(face);
            face->setParent(this);
        }
    }
}
