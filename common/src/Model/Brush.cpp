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
            BrushFaceList& m_droppedFaces;
        public:
            AddFaceToGeometryCallback(BrushFace* addedFace, BrushFaceList& droppedFaces) :
            m_addedFace(addedFace),
            m_droppedFaces(droppedFaces) {
                assert(m_addedFace != NULL);
            }
            
            void faceWasCreated(BrushGeometry::Face* face) {
                face->setPayload(m_addedFace);
                m_addedFace->setGeometry(face);
            }

            void faceWillBeDeleted(BrushGeometry::Face* face) {
                BrushFace* brushFace = face->payload();
                if (brushFace != NULL) {
                    assert(!VectorUtils::contains(m_droppedFaces, brushFace));
                    m_droppedFaces.push_back(brushFace);
                }
            }
        };

        class Brush::AddFacesToGeometry {
        private:
            BrushGeometry& m_geometry;
            BrushFaceList m_addedFaces;
            BrushFaceList m_droppedFaces;
            BrushFaceList m_redundandFaces;
            bool m_brushEmpty;
        public:
            AddFacesToGeometry(BrushGeometry& geometry, const BrushFaceList& facesToAdd) :
            m_geometry(geometry),
            m_brushEmpty(false) {
                BrushFaceList::const_iterator it, end;
                size_t droppedFaceCount = 0;
                for (it = facesToAdd.begin(), end = facesToAdd.end(); it != end && !m_brushEmpty; ++it) {
                    BrushFace* face = *it;
                    AddFaceToGeometryCallback callback(face, m_droppedFaces);
                    const BrushGeometry::ClipResult result = m_geometry.clip(face->boundary(), callback);
                    if (result.unchanged())
                        m_redundandFaces.push_back(face);
                    else if (result.empty())
                        m_brushEmpty = true;
                    else
                        m_addedFaces.push_back(face);
                    if (droppedFaceCount < m_droppedFaces.size()) {
                        BrushFaceList::iterator dIt = m_droppedFaces.begin();
                        std::advance(dIt, droppedFaceCount);
                        VectorUtils::eraseAll(m_addedFaces, dIt, m_droppedFaces.end());
                        droppedFaceCount = m_droppedFaces.size();
                    }
                }
            }
            
            const BrushFaceList& addedFaces() const {
                return m_addedFaces;
            }
            
            const BrushFaceList& droppedFaces() const {
                return m_droppedFaces;
            }
            
            const BrushFaceList& redundandFaces() const {
                return m_redundandFaces;
            }
            
            bool hasRedundandFaces() const {
                return !m_redundandFaces.empty();
            }
            
            bool hasDroppedFaces() const {
                return !m_droppedFaces.empty();
            }
            
            bool brushEmpty() const {
                return m_brushEmpty;
            }
            
            bool fullySpecified() const {
                BrushFaceGeometry* first = m_geometry.faces().front();
                BrushFaceGeometry* current = first;
                do {
                    if (current->payload() == NULL)
                        return false;
                    current = current->next();
                } while (current != first);
                return true;
            }
        };

        class Brush::MoveVerticesCallback : public BrushGeometry::Callback {
        private:
            typedef std::map<BrushFace*, BrushFace*> ClonedFacesMap;
            
            // Maps cloned faces to the faces of which they were originally cloned.
            // If a face is created from a clone, the new face should also map to the original of the clone.
            ClonedFacesMap m_clonedFaces;
            BrushFaceList m_droppedFaces;
        public:
            void faceWillBeDeleted(BrushFaceGeometry* faceGeometry) {
                BrushFace* face = faceGeometry->payload();
                ClonedFacesMap::iterator it = m_clonedFaces.find(face);
                if (it != m_clonedFaces.end()) {
                    m_clonedFaces.erase(it);
                    delete face;
                } else {
                    m_droppedFaces.push_back(face);
                }
            }
            
            void faceDidChange(BrushFaceGeometry* faceGeometry) {
                BrushFace* face = faceGeometry->payload();
                face->updatePointsFromVertices();
            }
            
            void faceWasSplit(BrushFaceGeometry* originalGeometry, BrushFaceGeometry* cloneGeometry) {
                BrushFace* originalFace = findOriginal(originalGeometry->payload());
                assert(originalFace != NULL);
                assert(cloneGeometry->payload() == NULL);
                
                BrushFace* clonedFace = originalFace->clone();
                cloneGeometry->setPayload(clonedFace);
                clonedFace->setGeometry(cloneGeometry);

                m_clonedFaces[clonedFace] = originalFace;
            }
            
            void facesWillBeMerged(BrushFaceGeometry* remainingGeometry, BrushFaceGeometry* geometryToDelete) {
                BrushFace* remainingFace = remainingGeometry->payload();
                BrushFace* faceToDelete = geometryToDelete->payload();
                assert(remainingFace != NULL);
                assert(faceToDelete != NULL);
                
                ClonedFacesMap::iterator remainingFaceIt = m_clonedFaces.find(remainingFace);
                ClonedFacesMap::iterator faceToDeleteIt = m_clonedFaces.find(faceToDelete);

                if (faceToDeleteIt != m_clonedFaces.end()) {
                    // If the face to delete is a clone, we just delete it:
                    m_clonedFaces.erase(faceToDeleteIt);
                    delete faceToDelete;
                } else {
                    // The face to delete is an original face.
                    if (remainingFaceIt != m_clonedFaces.end()) {
                        // If the remaining face is a clone, we swap them
                        swapFaces(remainingGeometry, geometryToDelete);
                        // and then we delete the remaining face
                        m_clonedFaces.erase(remainingFaceIt);
                        delete remainingFace;
                    } else {
                        // The remaining face is an original, too, we add the face to delete to the dropped faces:
                        m_droppedFaces.push_back(faceToDelete);
                    }
                }
            }
            
            bool hasAddedFaces() const {
                return !m_clonedFaces.empty();
            }
            
            bool hasDroppedFaces() const {
                return m_droppedFaces.empty();
            }
            
            BrushFaceList addedFaces() const {
                return MapUtils::keyList(m_clonedFaces);
            }
            
            const BrushFaceList& droppedFaces() const {
                return m_droppedFaces;
            }
        private:
            BrushFace* findOriginal(BrushFace* possibleClone) const {
                return MapUtils::find(m_clonedFaces, possibleClone, possibleClone);
            }
            
            void swapFaces(BrushFaceGeometry* geometry1, BrushFaceGeometry* geometry2) const {
                BrushFace* face1 = geometry1->payload();
                BrushFace* face2 = geometry2->payload();
                
                geometry1->setPayload(face2);
                face2->setGeometry(geometry1);
                geometry2->setPayload(face1);
                face1->setGeometry(geometry2);
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
            AddFacesToGeometry addFaces(testGeometry, testFaces);
            const bool inWorldBounds = worldBounds.contains(testGeometry.bounds()) && testGeometry.closed();

            m_geometry->restoreFaceLinks();
            delete testFace;
            
            return (inWorldBounds &&
                    addFaces.fullySpecified() &&
                    !addFaces.brushEmpty() &&
                    !addFaces.hasRedundandFaces() &&
                    !addFaces.hasDroppedFaces());
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
            updateBrushAfterVertexMove(worldBounds, callback);
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

            updateBrushAfterVertexMove(worldBounds, callback);
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
            return result.allVerticesMoved() && worldBounds.contains(testGeometry.bounds());
        }
        
        Edge3::List Brush::moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!edgePositions.empty());
            assert(canMoveEdges(worldBounds, edgePositions, delta));
            
            const NotifyNodeChange nodeChange(this);
            MoveVerticesCallback callback;
            const Vec3::List vertexPositions = Edge3::asVertexList(edgePositions);
            m_geometry->moveVertices(vertexPositions, delta, false, callback);
            updateBrushAfterVertexMove(worldBounds, callback);
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
            updateBrushAfterVertexMove(worldBounds, callback);
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
            return result.allVerticesMoved() && worldBounds.contains(testGeometry.bounds());
        }
        
        Polygon3::List Brush::moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!facePositions.empty());
            assert(canMoveFaces(worldBounds, facePositions, delta));
            
            const NotifyNodeChange nodeChange(this);
            MoveVerticesCallback callback;
            const Vec3::List vertexPositions = Polygon3::asVertexList(facePositions);
            m_geometry->moveVertices(vertexPositions, delta, false, callback);
            updateBrushAfterVertexMove(worldBounds, callback);
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
            updateBrushAfterVertexMove(worldBounds, callback);
            nodeBoundsDidChange();
            
            return result.newVertexPositions.front();
        }

        void Brush::updateBrushAfterVertexMove(const BBox3& worldBounds, const MoveVerticesCallback& result) {
            if (!result.hasAddedFaces() && !result.hasDroppedFaces())
                return;
            
            const BrushFaceList addedFaces = result.addedFaces();
            const BrushFaceList& droppedFaces = result.droppedFaces();
            
            removeFaces(droppedFaces.begin(), droppedFaces.end());
            VectorUtils::deleteAll(droppedFaces);
            
            invalidateFaces();
            addFaces(addedFaces);

            assert(checkGeometry());
        }

        void Brush::invalidateFaces() {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->invalidate();
            }
        }

        void Brush::rebuildGeometry(const BBox3& worldBounds) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds);
            
            AddFacesToGeometry addFacesToGeometry(*m_geometry, m_faces);
            const BrushFaceList& addedFaces = addFacesToGeometry.addedFaces();
            const BrushFaceList& droppedFaces = addFacesToGeometry.droppedFaces();
            
            detachFaces(droppedFaces);
            VectorUtils::deleteAll(droppedFaces);
            m_faces.clear();

            BrushFaceList::const_iterator it, end;
            for (it = addedFaces.begin(), end = addedFaces.end(); it != end; ++it) {
                BrushFace* face = *it;
                if (face->brush() == NULL)
                    addFace(face);
                else
                    m_faces.push_back(face);
                face->invalidate();
            }

            invalidateContentType();
            nodeBoundsDidChange();
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
                if (!m_this->bounds().contains(brush->bounds()))
                    return false;
                const BrushGeometry::Vertex* first = brush->m_geometry->vertices().front();
                const BrushGeometry::Vertex* current = first;
                do {
                    if (!m_this->containsPoint(current->position()))
                        return false;
                    current = current->next();
                } while (current != first);
                return true;
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
                if (!m_this->bounds().intersects(bounds))
                    return false;
                const Vec3::List vertices = bBoxVertices(bounds);
                for (size_t i = 0; i < vertices.size(); ++i) {
                    if (m_this->containsPoint(vertices[i]))
                        return true;
                }
                return false;
            }
            
            bool intersects(const Brush* brush) {
                if (!m_this->bounds().intersects(brush->bounds()))
                    return false;
                
                // separating axis theorem
                // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
                
                BrushFaceList::const_iterator faceIt, faceEnd;
                
                const BrushVertexList& myVertices = m_this->m_geometry->vertices();
                const BrushFaceList& theirFaces = brush->faces();
                for (faceIt = theirFaces.begin(), faceEnd = theirFaces.end(); faceIt != faceEnd; ++faceIt) {
                    const BrushFace* theirFace = *faceIt;
                    if (pointStatus(theirFace->boundary(), myVertices) == Math::PointStatus::PSAbove)
                        return false;
                }
                
                const BrushVertexList& theirVertices = brush->m_geometry->vertices();
                const BrushFaceList& myFaces = m_this->faces();
                for (faceIt = myFaces.begin(), faceEnd = myFaces.end(); faceIt != faceEnd; ++faceIt) {
                    const BrushFace* myFace = *faceIt;
                    if (pointStatus(myFace->boundary(), theirVertices) == Math::PointStatus::PSAbove)
                        return false;
                }
                
                const EdgeList& myEdges = m_this->m_geometry->edges();
                const EdgeList& theirEdges = brush->m_geometry->edges();
                EdgeList::const_iterator myEdgeIt, myEdgeEnd, theirEdgeIt, theirEdgeEnd;
                for (myEdgeIt = myEdges.begin(), myEdgeEnd = myEdges.end(); myEdgeIt != myEdgeEnd; ++myEdgeIt) {
                    const BrushEdge* myEdge = *myEdgeIt;
                    const Vec3 myEdgeVec = myEdge->vector();
                    const Vec3& origin = myEdge->firstVertex()->position();
                    
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
            
            Math::PointStatus::Type pointStatus(const Plane3& plane, const BrushVertexList& vertices) const {
                size_t above = 0;
                size_t below = 0;
                const BrushVertex* first = vertices.front();
                const BrushVertex* current = first;
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Math::PointStatus::Type status = plane.pointStatus(current->position());
                    if (status == Math::PointStatus::PSAbove)
                        ++above;
                    else if (status == Math::PointStatus::PSBelow)
                        ++below;
                    if (above > 0 && below > 0)
                        return Math::PointStatus::PSInside;
                    current = current->next();
                }
                return above > 0 ? Math::PointStatus::PSAbove : Math::PointStatus::PSBelow;
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
