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
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Brush::BrushHit = Hit::freeHitType();

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

        class FindBrushOwner : public NodeVisitor, public NodeQuery<Attributable*> {
        private:
            void doVisit(World* world)   { setResult(world); cancel(); }
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   {}
            void doVisit(Entity* entity) { setResult(entity); cancel(); }
            void doVisit(Brush* brush)   {}
        };
        
        Attributable* Brush::entity() const {
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
                incFamilyMemberSelectionCount(1);
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
                decFamilyMemberSelectionCount(1);
            face->setBrush(NULL);
            invalidateContentType();
        }
        
        bool Brush::clip(const BBox3& worldBounds, BrushFace* face) {
            nodeWillChange();
            try {
                addFace(face);
                rebuildGeometry(worldBounds);
                nodeDidChange();
                return !m_faces.empty();
            } catch (GeometryException&) {
                nodeDidChange();
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
            const AddFaceResult result = testGeometry.addFaces(testFaces);
            const bool inWorldBounds = worldBounds.contains(testGeometry.bounds) && testGeometry.isClosed();

            m_geometry->restoreFaceGeometries();
            delete testFace;
            
            return (inWorldBounds &&
                    result.resultCode != AddFaceResult::Code_BrushNull &&
                    result.resultCode != AddFaceResult::Code_FaceRedundant &&
                    result.droppedFaces.empty());
        }
        
        void Brush::moveBoundary(const BBox3& worldBounds, BrushFace* face, const Vec3& delta, const bool lockTexture) {
            assert(canMoveBoundary(worldBounds, face, delta));
            
            nodeWillChange();
            face->transform(translationMatrix(delta), lockTexture);
            rebuildGeometry(worldBounds);
            nodeDidChange();
        }

        size_t Brush::vertexCount() const {
            assert(m_geometry != NULL);
            return m_geometry->vertices.size();
        }
        
        const BrushVertexList& Brush::vertices() const {
            assert(m_geometry != NULL);
            return m_geometry->vertices;
        }
        
        size_t Brush::edgeCount() const {
            assert(m_geometry != NULL);
            return m_geometry->edges.size();
        }
        
        const BrushEdgeList& Brush::edges() const {
            assert(m_geometry != NULL);
            return m_geometry->edges;
        }
        
        bool Brush::canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!vertexPositions.empty());
            const bool result = m_geometry->canMoveVertices(worldBounds, vertexPositions, delta);
            assert(checkGeometry());
            return result;
        }
        
        Vec3::List Brush::moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!vertexPositions.empty());
            assert(canMoveVertices(worldBounds, vertexPositions, delta));
            
            nodeWillChange();
            const MoveVerticesResult result = m_geometry->moveVertices(worldBounds, vertexPositions, delta);
            processBrushAlgorithmResult(worldBounds, result);
            nodeDidChange();
            
            return result.newVertexPositions;
        }

        bool Brush::canMoveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!edgePositions.empty());
            const bool result = m_geometry->canMoveEdges(worldBounds, edgePositions, delta);
            assert(checkGeometry());
            return result;
        }
        
        Edge3::List Brush::moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!edgePositions.empty());
            assert(canMoveEdges(worldBounds, edgePositions, delta));
            
            nodeWillChange();
            const MoveEdgesResult result = m_geometry->moveEdges(worldBounds, edgePositions, delta);
            processBrushAlgorithmResult(worldBounds, result);
            nodeDidChange();
            
            return result.newEdgePositions;
        }
        
        bool Brush::canSplitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canSplitEdge(worldBounds, edgePosition, delta);
            assert(checkGeometry());
            return result;
        }
        
        Vec3 Brush::splitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canSplitEdge(worldBounds, edgePosition, delta));
            
            nodeWillChange();
            const SplitResult result = m_geometry->splitEdge(worldBounds, edgePosition, delta);
            processBrushAlgorithmResult(worldBounds, result);
            nodeDidChange();
            
            return result.newVertexPosition;
        }

        bool Brush::canMoveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!facePositions.empty());
            const bool result = m_geometry->canMoveFaces(worldBounds, facePositions, delta);
            assert(checkGeometry());
            return result;
        }
        
        Polygon3::List Brush::moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(!facePositions.empty());
            assert(canMoveFaces(worldBounds, facePositions, delta));
            
            nodeWillChange();
            const MoveFacesResult result = m_geometry->moveFaces(worldBounds, facePositions, delta);
            processBrushAlgorithmResult(worldBounds, result);
            nodeDidChange();
            
            return result.newFacePositions;
        }

        bool Brush::canSplitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            const bool result = m_geometry->canSplitFace(worldBounds, facePosition, delta);
            assert(checkGeometry());
            return result;
        }
        
        Vec3 Brush::splitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            assert(m_geometry != NULL);
            assert(canSplitFace(worldBounds, facePosition, delta));
            
            nodeWillChange();
            const SplitResult result = m_geometry->splitFace(worldBounds, facePosition, delta);
            processBrushAlgorithmResult(worldBounds, result);
            nodeDidChange();
            
            return result.newVertexPosition;
        }

        void Brush::processBrushAlgorithmResult(const BBox3& worldBounds, const BrushAlgorithmResult& result) {
            if (result.addedFaces.empty() && result.droppedFaces.empty())
                return;
            
            removeFaces(result.droppedFaces.begin(), result.droppedFaces.end());
            VectorUtils::deleteAll(result.droppedFaces);
            
            invalidateFaces();
            addFaces(result.addedFaces);

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
            const AddFaceResult result = m_geometry->addFaces(m_faces);
            
            detachFaces(result.droppedFaces);
            VectorUtils::deleteAll(result.droppedFaces);
            m_faces.clear();

            BrushFaceList::const_iterator it, end;
            for (it = result.addedFaces.begin(), end = result.addedFaces.end(); it != end; ++it) {
                BrushFace* face = *it;
                if (face->brush() == NULL)
                    addFace(face);
                else
                    m_faces.push_back(face);
                face->invalidate();
            }

            invalidateContentType();
        }

        bool Brush::checkGeometry() const {
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

        void Brush::setContentTypeBuilder(BrushContentTypeBuilder* contentTypeBuilder) {
            m_contentTypeBuilder = contentTypeBuilder;
            invalidateContentType();
        }

        void Brush::invalidateContentType() {
            m_contentTypeValid = false;
        }
        
        void Brush::validateContentType() const {
            if (m_contentTypeBuilder != NULL) {
                const BrushContentTypeBuilder::Result result = m_contentTypeBuilder->buildContentType(this);
                m_contentType = result.contentType;
                m_transparent = result.transparent;
                m_contentTypeValid = true;
            }
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
            return brush;
        }

        bool Brush::doCanAddChild(const Node* child) const {
            return false;
        }
        
        bool Brush::doCanRemoveChild(const Node* child) const {
            return false;
        }
        
        bool Brush::doSelectable() const {
            return true;
        }
        
        void Brush::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Brush::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        const BBox3& Brush::doGetBounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds;
        }
        
        void Brush::doPick(const Ray3& ray, Hits& hits) const {
            if (Math::isnan(bounds().intersectWithRay(ray)))
                return;
            
            BrushFaceList::const_iterator it, end;
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

        void Brush::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {}
        bool Brush::doContains(const Node* node) const {}
        bool Brush::doIntersects(const Node* node) const {}
    }
}
