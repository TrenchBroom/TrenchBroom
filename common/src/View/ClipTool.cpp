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

#include "ClipTool.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "FloatType.h"
#include "Model/AssortNodesVisitor.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/MapDocument.h"
#include "View/Selection.h"

#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/set_temp.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/ray.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <algorithm>
#include <map>
#include <optional>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type ClipTool::PointHitType = Model::HitType::freeType();

        ClipTool::ClipStrategy::~ClipStrategy() = default;

        void ClipTool::ClipStrategy::pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            doPick(pickRay, camera, pickResult);
        }

        void ClipTool::ClipStrategy::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            doRender(renderContext, renderBatch, pickResult);
        }

        void ClipTool::ClipStrategy::renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& point) const {
            doRenderFeedback(renderContext, renderBatch, point);
        }

        bool ClipTool::ClipStrategy::computeThirdPoint(vm::vec3& point) const {
            return doComputeThirdPoint(point);
        }

        bool ClipTool::ClipStrategy::canClip() const {
            return doCanClip();
        }

        bool ClipTool::ClipStrategy::hasPoints() const {
            return doHasPoints();
        }

        bool ClipTool::ClipStrategy::canAddPoint(const vm::vec3& point) const {
            return doCanAddPoint(point);
        }

        void ClipTool::ClipStrategy::addPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors) {
            assert(canAddPoint(point));
            return doAddPoint(point, helpVectors);
        }

        bool ClipTool::ClipStrategy::canRemoveLastPoint() const {
            return doCanRemoveLastPoint();
        }

        void ClipTool::ClipStrategy::removeLastPoint() {
            doRemoveLastPoint();
        }

        bool ClipTool::ClipStrategy::canDragPoint(const Model::PickResult& pickResult, vm::vec3& initialPosition) const {
            return doCanDragPoint(pickResult, initialPosition);
        }

        void ClipTool::ClipStrategy::beginDragPoint(const Model::PickResult& pickResult) {
            doBeginDragPoint(pickResult);
        }

        void ClipTool::ClipStrategy::beginDragLastPoint() {
            doBeginDragLastPoint();
        }

        bool ClipTool::ClipStrategy::dragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors) {
            return doDragPoint(newPosition, helpVectors);
        }

        void ClipTool::ClipStrategy::endDragPoint() {
            doEndDragPoint();
        }

        void ClipTool::ClipStrategy::cancelDragPoint() {
            doCancelDragPoint();
        }

        bool ClipTool::ClipStrategy::setFace(const Model::BrushFaceHandle& faceHandle) {
            return doSetFace(faceHandle);
        }

        void ClipTool::ClipStrategy::reset() {
            doReset();
        }

        size_t ClipTool::ClipStrategy::getPoints(vm::vec3& point1, vm::vec3& point2, vm::vec3& point3) const {
            return doGetPoints(point1, point2, point3);
        }

        class ClipTool::PointClipStrategy : public ClipTool::ClipStrategy {
        private:
            struct ClipPoint {
                vm::vec3 point;
                std::vector<vm::vec3> helpVectors;

                ClipPoint() = default;

                ClipPoint(const vm::vec3& i_point, const std::vector<vm::vec3>& i_helpVectors) :
                point(i_point),
                helpVectors(i_helpVectors) {}
            };

            ClipPoint m_points[3];
            size_t m_numPoints;
            size_t m_dragIndex;
            ClipPoint m_originalPoint;
        public:
            PointClipStrategy() :
            m_numPoints(0),
            m_dragIndex(4) {}
        private:
            void doPick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override {
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const auto& point = m_points[i].point;
                    const auto distance = camera.pickPointHandle(pickRay, point, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                    if (!vm::is_nan(distance)) {
                        const auto hitPoint = vm::point_at_distance(pickRay, distance);
                        pickResult.addHit(Model::Hit(PointHitType, distance, hitPoint, i));
                    }
                }
            }

            void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) override {
                renderPoints(renderContext, renderBatch);
                renderHighlight(renderContext, renderBatch, pickResult);
            }

            void doRenderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& point) const override {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.renderHandle(vm::vec3f(point));
            }

            bool doComputeThirdPoint(vm::vec3& point) const override {
                ensure(m_numPoints == 2, "invalid numPoints");
                point = m_points[1].point + 128.0 * computeHelpVector();
                return !vm::is_colinear(m_points[0].point, m_points[1].point, point);
            }

            vm::vec3 computeHelpVector() const {
                size_t counts[6];
                counts[0] = counts[1] = counts[2] = counts[3] = counts[4] = counts[5] = 0;

                const auto helpVectors = combineHelpVectors();
                for (size_t i = 0; i < helpVectors.size(); ++i) {
                    const auto axis = vm::find_abs_max_component(helpVectors[i]);
                    const auto index = helpVectors[i][axis] > 0.0 ? axis : axis + 3;
                    counts[index]++;
                }

                const auto first = std::max_element(std::begin(counts), std::end(counts));
                const auto next  = std::max_element(std::next(first),   std::end(counts));

                const auto firstIndex = first - std::begin(counts);
                const auto nextIndex = next - std::begin(counts);

                if (counts[firstIndex] > counts[nextIndex]) {
                    return vm::vec3::axis(static_cast<size_t>(firstIndex % 3));
                } else {
                    // two counts are equal
                    if (firstIndex % 3 == 2 || nextIndex % 3 == 2) {
                        // prefer the Z axis if possible:
                        return vm::vec3::pos_z();
                    } else {
                        // Z axis cannot win, so X and Y axis are a tie, prefer the X axis:
                        return vm::vec3::pos_x();
                    }
                }
            }

            std::vector<vm::vec3> combineHelpVectors() const {
                std::vector<vm::vec3> result;
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const std::vector<vm::vec3>& helpVectors = m_points[i].helpVectors;
                    kdl::vec_append(result, helpVectors);
                }

                return result;
            }

            bool doCanClip() const override {
                if (m_numPoints < 2) {
                    return false;
                } else if (m_numPoints == 2) {
                    vm::vec3 point3;
                    if (!computeThirdPoint(point3)) {
                        return false;
                    }
                }
                return true;
            }

            bool doHasPoints() const override {
                return m_numPoints > 0;
            }

            bool doCanAddPoint(const vm::vec3& point) const override {
                if (m_numPoints == 3) {
                    return false;
                } else if (m_numPoints == 2 && vm::is_colinear(m_points[0].point, m_points[1].point, point)) {
                    return false;
                } else {
                    // Don't allow to place a point onto another point!
                    for (size_t i = 0; i < m_numPoints; ++i) {
                        if (vm::is_equal(m_points[i].point, point, vm::C::almost_zero())) {
                            return false;
                        }
                    }
                    return true;
                }
            }

            void doAddPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors) override {
                m_points[m_numPoints] = ClipPoint(point, helpVectors);
                ++m_numPoints;
            }

            bool doCanRemoveLastPoint() const override {
                return m_numPoints > 0;
            }

            void doRemoveLastPoint() override {
                ensure(canRemoveLastPoint(), "can't remove last point");
                --m_numPoints;
            }

            bool doCanDragPoint(const Model::PickResult& pickResult, vm::vec3& initialPosition) const override {
                const auto& hit = pickResult.query().type(PointHitType).occluded().first();
                if (!hit.isMatch()) {
                    return false;
                } else {
                    const auto index = hit.target<size_t>();
                    initialPosition = m_points[index].point;
                    return true;
                }
            }

            void doBeginDragPoint(const Model::PickResult& pickResult) override {
                const auto& hit = pickResult.query().type(PointHitType).occluded().first();
                assert(hit.isMatch());
                m_dragIndex = hit.target<size_t>();
                m_originalPoint = m_points[m_dragIndex];
            }

            void doBeginDragLastPoint() override {
                ensure(m_numPoints > 0, "invalid numPoints");
                m_dragIndex = m_numPoints - 1;
                m_originalPoint = m_points[m_dragIndex];
            }

            bool doDragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors) override {
                ensure(m_dragIndex < m_numPoints, "drag index out of range");

                // Don't allow to drag a point onto another point!
                for (size_t i = 0; i < m_numPoints; ++i) {
                    if (m_dragIndex != i && vm::is_equal(m_points[i].point, newPosition, vm::C::almost_zero())) {
                        return false;
                    }
                }

                if (m_numPoints == 3) {
                    size_t index0, index1;
                    switch (m_dragIndex) {
                        case 0:
                            index0 = 1;
                            index1 = 2;
                            break;
                        case 1:
                            index0 = 0;
                            index1 = 2;
                            break;
                        case 2:
                        default:
                            index0 = 0;
                            index1 = 1;
                            break;
                    }

                    if (vm::is_colinear(m_points[index0].point, m_points[index1].point, newPosition)) {
                        return false;
                    }
                }

                if (helpVectors.empty()) {
                    m_points[m_dragIndex] = ClipPoint(newPosition, m_points[m_dragIndex].helpVectors);
                } else {
                    m_points[m_dragIndex] = ClipPoint(newPosition, helpVectors);
                }
                return true;
            }

            void doEndDragPoint() override {
                m_dragIndex = 4;
            }

            void doCancelDragPoint() override {
                ensure(m_dragIndex < m_numPoints, "drag index out of range");
                m_points[m_dragIndex] = m_originalPoint;
                m_dragIndex = 4;
            }

            bool doSetFace(const Model::BrushFaceHandle& /* faceHandle */) override {
                return false;
            }

            void doReset() override {
                m_numPoints = 0;
            }

            size_t doGetPoints(vm::vec3& point1, vm::vec3& point2, vm::vec3& point3) const override {
                switch (m_numPoints) {
                    case 0:
                        return 0;
                    case 1:
                        point1 = m_points[0].point;
                        return 1;
                    case 2:
                        point1 = m_points[0].point;
                        point2 = m_points[1].point;
                        if (computeThirdPoint(point3))
                            return 3;
                        return 2;
                    case 3:
                        point1 = m_points[0].point;
                        point2 = m_points[1].point;
                        point3 = m_points[2].point;
                        return 3;
                    default:
                        ensure(false, "invalid numPoints");
                        return 0;
                }
            }
        private:
            void renderPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.setShowOccludedObjects();

                if (m_numPoints > 1) {
                    renderService.renderLine(vm::vec3f(m_points[0].point), vm::vec3f(m_points[1].point));
                    if (m_numPoints > 2) {
                        renderService.renderLine(vm::vec3f(m_points[1].point), vm::vec3f(m_points[2].point));
                        renderService.renderLine(vm::vec3f(m_points[2].point), vm::vec3f(m_points[0].point));
                    }
                }

                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));

                for (size_t i = 0; i < m_numPoints; ++i) {
                    const auto& point = m_points[i].point;
                    renderService.renderHandle(vm::vec3f(point));
                    renderService.renderString(kdl::str_to_string(i+1, ": ", point), vm::vec3f(point));
                }
            }

            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
                if (m_dragIndex < m_numPoints) {
                    renderHighlight(renderContext, renderBatch, m_dragIndex);
                } else {
                    const auto& hit = pickResult.query().type(PointHitType).occluded().first();
                    if (hit.isMatch()) {
                        const auto index = hit.target<size_t>();
                        renderHighlight(renderContext, renderBatch, index);
                    }
                }
            }

            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const size_t index) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
                renderService.renderHandleHighlight(vm::vec3f(m_points[index].point));
            }
        };

        class ClipTool::FaceClipStrategy : public ClipTool::ClipStrategy {
        private:
            std::optional<Model::BrushFaceHandle> m_faceHandle;
        private:
            void doPick(const vm::ray3& /* pickRay */, const Renderer::Camera& /* camera */, Model::PickResult&) const override {}

            void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult&) override {
                if (m_faceHandle) {
                    Renderer::RenderService renderService(renderContext, renderBatch);

                    const auto vertices = m_faceHandle->face().vertices();

                    std::vector<vm::vec3f> positions;
                    positions.reserve(vertices.size());

                    for (const Model::BrushVertex* vertex : vertices) {
                        positions.push_back(vm::vec3f(vertex->position()));
                    }

                    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                    renderService.renderPolygonOutline(positions);

                    renderService.setForegroundColor(pref(Preferences::ClipFaceColor));
                    renderService.renderFilledPolygon(positions);
                }
            }

            void doRenderFeedback(Renderer::RenderContext&, Renderer::RenderBatch&, const vm::vec3& /* point */) const override {}

            vm::vec3 doGetHelpVector() const { return vm::vec3::zero(); }

            bool doComputeThirdPoint(vm::vec3& /* point */) const override { return false; }

            bool doCanClip() const override { return m_faceHandle.has_value(); }
            bool doHasPoints() const override { return false; }
            bool doCanAddPoint(const vm::vec3& /* point */) const override { return false; }
            void doAddPoint(const vm::vec3& /* point */, const std::vector<vm::vec3>& /* helpVectors */) override {}
            bool doCanRemoveLastPoint() const override { return false; }
            void doRemoveLastPoint() override {}

            bool doCanDragPoint(const Model::PickResult&, vm::vec3& /* initialPosition */) const override { return false; }
            void doBeginDragPoint(const Model::PickResult&) override {}
            void doBeginDragLastPoint() override {}
            bool doDragPoint(const vm::vec3& /* newPosition */, const std::vector<vm::vec3>& /* helpVectors */) override { return false; }
            void doEndDragPoint() override {}
            void doCancelDragPoint() override {}

            bool doSetFace(const Model::BrushFaceHandle& faceHandle) override {
                m_faceHandle = faceHandle;
                return true;
            }

            void doReset() override {
                m_faceHandle = std::nullopt;
            }

            size_t doGetPoints(vm::vec3& point1, vm::vec3& point2, vm::vec3& point3) const override {
                if (m_faceHandle) {
                    const auto& points = m_faceHandle->face().points();
                    point1 = points[0];
                    point2 = points[1];
                    point3 = points[2];
                    return 3;
                } else {
                    return 0;
                }
            }
        };

        ClipTool::ClipTool(std::weak_ptr<MapDocument> document) :
        Tool(false),
        m_document(std::move(document)),
        m_clipSide(ClipSide_Front),
        m_strategy(nullptr),
        m_remainingBrushRenderer(std::make_unique<Renderer::BrushRenderer>()),
        m_clippedBrushRenderer(std::make_unique<Renderer::BrushRenderer>()),
        m_ignoreNotifications(false),
        m_dragging(false) {}

        ClipTool::~ClipTool() {
            kdl::map_clear_and_delete(m_frontBrushes);
            kdl::map_clear_and_delete(m_backBrushes);
        }

        const Grid& ClipTool::grid() const {
            return kdl::mem_lock(m_document)->grid();
        }

        void ClipTool::toggleSide() {
            if (canClip()) {
                switch (m_clipSide) {
                    case ClipSide_Front:
                        m_clipSide = ClipSide_Both;
                        break;
                    case ClipSide_Both:
                        m_clipSide = ClipSide_Back;
                        break;
                    case ClipSide_Back:
                        m_clipSide = ClipSide_Front;
                        break;
                }
                update();
            }
        }

        void ClipTool::pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            if (m_strategy != nullptr) {
                m_strategy->pick(pickRay, camera, pickResult);
            }
        }

        void ClipTool::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            renderBrushes(renderContext, renderBatch);
            renderStrategy(renderContext, renderBatch, pickResult);
        }

        void ClipTool::renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_remainingBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_remainingBrushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
            m_remainingBrushRenderer->setShowEdges(true);
            m_remainingBrushRenderer->setShowOccludedEdges(true);
            m_remainingBrushRenderer->setOccludedEdgeColor(pref(Preferences::OccludedSelectedEdgeColor));
            m_remainingBrushRenderer->setTint(true);
            m_remainingBrushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
            m_remainingBrushRenderer->render(renderContext, renderBatch);

            m_clippedBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_clippedBrushRenderer->setEdgeColor(Color(pref(Preferences::EdgeColor), 0.5f));
            m_clippedBrushRenderer->setShowEdges(true);
            m_clippedBrushRenderer->setTint(false);
            m_clippedBrushRenderer->setForceTransparent(true);
            m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
            m_clippedBrushRenderer->render(renderContext, renderBatch);
        }

        void ClipTool::renderStrategy(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            if (m_strategy != nullptr) {
                m_strategy->render(renderContext, renderBatch, pickResult);
            }
        }

        void ClipTool::renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& point) const {
            if (m_strategy != nullptr) {
                m_strategy->renderFeedback(renderContext, renderBatch, point);
            } else {
                PointClipStrategy().renderFeedback(renderContext, renderBatch, point);
            }
        }

        bool ClipTool::hasBrushes() const {
            const auto document = kdl::mem_lock(m_document);
            return document->selectedNodes().hasBrushes();
        }

        bool ClipTool::canClip() const {
            return m_strategy != nullptr && m_strategy->canClip();
        }

        void ClipTool::performClip() {
            if (!m_dragging && canClip()) {
                const kdl::set_temp ignoreNotifications(m_ignoreNotifications);

                auto document = kdl::mem_lock(m_document);
                const Transaction transaction(document, "Clip Brushes");

                // need to make a copies here so that we are not affected by the deselection
                const auto toAdd = clipBrushes();
                const auto toRemove = document->selectedNodes().nodes();
                const auto addedNodes = document->addNodes(toAdd);

                document->deselectAll();
                document->removeNodes(toRemove);
                document->select(addedNodes);

                update();
            }
        }

        std::map<Model::Node*, std::vector<Model::Node*>> ClipTool::clipBrushes() {
            std::map<Model::Node*, std::vector<Model::Node*>> result;
            if (!m_frontBrushes.empty()) {
                if (keepFrontBrushes()) {
                    result = kdl::map_merge(result, m_frontBrushes);
                    m_frontBrushes.clear();
                } else {
                    kdl::map_clear_and_delete(m_frontBrushes);
                }
            }

            if (!m_backBrushes.empty()) {
                if (keepBackBrushes()) {
                    result = kdl::map_merge(result, m_backBrushes);
                    m_backBrushes.clear();
                } else {
                    kdl::map_clear_and_delete(m_backBrushes);
                }
            }

            resetStrategy();
            return result;
        }

        vm::vec3 ClipTool::defaultClipPointPos() const {
            auto document = kdl::mem_lock(m_document);
            return document->selectionBounds().center();
        }

        bool ClipTool::canAddPoint(const vm::vec3& point) const {
            return m_strategy == nullptr || m_strategy->canAddPoint(point);
        }

        bool ClipTool::hasPoints() const {
            return m_strategy != nullptr && m_strategy->hasPoints();
        }

        void ClipTool::addPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors) {
            assert(canAddPoint(point));
            if (m_strategy == nullptr) {
                m_strategy = std::make_unique<PointClipStrategy>();
            }

            m_strategy->addPoint(point, helpVectors);
            update();
        }

        bool ClipTool::canRemoveLastPoint() const {
            return m_strategy != nullptr && m_strategy->canRemoveLastPoint();
        }

        bool ClipTool::removeLastPoint() {
            if (canRemoveLastPoint()) {
                m_strategy->removeLastPoint();
                update();
                return true;
            }
            return false;
        }

        bool ClipTool::beginDragPoint(const Model::PickResult& pickResult, vm::vec3& initialPosition) {
            assert(!m_dragging);
            if (m_strategy == nullptr) {
                return false;
            } else if (!m_strategy->canDragPoint(pickResult, initialPosition)) {
                return false;
            } else {
                m_strategy->beginDragPoint(pickResult);
                m_dragging = true;
                return true;
            }
        }

        void ClipTool::beginDragLastPoint() {
            assert(!m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            m_strategy->beginDragLastPoint();
            m_dragging = true;
        }

        bool ClipTool::dragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors) {
            assert(m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            if (!m_strategy->dragPoint(newPosition, helpVectors)) {
                return false;
            } else {
                update();
                return true;
            }
        }

        void ClipTool::endDragPoint() {
            assert(m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            m_strategy->endDragPoint();
            m_dragging = false;
            refreshViews();
        }

        void ClipTool::cancelDragPoint() {
            assert(m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            m_strategy->cancelDragPoint();
            m_dragging = false;
            refreshViews();
        }

        void ClipTool::setFace(const Model::BrushFaceHandle& faceHandle) {
            m_strategy = std::make_unique<FaceClipStrategy>();
            m_strategy->setFace(faceHandle);
            update();
        }

        bool ClipTool::reset() {
            if (m_strategy != nullptr) {
                resetStrategy();
                return true;
            } else {
                return false;
            }
        }

        void ClipTool::resetStrategy() {
            m_strategy.reset();
            update();
        }

        void ClipTool::update() {
            clearRenderers();
            clearBrushes();

            updateBrushes();
            updateRenderers();

            refreshViews();
        }

        void ClipTool::clearBrushes() {
            kdl::map_clear_and_delete(m_frontBrushes);
            kdl::map_clear_and_delete(m_backBrushes);
        }

        void ClipTool::updateBrushes() {
            auto document = kdl::mem_lock(m_document);
            const auto& brushNodes = document->selectedNodes().brushes();
            const auto& worldBounds = document->worldBounds();

            if (canClip()) {
                vm::vec3 point1, point2, point3;
                const auto numPoints = m_strategy->getPoints(point1, point2, point3);
                unused(numPoints);
                ensure(numPoints == 3, "invalid number of points");

                auto* world = document->world();
                for (auto* brushNode : brushNodes) {
                    auto* parent = brushNode->parent();

                    auto frontFace = world->createFace(point1, point2, point3, document->currentTextureName());
                    auto backFace = world->createFace(point1, point3, point2, document->currentTextureName());
                    setFaceAttributes(brushNode->brush().faces(), frontFace, backFace);

                    auto frontBrush = brushNode->brush();
                    if (frontBrush.clip(worldBounds, std::move(frontFace))) {
                        m_frontBrushes[parent].push_back(new Model::BrushNode(std::move(frontBrush)));
                    }
                    
                    auto backBrush = brushNode->brush();
                    if (backBrush.clip(worldBounds, std::move(backFace))) {
                        m_backBrushes[parent].push_back(new Model::BrushNode(std::move(backBrush)));
                    }
                }
            } else {
                for (auto* brushNode : brushNodes) {
                    auto* parent = brushNode->parent();
                    m_frontBrushes[parent].push_back(new Model::BrushNode(brushNode->brush()));
                }
            }
        }

        void ClipTool::setFaceAttributes(const std::vector<Model::BrushFace>& faces, Model::BrushFace& frontFace, Model::BrushFace& backFace) const {
            ensure(!faces.empty(), "no faces");

            auto faceIt = std::begin(faces);
            const auto faceEnd = std::end(faces);
            const auto* bestFrontFace = &*faceIt++;
            const auto* bestBackFace = bestFrontFace;

            while (faceIt != faceEnd) {
                const auto& face = *faceIt;

                const auto bestFrontDiff = bestFrontFace->boundary().normal - frontFace.boundary().normal;
                const auto frontDiff = face.boundary().normal - frontFace.boundary().normal;
                if (vm::squared_length(frontDiff) < vm::squared_length(bestFrontDiff)) {
                    bestFrontFace = &face;
                }

                const auto bestBackDiff = bestBackFace->boundary().normal - backFace.boundary().normal;
                const auto backDiff = face.boundary().normal - backFace.boundary().normal;
                if (vm::squared_length(backDiff) < vm::squared_length(bestBackDiff)) {
                    bestBackFace = &face;
                }

                ++faceIt;
            }

            ensure(bestFrontFace != nullptr, "bestFrontFace is null");
            ensure(bestBackFace != nullptr, "bestBackFace is null");
            frontFace.setAttributes(bestFrontFace);
            backFace.setAttributes(bestBackFace);
        }

        void ClipTool::clearRenderers() {
            m_remainingBrushRenderer->clear();
            m_clippedBrushRenderer->clear();
        }

        void ClipTool::updateRenderers() {
            if (canClip()) {
                if (keepFrontBrushes()) {
                    addBrushesToRenderer(m_frontBrushes, *m_remainingBrushRenderer);
                } else {
                    addBrushesToRenderer(m_frontBrushes, *m_clippedBrushRenderer);
                }

                if (keepBackBrushes()) {
                    addBrushesToRenderer(m_backBrushes, *m_remainingBrushRenderer);
                } else {
                    addBrushesToRenderer(m_backBrushes, *m_clippedBrushRenderer);
                }
            } else {
                addBrushesToRenderer(m_frontBrushes, *m_remainingBrushRenderer);
                addBrushesToRenderer(m_backBrushes, *m_remainingBrushRenderer);
            }
        }

        void ClipTool::addBrushesToRenderer(const std::map<Model::Node*, std::vector<Model::Node*>>& map, Renderer::BrushRenderer& renderer) {
            Model::CollectBrushesVisitor collect;

            for (const auto& entry : map) {
                const auto& brushes = entry.second;
                Model::Node::accept(std::begin(brushes), std::end(brushes), collect);
            }

            renderer.addBrushes(collect.brushes());
        }

        bool ClipTool::keepFrontBrushes() const {
            return m_clipSide != ClipSide_Back;
        }

        bool ClipTool::keepBackBrushes() const {
            return m_clipSide != ClipSide_Front;
        }

        bool ClipTool::doActivate() {
            auto document = kdl::mem_lock(m_document);
            if (!document->selectedNodes().hasOnlyBrushes()) {
                return false;
            } else {
                bindObservers();
                resetStrategy();
                return true;
            }
        }

        bool ClipTool::doDeactivate() {
            unbindObservers();

            m_strategy.reset();
            clearRenderers();
            clearBrushes();

            return true;
        }

        bool ClipTool::doRemove() {
            return removeLastPoint();
        }

        void ClipTool::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &ClipTool::selectionDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ClipTool::nodesWillChange);
            document->nodesDidChangeNotifier.addObserver(this, &ClipTool::nodesDidChange);
            document->brushFacesDidChangeNotifier.addObserver(this, &ClipTool::brushFacesDidChange);
        }

        void ClipTool::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &ClipTool::selectionDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ClipTool::nodesWillChange);
                document->nodesDidChangeNotifier.removeObserver(this, &ClipTool::nodesDidChange);
                document->brushFacesDidChangeNotifier.removeObserver(this, &ClipTool::brushFacesDidChange);
            }
        }

        void ClipTool::selectionDidChange(const Selection&) {
            if (!m_ignoreNotifications) {
                update();
            }
        }

        void ClipTool::nodesWillChange(const std::vector<Model::Node*>&) {
            if (!m_ignoreNotifications) {
                update();
            }
        }

        void ClipTool::nodesDidChange(const std::vector<Model::Node*>&) {
            if (!m_ignoreNotifications) {
                update();
            }
        }

        void ClipTool::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&) {
            if (!m_ignoreNotifications) {
                update();
            }
        }
    }
}
