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

#include "Brush.h"

#include "CollectionUtils.h"
#include "Hit.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/Layer.h"
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
        
        Brush::Brush(const BBox3& worldBounds, BrushContentTypeBuilder::Ptr contentTypeBuilder, const BrushFaceList& faces) :
        m_contentTypeBuilder(contentTypeBuilder),
        m_entity(NULL),
        m_geometry(NULL),
        m_contentType(0),
        m_transparent(false),
        m_contentTypeValid(true) {
            addFaces(faces);
            rebuildGeometry(worldBounds);
        }

        Brush::~Brush() {
            m_entity = NULL;
            delete m_geometry;
            m_geometry = NULL;
            VectorUtils::clearAndDelete(m_faces);
        }

        Brush* Brush::clone(const BBox3& worldBounds) const {
            return static_cast<Brush*>(doClone(worldBounds));
        }

        BrushSnapshot Brush::takeSnapshot() {
            return BrushSnapshot(*this);
        }

        Entity* Brush::entity() const {
            return m_entity;
        }
        
        void Brush::setEntity(Entity* entity) {
            if (entity == m_entity)
                return;
            m_entity = entity;
            invalidateContentType();
        }

        void Brush::select() {
            Object::select();
            if (m_entity != NULL)
                m_entity->childSelectionChanged(selected());
        }
        
        void Brush::deselect() {
            Object::deselect();
            if (m_entity != NULL)
                m_entity->childSelectionChanged(selected());
        }
        
        const BBox3& Brush::bounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds;
        }

        void Brush::pick(const Ray3& ray, Hits& hits) {
            if (Math::isnan(bounds().intersectWithRay(ray)))
                return;
            
            BrushFaceList::iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                const FloatType distance = face->intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    hits.addHit(Hit(BrushHit, distance, hitPoint, face));
                    break;
                }
            }
        }

        const BrushFaceList& Brush::faces() const {
            return m_faces;
        }

        const BrushEdgeList& Brush::edges() const {
            return m_geometry->edges;
        }

        const BrushVertexList& Brush::vertices() const {
            return m_geometry->vertices;
        }

        BrushFaceList Brush::incidentFaces(const BrushVertex& vertex) const {
            const BrushFaceGeometryList sides = m_geometry->incidentSides(&vertex);
            BrushFaceList result;
            result.reserve(sides.size());
            
            BrushFaceGeometryList::const_iterator it, end;
            for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                const BrushFaceGeometry& side = **it;
                result.push_back(side.face);
            }
            
            return result;
        }

        void Brush::addEdges(Vertex::List& vertices) const {
            BrushEdgeList::const_iterator it, end;
            for (it = m_geometry->edges.begin(), end = m_geometry->edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                vertices.push_back(Vertex(edge->start->position));
                vertices.push_back(Vertex(edge->end->position));
            }
        }
        
        bool Brush::clip(const BBox3& worldBounds, BrushFace* face) {
            try {
                addFace(face);
                rebuildGeometry(worldBounds);
                notifyParent();
                return !m_faces.empty();
            } catch (GeometryException&) {
                return false;
            }
        }

        bool Brush::canMoveBoundary(const BBox3& worldBounds, const BrushFace& face, const Vec3& delta) const {
            BrushFace* testFace = face.clone();
            testFace->transform(translationMatrix(delta), false);
            
            BrushFaceList testFaces;
            testFaces.push_back(testFace);
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                if (brushFace != &face)
                    testFaces.push_back(brushFace);
            }
            
            BrushGeometry testGeometry(worldBounds);
            const AddFaceResult result = testGeometry.addFaces(testFaces);
            const bool inWorldBounds = worldBounds.contains(testGeometry.bounds) && testGeometry.isClosed();
            
            m_geometry->restoreFaceGeometries();
            delete testFace;
            
            return (inWorldBounds &&
                    result.resultCode != AddFaceResult::Code_BrushNull &&
                    result.resultCode != AddFaceResult::Code_FaceRedundant &&
                    result.droppedFaces.empty());
        }
        
        void Brush::moveBoundary(const BBox3& worldBounds, BrushFace& face, const Vec3& delta, const bool lockTexture) {
            assert(canMoveBoundary(worldBounds, face, delta));
            
            face.transform(translationMatrix(delta), lockTexture);
            rebuildGeometry(worldBounds);
            notifyParent();
        }
        
        bool Brush::canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canMoveVertices(worldBounds, vertexPositions, delta);
            assert(checkFaceGeometryLinks());
            return result;
        }
        
        Vec3::List Brush::moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canMoveVertices(worldBounds, vertexPositions, delta));
            
            const MoveVerticesResult result = m_geometry->moveVertices(worldBounds, vertexPositions, delta);
            processBrushAlgorithmResult(worldBounds, result);
            assert(checkFaceGeometryLinks());
            notifyParent();
            
            return result.newVertexPositions;
        }

        bool Brush::canMoveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canMoveEdges(worldBounds, edgePositions, delta);
            assert(checkFaceGeometryLinks());
            return result;
        }
        
        Edge3::List Brush::moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canMoveEdges(worldBounds, edgePositions, delta));
            
            const MoveEdgesResult result = m_geometry->moveEdges(worldBounds, edgePositions, delta);
            processBrushAlgorithmResult(worldBounds, result);
            assert(checkFaceGeometryLinks());
            notifyParent();

            return result.newEdgePositions;
        }

        bool Brush::canMoveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canMoveFaces(worldBounds, facePositions, delta);
            assert(checkFaceGeometryLinks());
            return result;
        }
        
        Polygon3::List Brush::moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canMoveFaces(worldBounds, facePositions, delta));
            
            const MoveFacesResult result = m_geometry->moveFaces(worldBounds, facePositions, delta);
            processBrushAlgorithmResult(worldBounds, result);
            assert(checkFaceGeometryLinks());
            notifyParent();

            return result.newFacePositions;
        }

        bool Brush::canSplitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canSplitEdge(worldBounds, edgePosition, delta);
            assert(checkFaceGeometryLinks());
            return result;
        }
        
        Vec3 Brush::splitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canSplitEdge(worldBounds, edgePosition, delta));
            
            const SplitResult result = m_geometry->splitEdge(worldBounds, edgePosition, delta);
            processBrushAlgorithmResult(worldBounds, result);
            assert(checkFaceGeometryLinks());
            notifyParent();

            return result.newVertexPosition;
        }
        
        bool Brush::canSplitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canSplitFace(worldBounds, facePosition, delta);
            assert(checkFaceGeometryLinks());
            return result;
        }
        
        Vec3 Brush::splitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canSplitFace(worldBounds, facePosition, delta));
            
            const SplitResult result = m_geometry->splitFace(worldBounds, facePosition, delta);
            processBrushAlgorithmResult(worldBounds, result);
            assert(checkFaceGeometryLinks());
            notifyParent();

            return result.newVertexPosition;
        }

        Vec3::List Brush::snapVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const size_t snapTo) {
            assert(m_geometry != NULL);
            
            const SnapVerticesResult result = m_geometry->snapVertices(vertexPositions, snapTo);
            processBrushAlgorithmResult(worldBounds, result);
            assert(checkFaceGeometryLinks());
            notifyParent();

            return result.newVertexPositions;
        }

        void Brush::snapPlanePointsToInteger(const BBox3& worldBounds) {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                brushFace->snapPlanePointsToInteger();
            }
            rebuildGeometry(worldBounds);
            notifyParent();
        }
        
        void Brush::findIntegerPlanePoints(const BBox3& worldBounds) {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                brushFace->findIntegerPlanePoints();
            }
            rebuildGeometry(worldBounds);
            notifyParent();
        }

        void Brush::rebuildGeometry(const BBox3& worldBounds) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds);
            const AddFaceResult result = m_geometry->addFaces(m_faces);
            detachFaces(m_faces);
            m_faces.clear();
            processBrushAlgorithmResult(worldBounds, result);
            notifyParent();
        }
        
        bool Brush::transparent() const {
            validateContentType();
            return m_transparent;
        }

        bool Brush::hasContentType(const BrushContentType& contentType) const {
            return hasContentType(contentType.flagValue());
        }

        bool Brush::hasContentType(const BrushContentType::FlagType contentTypeMask) const {
            return (contentTypeFlags() & contentTypeMask) != 0;
        }

        void Brush::invalidateContentType() {
            m_contentTypeValid = false;
        }
        
        bool Brush::containsPoint(const Vec3& point) const {
            if (!bounds().contains(point))
                return false;
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                if (face->boundary().pointStatus(point) == Math::PointStatus::PSAbove)
                    return false;
            }
            return true;
        }
        
        BrushContentType::FlagType Brush::contentTypeFlags() const {
            validateContentType();
            return m_contentType;
        }

        void Brush::validateContentType() const {
            if (!m_contentTypeValid) {
                const BrushContentTypeBuilder::Result result = m_contentTypeBuilder->buildContentType(this);
                m_contentType = result.contentType;
                m_transparent = result.transparent;
                m_contentTypeValid = true;
            }
        }

        void Brush::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            each(m_faces.begin(), m_faces.end(), Transform(transformation, lockTextures, worldBounds), MatchAll());
            rebuildGeometry(worldBounds);
            notifyParent();
        }

        class BrushContains : public ConstObjectVisitor {
        private:
            const Brush* m_this;
            bool m_result;
        public:
            BrushContains(const Brush* i_this) :
            m_this(i_this),
            m_result(false) {}
            
            bool result() const {
                return m_result;
            }
        private:
            void doVisit(const Entity* entity) {
                m_result = contains(entity);
            }
            
            bool contains(const Entity* entity) const {
                if (m_this->bounds().contains(entity->bounds()))
                    return true;
                const Vec3::List vertices = bBoxVertices(entity->bounds());
                for (size_t i = 0; i < vertices.size(); ++i) {
                    if (!m_this->containsPoint(vertices[i]))
                        return false;
                }
                return true;
            }
            
            void doVisit(const Brush* brush) {
                m_result = contains(brush);
            }
            
            bool contains(const Brush* brush) const {
                if (!m_this->bounds().contains(brush->bounds()))
                    return false;
                const BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    if (!m_this->containsPoint(vertices[i]->position))
                        return false;
                }
                return true;
            }
        };
        
        bool Brush::doContains(const Object& object) const {
            BrushContains contains(this);
            object.accept(contains);
            return contains.result();
        }
        
        class BrushIntersects : public ConstObjectVisitor {
        private:
            const Brush* m_this;
            bool m_result;
        public:
            BrushIntersects(const Brush* i_this) :
            m_this(i_this),
            m_result(false) {}
            
            bool result() const {
                return m_result;
            }
        private:
            void doVisit(const Entity* entity) {
                m_result = intersects(entity);
            }
            
            bool intersects(const Entity* entity) const {
                if (!m_this->bounds().intersects(entity->bounds()))
                    return false;
                const Vec3::List vertices = bBoxVertices(entity->bounds());
                for (size_t i = 0; i < vertices.size(); ++i) {
                    if (m_this->containsPoint(vertices[i]))
                        return true;
                }
                return false;
            }
            
            void doVisit(const Brush* brush) {
                m_result = intersects(brush);
            }
            
            bool intersects(const Brush* brush) {
                if (!m_this->bounds().intersects(brush->bounds()))
                    return false;
                
                // separating axis theorem
                // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
                
                BrushFaceList::const_iterator faceIt, faceEnd;
                
                const BrushVertexList& myVertices = m_this->vertices();
                const BrushFaceList& theirFaces = brush->faces();
                for (faceIt = theirFaces.begin(), faceEnd = theirFaces.end(); faceIt != faceEnd; ++faceIt) {
                    const BrushFace* theirFace = *faceIt;
                    if (pointStatus(theirFace->boundary(), myVertices) == Math::PointStatus::PSAbove)
                        return false;
                }
                
                const BrushVertexList& theirVertices = brush->vertices();
                const BrushFaceList& myFaces = m_this->faces();
                for (faceIt = myFaces.begin(), faceEnd = myFaces.end(); faceIt != faceEnd; ++faceIt) {
                    const BrushFace* myFace = *faceIt;
                    if (pointStatus(myFace->boundary(), theirVertices) == Math::PointStatus::PSAbove)
                        return false;
                }
                
                const BrushEdgeList& myEdges = m_this->edges();
                const BrushEdgeList& theirEdges = brush->edges();
                BrushEdgeList::const_iterator myEdgeIt, myEdgeEnd, theirEdgeIt, theirEdgeEnd;
                for (myEdgeIt = myEdges.begin(), myEdgeEnd = myEdges.end(); myEdgeIt != myEdgeEnd; ++myEdgeIt) {
                    const BrushEdge* myEdge = *myEdgeIt;
                    const Vec3 myEdgeVec = myEdge->vector();
                    const Vec3& origin = myEdge->start->position;
                    
                    for (theirEdgeIt = theirEdges.begin(), theirEdgeEnd = theirEdges.end(); theirEdgeIt != theirEdgeEnd; ++theirEdgeIt) {
                        const BrushEdge* theirEdge = *theirEdgeIt;
                        const Vec3 theirEdgeVec = theirEdge->vector();
                        const Vec3 direction = crossed(myEdgeVec, theirEdgeVec);
                        const Plane3 plane(origin, direction);
                        
                        const Math::PointStatus::Type myStatus = pointStatus(plane, myVertices);
                        if (myStatus != Math::PointStatus::PSInside) {
                            const Math::PointStatus::Type theirStatus = pointStatus(plane, theirVertices);
                            if (theirStatus != Math::PointStatus::PSInside) {
                                if (myStatus != theirStatus)
                                    return false;
                            }
                        }
                    }
                }
                
                return true;
            }
        };
        
        bool Brush::doIntersects(const Object& object) const {
            BrushIntersects intersects(this);
            object.accept(intersects);
            return intersects.result();
        }

        void Brush::doAccept(ObjectVisitor& visitor) {
            visitor.visit(this);
        }

        void Brush::doAccept(ConstObjectVisitor& visitor) const {
            visitor.visit(this);
        }

        void Brush::doAcceptRecursively(ObjectVisitor& visitor) {
            visitor.visit(this);
        }

        void Brush::doAcceptRecursively(ConstObjectVisitor& visitor) const {
            visitor.visit(this);
        }
        
        Object* Brush::doClone(const BBox3& worldBounds) const {
            BrushFaceList newFaces;
            newFaces.reserve(m_faces.size());
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                BrushFace* newFace = face->clone();
                newFaces.push_back(newFace);
            }
            
            return new Brush(worldBounds, m_contentTypeBuilder, newFaces);
        }
        
        void Brush::notifyParent() const {
            if (m_entity != NULL)
                m_entity->childBrushChanged();
        }
        
        void Brush::restoreFaces(const BBox3& worldBounds, const BrushFaceList& faces) {
            detachFaces(m_faces);
            VectorUtils::clearAndDelete(m_faces);
            addFaces(faces);
            rebuildGeometry(worldBounds);
            notifyParent();
        }
        
        void Brush::processBrushAlgorithmResult(const BBox3& worldBounds, const BrushAlgorithmResult& result) {
            if (result.addedFaces.empty() && result.droppedFaces.empty())
                return;
            
            BrushFaceList::const_iterator it, end;
            detachFaces(result.droppedFaces);
            VectorUtils::eraseAll(m_faces, result.droppedFaces);
            VectorUtils::deleteAll(result.droppedFaces);
            
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->invalidate();
            }
            
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
            if (face->selected())
                incChildSelectionCount();
            invalidateContentType();
        }

        void Brush::detachFaces(const BrushFaceList& faces) {
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                detachFace(face);
            }
        }

        void Brush::detachFace(BrushFace* face) {
            face->setParent(NULL);
            if (face->selected())
                decChildSelectionCount();
        }

        bool Brush::checkFaceGeometryLinks() const {
            BrushFaceList::const_iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                const BrushFace* face = *fIt;
                if (face->side() == NULL)
                    return false;
                if (!VectorUtils::contains(m_geometry->sides, face->side()))
                    return false;
            }
            
            BrushFaceGeometryList::const_iterator sIt, sEnd;
            for (sIt = m_geometry->sides.begin(), sEnd = m_geometry->sides.end(); sIt != sEnd; ++sIt) {
                const BrushFaceGeometry* side = *sIt;
                if (side->face == NULL)
                    return false;
                if (!VectorUtils::contains(m_faces, side->face))
                    return false;
            }
            
            return true;
        }
    }
}
