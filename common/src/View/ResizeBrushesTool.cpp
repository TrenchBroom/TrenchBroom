/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/FindMatchingBrushFaceVisitor.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ResizeBrushesTool::ResizeHit2D = Model::Hit::freeHitType();
        const Model::Hit::HitType ResizeBrushesTool::ResizeHit3D = Model::Hit::freeHitType();

        ResizeBrushesTool::ResizeBrushesTool(MapDocumentWPtr document) :
        Tool(true),
        m_document(document),
        m_splitBrushes(false),
        m_resizing(false) {
            bindObservers();
        }
        
        ResizeBrushesTool::~ResizeBrushesTool() {
            unbindObservers();
        }

        bool ResizeBrushesTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().hasBrushes();
        }
        
        Model::Hit ResizeBrushesTool::pick2D(const ray3& pickRay, const Model::PickResult& pickResult) {
            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().selected().first();
            if (hit.isMatch())
                return Model::Hit::NoHit;
            return pickProximateFace(ResizeHit2D, pickRay);
        }
        
        Model::Hit ResizeBrushesTool::pick3D(const ray3& pickRay, const Model::PickResult& pickResult) {
            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().selected().first();
            if (hit.isMatch())
                return Model::Hit(ResizeHit3D, hit.distance(), hit.hitPoint(), Model::hitToFace(hit));
            return pickProximateFace(ResizeHit3D, pickRay);
        }
        
        class ResizeBrushesTool::PickProximateFace : public Model::ConstNodeVisitor, public Model::NodeQuery<Model::Hit> {
        private:
            const Model::Hit::HitType m_hitType;
            const ray3& m_pickRay;
            FloatType m_closest;
        public:
            PickProximateFace(const Model::Hit::HitType hitType, const ray3& pickRay) :
            NodeQuery(Model::Hit::NoHit),
            m_hitType(hitType),
            m_pickRay(pickRay),
            m_closest(std::numeric_limits<FloatType>::max()) {}
        private:
            void doVisit(const Model::World* world) override   {}
            void doVisit(const Model::Layer* layer) override   {}
            void doVisit(const Model::Group* group) override   {}
            void doVisit(const Model::Entity* entity) override {}
            void doVisit(const Model::Brush* brush) override   {
                for (const auto edge : brush->edges())
                    visitEdge(edge);
            }
            
            void visitEdge(Model::BrushEdge* edge) {
                auto* left = edge->firstFace()->payload();
                auto* right = edge->secondFace()->payload();
                const auto leftDot  = dot(left->boundary().normal,  m_pickRay.direction);
                const auto rightDot = dot(right->boundary().normal, m_pickRay.direction);
                
                if ((leftDot > 0.0) != (rightDot > 0.0)) {
                    const auto result = distance(m_pickRay, segment3(edge->firstVertex()->position(), edge->secondVertex()->position()));
                    if (!Math::isnan(result.distance) && result.distance < m_closest) {
                        m_closest = result.distance;
                        const auto hitPoint = m_pickRay.pointAtDistance(result.rayDistance);
                        if (m_hitType == ResizeBrushesTool::ResizeHit2D) {
                            Model::BrushFaceList faces;
                            if (Math::zero(leftDot)) {
                                faces.push_back(left);
                            } else if (Math::zero(rightDot)) {
                                faces.push_back(right);
                            } else {
                                if (Math::abs(leftDot) < 1.0) {
                                    faces.push_back(left);
                                }
                                if (Math::abs(rightDot) < 1.0) {
                                    faces.push_back(right);
                                }
                            }
                            setResult(Model::Hit(m_hitType, result.rayDistance, hitPoint, faces));
                        } else {
                            auto* face = leftDot > rightDot ? left : right;
                            setResult(Model::Hit(m_hitType, result.rayDistance, hitPoint, face));
                        }
                    }
                }
            }
        };
        
        Model::Hit ResizeBrushesTool::pickProximateFace(const Model::Hit::HitType hitType, const ray3& pickRay) const {
            PickProximateFace visitor(hitType, pickRay);
            
            auto document = lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
            
            if (!visitor.hasResult()) {
                return Model::Hit::NoHit;
            } else {
                return visitor.result();
            }
        }

        bool ResizeBrushesTool::hasDragFaces() const {
            return !m_dragFaces.empty();
        }
        
        const Model::BrushFaceList& ResizeBrushesTool::dragFaces() const {
            return m_dragFaces;
        }
        
        void ResizeBrushesTool::updateDragFaces(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(ResizeHit2D | ResizeHit3D).occluded().first();
            Model::BrushFaceList newDragFaces = getDragFaces(hit);
            if (newDragFaces != m_dragFaces)
                refreshViews();
            
            using std::swap;
            swap(m_dragFaces, newDragFaces);
        }
        
        Model::BrushFaceList ResizeBrushesTool::getDragFaces(const Model::Hit& hit) const {
            return !hit.isMatch() ? Model::EmptyBrushFaceList : collectDragFaces(hit);
        }

        class ResizeBrushesTool::MatchFaceBoundary {
        private:
            const Model::BrushFace* m_reference;
        public:
            MatchFaceBoundary(const Model::BrushFace* reference) :
            m_reference(reference) {
                ensure(m_reference != nullptr, "reference is null");
            }
            
            bool operator()(Model::BrushFace* face) const {
                return face != m_reference && equal(face->boundary(), m_reference->boundary(), Math::Constants<FloatType>::almostZero());
            }
        };
        
        Model::BrushFaceList ResizeBrushesTool::collectDragFaces(const Model::Hit& hit) const {
            assert(hit.isMatch());
            assert(hit.type() == ResizeHit2D || hit.type() == ResizeHit3D);
            
            Model::BrushFaceList result;
            if (hit.type() == ResizeHit2D) {
                const Model::BrushFaceList& faces = hit.target<Model::BrushFaceList>();
                assert(!faces.empty());
                VectorUtils::append(result, faces);
                VectorUtils::append(result, collectDragFaces(faces[0]));
                if (faces.size() > 1)
                    VectorUtils::append(result, collectDragFaces(faces[1]));
            } else {
                Model::BrushFace* face = hit.target<Model::BrushFace*>();
                result.push_back(face);
                VectorUtils::append(result, collectDragFaces(face));
            }

            return result;
        }

        Model::BrushFaceList ResizeBrushesTool::collectDragFaces(Model::BrushFace* face) const {
            Model::CollectMatchingBrushFacesVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(face)));
            
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
            return visitor.faces();
        }

        bool ResizeBrushesTool::beginResize(const Model::PickResult& pickResult, const bool split) {
            const Model::Hit& hit = pickResult.query().type(ResizeHit2D | ResizeHit3D).occluded().first();
            if (!hit.isMatch())
                return false;
            
            m_dragOrigin = hit.hitPoint();
            m_totalDelta = vm::vec3::zero;
            m_splitBrushes = split;

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            m_resizing = true;
            return true;
        }
        
        bool ResizeBrushesTool::resize(const ray3& pickRay, const Renderer::Camera& camera) {
            assert(!m_dragFaces.empty());
            
            auto* dragFace = m_dragFaces.front();
            const auto& faceNormal = dragFace->boundary().normal;

            const auto dist = distance(pickRay, vm::line3(m_dragOrigin, faceNormal));
            if (dist.parallel) {
                return true;
            }

            const auto dragDist = dist.lineDistance;
            
            auto document = lock(m_document);
            const auto& grid = document->grid();
            const auto relativeFaceDelta = grid.snap(dragDist) * faceNormal;
            const auto absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal * dragDist);
            
            const auto faceDelta = selectDelta(relativeFaceDelta, absoluteFaceDelta, dragDist);
            if (isZero(faceDelta)) {
                return true;
            }

            if (m_splitBrushes) {
                if (splitBrushes(faceDelta)) {
                    m_totalDelta = m_totalDelta + faceDelta;
                    m_dragOrigin = m_dragOrigin + faceDelta;
                    m_splitBrushes = false;
                }
            } else {
                if (document->resizeBrushes(dragFaceDescriptors(), faceDelta)) {
                    m_totalDelta = m_totalDelta + faceDelta;
                    m_dragOrigin = m_dragOrigin + faceDelta;
                }
            }
            
            return true;
        }
        
        vm::vec3 ResizeBrushesTool::selectDelta(const vm::vec3& relativeDelta, const vm::vec3& absoluteDelta, const FloatType mouseDistance) const {
            // select the delta that is closest to the actual delta indicated by the mouse cursor
            const FloatType mouseDistance2 = mouseDistance * mouseDistance;
            return (Math::abs(squaredLength(relativeDelta) - mouseDistance2) <
                    Math::abs(squaredLength(absoluteDelta) - mouseDistance2) ?
                    relativeDelta :
                    absoluteDelta);
        }

        void ResizeBrushesTool::commitResize() {
            MapDocumentSPtr document = lock(m_document);
            if (isZero(m_totalDelta)) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_dragFaces.clear();
            m_resizing = false;
        }
        
        void ResizeBrushesTool::cancelResize() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_dragFaces.clear();
            m_resizing = false;
        }

        bool ResizeBrushesTool::splitBrushes(const vm::vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);
            
            // First ensure that the drag can be applied at all. For this, check whether each drag faces is moved
            // "up" along its normal.
            if (!std::all_of(std::begin(m_dragFaces), std::end(m_dragFaces),
                            [&delta](const Model::BrushFace* face) { return Math::pos(dot(face->boundary().normal, delta)); }))
                return false;

            Model::BrushList newBrushes;
            Model::BrushFaceList newDragFaces;
            Model::ParentChildrenMap newNodes;

            for (Model::BrushFace* dragFace : m_dragFaces) {
                Model::Brush* brush = dragFace->brush();

                Model::Brush* newBrush = brush->clone(worldBounds);
                Model::BrushFace* newDragFace = findMatchingFace(newBrush, dragFace);

                newBrushes.push_back(newBrush);
                newDragFaces.push_back(newDragFace);

                if (!newBrush->canMoveBoundary(worldBounds, newDragFace, delta)) {
                    // There is a brush for which the move is not applicable. Abort.
                    VectorUtils::deleteAll(newBrushes);
                    return false;
                } else {
                    Model::BrushFace* clipFace = newDragFace->clone();
                    clipFace->invert();

                    newBrush->moveBoundary(worldBounds, newDragFace, delta, lockTextures);

                    // This should never happen, but let's be on the safe side.
                    if (!newBrush->clip(worldBounds, clipFace)) {
                        delete clipFace;
                        VectorUtils::deleteAll(newBrushes);
                        return false;
                    }

                    newNodes[brush->parent()].push_back(newBrush);
                }
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
                return nullptr;
            return visitor.result();
        }

        polygon3::List ResizeBrushesTool::dragFaceDescriptors() const {
            polygon3::List result;
            result.reserve(m_dragFaces.size());
            std::transform(std::begin(m_dragFaces), std::end(m_dragFaces), std::back_inserter(result), [](const Model::BrushFace* face) { return face->polygon(); });
            return result;
        }

        void ResizeBrushesTool::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &ResizeBrushesTool::nodesDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ResizeBrushesTool::nodesDidChange);
            document->nodesWillBeRemovedNotifier.addObserver(this, &ResizeBrushesTool::nodesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &ResizeBrushesTool::selectionDidChange);
        }
        
        void ResizeBrushesTool::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &ResizeBrushesTool::nodesDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ResizeBrushesTool::nodesDidChange);
                document->nodesWillBeRemovedNotifier.removeObserver(this, &ResizeBrushesTool::nodesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &ResizeBrushesTool::selectionDidChange);
            }
        }

        void ResizeBrushesTool::nodesDidChange(const Model::NodeList& nodes) {
            if (!m_resizing)
                m_dragFaces.clear();
        }

        void ResizeBrushesTool::selectionDidChange(const Selection& selection) {
            if (!m_resizing)
                m_dragFaces.clear();
        }
    }
}
