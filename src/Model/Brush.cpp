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
#include "Model/ModelUtils.h"

namespace TrenchBroom {
    namespace Model {
        BrushSnapshot::FacesHolder::~FacesHolder() {
            VectorUtils::clearAndDelete(faces);
        }

        BrushSnapshot::BrushSnapshot(Brush& brush) :
        m_brush(&brush),
        m_holder(new FacesHolder()) {
            const Model::BrushFaceList& faces = m_brush->faces();
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::BrushFace* face = *it;
                m_holder->faces.push_back(face->clone());
            }
        }
        
        void BrushSnapshot::restore(const BBox3& worldBounds) {
            m_brush->restoreFaces(worldBounds, m_holder->faces);
            m_holder->faces.clear(); // must not delete the faces if restored
        }

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

        Brush* Brush::clone(const BBox3& worldBounds) const {
            BrushFaceList newFaces;
            newFaces.reserve(m_faces.size());
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                BrushFace* newFace = face->clone();
                newFaces.push_back(newFace);
            }
            
            return new Brush(worldBounds, newFaces);
        }

        BrushSnapshot Brush::takeSnapshot() {
            return BrushSnapshot(*this);
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

        const BrushEdgeList& Brush::edges() const {
            return m_geometry->edges();
        }

        BrushFaceList Brush::incidentFaces(const BrushVertex& vertex) const {
            const BrushFaceGeometryList sides = m_geometry->incidentSides(vertex);
            BrushFaceList result;
            result.reserve(sides.size());
            
            BrushFaceGeometryList::const_iterator it, end;
            for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                const BrushFaceGeometry& side = **it;
                result.push_back(side.face());
            }
            
            return result;
        }

        void Brush::addEdges(Vertex::List& vertices) const {
            const BrushEdgeList edges = m_geometry->edges();
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                vertices.push_back(Vertex(edge->start()->position()));
                vertices.push_back(Vertex(edge->end()->position()));
            }
        }
        
        bool Brush::clip(const BBox3& worldBounds, BrushFace* face) {
            try {
                Model::BrushFaceList newFaces(m_faces);
                newFaces.push_back(face);
                face->setParent(this);

                rebuildGeometry(worldBounds, newFaces);
                return !m_faces.empty();
            } catch (GeometryException&) {
                return false;
            }
        }

        bool Brush::canMoveBoundary(const BBox3& worldBounds, const BrushFace& face, const Vec3& delta) const {
            BrushFace* testFace = face.clone();
            testFace->transform(translationMatrix(delta), false, false);
            
            BrushFaceList testFaces;
            testFaces.push_back(testFace);
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                if (brushFace != &face)
                    testFaces.push_back(brushFace);
            }
            
            BrushGeometry testGeometry(worldBounds);
            const BrushGeometry::AddFaceResult result = testGeometry.addFaces(testFaces);
            const bool inWorldBounds = worldBounds.contains(testGeometry.bounds());
            
            m_geometry->restoreFaceGeometries();
            delete testFace;
            
            return (inWorldBounds &&
                    result.resultCode != BrushGeometry::BrushIsNull &&
                    result.resultCode != BrushGeometry::FaceIsRedundant &&
                    result.droppedFaces.empty());
        }
        
        void Brush::moveBoundary(const BBox3& worldBounds, BrushFace& face, const Vec3& delta, const bool lockTexture) {
            assert(canMoveBoundary(worldBounds, face, delta));
            
            face.transform(translationMatrix(delta), lockTexture, false);
            rebuildGeometry(worldBounds, m_faces);
        }

        void Brush::doTransform(const Mat4x4& transformation, const bool lockTextures, const bool invertFaceOrientation, const BBox3& worldBounds) {
            each(m_faces.begin(), m_faces.end(), Transform(transformation, lockTextures, invertFaceOrientation), MatchAll());
            rebuildGeometry(worldBounds, m_faces);
        }

        void Brush::restoreFaces(const BBox3& worldBounds, const BrushFaceList& faces) {
            detachFaces(m_faces);
            VectorUtils::clearAndDelete(m_faces);
            rebuildGeometry(worldBounds, faces);
        }

        void Brush::rebuildGeometry(const BBox3& worldBounds, const BrushFaceList faces) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds);
            BrushGeometry::AddFaceResult result = m_geometry->addFaces(faces);
            
            BrushFaceList deleteFaces = VectorUtils::difference(m_faces, result.addedFaces);
            detachFaces(deleteFaces);
            VectorUtils::clearAndDelete(deleteFaces);
            
            m_faces.clear();
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

        void Brush::detachFaces(const BrushFaceList& faces) {
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->setParent(NULL);
            }
        }
    }
}
