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
            nodeWillChange();
            detachFaces(m_faces);
            VectorUtils::clearAndDelete(m_faces);
            addFaces(faces);
            rebuildGeometry(worldBounds);
            nodeDidChange();
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
            const BrushFaceGeometryList sides = m_geometry->incidentSides(vertex);
            BrushFaceList result;
            result.reserve(sides.size());
            
            BrushFaceGeometryList::const_iterator it, end;
            for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                const BrushFaceGeometry& side = **it;
                result.push_back(side.face);
            }
            
            return result;
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

        Vec3::List Brush::snapVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const size_t snapTo) {
            assert(m_geometry != NULL);
            assert(!vertexPositions.empty());
            
            nodeWillChange();
            const SnapVerticesResult result = m_geometry->snapVertices(vertexPositions, snapTo);
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

        void Brush::findIntegerPlanePoints(const BBox3& worldBounds) {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* brushFace = *it;
                brushFace->findIntegerPlanePoints();
            }
            rebuildGeometry(worldBounds);
            nodeDidChange();
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
        
        bool Brush::doRemoveIfEmpty() const {
            return false;
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

        const BBox3& Brush::doGetBounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds;
        }

        void Brush::doPick(const Ray3& ray, PickResult& pickResult) const {
            if (Math::isnan(bounds().intersectWithRay(ray)))
                return;
            
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                const FloatType distance = face->intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    pickResult.addHit(Hit(BrushHit, distance, hitPoint, face));
                    break;
                }
            }
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
            FindGroupVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }
        
        void Brush::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {
            nodeWillChange();
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->transform(transformation, lockTextures);
            }
            rebuildGeometry(worldBounds);
            nodeDidChange();
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
                const BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    if (!m_this->containsPoint(vertices[i]->position))
                        return false;
                }
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
        
        bool Brush::doIntersects(const Node* node) const {
            Intersects intersects(this);
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }
    }
}
