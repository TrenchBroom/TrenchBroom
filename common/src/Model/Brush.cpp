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
#include "Model/BrushContentTypeBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushSnapshot.h"
#include "Model/Entity.h"
#include "Model/FindContainerVisitor.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Group.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Brush::BrushHit = Hit::freeHitType();
        
        BrushVertex*& Brush::ProjectToVertex::project(BrushVertex*& vertex) {
            return vertex;
        }
        
        BrushEdge*& Brush::ProjectToEdge::project(BrushEdge*& edge) {
            return edge;
        }

        class Brush::AddFaceToGeometryCallback : public BrushGeometry::Callback {
        private:
            BrushFace* m_addedFace;
        public:
            AddFaceToGeometryCallback(BrushFace* addedFace) :
            m_addedFace(addedFace) {
                assert(m_addedFace != NULL);
            }
            
            void faceWasCreated(BrushGeometry::Face* face) {
                face->setPayload(m_addedFace);
                m_addedFace->setGeometry(face);
            }

            void faceWillBeDeleted(BrushGeometry::Face* face) {
                BrushFace* brushFace = face->payload();
                if (brushFace != NULL) {
                    assert(!brushFace->selected());
                    
                    delete brushFace;
                    face->setPayload(NULL);
                }
            }
        };

        class Brush::AddFacesToGeometry {
        private:
            BrushGeometry& m_geometry;
            bool m_brushEmpty;
        public:
            AddFacesToGeometry(BrushGeometry& geometry, const BrushFaceList& facesToAdd) :
            m_geometry(geometry),
            m_brushEmpty(false) {
                BrushFaceList::const_iterator it, end;
                for (it = facesToAdd.begin(), end = facesToAdd.end(); it != end && !m_brushEmpty; ++it) {
                    BrushFace* face = *it;
                    AddFaceToGeometryCallback callback(face);
                    const BrushGeometry::ClipResult result = m_geometry.clip(face->boundary(), callback);
                    if (result.empty())
                        m_brushEmpty = true;
                }
                m_geometry.correctVertexPositions();
            }
            
            bool brushEmpty() const {
                return m_brushEmpty;
            }
        };

        class Brush::CanMoveBoundaryCallback : public BrushGeometry::Callback {
        private:
            BrushFace* m_addedFace;
            bool m_hasDroppedFaces;
        public:
            CanMoveBoundaryCallback(BrushFace* addedFace) :
            m_addedFace(addedFace),
            m_hasDroppedFaces(false) {
                assert(m_addedFace != NULL);
            }
            
            void faceWasCreated(BrushGeometry::Face* face) {
                face->setPayload(m_addedFace);
                m_addedFace->setGeometry(face);
            }
            
            void faceWillBeDeleted(BrushGeometry::Face* face) {
                if (face->payload() != NULL)
                    m_hasDroppedFaces = true;
            }
            
            bool hasDroppedFaces() const {
                return m_hasDroppedFaces;
            }
        };
        
        class Brush::CanMoveBoundary {
        private:
            BrushGeometry& m_geometry;
            bool m_hasDroppedFaces;
            bool m_hasRedundandFaces;
            bool m_brushEmpty;
        public:
            CanMoveBoundary(BrushGeometry& geometry, const BrushFaceList& facesToAdd) :
            m_geometry(geometry),
            m_hasDroppedFaces(false),
            m_hasRedundandFaces(false),
            m_brushEmpty(false) {
                BrushFaceList::const_iterator it, end;
                for (it = facesToAdd.begin(), end = facesToAdd.end(); it != end && !m_brushEmpty; ++it) {
                    BrushFace* face = *it;
                    CanMoveBoundaryCallback callback(face);
                    const BrushGeometry::ClipResult result = m_geometry.clip(face->boundary(), callback);
                    if (result.unchanged())
                        m_hasRedundandFaces = true;
                    else if (result.empty())
                        m_brushEmpty = true;
                    else
                        m_hasDroppedFaces |= callback.hasDroppedFaces();
                }
            }

            bool hasDroppedFaces() const {
                return m_hasDroppedFaces;
            }
            
            bool hasRedundandFaces() const {
                return m_hasRedundandFaces;
            }
            
            bool brushEmpty() const {
                return m_brushEmpty;
            }
        };

        class Brush::MoveVerticesCallback : public BrushGeometry::Callback {
        public:
            void faceWillBeDeleted(BrushFaceGeometry* faceGeometry) {
                BrushFace* face = faceGeometry->payload();
                assert(!face->selected());
                delete face;
                faceGeometry->setPayload(NULL);
            }
            
            void faceDidChange(BrushFaceGeometry* faceGeometry) {
                BrushFace* face = faceGeometry->payload();
                face->updatePointsFromVertices();
            }
            
            void faceWasSplit(BrushFaceGeometry* originalGeometry, BrushFaceGeometry* cloneGeometry) {
                BrushFace* originalFace = originalGeometry->payload();
                assert(originalFace != NULL);
                assert(cloneGeometry->payload() == NULL);
                
                BrushFace* clonedFace = originalFace->clone();
                cloneGeometry->setPayload(clonedFace);
                clonedFace->setGeometry(cloneGeometry);
                
                originalFace->invalidate();
                clonedFace->invalidate();
            }
            
            void facesWillBeMerged(BrushFaceGeometry* remainingGeometry, BrushFaceGeometry* geometryToDelete) {
                BrushFace* remainingFace = remainingGeometry->payload();
                assert(remainingFace != NULL);
                remainingFace->invalidate();
                
                BrushFace* faceToDelete = geometryToDelete->payload();
                assert(faceToDelete != NULL);
                assert(!faceToDelete->selected());
                
                delete faceToDelete;
                geometryToDelete->setPayload(NULL);
            }
        };

        class Brush::QueryCallback : public BrushGeometry::Callback {
        public:
            Plane3 plane(const BrushGeometry::Face* face) const {
                return face->payload()->boundary();
            }
        };
        
        Brush::Brush(const BBox3& worldBounds, const BrushFaceList& faces) :
        m_geometry(NULL),
        m_contentTypeBuilder(NULL),
        m_contentType(0),
        m_transparent(false),
        m_contentTypeValid(true) {
            addFaces(faces);
            rebuildGeometry(worldBounds);
        }

        Brush::~Brush() {
            delete m_geometry;
            m_geometry = NULL;
            VectorUtils::clearAndDelete(m_faces);
            m_contentTypeBuilder = NULL;
        }

        Brush* Brush::clone(const BBox3& worldBounds) const {
            return static_cast<Brush*>(Node::clone(worldBounds));
        }

        NodeSnapshot* Brush::doTakeSnapshot() {
            return new BrushSnapshot(this);
        }
        
        class FindBrushOwner : public NodeVisitor, public NodeQuery<AttributableNode*> {
        private:
            void doVisit(World* world)   { setResult(world); cancel(); }
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   {}
            void doVisit(Entity* entity) { setResult(entity); cancel(); }
            void doVisit(Brush* brush)   {}
        };
        
        AttributableNode* Brush::entity() const {
            if (parent() == NULL)
                return NULL;
            FindBrushOwner visitor;
            parent()->acceptAndEscalate(visitor);
            if (!visitor.hasResult())
                return NULL;
            return visitor.result();
        }

        const BrushFaceList& Brush::faces() const {
            return m_faces;
        }

        void Brush::setFaces(const BBox3& worldBounds, const BrushFaceList& faces) {
            const NotifyNodeChange nodeChange(this);
            detachFaces(m_faces);
            VectorUtils::clearAndDelete(m_faces);
            addFaces(faces);
            rebuildGeometry(worldBounds);
        }
        
        BrushFace* Brush::findFaceByNormal(const Vec3& normal) const {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                if (face->boundary().normal.equals(normal))
                    return face;
            }
            return NULL;
        }

        bool Brush::fullySpecified() const {
            assert(m_geometry != NULL);
            
            BrushFaceGeometry* first = m_geometry->faces().front();
            BrushFaceGeometry* current = first;
            do {
                if (current->payload() == NULL)
                    return false;
                current = current->next();
            } while (current != first);
            return true;
        }

        void Brush::faceDidChange() {
            invalidateContentType();
        }

        void Brush::addFaces(const BrushFaceList& faces) {
            addFaces(faces.begin(), faces.end(), faces.size());
        }

        void Brush::addFace(BrushFace* face) {
            assert(face != NULL);
            assert(face->brush() == NULL);
            assert(!VectorUtils::contains(m_faces, face));
            
            m_faces.push_back(face);
            face->setBrush(this);
            invalidateContentType();
            if (face->selected())
                incChildSelectionCount(1);
        }

        void Brush::removeFace(BrushFace* face) {
            m_faces.erase(doRemoveFace(m_faces.begin(), m_faces.end(), face), m_faces.end());
        }

        BrushFaceList::iterator Brush::doRemoveFace(BrushFaceList::iterator begin, BrushFaceList::iterator end, BrushFace* face) {
            assert(face != NULL);

            BrushFaceList::iterator it = std::remove(begin, end, face);
            assert(it != m_faces.end());
            detachFace(face);
            return it;
        }

        void Brush::detachFaces(const BrushFaceList& faces) {
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                detachFace(*it);
        }

        void Brush::detachFace(BrushFace* face) {
            assert(face != NULL);
            assert(face->brush() == this);

            if (face->selected())
                decChildSelectionCount(1);
            face->setBrush(NULL);
            invalidateContentType();
        }
        
        void Brush::cloneFaceAttributesFrom(const BrushList& brushes) {
            BrushList::const_iterator bIt, bEnd;
            for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                const Brush* brush = *bIt;
                cloneFaceAttributesFrom(brush);
            }
        }

        void Brush::cloneFaceAttributesFrom(const Brush* brush) {
            BrushFaceList::iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                BrushFace* destination = *fIt;
                const BrushFace* source = brush->findFaceWithBoundary(destination->boundary());
                if (source != NULL) {
                    destination->setAttribs(source->attribs());
                }
            }
        }

        void Brush::cloneInvertedFaceAttributesFrom(const BrushList& brushes) {
            BrushList::const_iterator bIt, bEnd;
            for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                const Brush* brush = *bIt;
                cloneInvertedFaceAttributesFrom(brush);
            }
        }

        void Brush::cloneInvertedFaceAttributesFrom(const Brush* brush) {
            BrushFaceList::iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                BrushFace* destination = *fIt;
                const BrushFace* source = brush->findFaceWithBoundary(destination->boundary().flipped());
                if (source != NULL) {
                    // Todo: invert the face attributes?
                    destination->setAttribs(source->attribs());
                }
            }
        }

        BrushFace* Brush::findFaceWithBoundary(const Plane3& boundary) const {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                if (face->boundary().equals(boundary))
                    return face;
            }
            return NULL;
        }

        bool Brush::clip(const BBox3& worldBounds, BrushFace* face) {
            const NotifyNodeChange nodeChange(this);
            try {
                addFace(face);
                rebuildGeometry(worldBounds);
                return !m_faces.empty();
            } catch (GeometryException&) {
                return false;
            }
        }

        bool Brush::canMoveBoundary(const BBox3& worldBounds, const BrushFace* face, const Vec3& delta) const {
            BrushFace* testFace = face->clone();
            testFace->transform(translationMatrix(delta), false);
            
            BrushFaceList testFaces;
            testFaces.push_back(testFace);
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                if (brushFace != face)
                    testFaces.push_back(brushFace);
            }
            
            BrushGeometry testGeometry(worldBounds);
            CanMoveBoundary canMove(testGeometry, testFaces);
            const bool inWorldBounds = worldBounds.contains(testGeometry.bounds()) && testGeometry.closed();

            restoreFaceLinks(m_geometry);
            delete testFace;
            
            return (inWorldBounds &&
                    !canMove.brushEmpty() &&
                    !canMove.hasRedundandFaces() &&
                    !canMove.hasDroppedFaces());
        }
        
        void Brush::moveBoundary(const BBox3& worldBounds, BrushFace* face, const Vec3& delta, const bool lockTexture) {
            assert(canMoveBoundary(worldBounds, face, delta));
            
            const NotifyNodeChange nodeChange(this);
            face->transform(translationMatrix(delta), lockTexture);
            rebuildGeometry(worldBounds);
        }

        size_t Brush::vertexCount() const {
            assert(m_geometry != NULL);
            return m_geometry->vertexCount();
        }
        
        Brush::VertexList Brush::vertices() const {
            assert(m_geometry != NULL);
            return VertexList(m_geometry->vertices());
        }
        
        size_t Brush::edgeCount() const {
            assert(m_geometry != NULL);
            return m_geometry->edgeCount();
        }
        
        Brush::EdgeList Brush::edges() const {
            assert(m_geometry != NULL);
            return EdgeList(m_geometry->edges());
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

        BrushFaceList Brush::incidentFaces(const BrushVertex* vertex) const {
            BrushFaceList result;
            result.reserve(m_faces.size());

            BrushHalfEdge* first = vertex->leaving();
            BrushHalfEdge* current = first;
            do {
                result.push_back(current->face()->payload());
                current = current->nextIncident();
            } while (current != first);
            
            return result;
        }

        bool Brush::canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!vertexPositions.empty());
            if (delta.null())
                return false;
            
            BrushGeometry testGeometry(*m_geometry);
            const SetTempFaceLinks setFaceLinks(this, testGeometry);
            
            const BrushGeometry::MoveVerticesResult result = testGeometry.moveVertices(vertexPositions, delta, true);
            return !result.hasUnchangedVertices() && !result.hasUnknownVertices() && worldBounds.contains(testGeometry.bounds());
        }
        
        Vec3::List Brush::moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!vertexPositions.empty());
            assert(canMoveVertices(worldBounds, vertexPositions, delta));
            
            const NotifyNodeChange nodeChange(this);
            MoveVerticesCallback callback;
            const BrushGeometry::MoveVerticesResult result = m_geometry->moveVertices(vertexPositions, delta, true, callback);
            updateFacesFromGeometry(worldBounds);
            nodeBoundsDidChange();
            
            return result.newVertexPositions;
        }

        Vec3::List Brush::snapVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const size_t snapTo) {
            assert(m_geometry != NULL);
            assert(!vertexPositions.empty());
            
            const FloatType snapToF = static_cast<FloatType>(snapTo);
            Vec3::Set newVertexPositions;
            
            const NotifyNodeChange nodeChange(this);
            MoveVerticesCallback callback;

            Vec3::List::const_iterator it, end;
            for (it = vertexPositions.begin(), end = vertexPositions.end(); it != end; ++it) {
                const Vec3 origin = *it;
                const Vec3 destination = snapToF * origin.rounded() / snapToF;
                if (!origin.equals(destination)) {
                    const Vec3 delta = destination - origin;
                    const BrushGeometry::MoveVerticesResult result = m_geometry->moveVertices(Vec3::List(1, origin), delta, true, callback);
                    newVertexPositions.insert(result.newVertexPositions.front());
                }
            }

            updateFacesFromGeometry(worldBounds);
            updatePointsFromVertices(worldBounds);
            nodeBoundsDidChange();
            
            return Vec3::List(newVertexPositions.begin(), newVertexPositions.end());
        }

        bool Brush::canMoveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!edgePositions.empty());
            if (delta.null())
                return true;
            
            BrushGeometry testGeometry(*m_geometry);
            const SetTempFaceLinks setFaceLinks(this, testGeometry);

            const Vec3::List vertexPositions = Edge3::asVertexList(edgePositions);
            const BrushGeometry::MoveVerticesResult result = testGeometry.moveVertices(vertexPositions, delta, false);
            if (!result.allVerticesMoved() || !worldBounds.contains(testGeometry.bounds()))
                return false;
            
            Edge3::List::const_iterator it, end;
            for (it = edgePositions.begin(), end = edgePositions.end(); it != end; ++it) {
                const Edge3& edge = *it;
                const Edge3 newEdge(edge.start() + delta, edge.end() + delta);
                if (!testGeometry.hasEdge(newEdge.start(), newEdge.end()))
                    return false;
            }
            
            return true;
        }
        
        Edge3::List Brush::moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!edgePositions.empty());
            assert(canMoveEdges(worldBounds, edgePositions, delta));
            
            const NotifyNodeChange nodeChange(this);
            MoveVerticesCallback callback;
            const Vec3::List vertexPositions = Edge3::asVertexList(edgePositions);
            m_geometry->moveVertices(vertexPositions, delta, false, callback);
            updateFacesFromGeometry(worldBounds);
            nodeBoundsDidChange();
            
            Edge3::List result;
            result.reserve(edgePositions.size());
            
            Edge3::List::const_iterator it, end;
            for (it = edgePositions.begin(), end = edgePositions.end(); it != end; ++it) {
                const Edge3& edge = *it;
                const Edge3 newEdge(edge.start() + delta, edge.end() + delta);
                assert(m_geometry->hasEdge(newEdge.start(), newEdge.end()));
                result.push_back(newEdge);
            }
            
            return result;
        }
        
        bool Brush::canSplitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            if (delta.null())
                return false;
            
            BrushGeometry testGeometry(*m_geometry);
            const SetTempFaceLinks setFaceLinks(this, testGeometry);
            
            const BrushGeometry::MoveVerticesResult result = testGeometry.splitEdge(edgePosition.start(), edgePosition.end(), delta);
            return result.allVerticesMoved() && worldBounds.contains(testGeometry.bounds());
        }
        
        Vec3 Brush::splitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canSplitEdge(worldBounds, edgePosition, delta));
            
            const NotifyNodeChange nodeChange(this);

            MoveVerticesCallback callback;
            const BrushGeometry::MoveVerticesResult result = m_geometry->splitEdge(edgePosition.start(), edgePosition.end(), delta, callback);
            
            assert(result.allVerticesMoved());
            updateFacesFromGeometry(worldBounds);
            nodeBoundsDidChange();
            
            return result.newVertexPositions.front();
        }

        bool Brush::canMoveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!facePositions.empty());
            if (delta.null())
                return false;
            
            BrushGeometry testGeometry(*m_geometry);
            const SetTempFaceLinks setFaceLinks(this, testGeometry);
            
            const Vec3::List vertexPositions = Polygon3::asVertexList(facePositions);
            const BrushGeometry::MoveVerticesResult result = testGeometry.moveVertices(vertexPositions, delta, false);

            if (!result.allVerticesMoved() || !worldBounds.contains(testGeometry.bounds()))
                return false;

            Polygon3::List::const_iterator fIt, fEnd;
            for (fIt = facePositions.begin(), fEnd = facePositions.end(); fIt != fEnd; ++fIt) {
                const Polygon3& face = *fIt;
                const Polygon3 newFace(face.vertices() + delta);
                if (!testGeometry.hasFace(newFace.vertices()))
                    return false;
            }

            return true;
        }
        
        Polygon3::List Brush::moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!facePositions.empty());
            assert(canMoveFaces(worldBounds, facePositions, delta));
            
            const NotifyNodeChange nodeChange(this);
            MoveVerticesCallback callback;
            const Vec3::List vertexPositions = Polygon3::asVertexList(facePositions);
            m_geometry->moveVertices(vertexPositions, delta, false, callback);
            updateFacesFromGeometry(worldBounds);
            nodeBoundsDidChange();
            
            Polygon3::List result;
            result.reserve(facePositions.size());
            
            Polygon3::List::const_iterator fIt, fEnd;
            for (fIt = facePositions.begin(), fEnd = facePositions.end(); fIt != fEnd; ++fIt) {
                const Polygon3& face = *fIt;
                const Polygon3 newFace(face.vertices() + delta);
                assert(m_geometry->hasFace(newFace.vertices()));
                result.push_back(newFace);
            }
            
            return result;
        }

        bool Brush::canSplitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            if (delta.null())
                return false;
            
            BrushGeometry testGeometry(*m_geometry);
            const SetTempFaceLinks setFaceLinks(this, testGeometry);
            const BrushGeometry::MoveVerticesResult result = testGeometry.splitFace(facePosition.vertices(), delta);
            return result.allVerticesMoved() && worldBounds.contains(testGeometry.bounds());
        }
        
        Vec3 Brush::splitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canSplitFace(worldBounds, facePosition, delta));
            
            const NotifyNodeChange nodeChange(this);
            
            MoveVerticesCallback callback;
            const BrushGeometry::MoveVerticesResult result = m_geometry->splitFace(facePosition.vertices(), delta, callback);
            assert(result.allVerticesMoved());
            updateFacesFromGeometry(worldBounds);
            nodeBoundsDidChange();
            
            return result.newVertexPositions.front();
        }

        BrushList Brush::subtract(const ModelFactory& factory, const BBox3& worldBounds, const String& defaultTextureName, const Brush* subtrahend) const {
            const BrushGeometry::SubtractResult result = m_geometry->subtract(*subtrahend->m_geometry);
            
            BrushList brushes(0);
            brushes.reserve(result.size());
            
            BrushGeometry::SubtractResult::const_iterator it, end;
            for (it = result.begin(), end = result.end(); it != end; ++it) {
                const BrushGeometry& geometry = *it;
                Brush* brush = createBrush(factory, worldBounds, defaultTextureName, geometry, subtrahend);
                brushes.push_back(brush);
            }
            
            return brushes;
        }

        bool Brush::intersect(const BBox3& worldBounds, const Brush* brush) {
            const BrushFaceList& theirFaces = brush->faces();
            
            BrushFaceList::const_iterator it, end;
            for (it = theirFaces.begin(), end = theirFaces.end(); it != end; ++it) {
                const BrushFace* theirFace = *it;
                addFace(theirFace->clone());
            }
            
            return rebuildGeometry(worldBounds);
        }

        BrushList Brush::partition(const ModelFactory& factory, const BBox3& worldBounds, const String& defaultTextureName, const Brush* other) const {
            Brush* intersection = clone(worldBounds);
            if (!intersection->intersect(worldBounds, other)) {
                delete intersection;
                return EmptyBrushList;
            }
            
            BrushList result(1, intersection);
            VectorUtils::append(result,  this->subtract(factory, worldBounds, defaultTextureName, intersection));
            VectorUtils::append(result, other->subtract(factory, worldBounds, defaultTextureName, intersection));
            
            return result;
        }

        Brush* Brush::createBrush(const ModelFactory& factory, const BBox3& worldBounds, const String& defaultTextureName, const BrushGeometry& geometry, const Brush* subtrahend) const {
            BrushFaceList faces(0);
            faces.reserve(geometry.faceCount());
            
            BrushGeometry::Face* firstFace = geometry.faces().front();
            BrushGeometry::Face* currentFace = firstFace;
            do {
                const BrushGeometry::HalfEdge* h1 = currentFace->boundary().front();
                const BrushGeometry::HalfEdge* h0 = h1->next();
                const BrushGeometry::HalfEdge* h2 = h0->next();
                
                const Vec3& p0 = h0->origin()->position();
                const Vec3& p1 = h1->origin()->position();
                const Vec3& p2 = h2->origin()->position();
                
                BrushFaceAttributes attribs(defaultTextureName);
                faces.push_back(factory.createFace(p0, p1, p2, attribs));
                
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
            
            Brush* brush = factory.createBrush(worldBounds, faces);
            brush->cloneFaceAttributesFrom(this);
            brush->cloneInvertedFaceAttributesFrom(subtrahend);
            return brush;
        }

        void Brush::updateFacesFromGeometry(const BBox3& worldBounds) {
            m_faces.clear();
            
            BrushGeometry::Face* first = m_geometry->faces().front();
            BrushGeometry::Face* current = first;
            do {
                BrushFace* face = current->payload();
                if (face != NULL) { // could happen if the brush isn't fully specified
                    if (face->brush() == NULL)
                        addFace(face);
                    else
                        m_faces.push_back(face);
                }
                current = current->next();
            } while (current != first);
            
            invalidateContentType();
        }

        void Brush::updatePointsFromVertices(const BBox3& worldBounds) {
            BrushGeometry::Face* first = m_geometry->faces().front();
            BrushGeometry::Face* current = first;
            do {
                BrushFace* face = current->payload();
                face->updatePointsFromVertices();
                current = current->next();
            } while (current != first);
            
            rebuildGeometry(worldBounds);
        }

        bool Brush::rebuildGeometry(const BBox3& worldBounds) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds.expanded(1.0));
            
            AddFacesToGeometry addFacesToGeometry(*m_geometry, m_faces);
            updateFacesFromGeometry(worldBounds);
            if (!fullySpecified())
                throw GeometryException("Brush is not fully specified");
            nodeBoundsDidChange();
            return !addFacesToGeometry.brushEmpty();
        }

        void Brush::findIntegerPlanePoints(const BBox3& worldBounds) {
            const NotifyNodeChange nodeChange(this);
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                brushFace->findIntegerPlanePoints();
            }
            rebuildGeometry(worldBounds);
        }

        bool Brush::checkGeometry() const {
            BrushFaceList::const_iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                const BrushFace* face = *fIt;
                if (face->geometry() == NULL)
                    return false;
                if (!m_geometry->faces().contains(face->geometry()))
                    return false;
            }
            
            const BrushGeometry::FaceList& faceGeometries = m_geometry->faces();
            BrushGeometry::FaceList::const_iterator gIt, gEnd;
            for (gIt = faceGeometries.begin(), gEnd = faceGeometries.end(); gIt != gEnd; ++gIt) {
                const BrushFaceGeometry* geometry = *gIt;
                if (geometry->payload() == NULL)
                    return false;
                if (!VectorUtils::contains(m_faces, geometry->payload()))
                    return false;
            }
            
            return true;
        }

        bool Brush::transparent() const {
            if (!m_contentTypeValid)
                validateContentType();
            return m_transparent;
        }
        
        bool Brush::hasContentType(const BrushContentType& contentType) const {
            return hasContentType(contentType.flagValue());
        }
        
        bool Brush::hasContentType(const BrushContentType::FlagType contentTypeMask) const {
            return (contentTypeFlags() & contentTypeMask) != 0;
        }

        void Brush::setContentTypeBuilder(const BrushContentTypeBuilder* contentTypeBuilder) {
            m_contentTypeBuilder = contentTypeBuilder;
            invalidateContentType();
        }

        BrushContentType::FlagType Brush::contentTypeFlags() const {
            if (!m_contentTypeValid)
                validateContentType();
            return m_contentType;
        }

        void Brush::invalidateContentType() {
            m_contentTypeValid = false;
        }
        
        void Brush::validateContentType() const {
            assert(!m_contentTypeValid);
            if (m_contentTypeBuilder != NULL) {
                const BrushContentTypeBuilder::Result result = m_contentTypeBuilder->buildContentType(this);
                m_contentType = result.contentType;
                m_transparent = result.transparent;
                m_contentTypeValid = true;
            }
        }

        const String& Brush::doGetName() const {
            static const String name("brush");
            return name;
        }

        const BBox3& Brush::doGetBounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds();
        }

        Node* Brush::doClone(const BBox3& worldBounds) const {
            BrushFaceList faceClones;
            faceClones.reserve(m_faces.size());
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                faceClones.push_back(face->clone());
            }
            
            Brush* brush = new Brush(worldBounds, faceClones);
            brush->setContentTypeBuilder(m_contentTypeBuilder);
            cloneAttributes(brush);
            return brush;
        }

        bool Brush::doCanAddChild(const Node* child) const {
            return false;
        }
        
        bool Brush::doCanRemoveChild(const Node* child) const {
            return false;
        }
        
        bool Brush::doRemoveIfEmpty() const {
            return false;
        }

        void Brush::doParentDidChange() {
            invalidateContentType();
        }

        bool Brush::doSelectable() const {
            return true;
        }
        
        void Brush::doGenerateIssues(const IssueGenerator* generator, IssueList& issues) {
            generator->generate(this, issues);
        }

        void Brush::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Brush::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void Brush::doPick(const Ray3& ray, PickResult& pickResult) const {
            const BrushFaceHit hit = findFaceHit(ray);
            if (hit.face != NULL) {
                assert(!Math::isnan(hit.distance));
                const Vec3 hitPoint = ray.pointAtDistance(hit.distance);
                pickResult.addHit(Hit(BrushHit, hit.distance, hitPoint, hit.face));
            }
        }
        
        FloatType Brush::doIntersectWithRay(const Ray3& ray) const {
            const BrushFaceHit hit = findFaceHit(ray);
            return hit.distance;
        }

		Brush::BrushFaceHit::BrushFaceHit() : face(NULL), distance(Math::nan<FloatType>()) {}

        Brush::BrushFaceHit::BrushFaceHit(BrushFace* i_face, const FloatType i_distance) : face(i_face), distance(i_distance) {}

        Brush::BrushFaceHit Brush::findFaceHit(const Ray3& ray) const {
            if (Math::isnan(bounds().intersectWithRay(ray)))
                return BrushFaceHit();
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                const FloatType distance = face->intersectWithRay(ray);
                if (!Math::isnan(distance))
                    return BrushFaceHit(face, distance);
            }
            return BrushFaceHit();
        }

        Node* Brush::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }

        Layer* Brush::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }
        
        Group* Brush::doGetGroup() const {
            FindGroupVisitor visitor(false);
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }
        
        void Brush::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {
            const NotifyNodeChange nodeChange(this);

            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->transform(transformation, lockTextures);
            }
            rebuildGeometry(worldBounds);
        }
        
        class Brush::Contains : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const Brush* m_this;
        public:
            Contains(const Brush* i_this) :
            m_this(i_this) {}
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(false); }
            void doVisit(const Group* group)   { setResult(contains(group->bounds())); }
            void doVisit(const Entity* entity) { setResult(contains(entity->bounds())); }
            void doVisit(const Brush* brush)   { setResult(contains(brush)); }
            
            bool contains(const BBox3& bounds) const {
                if (m_this->bounds().contains(bounds))
                    return true;
                const Vec3::List vertices = bBoxVertices(bounds);
                for (size_t i = 0; i < vertices.size(); ++i) {
                    if (!m_this->containsPoint(vertices[i]))
                        return false;
                }
                return true;
            }
            
            bool contains(const Brush* brush) const {
                return m_this->m_geometry->contains(*brush->m_geometry, QueryCallback());
            }
        };

        bool Brush::doContains(const Node* node) const {
            Contains contains(this);
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }
        
        class Brush::Intersects : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const Brush* m_this;
        public:
            Intersects(const Brush* i_this) :
            m_this(i_this) {}
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(false); }
            void doVisit(const Group* group)   { setResult(intersects(group->bounds())); }
            void doVisit(const Entity* entity) { setResult(intersects(entity->bounds())); }
            void doVisit(const Brush* brush)   { setResult(intersects(brush)); }

            bool intersects(const BBox3& bounds) const {
                return m_this->bounds().intersects(bounds);
            }
            
            bool intersects(const Brush* brush) {
                return m_this->m_geometry->intersects(*brush->m_geometry, QueryCallback());
            }
        };
        
        bool Brush::doIntersects(const Node* node) const {
            Intersects intersects(this);
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }
    }
}
