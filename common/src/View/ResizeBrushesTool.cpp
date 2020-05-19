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
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/FindMatchingBrushFaceVisitor.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/collection_utils.h>
#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/distance.h>
#include <vecmath/intersection.h>
#include <vecmath/scalar.h>

#include <limits>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type ResizeBrushesTool::Resize2DHitType = Model::HitType::freeType();
        const Model::HitType::Type ResizeBrushesTool::Resize3DHitType = Model::HitType::freeType();

        ResizeBrushesTool::ResizeBrushesTool(std::weak_ptr<MapDocument> document) :
        Tool(true),
        m_document(std::move(document)),
        m_splitBrushes(false),
        m_dragging(false) {
            bindObservers();
        }

        ResizeBrushesTool::~ResizeBrushesTool() {
            unbindObservers();
        }

        bool ResizeBrushesTool::applies() const {
            auto document = kdl::mem_lock(m_document);
            return document->selectedNodes().hasBrushes();
        }

        Model::Hit ResizeBrushesTool::pick2D(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            auto document = kdl::mem_lock(m_document);
            const auto& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHitType).occluded().selected().first();
            if (hit.isMatch()) {
                return Model::Hit::NoHit;
            } else {
                return pickProximateFace(Resize2DHitType, pickRay);
            }
        }

        Model::Hit ResizeBrushesTool::pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            auto document = kdl::mem_lock(m_document);
            const auto& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHitType).occluded().selected().first();
            if (hit.isMatch()) {
                auto* face = Model::hitToFace(hit);
                auto* node = Model::hitToBrush(hit);
                return Model::Hit(Resize3DHitType, hit.distance(), hit.hitPoint(), Model::BrushFaceHandle(node, face));
            } else {
                return pickProximateFace(Resize3DHitType, pickRay);
            }
        }

        class ResizeBrushesTool::PickProximateFace : public Model::NodeVisitor, public Model::NodeQuery<Model::Hit> {
        private:
            const Model::HitType::Type m_hitType;
            const vm::ray3& m_pickRay;
            FloatType m_closest;
        public:
            PickProximateFace(const Model::HitType::Type hitType, const vm::ray3& pickRay) :
            NodeQuery(Model::Hit::NoHit),
            m_hitType(hitType),
            m_pickRay(pickRay),
            m_closest(std::numeric_limits<FloatType>::max()) {}
        private:
            void doVisit(Model::WorldNode*) override  {}
            void doVisit(Model::LayerNode*) override  {}
            void doVisit(Model::GroupNode*) override  {}
            void doVisit(Model::EntityNode*) override {}
            void doVisit(Model::BrushNode* brushNode) override   {
                const Model::Brush& brush = brushNode->brush();
                for (const auto* edge : brush.edges())
                    visitEdge(brushNode, edge);
            }

            void visitEdge(Model::BrushNode* node, const Model::BrushEdge* edge) {
                auto* left = edge->firstFace()->payload();
                auto* right = edge->secondFace()->payload();
                const auto leftDot  = dot(left->boundary().normal,  m_pickRay.direction);
                const auto rightDot = dot(right->boundary().normal, m_pickRay.direction);

                if ((leftDot > 0.0) != (rightDot > 0.0)) {
                    const auto result = vm::distance(m_pickRay, vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position()));
                    if (!vm::is_nan(result.distance) && result.distance < m_closest) {
                        m_closest = result.distance;
                        const auto hitPoint = vm::point_at_distance(m_pickRay, result.position1);
                        if (m_hitType == ResizeBrushesTool::Resize2DHitType) {
                            Resize2DHitData data;
                            if (vm::is_zero(leftDot, vm::C::almost_zero())) {
                                data.push_back(Model::BrushFaceHandle(node, left));
                            } else if (vm::is_zero(rightDot, vm::C::almost_zero())) {
                                data.push_back(Model::BrushFaceHandle(node, right));
                            } else {
                                if (vm::abs(leftDot) < 1.0) {
                                    data.push_back(Model::BrushFaceHandle(node, left));
                                }
                                if (vm::abs(rightDot) < 1.0) {
                                    data.push_back(Model::BrushFaceHandle(node, right));
                                }
                            }
                            setResult(Model::Hit(m_hitType, result.position1, hitPoint, data));
                        } else {
                            Model::BrushFace* face = leftDot > rightDot ? left : right;
                            setResult(Model::Hit(m_hitType, result.position1, hitPoint, Model::BrushFaceHandle(node, face)));
                        }
                    }
                }
            }
        };

        Model::Hit ResizeBrushesTool::pickProximateFace(const Model::HitType::Type hitType, const vm::ray3& pickRay) const {
            PickProximateFace visitor(hitType, pickRay);

            auto document = kdl::mem_lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);

            if (!visitor.hasResult()) {
                return Model::Hit::NoHit;
            } else {
                return visitor.result();
            }
        }

        bool ResizeBrushesTool::hasDragFaces() const {
            return !m_dragHandles.empty();
        }

        std::vector<Model::BrushFaceHandle> ResizeBrushesTool::dragFaces() const {
            return kdl::vec_transform(m_dragHandles, [](const auto& dragHandle) {
                auto* brushNode = std::get<0>(dragHandle);
                const auto& normal = std::get<1>(dragHandle);
                
                const auto& brush = brushNode->brush();
                const auto faceIndex = brush.findFace(normal);
                assert(faceIndex);
                return Model::BrushFaceHandle(brushNode, brush.face(*faceIndex));
            });
        }

        void ResizeBrushesTool::updateDragFaces(const Model::PickResult& pickResult) {
            const auto& hit = pickResult.query().type(Resize2DHitType | Resize3DHitType).occluded().first();
            auto newDragHandles = getDragHandles(hit);
            if (newDragHandles != m_dragHandles) {
                refreshViews();
            }

            using std::swap;
            swap(m_dragHandles, newDragHandles);
        }

        std::vector<ResizeBrushesTool::FaceHandle> ResizeBrushesTool::getDragHandles(const Model::Hit& hit) const {
            if (hit.isMatch()) {
                return collectDragHandles(hit);
            } else {
                return std::vector<FaceHandle>(0);
            }
        }

        class ResizeBrushesTool::MatchFaceBoundary {
        private:
            const Model::BrushFace* m_reference;
        public:
            explicit MatchFaceBoundary(const Model::BrushFace* reference) :
            m_reference(reference) {
                ensure(m_reference != nullptr, "reference is null");
            }

            bool operator()(const Model::BrushNode*, Model::BrushFace* face) const {
                return face != m_reference && vm::is_equal(face->boundary(), m_reference->boundary(),
                    vm::C::almost_zero());
            }
        };

        std::vector<ResizeBrushesTool::FaceHandle> ResizeBrushesTool::collectDragHandles(const Model::Hit& hit) const {
            assert(hit.isMatch());
            assert(hit.type() == Resize2DHitType || hit.type() == Resize3DHitType);

            std::vector<Model::BrushFaceHandle> result;
            if (hit.type() == Resize2DHitType) {
                const Resize2DHitData& data = hit.target<const Resize2DHitData&>();
                assert(!data.empty());
                kdl::vec_append(result, data, collectDragFaces(data[0]));
                if (data.size() > 1) {
                    kdl::vec_append(result, collectDragFaces(data[1]));
                }
            } else {
                const Resize3DHitData& data = hit.target<const Resize3DHitData&>();
                result.push_back(data);
                kdl::vec_append(result, collectDragFaces(data));
            }

            return kdl::vec_transform(result, [](const auto& handle) {
                return std::make_tuple(handle.node(), handle.face()->boundary().normal);
            });
        }

        std::vector<Model::BrushFaceHandle> ResizeBrushesTool::collectDragFaces(const Model::BrushFaceHandle& faceHandle) const {
            Model::CollectMatchingBrushFacesVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(faceHandle.face())));

            auto document = kdl::mem_lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
            return visitor.faces();
        }

        bool ResizeBrushesTool::beginResize(const Model::PickResult& pickResult, const bool split) {
            const auto& hit = pickResult.query().type(Resize2DHitType | Resize3DHitType).occluded().first();
            if (!hit.isMatch()) {
                return false;
            }

            m_dragOrigin = hit.hitPoint();
            m_totalDelta = vm::vec3::zero();
            m_splitBrushes = split;

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Resize Brushes");
            m_dragging = true;
            return true;
        }

        bool ResizeBrushesTool::resize(const vm::ray3& pickRay, const Renderer::Camera& /* camera */) {
            const auto dragFaceHandles = dragFaces();
            assert(!dragFaceHandles.empty());
            
            const auto dragFaceHandle = dragFaceHandles.front();
            auto* dragFace = dragFaceHandle.face();
            const auto& faceNormal = dragFace->boundary().normal;

            const auto dist = vm::distance(pickRay, vm::line3(m_dragOrigin, faceNormal));
            if (dist.parallel) {
                return true;
            }

            const auto dragDist = dist.position2;

            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();
            const auto relativeFaceDelta = grid.snap(dragDist) * faceNormal;
            const auto absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal * dragDist);

            const auto faceDelta = selectDelta(relativeFaceDelta, absoluteFaceDelta, dragDist);
            if (vm::is_zero(faceDelta, vm::C::almost_zero())) {
                return true;
            }

            if (m_splitBrushes) {
                if (splitBrushesOutward(faceDelta) || splitBrushesInward(faceDelta)) {
                    m_totalDelta = m_totalDelta + faceDelta;
                    m_dragOrigin = m_dragOrigin + faceDelta;
                    m_splitBrushes = false;
                }
            } else {
                // This handles ordinary resizing, splitting outward, and splitting inward
                // (in which case dragFaceDescriptors() is a list of polygons splitting the selected brushes)
                if (document->resizeBrushes(dragFaceDescriptors(), faceDelta)) {
                    m_totalDelta = m_totalDelta + faceDelta;
                    m_dragOrigin = m_dragOrigin + faceDelta;
                }
            }

            return true;
        }

        vm::vec3 ResizeBrushesTool::selectDelta(const vm::vec3& relativeDelta, const vm::vec3& absoluteDelta, const FloatType mouseDistance) const {
            // select the delta that is closest to the actual delta indicated by the mouse cursor
            const auto mouseDistance2 = mouseDistance * mouseDistance;
            return (vm::abs(vm::squared_length(relativeDelta) - mouseDistance2) <
                    vm::abs(vm::squared_length(absoluteDelta) - mouseDistance2) ?
                    relativeDelta :
                    absoluteDelta);
        }

        bool ResizeBrushesTool::beginMove(const Model::PickResult& pickResult) {
            const auto& hit = pickResult.query().type(Resize2DHitType).occluded().first();
            if (!hit.isMatch()) {
                return false;
            }

            m_dragOrigin = m_lastPoint = hit.hitPoint();
            m_totalDelta = vm::vec3::zero();
            m_splitBrushes = false;

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Move Faces");
            m_dragging = true;
            return true;
        }

        bool ResizeBrushesTool::move(const vm::ray3& pickRay, const Renderer::Camera& camera) {
            const auto dragPlane = vm::plane3(m_dragOrigin, vm::vec3(camera.direction()));
            const auto hitDist = vm::intersect_ray_plane(pickRay, dragPlane);
            if (vm::is_nan(hitDist)) {
                return true;
            }

            const auto hitPoint = vm::point_at_distance(pickRay, hitDist);

            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();
            const auto delta = grid.snap(hitPoint - m_lastPoint);
            if (vm::is_zero(delta, vm::C::almost_zero())) {
                return true;
            }

            std::map<vm::polygon3, std::vector<Model::BrushNode*>> brushMap;
            for (const auto& handle : dragFaces()) {
                auto* brush = handle.node();
                auto* face = handle.face();
                brushMap[face->polygon()] = { brush };
            }

            if (document->moveFaces(brushMap, delta)) {
                m_lastPoint = m_lastPoint + delta;
                m_totalDelta = m_totalDelta + delta;
            }

            return true;
        }

        void ResizeBrushesTool::commit() {
            auto document = kdl::mem_lock(m_document);
            if (vm::is_zero(m_totalDelta, vm::C::almost_zero())) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_dragHandles.clear();
            m_dragging = false;
        }

        void ResizeBrushesTool::cancel() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
            m_dragHandles.clear();
            m_dragging = false;
        }

        bool ResizeBrushesTool::splitBrushesOutward(const vm::vec3& delta) {
            auto document = kdl::mem_lock(m_document);
            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);

            // First ensure that the drag can be applied at all. For this, check whether each drag handle is moved
            // "up" along its normal.
            for (const auto& handle : m_dragHandles) {
                const auto& normal = std::get<1>(handle);
                if (vm::dot(normal, delta) <= FloatType(0)) {
                    return false;
                }
            }

            std::vector<FaceHandle> newDragHandles;
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            for (const auto& dragFaceHandle : dragFaces()) {
                auto* dragFace = dragFaceHandle.face();
                auto* brushNode = dragFaceHandle.node();

                auto newBrush = brushNode->brush();
                const auto newDragFaceIndex = newBrush.findFace(dragFace->boundary());
                if (!newDragFaceIndex) {
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }

                if (!newBrush.canMoveBoundary(worldBounds, *newDragFaceIndex, delta)) {
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }

                const auto* newDragFace = newBrush.face(*newDragFaceIndex);
                auto* clipFace = newDragFace->clone();
                clipFace->invert();
                newBrush.moveBoundary(worldBounds, *newDragFaceIndex, delta, lockTextures);
                
                if (!newBrush.clip(worldBounds, clipFace)) {
                    delete clipFace;
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }
                
                auto* newBrushNode = new Model::BrushNode(std::move(newBrush));
                newNodes[brushNode->parent()].push_back(newBrushNode);
                newDragHandles.emplace_back(newBrushNode, newDragFace->boundary().normal);
            }

            document->deselectAll();
            const auto addedNodes = document->addNodes(newNodes);
            document->select(addedNodes);
            m_dragHandles = std::move(newDragHandles);

            return true;
        }

        bool ResizeBrushesTool::splitBrushesInward(const vm::vec3& delta) {
            auto document = kdl::mem_lock(m_document);
            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);

            // First ensure that the drag can be applied at all. For this, check whether each drag handle is moved
            // "down" along its normal.
            for (const auto& handle : m_dragHandles) {
                const auto& normal = std::get<1>(handle);
                if (vm::dot(normal, delta) > 0.0) {
                    return false;
                }
            }

            const std::vector<FaceHandle> oldDragHandles = m_dragHandles;
            std::vector<FaceHandle> newDragHandles;
            // This map is to handle the case when the brushes being
            // extruded have different parents (e.g. different brush entities),
            // so each newly created brush should be made a sibling of the brush it was cloned from.
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            for (const auto& dragFaceHandle : dragFaces()) {
                auto* dragFace = dragFaceHandle.face();
                auto* brushNode = dragFaceHandle.node();

                auto newBrush = brushNode->brush();
                const auto newDragFaceIndex = newBrush.findFace(dragFace->boundary());
                if (!newDragFaceIndex) {
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }

                auto* newDragFace = newBrush.face(*newDragFaceIndex);
                auto* clipFace = newDragFace->clone();
                clipFace->invert();
                clipFace->transform(vm::translation_matrix(delta), lockTextures);

                if (!newBrush.clip(worldBounds, clipFace)) {
                    delete clipFace;
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }

                auto* newBrushNode = new Model::BrushNode(std::move(newBrush));
                newNodes[brushNode->parent()].push_back(newBrushNode);
                newDragHandles.emplace_back(newBrushNode, newDragFace->boundary().normal);
            }

            // Now that the newly split off brushes are ready to insert (but not selected),
            // resize the original brushes, which are still selected at this point.
            if (!document->resizeBrushes(dragFaceDescriptors(), delta)) {
                kdl::map_clear_and_delete(newNodes);
                return false;
            }

            // Add the newly split off brushes and select them (keeping the original brushes selected).
            const auto addedNodes = document->addNodes(newNodes);
            document->select(addedNodes);

            m_dragHandles = kdl::vec_concat(oldDragHandles, newDragHandles);

            return true;
        }

        std::vector<vm::polygon3> ResizeBrushesTool::dragFaceDescriptors() const {
            const auto dragFaces = this->dragFaces();

            std::vector<vm::polygon3> result;
            result.reserve(dragFaces.size());
            for (const auto& dragFaceHandle : dragFaces) {
                const auto* dragFace = dragFaceHandle.face();
                result.push_back(dragFace->polygon());
            }

            return result;
        }

        void ResizeBrushesTool::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &ResizeBrushesTool::nodesDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ResizeBrushesTool::nodesDidChange);
            document->nodesWillBeRemovedNotifier.addObserver(this, &ResizeBrushesTool::nodesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &ResizeBrushesTool::selectionDidChange);
        }

        void ResizeBrushesTool::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &ResizeBrushesTool::nodesDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ResizeBrushesTool::nodesDidChange);
                document->nodesWillBeRemovedNotifier.removeObserver(this, &ResizeBrushesTool::nodesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &ResizeBrushesTool::selectionDidChange);
            }
        }

        void ResizeBrushesTool::nodesDidChange(const std::vector<Model::Node*>&) {
            if (!m_dragging) {
                m_dragHandles.clear();
            }
        }

        void ResizeBrushesTool::selectionDidChange(const Selection&) {
            if (!m_dragging) {
                m_dragHandles.clear();
            }
        }
    }
}
