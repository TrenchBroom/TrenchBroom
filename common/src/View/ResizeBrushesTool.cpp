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

#include "Exceptions.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/collection_utils.h>
#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
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

        // DragHandle

        DragHandle::DragHandle(const Model::BrushFaceHandle& handle) :
        node(handle.node()),
        brushAtDragStart(handle.node()->brush()),
        faceIndex(handle.faceIndex()) {}

        const Model::BrushFace& DragHandle::faceAtDragStart() const {
            return brushAtDragStart.face(faceIndex);
        }

        vm::vec3 DragHandle::faceNormal() const {
            return faceAtDragStart().normal();
        }

        bool DragHandle::operator==(const DragHandle& other) const {
            return node == other.node
                && faceIndex == other.faceIndex;
        }

        bool DragHandle::operator!=(const DragHandle& other) const {
            return !(*this == other);
        }

        // ResizeBrushesTool

        ResizeBrushesTool::ResizeBrushesTool(std::weak_ptr<MapDocument> document) :
        Tool(true),
        m_document(std::move(document)),
        m_dragging(false),
        m_splitBrushes(false) {
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
            if (const auto faceHandle = hitToFaceHandle(hit)) {
                return Model::Hit(Resize3DHitType, hit.distance(), hit.hitPoint(), *faceHandle);
            } else {
                return pickProximateFace(Resize3DHitType, pickRay);
            }
        }

        Model::Hit ResizeBrushesTool::pickProximateFace(const Model::HitType::Type hitType, const vm::ray3& pickRay) const {
            auto document = kdl::mem_lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();

            auto closestDistance = std::numeric_limits<FloatType>::max();
            auto hit = Model::Hit::NoHit;

            for (auto* node : nodes) {
                node->accept(kdl::overload(
                    [] (Model::WorldNode*) {},
                    [] (Model::LayerNode*) {},
                    [] (Model::GroupNode*) {},
                    [] (Model::EntityNode*){},
                    [&](Model::BrushNode* brushNode) {
                        const auto& brush = brushNode->brush();
                        for (const auto* edge : brush.edges()) {
                            const auto leftFaceIndex = edge->firstFace()->payload();
                            const auto rightFaceIndex = edge->secondFace()->payload();
                            assert(leftFaceIndex && rightFaceIndex);
                            
                            const auto& leftFace = brush.face(*leftFaceIndex);
                            const auto& rightFace = brush.face(*rightFaceIndex);
                            const auto leftDot  = dot(leftFace.boundary().normal,  pickRay.direction);
                            const auto rightDot = dot(rightFace.boundary().normal, pickRay.direction);

                            const auto leftFaceHandle = Model::BrushFaceHandle(brushNode, *leftFaceIndex);
                            const auto rightFaceHandle = Model::BrushFaceHandle(brushNode, *rightFaceIndex);
                            
                            if ((leftDot > 0.0) != (rightDot > 0.0)) {
                                const auto result = vm::distance(pickRay, vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position()));
                                if (!vm::is_nan(result.distance) && result.distance < closestDistance) {
                                    closestDistance = result.distance;
                                    const auto hitPoint = vm::point_at_distance(pickRay, result.position1);
                                    if (hitType == ResizeBrushesTool::Resize2DHitType) {
                                        Resize2DHitData data;
                                        if (vm::is_zero(leftDot, vm::C::almost_zero())) {
                                            data.push_back(leftFaceHandle);
                                        } else if (vm::is_zero(rightDot, vm::C::almost_zero())) {
                                            data.push_back(rightFaceHandle);
                                        } else {
                                            if (vm::abs(leftDot) < 1.0) {
                                                data.push_back(leftFaceHandle);
                                            }
                                            if (vm::abs(rightDot) < 1.0) {
                                                data.push_back(rightFaceHandle);
                                            }
                                        }
                                        hit = Model::Hit(hitType, result.position1, hitPoint, data);
                                    } else {
                                        const auto faceHandle = leftDot > rightDot ? leftFaceHandle : rightFaceHandle;
                                        hit = Model::Hit(hitType, result.position1, hitPoint, faceHandle);
                                    }
                                }
                            }
                        }
                    }
                ));
            }

            return hit;
        }

        bool ResizeBrushesTool::hasVisualHandles() const {
            return !visualHandles().empty();
        }

        /**
         * Returns the current handles to render. 
         * 
         * - If not currently dragging, returns the proposed handles that would be resized if a drag started
         * - Called "visual" because these can be clipped away, e.g. when raising the top of a trapezoid 
         *   until it turns into a traingle, while the drag state is stored elsewhere.
         */
        std::vector<Model::BrushFaceHandle> ResizeBrushesTool::visualHandles() const {
            if (m_dragging) {
                return m_currentDragVisualHandles;
            }

            return kdl::vec_transform(m_proposedDragHandles, [](const DragHandle& handle) {
                return Model::BrushFaceHandle(handle.node, handle.faceIndex);
            });
        }

        void ResizeBrushesTool::updateProposedDragHandles(const Model::PickResult& pickResult) {
            if (m_dragging) {
                // FIXME: this should be turned into an ensure failure, but it's easy to make it fail
                // currently by spamming drags/modifiers.
                // Indicates a bug in ResizeBrushesToolController thinking we are not dragging when we actually still are.
                kdl::mem_lock(m_document)->error() << "updateProposedDragHandles called during a drag";
                return;
            }

            const auto& hit = pickResult.query().type(Resize2DHitType | Resize3DHitType).occluded().first();
            auto newDragHandles = getDragHandles(hit);
            if (newDragHandles != m_proposedDragHandles) {
                refreshViews();
            }

            m_proposedDragHandles = newDragHandles;
        }

        std::vector<DragHandle> ResizeBrushesTool::getDragHandles(const Model::Hit& hit) const {
            if (hit.isMatch()) {
                return collectDragHandles(hit);
            } else {
                return std::vector<DragHandle>{};
            }
        }

        std::vector<DragHandle> ResizeBrushesTool::collectDragHandles(const Model::Hit& hit) const {
            assert(hit.isMatch());
            assert(hit.type() == Resize2DHitType || hit.type() == Resize3DHitType);

            std::vector<Model::BrushFaceHandle> result;
            if (hit.type() == Resize2DHitType) {
                const Resize2DHitData& data = hit.target<const Resize2DHitData&>();
                assert(!data.empty());
                result = kdl::vec_concat(std::move(result), data, collectDragFaces(data[0]));
                if (data.size() > 1) {
                    result = kdl::vec_concat(std::move(result), collectDragFaces(data[1]));
                }
            } else {
                const Resize3DHitData& data = hit.target<const Resize3DHitData&>();
                result.push_back(data);
                result = kdl::vec_concat(std::move(result), collectDragFaces(data));
            }

            return kdl::vec_transform(result, [](const auto& handle) {
                return DragHandle(handle);
            });
        }

        std::vector<Model::BrushFaceHandle> ResizeBrushesTool::collectDragFaces(const Model::BrushFaceHandle& faceHandle) const {
            auto result = std::vector<Model::BrushFaceHandle>{};

            auto document = kdl::mem_lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();

            const auto& referenceFace = faceHandle.face();
            for (auto* node : nodes) {
                node->accept(kdl::overload(
                    [] (Model::WorldNode*) {},
                    [] (Model::LayerNode*) {},
                    [] (Model::GroupNode*) {},
                    [] (Model::EntityNode*){},
                    [&](Model::BrushNode* brushNode) {
                        const auto& brush = brushNode->brush();
                        for (size_t i = 0; i < brush.faceCount(); ++i) {
                            const auto& face = brush.face(i);
                            if (&face == &referenceFace) {
                                continue;
                            }

                            if (!face.coplanarWith(referenceFace.boundary())) {
                                continue;
                            }

                            result.emplace_back(brushNode, i);
                        }
                    }
                ));
            }

            return result;            
        }

        /**
         * Starts resizing the faces determined by the previous call to updateProposedDragHandles
         */
        bool ResizeBrushesTool::beginResize(const Model::PickResult& pickResult, const bool split) {
            ensure(!m_dragging, "may not be called during a drag");

            const auto& hit = pickResult.query().type(Resize2DHitType | Resize3DHitType).occluded().first();
            if (!hit.isMatch()) {
                return false;
            }

            m_dragOrigin = hit.hitPoint();
            m_totalDelta = vm::vec3::zero();
            m_splitBrushes = split;
            m_dragHandlesAtDragStart = m_proposedDragHandles;
            m_currentDragVisualHandles = kdl::vec_transform(m_proposedDragHandles, [](const DragHandle& handle) {
                return Model::BrushFaceHandle(handle.node, handle.faceIndex);
            });

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Resize Brushes");
            m_dragging = true;
            return true;
        }

        bool ResizeBrushesTool::resize(const vm::ray3& pickRay, const Renderer::Camera& /* camera */) {
            ensure(m_dragging, "may only be called during a drag");

            const DragHandle& dragFaceHandle = m_dragHandlesAtDragStart.at(0);
            const Model::BrushFace& dragFace = dragFaceHandle.faceAtDragStart();
            const vm::vec3& faceNormal = dragFace.boundary().normal;

            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();

            auto dragDistToSnappedDelta = [&](const FloatType dist) -> vm::vec3 {
                const auto unsnappedDelta = faceNormal * dist;
                return grid.snap() ? grid.moveDelta(dragFace, unsnappedDelta) : unsnappedDelta;
            };

            const vm::line_distance<FloatType> dist = vm::distance(pickRay, vm::line3(m_dragOrigin, faceNormal));
            if (dist.parallel) {
                return true;
            }

            const FloatType dragDist = dist.position2;
            const vm::vec3 faceDelta = dragDistToSnappedDelta(dragDist);

            if (vm::is_equal(faceDelta, m_totalDelta, vm::C::almost_zero())) {
                return true;
            }

            if (m_splitBrushes) {
                if (splitBrushesOutward(faceDelta) || splitBrushesInward(faceDelta)) {
                    return true;
                }
            } else {
                document->rollbackTransaction();
                if (document->resizeBrushes(polygonsAtDragStart(), faceDelta)) {
                    m_totalDelta = faceDelta;
                } else {
                    // resizeBrushes() fails if some brushes were completely clipped away.
                    // In that case, restore the last m_totalDelta to be successfully applied.
                    document->resizeBrushes(polygonsAtDragStart(), m_totalDelta);
                }

                // Update m_currentDragVisualHandles
                m_currentDragVisualHandles.clear();
                for (const auto& dragHandle : m_dragHandlesAtDragStart) {
                    // We assume that the node pointer at the start of the drag is still valid
                    const Model::Brush& brush = dragHandle.node->brush();
                    if (const std::optional<size_t> faceIndex = brush.findFace(dragHandle.faceNormal())) {
                        m_currentDragVisualHandles.push_back(Model::BrushFaceHandle(dragHandle.node, *faceIndex));
                    }
                }
            }

            return true;
        }

        bool ResizeBrushesTool::beginMove(const Model::PickResult& pickResult) {
            ensure(!m_dragging, "may not be called during a drag");

            const auto& hit = pickResult.query().type(Resize2DHitType).occluded().first();
            if (!hit.isMatch()) {
                return false;
            }

            m_dragOrigin = hit.hitPoint();
            m_totalDelta = vm::vec3::zero();
            m_splitBrushes = false;
            m_dragHandlesAtDragStart = m_proposedDragHandles;
            m_currentDragVisualHandles = kdl::vec_transform(m_proposedDragHandles, [](const DragHandle& handle) {
                return Model::BrushFaceHandle(handle.node, handle.faceIndex);
            });

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Move Faces");
            m_dragging = true;
            return true;
        }

        bool ResizeBrushesTool::move(const vm::ray3& pickRay, const Renderer::Camera& camera) {
            ensure(m_dragging, "may only be called during a drag");

            const auto dragPlane = vm::plane3(m_dragOrigin, vm::vec3(camera.direction()));
            const auto hitDist = vm::intersect_ray_plane(pickRay, dragPlane);
            if (vm::is_nan(hitDist)) {
                return true;
            }

            const vm::vec3 hitPoint = vm::point_at_distance(pickRay, hitDist);

            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();
            const vm::vec3 delta = grid.snap(hitPoint - m_dragOrigin);
            if (vm::is_zero(delta, vm::C::almost_zero())) {
                return true;
            }

            document->rollbackTransaction();
            if (document->moveFaces(polygonsAtDragStart(), delta)) {
                m_totalDelta = delta;

                // Update m_currentDragVisualHandles
                m_currentDragVisualHandles.clear();
                for (const auto& dragHandle : m_dragHandlesAtDragStart) {
                    // We assume that the node pointer at the start of the drag is still valid
                    const Model::Brush& brush = dragHandle.node->brush();
                    if (const std::optional<size_t> faceIndex = brush.findFace(dragHandle.faceNormal())) {
                        m_currentDragVisualHandles.push_back(Model::BrushFaceHandle(dragHandle.node, *faceIndex));
                    }
                }
            } else {
                // restore the last successful position
                document->moveFaces(polygonsAtDragStart(), m_totalDelta);
            }

            return true;
        }

        void ResizeBrushesTool::commit() {
            ensure(m_dragging, "may only be called during a drag");

            auto document = kdl::mem_lock(m_document);
            if (vm::is_zero(m_totalDelta, vm::C::almost_zero())) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_proposedDragHandles.clear();
            m_dragHandlesAtDragStart.clear();
            m_currentDragVisualHandles.clear();
            m_dragging = false;
        }

        void ResizeBrushesTool::cancel() {
            ensure(m_dragging, "may only be called during a drag");

            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
            m_proposedDragHandles.clear();
            m_dragHandlesAtDragStart.clear();
            m_currentDragVisualHandles.clear();
            m_dragging = false;
        }

        /**
         * Splits off new brush "outward" from the drag handles.
         * 
         * Returns false if the given delta isn't suitable for splitting "outward".
         * 
         * Otherwise:
         * - rolls back the transaction
         * - applies a split outward with the given delta
         * - sets m_totalDelta to the given delta
         * - returns true
         */
        bool ResizeBrushesTool::splitBrushesOutward(const vm::vec3& delta) {
            ensure(m_dragging, "may only be called during a drag");
            auto document = kdl::mem_lock(m_document);

            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);

            // First ensure that the drag can be applied at all. For this, check whether each drag handle is moved
            // "up" along its normal.
            for (const auto& handle : m_dragHandlesAtDragStart) {
                const auto& normal = handle.faceNormal();
                if (vm::dot(normal, delta) <= FloatType(0)) {
                    return false;
                }
            }

            std::vector<Model::BrushFaceHandle> newDragHandles;
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            return kdl::for_each_result(m_dragHandlesAtDragStart, [&](const auto& dragFaceHandle) {
                auto* brushNode = dragFaceHandle.node;

                const auto& oldBrush = dragFaceHandle.brushAtDragStart;
                const auto dragFaceIndex = dragFaceHandle.faceIndex;
                const auto newDragFaceNormal = dragFaceHandle.faceNormal();

                auto newBrush = oldBrush;
                return newBrush.moveBoundary(worldBounds, dragFaceIndex, delta, lockTextures)
                    .and_then([&]() {
                        auto clipFace = oldBrush.face(dragFaceIndex);
                        clipFace.invert();
                        return newBrush.clip(worldBounds, std::move(clipFace));
                    }).and_then([&]() {
                        auto* newBrushNode = new Model::BrushNode(std::move(newBrush));
                        newNodes[brushNode->parent()].push_back(newBrushNode);

                        // Look up the new face index of the new drag handle
                        if (const auto newDragFaceIndex = newBrushNode->brush().findFace(newDragFaceNormal)) {
                            newDragHandles.push_back(Model::BrushFaceHandle(newBrushNode, *newDragFaceIndex));
                        }
                    });
            }).and_then([&]() {
                // Apply the changes calculated above
                document->rollbackTransaction();

                document->deselectAll();
                const auto addedNodes = document->addNodes(newNodes);
                document->select(addedNodes);
                m_currentDragVisualHandles = std::move(newDragHandles);
                m_totalDelta = delta;
            }).handle_errors(
                [&](const Model::BrushError e) {
                    document->error() << "Could not extrude brush: " << e;
                    kdl::map_clear_and_delete(newNodes);
                }
            );
        }

        /**
         * Splits brushes "inwards" effectively clipping the selected brushes into two halves.
         * 
         * Returns false if the given delta isn't suitable for splitting inward.
         * 
         * Otherwise:
         * - rolls back the transaction
         * - applies a split inward with the given delta
         * - sets m_totalDelta to the given delta
         * - returns true
         */
        bool ResizeBrushesTool::splitBrushesInward(const vm::vec3& delta) {
            ensure(m_dragging, "may only be called during a drag");
            auto document = kdl::mem_lock(m_document);
            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);

            // First ensure that the drag can be applied at all. For this, check whether each drag handle is moved
            // "down" along its normal.
            for (const auto& handle : m_dragHandlesAtDragStart) {
                const auto& normal = handle.faceNormal();
                if (vm::dot(normal, delta) > 0.0) {
                    return false;
                }
            }

            std::vector<Model::BrushFaceHandle> newDragHandles;
            // This map is to handle the case when the brushes being
            // extruded have different parents (e.g. different brush entities),
            // so each newly created brush should be made a sibling of the brush it was cloned from.
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;
            std::vector<std::pair<Model::Node*, Model::NodeContents>> nodesToUpdate;

            for (const auto& dragFaceHandle : m_dragHandlesAtDragStart) {
                auto* brushNode = dragFaceHandle.node;

                // "Front" means the part closer to the drag handles at the drag start
                auto frontBrush = dragFaceHandle.brushAtDragStart;
                auto backBrush = dragFaceHandle.brushAtDragStart;

                auto clipFace = frontBrush.face(dragFaceHandle.faceIndex);
                                
                if (clipFace.transform(vm::translation_matrix(delta), lockTextures).is_error()) {
                    document->error() << "Could not extrude inwards: Error transforming face";
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }

                auto clipFaceInverted = clipFace;
                clipFaceInverted.invert();

                // Front brush should always be valid
                if (frontBrush.clip(worldBounds, clipFaceInverted).is_error()) {
                    document->error() << "Could not extrude inwards: Front brush is empty";
                    kdl::map_clear_and_delete(newNodes);
                    return false;
                }

                nodesToUpdate.emplace_back(brushNode, std::move(frontBrush));

                // Back brush 
                if (backBrush.clip(worldBounds, clipFace).is_success()) {
                    auto* newBrushNode = new Model::BrushNode(std::move(backBrush));
                    newNodes[brushNode->parent()].push_back(newBrushNode);

                    // Look up the new face index of the new drag handle
                    if (const auto newDragFaceIndex = newBrushNode->brush().findFace(clipFace.normal())) {
                        newDragHandles.push_back(Model::BrushFaceHandle(newBrushNode, *newDragFaceIndex));
                    }
                }
            }
            
            // Apply changes calculated above

            m_currentDragVisualHandles.clear();
            document->rollbackTransaction();

            // FIXME: deal with linked group update failure (needed for #3647)
            const bool success = document->swapNodeContents("Resize Brushes", nodesToUpdate);
            unused(success);

            // Add the newly split off brushes and select them (keeping the original brushes selected).
            // FIXME: deal with linked group update failure (needed for #3647)
            const auto addedNodes = document->addNodes(std::move(newNodes));
            document->select(addedNodes);

            m_currentDragVisualHandles = std::move(newDragHandles);
            m_totalDelta = delta;

            return true;
        }

        std::vector<vm::polygon3> ResizeBrushesTool::polygonsAtDragStart() const {
            return kdl::vec_transform(m_dragHandlesAtDragStart, [](const auto& handle) {
                return handle.brushAtDragStart.face(handle.faceIndex).polygon();
            });
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
                m_proposedDragHandles.clear();
            }
        }

        void ResizeBrushesTool::selectionDidChange(const Selection&) {
            if (!m_dragging) {
                m_proposedDragHandles.clear();
            }
        }
    }
}
