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
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
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
        const Model::HitType::Type ResizeBrushesTool::ResizeHit2D = Model::HitType::freeType();
        const Model::HitType::Type ResizeBrushesTool::ResizeHit3D = Model::HitType::freeType();

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
            const auto& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHit).occluded().selected().first();
            if (hit.isMatch()) {
                return Model::Hit::NoHit;
            } else {
                return pickProximateFace(ResizeHit2D, pickRay);
            }
        }

        Model::Hit ResizeBrushesTool::pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            auto document = kdl::mem_lock(m_document);
            const auto& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHit).occluded().selected().first();
            if (hit.isMatch()) {
                return Model::Hit(ResizeHit3D, hit.distance(), hit.hitPoint(), Model::hitToFace(hit));
            } else {
                return pickProximateFace(ResizeHit3D, pickRay);
            }
        }

        class ResizeBrushesTool::PickProximateFace : public Model::ConstNodeVisitor, public Model::NodeQuery<Model::Hit> {
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
            void doVisit(const Model::World*) override  {}
            void doVisit(const Model::LayerNode*) override  {}
            void doVisit(const Model::Group*) override  {}
            void doVisit(const Model::Entity*) override {}
            void doVisit(const Model::BrushNode* brush) override   {
                for (const auto* edge : brush->edges())
                    visitEdge(edge);
            }

            void visitEdge(const Model::BrushEdge* edge) {
                auto* left = edge->firstFace()->payload();
                auto* right = edge->secondFace()->payload();
                const auto leftDot  = dot(left->boundary().normal,  m_pickRay.direction);
                const auto rightDot = dot(right->boundary().normal, m_pickRay.direction);

                if ((leftDot > 0.0) != (rightDot > 0.0)) {
                    const auto result = vm::distance(m_pickRay, vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position()));
                    if (!vm::is_nan(result.distance) && result.distance < m_closest) {
                        m_closest = result.distance;
                        const auto hitPoint = vm::point_at_distance(m_pickRay, result.position1);
                        if (m_hitType == ResizeBrushesTool::ResizeHit2D) {
                            std::vector<Model::BrushFace*> faces;
                            if (vm::is_zero(leftDot, vm::C::almost_zero())) {
                                faces.push_back(left);
                            } else if (vm::is_zero(rightDot, vm::C::almost_zero())) {
                                faces.push_back(right);
                            } else {
                                if (vm::abs(leftDot) < 1.0) {
                                    faces.push_back(left);
                                }
                                if (vm::abs(rightDot) < 1.0) {
                                    faces.push_back(right);
                                }
                            }
                            setResult(Model::Hit(m_hitType, result.position1, hitPoint, faces));
                        } else {
                            auto* face = leftDot > rightDot ? left : right;
                            setResult(Model::Hit(m_hitType, result.position1, hitPoint, face));
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

        std::vector<Model::BrushFace*> ResizeBrushesTool::dragFaces() const {
            std::vector<Model::BrushFace*> result;
            for (const auto& handle : m_dragHandles) {
                const auto* brush = std::get<0>(handle);
                const auto& normal = std::get<1>(handle);
                auto* face = brush->findFace(normal);
                assert(face != nullptr);
                result.push_back(face);
            }
            return result;
        }

        void ResizeBrushesTool::updateDragFaces(const Model::PickResult& pickResult) {
            const auto& hit = pickResult.query().type(ResizeHit2D | ResizeHit3D).occluded().first();
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

            bool operator()(Model::BrushFace* face) const {
                return face != m_reference && vm::is_equal(face->boundary(), m_reference->boundary(),
                    vm::C::almost_zero());
            }
        };

        std::vector<ResizeBrushesTool::FaceHandle> ResizeBrushesTool::collectDragHandles(const Model::Hit& hit) const {
            assert(hit.isMatch());
            assert(hit.type() == ResizeHit2D || hit.type() == ResizeHit3D);

            std::vector<Model::BrushFace*> result;
            if (hit.type() == ResizeHit2D) {
                const std::vector<Model::BrushFace*>& faces = hit.target<const std::vector<Model::BrushFace*>&>();
                assert(!faces.empty());
                kdl::vec_append(result, faces, collectDragFaces(faces[0]));
                if (faces.size() > 1) {
                    kdl::vec_append(result, collectDragFaces(faces[1]));
                }
            } else {
                Model::BrushFace* face = hit.target<Model::BrushFace*>();
                result.push_back(face);
                kdl::vec_append(result, collectDragFaces(face));
            }

            return getDragHandles(result);
        }

        std::vector<Model::BrushFace*> ResizeBrushesTool::collectDragFaces(Model::BrushFace* face) const {
            Model::CollectMatchingBrushFacesVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(face)));

            auto document = kdl::mem_lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
            return visitor.faces();
        }

        std::vector<ResizeBrushesTool::FaceHandle> ResizeBrushesTool::getDragHandles(const std::vector<Model::BrushFace*>& faces) const {
            std::vector<FaceHandle> result;
            for (auto* face : faces) {
                result.push_back(std::make_tuple(face->brush(), face->boundary().normal));
            }
            return result;
        }

        bool ResizeBrushesTool::beginResize(const Model::PickResult& pickResult, const bool split) {
            const auto& hit = pickResult.query().type(ResizeHit2D | ResizeHit3D).occluded().first();
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
            assert(hasDragFaces());

            auto* dragFace = dragFaces().front();
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
            const auto& hit = pickResult.query().type(ResizeHit2D).occluded().first();
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
            for (const auto* face : dragFaces()) {
                brushMap[face->polygon()] = { face->brush() };
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

            std::vector<Model::BrushNode*> newBrushes;
            std::vector<FaceHandle> newDragHandles;
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            for (auto* dragFace : dragFaces()) {
                auto* brush = dragFace->brush();

                auto* newBrush = brush->clone(worldBounds);
                auto* newDragFace = findMatchingFace(newBrush, dragFace);

                newBrushes.push_back(newBrush);
                newDragHandles.emplace_back(newDragFace->brush(), newDragFace->boundary().normal);

                if (!newBrush->canMoveBoundary(worldBounds, newDragFace, delta)) {
                    // There is a brush for which the move is not applicable. Abort.
                    kdl::col_delete_all(newBrushes);
                    return false;
                } else {
                    auto* clipFace = newDragFace->clone();
                    clipFace->invert();

                    newBrush->moveBoundary(worldBounds, newDragFace, delta, lockTextures);

                    // This should never happen, but let's be on the safe side.
                    if (!newBrush->clip(worldBounds, clipFace)) {
                        delete clipFace;
                        kdl::col_delete_all(newBrushes);
                        return false;
                    }

                    newNodes[brush->parent()].push_back(newBrush);
                }
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
            std::vector<Model::BrushNode*> newBrushes;
            std::vector<FaceHandle> newDragHandles;
            // This map is to handle the case when the brushes being
            // extruded have different parents (e.g. different brush entities),
            // so each newly created brush should be made a sibling of the brush it was cloned from.
            std::map<Model::Node*, std::vector<Model::Node*>> newNodes;

            for (auto* dragFace : dragFaces()) {
                auto* brush = dragFace->brush();

                auto* newBrush = brush->clone(worldBounds);
                auto* newDragFace = findMatchingFace(newBrush, dragFace);

                auto* clipFace = newDragFace->clone();
                clipFace->invert();
                clipFace->transform(vm::translation_matrix(delta), lockTextures);

                if (!newBrush->clip(worldBounds, clipFace)) {
                    delete clipFace;
                    kdl::col_delete_all(newBrushes);
                    return false;
                }

                newBrushes.push_back(newBrush);
                newDragHandles.emplace_back(newBrush, clipFace->boundary().normal);
                newNodes[brush->parent()].push_back(newBrush);
            }

            // Now that the newly split off brushes are ready to insert (but not selected),
            // resize the original brushes, which are still selected at this point.
            if (!document->resizeBrushes(dragFaceDescriptors(), delta)) {
                kdl::col_delete_all(newBrushes);
                return false;
            }

            // Add the newly split off brushes and select them (keeping the original brushes selected).
            const auto addedNodes = document->addNodes(newNodes);
            document->select(addedNodes);

            m_dragHandles = kdl::vec_concat(oldDragHandles, newDragHandles);

            return true;
        }

        Model::BrushFace* ResizeBrushesTool::findMatchingFace(Model::BrushNode* brush, const Model::BrushFace* reference) const {
            Model::FindMatchingBrushFaceVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(reference)));
            visitor.visit(brush);
            if (!visitor.hasResult()) {
                return nullptr;
            } else {
                return visitor.result();
            }
        }

        std::vector<vm::polygon3> ResizeBrushesTool::dragFaceDescriptors() const {
            const auto dragFaces = this->dragFaces();

            std::vector<vm::polygon3> result;
            result.reserve(dragFaces.size());
            for (const auto* face : dragFaces) {
                result.push_back(face->polygon());
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
