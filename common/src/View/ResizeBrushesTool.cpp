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
#include <vecmath/vec_io.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/distance.h>
#include <vecmath/intersection.h>
#include <vecmath/scalar.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type ResizeBrushesTool::Resize2DHitType = Model::HitType::freeType();
        const Model::HitType::Type ResizeBrushesTool::Resize3DHitType = Model::HitType::freeType();

        FaceHandle::FaceHandle(const Model::BrushFaceHandle& handle) :
        node(handle.node()),
        brushAtDragStart(std::make_unique<Model::Brush>(handle.node()->brush())),
        faceIndex(handle.faceIndex()) {}

        Model::BrushFace& FaceHandle::faceAtDragStart() const {
            return brushAtDragStart->face(faceIndex);
        }

        vm::vec3 FaceHandle::faceNormal() const {
            return faceAtDragStart().normal();
        }

        vm::polygon3 FaceHandle::polygonAtDragStart() const {
            return faceAtDragStart().polygon();
        }

        bool FaceHandle::operator==(const FaceHandle& other) const {
            return node == other.node
                && *brushAtDragStart == *other.brushAtDragStart
                && faceIndex == other.faceIndex;
            //&& polygonAtDragStart == other.polygonAtDragStart;
        }

        bool FaceHandle::operator!=(const FaceHandle& other) const {
            return !(*this == other);
        }

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

        bool ResizeBrushesTool::hasDragFaces() const {
            return !m_currentDragVisualHandles.empty();
        }

        /**
         * Used for rendering -> returns the "current" handles (may be none if all brushes are deleted)
         */
        std::vector<Model::BrushFaceHandle> ResizeBrushesTool::dragFaces() const {
            return m_currentDragVisualHandles;

            //std::vector<Model::BrushFaceHandle> result;
            //result.reserve(m_dragHandles.size());
            //for (const auto& dragHandle : m_dragHandles) {
            //    auto* brushNode = dragHandle.node;
            //    const auto& normal = dragHandle.faceNormal();
            //    if (brushNode->parent() == nullptr) {
            //        // don't draw faces on brushes that were collapsed and deleted
            //        continue;
            //    }
            //    const auto& brush = brushNode->brush();
            //    const auto faceIndex = brush.findFace(normal);
            //    if (faceIndex) {
            //        result.push_back(Model::BrushFaceHandle(brushNode, *faceIndex));
            //    }
            //}
            //return result;
        }

        void ResizeBrushesTool::updateDragFaces(const Model::PickResult& pickResult) {
            // FIXME: assert not dragging
            std::cout << "update drag faces\n";
            const auto& hit = pickResult.query().type(Resize2DHitType | Resize3DHitType).occluded().first();
            auto newDragHandles = getDragHandles(hit);
            if (newDragHandles != m_dragHandles) {
                refreshViews();
            }

            using std::swap;
            swap(m_dragHandles, newDragHandles);
        }

        std::vector<FaceHandle> ResizeBrushesTool::getDragHandles(const Model::Hit& hit) const {
            if (hit.isMatch()) {
                return collectDragHandles(hit);
            } else {
                return std::vector<FaceHandle>{};
            }
        }

        std::vector<FaceHandle> ResizeBrushesTool::collectDragHandles(const Model::Hit& hit) const {
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
                return FaceHandle(handle);
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
         * Starts resizing the faces determined by the previous call to updateDragFaces
         */
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
            const FaceHandle& dragFaceHandle = m_dragHandles.at(0);
            const Model::BrushFace& dragFace = dragFaceHandle.faceAtDragStart();
            const vm::vec3& faceNormal = dragFace.boundary().normal;

            const vm::line_distance<FloatType> dist = vm::distance(pickRay, vm::line3(m_dragOrigin, faceNormal));
            if (dist.parallel) {
                return true;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();

            auto dragDistToSnappedDelta = [&](const FloatType dist) -> vm::vec3 {
                const auto unsnappedDelta = faceNormal * dist;
                return grid.snap() ? grid.moveDelta(dragFace, unsnappedDelta) : unsnappedDelta;
            };
            auto deltaToDist = [&](const vm::vec3& delta) -> FloatType {
                return vm::dot(delta, faceNormal);
            };

            const FloatType dragDist = dist.position2;
            const vm::vec3 faceDelta = dragDistToSnappedDelta(dragDist);

            if (vm::is_equal(faceDelta, m_totalDelta, vm::C::almost_zero())) {
                return true;
            }

            if (m_splitBrushes) {
                if (splitBrushesOutward(faceDelta) || splitBrushesInward(faceDelta)) {
                    m_totalDelta = faceDelta;
                    return true;
                }
            } else {
                // This handles ordinary resizing, splitting outward, and splitting inward
                // (in which case dragFaceDescriptors() is a list of polygons splitting the selected brushes)
                document->rollbackTransaction();
                if (document->resizeBrushes(polygonsAtDragStart(), faceDelta)) {
                    m_totalDelta = faceDelta;
                } else {
                    qDebug() << "went too far";
                    document->resizeBrushes(polygonsAtDragStart(), m_totalDelta);
                }
            }

            return true;
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

            auto facesToMove = kdl::vec_transform(dragFaces(), [](const auto& handle) { return handle.face().polygon(); });
            if (document->moveFaces(std::move(facesToMove), delta)) {
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
            m_currentDragVisualHandles.clear();
            m_dragging = false;
        }

        void ResizeBrushesTool::cancel() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
            m_dragHandles.clear();
            m_currentDragVisualHandles.clear();
            m_dragging = false;
        }

        /**
         * Splits off new brush "outward" from the drag handles.
         */
        bool ResizeBrushesTool::splitBrushesOutward(const vm::vec3& delta) {
            auto document = kdl::mem_lock(m_document);

            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);

            // First ensure that the drag can be applied at all. For this, check whether each drag handle is moved
            // "up" along its normal.
            for (const auto& handle : m_dragHandles) {
                const auto& normal = handle.faceNormal();
                if (vm::dot(normal, delta) <= FloatType(0)) {
                    return false;
                }
            }

            m_currentDragVisualHandles.clear();
            document->rollbackTransaction();

            std::vector<Model::BrushFaceHandle> newDragHandles;
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            return kdl::for_each_result(m_dragHandles, [&](const auto& dragFaceHandle) {
                auto* brushNode = dragFaceHandle.node;

                const auto& oldBrush = brushNode->brush();
                const auto dragFaceIndex = dragFaceHandle.faceIndex;
                const auto newDragFaceNormal = oldBrush.face(dragFaceIndex).boundary().normal;

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
                document->deselectAll();
                const auto addedNodes = document->addNodes(newNodes);
                document->select(addedNodes);
                m_currentDragVisualHandles = std::move(newDragHandles);
            }).handle_errors(
                [&](const Model::BrushError e) {
                    document->error() << "Could not extrude brush: " << e;
                    kdl::map_clear_and_delete(newNodes);
                }
            );
        }

        namespace {
            struct ResizeError {
                std::string msg;
            };

            std::ostream& operator<<(std::ostream& str, const ResizeError& e) {
                str << e.msg;
                return str;
            }
        }

        /**
         * Splits brushes "inwards" effectively clipping the selected brushes into two halves.
         */
        bool ResizeBrushesTool::splitBrushesInward(const vm::vec3& delta) {
            auto document = kdl::mem_lock(m_document);
            const vm::bbox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);

            // First ensure that the drag can be applied at all. For this, check whether each drag handle is moved
            // "down" along its normal.
            for (const auto& handle : m_dragHandles) {
                const auto& normal = handle.faceNormal();
                if (vm::dot(normal, delta) > 0.0) {
                    return false;
                }
            }

            m_currentDragVisualHandles.clear();
            document->rollbackTransaction();

            std::vector<Model::BrushFaceHandle> newDragHandles;
            // This map is to handle the case when the brushes being
            // extruded have different parents (e.g. different brush entities),
            // so each newly created brush should be made a sibling of the brush it was cloned from.
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            return kdl::for_each_result(m_dragHandles, [&](const auto& dragFaceHandle) -> kdl::result<void, ResizeError, Model::BrushError> {
                const auto& dragFace = dragFaceHandle.faceAtDragStart();
                auto* brushNode = dragFaceHandle.node;

                auto newBrush = brushNode->brush();
                const auto newDragFaceIndex = newBrush.findFace(dragFace.boundary());
                if (!newDragFaceIndex) {
                    return ResizeError{"Face not found"};
                }

                auto clipFace = newBrush.face(*newDragFaceIndex);
                clipFace.invert();
                
                return clipFace.transform(vm::translation_matrix(delta), lockTextures)
                    .and_then([&]() {
                        return newBrush.clip(worldBounds, clipFace);
                    }).and_then([&]() {
                        auto* newBrushNode = new Model::BrushNode(std::move(newBrush));
                        newNodes[brushNode->parent()].push_back(newBrushNode);

                        // Look up the new face index of the new drag handle
                        if (const auto newDragFaceIndex = newBrushNode->brush().findFace(clipFace.normal())) {
                            newDragHandles.push_back(Model::BrushFaceHandle(newBrushNode, *newDragFaceIndex));
                        }
                    });
            }).and_then([&]() -> kdl::result<void, ResizeError> {
                // Now that the newly split off brushes are ready to insert (but not selected),
                // resize the original brushes, which are still selected at this point.
                if (!document->resizeBrushes(polygonsAtDragStart(), delta)) {
                    return ResizeError{"Resizing failed"};
                }

                // Add the newly split off brushes and select them (keeping the original brushes selected).
                const auto addedNodes = document->addNodes(newNodes);
                document->select(addedNodes);

                m_currentDragVisualHandles = std::move(newDragHandles);
                
                return kdl::void_success;
            }).handle_errors([&](const auto& e) {
                document->error() << "Could not extrude inwards: " << e;
                kdl::map_clear_and_delete(newNodes);
            });
        }

        std::vector<vm::polygon3> ResizeBrushesTool::polygonsAtDragStart() const {
            return kdl::vec_transform(m_dragHandles, [](const auto& handle) {
                return handle.brushAtDragStart->face(handle.faceIndex).polygon();
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
