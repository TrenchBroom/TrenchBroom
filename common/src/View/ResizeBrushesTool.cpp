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

#include "ResizeBrushesTool.h"

#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/FindMatchingBrushFaceVisitor.h"
#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
#include "Model/NodeVisitor.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType ResizeBrushesTool::ResizeHit = Hit::freeHitType();

        ResizeBrushesTool::ResizeBrushesTool(MapDocumentWPtr document) :
        Tool(true),
        m_document(document) {}
        
        bool ResizeBrushesTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().hasBrushes();
        }
        
        Hit ResizeBrushesTool::pick(const Ray3& pickRay, const Hits& hits) {
            MapDocumentSPtr document = lock(m_document);
            const Hit& hit = Model::firstHit(hits, Model::Brush::BrushHit, document->editorContext(), true, true);
            if (hit.isMatch())
                return Hit(ResizeHit, hit.distance(), hit.hitPoint(), Model::hitToFace(hit));
            return pickProximateFace(pickRay);
        }
        
        class ResizeBrushesTool::PickProximateFace : public Model::ConstNodeVisitor, public Model::NodeQuery<Hit> {
        private:
            const Ray3& m_pickRay;
            FloatType m_closest;
        public:
            PickProximateFace(const Ray3& pickRay) :
            NodeQuery(Hit::NoHit),
            m_pickRay(pickRay),
            m_closest(std::numeric_limits<FloatType>::max()) {}
        private:
            void doVisit(const Model::World* world)   {}
            void doVisit(const Model::Layer* layer)   {}
            void doVisit(const Model::Group* group)   {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush)   {
                const Model::BrushEdgeList& edges = brush->edges();
                Model::BrushEdgeList::const_iterator it, end;
                for (it = edges.begin(), end = edges.end(); it != end; ++it)
                    visitEdge(*it);
            }
            
            void visitEdge(Model::BrushEdge* edge) {
                Model::BrushFace* left = edge->leftFace();
                Model::BrushFace* right = edge->rightFace();
                const double leftDot = left->boundary().normal.dot(m_pickRay.direction);
                const double rightDot = right->boundary().normal.dot(m_pickRay.direction);
                
                if ((leftDot > 0.0) != (rightDot > 0.0)) {
                    const Ray3::LineDistance result = m_pickRay.distanceToSegment(edge->start->position, edge->end->position);
                    if (!Math::isnan(result.distance) && result.distance < m_closest) {
                        m_closest = result.distance;
                        const Vec3 hitPoint = m_pickRay.pointAtDistance(result.distance);
                        Model::BrushFace* face = leftDot > rightDot ? left : right;
                        setResult(Hit(ResizeBrushesTool::ResizeHit, result.rayDistance, hitPoint, face));
                    }
                }
            }
        };
        
        Hit ResizeBrushesTool::pickProximateFace(const Ray3& pickRay) const {
            PickProximateFace visitor(pickRay);
            
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
            
            if (!visitor.hasResult())
                return Hit::NoHit;
            return visitor.result();
        }

        bool ResizeBrushesTool::hasDragFaces() const {
            return !m_dragFaces.empty();
        }
        
        const Model::BrushFaceList& ResizeBrushesTool::dragFaces() const {
            return m_dragFaces;
        }
        
        void ResizeBrushesTool::updateDragFaces(const Hits& hits) {
            const Hit& hit = hits.findFirst(ResizeHit, true);
            if (!hit.isMatch())
                m_dragFaces.clear();
            else
                m_dragFaces = collectDragFaces(hit.target<Model::BrushFace*>());
        }
        
        class ResizeBrushesTool::MatchFaceBoundary {
        private:
            const Model::BrushFace* m_reference;
        public:
            MatchFaceBoundary(const Model::BrushFace* reference) :
            m_reference(reference) {
                assert(m_reference != NULL);
            }
            
            bool operator()(Model::BrushFace* face) const {
                return face != m_reference && face->boundary().equals(m_reference->boundary());
            }
        };
        
        Model::BrushFaceList ResizeBrushesTool::collectDragFaces(Model::BrushFace* face) const {
            Model::CollectMatchingBrushFacesVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(face)));
            
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
            
            Model::BrushFaceList result = visitor.faces();
            result.push_back(face);
            return result;
        }

        bool ResizeBrushesTool::beginResize(const Hits& hits, const bool split) {
            const Hit& hit = hits.findFirst(ResizeHit, true);
            if (!hit.isMatch())
                return false;
            
            m_dragOrigin = hit.hitPoint();
            m_totalDelta = Vec3::Null;
            m_splitBrushes = split;

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            return true;
        }
        
        bool ResizeBrushesTool::resize(const Ray3& pickRay, const Renderer::Camera& camera) {
            assert(!m_dragFaces.empty());
            
            const Plane3 dragPlane = orthogonalDragPlane(m_dragOrigin, Vec3(camera.direction()));
            
            Model::BrushFace* dragFace = m_dragFaces.front();
            const Vec3& faceNormal3D = dragFace->boundary().normal;
            const Vec3 faceNormal2D = dragPlane.project(faceNormal3D);
            const FloatType rayPointDist = dragPlane.intersectWithRay(pickRay);
            const Vec3 rayPoint = pickRay.pointAtDistance(rayPointDist);
            const Vec3 dragVector2D = rayPoint - m_dragOrigin;
            const FloatType dragDist = dragVector2D.dot(faceNormal2D);
            const FloatType dragDist2 = dragDist * dragDist;
            
            MapDocumentSPtr document = lock(m_document);
            const View::Grid& grid = document->grid();
            const Vec3 relativeFaceDelta = grid.snap(dragDist) * faceNormal3D;
            const Vec3 absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal3D * dragDist);
            
            // select the delta that is closest to the actual delta indicated by the mouse cursor
            const Vec3f faceDelta = (std::abs(relativeFaceDelta.squaredLength() - dragDist2) <
                                     std::abs(absoluteFaceDelta.squaredLength() - dragDist2) ?
                                     relativeFaceDelta :
                                     absoluteFaceDelta);
            
            if (faceDelta.null())
                return true;
            
            if (m_splitBrushes) {
                if (splitBrushes(faceDelta)) {
                    m_totalDelta += faceDelta;
                    m_dragOrigin += faceDelta;
                    m_splitBrushes = false;
                }
            } else {
                if (document->resizeBrushes(m_dragFaces, faceDelta)) {
                    m_totalDelta += faceDelta;
                    m_dragOrigin += faceDelta;
                }
            }
            
            return true;
        }
        
        void ResizeBrushesTool::commitResize() {
            MapDocumentSPtr document = lock(m_document);
            if (m_totalDelta.null())
                document->cancelTransaction();
            else
                document->commitTransaction();
            m_dragFaces.clear();
        }
        
        void ResizeBrushesTool::cancelResize() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_dragFaces.clear();
        }

        bool ResizeBrushesTool::splitBrushes(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            const bool lockTextures = document->textureLock();
            
            Model::BrushFaceList::const_iterator fIt, fEnd;
            
            // first ensure that the drag can be applied at all
            for (fIt = m_dragFaces.begin(), fEnd = m_dragFaces.end(); fIt != fEnd; ++fIt) {
                const Model::BrushFace* face = *fIt;
                const Model::Brush* brush = face->brush();
                if (!brush->canMoveBoundary(worldBounds, face, delta))
                    return false;
            }
            
            Model::ParentChildrenMap newNodes;
            Model::BrushFaceList newDragFaces;
            for (fIt = m_dragFaces.begin(), m_dragFaces.end(); fIt != fEnd; ++fIt) {
                Model::BrushFace* dragFace = *fIt;
                Model::Brush* brush = dragFace->brush();
                
                Model::Brush* newBrush = brush->clone(worldBounds);
                Model::BrushFace* newDragFace = findMatchingFace(newBrush, dragFace);
                Model::BrushFace* clipFace = newDragFace->clone();
                clipFace->invert();
                
                newBrush->moveBoundary(worldBounds, newDragFace, delta, lockTextures);
                const bool clipResult = newBrush->clip(worldBounds, clipFace);
                assert(clipResult);
                _UNUSED(clipResult);
                
                newNodes[brush->parent()].push_back(newBrush);
                newDragFaces.push_back(newDragFace);
            }
            
            document->deselectAll();
            const Model::NodeList addedNodes = document->addNodes(newNodes);
            document->select(addedNodes);
            m_dragFaces = newDragFaces;
            
            return true;
        }

        Model::BrushFace* ResizeBrushesTool::findMatchingFace(Model::Brush* brush, const Model::BrushFace* reference) const {
            Model::FindMatchingBrushFaceVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(reference)));
            visitor.visit(brush);
            if (!visitor.hasResult())
                return NULL;
            return visitor.result();
        }
    }
}
