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

#include "ToolController.h"

#include "Ensure.h"
#include "FloatType.h"
#include "Model/HitQuery.h"
#include "Model/HitType.h"
#include "View/Grid.h"
#include "View/Tool.h"

#include <vecmath/distance.h>
#include <vecmath/intersection.h>
#include <vecmath/line.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        PickingPolicy::~PickingPolicy() = default;

        NoPickingPolicy::~NoPickingPolicy() = default;
        void NoPickingPolicy::doPick(const InputState&, Model::PickResult&) {}

        KeyPolicy::~KeyPolicy() = default;

        NoKeyPolicy::~NoKeyPolicy() = default;
        void NoKeyPolicy::doModifierKeyChange(const InputState&) {}

        MousePolicy::~MousePolicy() = default;

        void MousePolicy::doMouseDown(const InputState&) {}
        void MousePolicy::doMouseUp(const InputState&) {}
        bool MousePolicy::doMouseClick(const InputState&) { return false; }
        bool MousePolicy::doMouseDoubleClick(const InputState&) { return false; }
        void MousePolicy::doMouseMove(const InputState&) {}
        void MousePolicy::doMouseScroll(const InputState&) {}

        MouseDragPolicy::~MouseDragPolicy() = default;

        NoMouseDragPolicy::~NoMouseDragPolicy() = default;

        bool NoMouseDragPolicy::doStartMouseDrag(const InputState&) { return false; }
        bool NoMouseDragPolicy::doMouseDrag(const InputState&) { return false; }
        void NoMouseDragPolicy::doEndMouseDrag(const InputState&) {}
        void NoMouseDragPolicy::doCancelMouseDrag() {}

        DragRestricter::~DragRestricter() = default;

        bool DragRestricter::hitPoint(const InputState& inputState, vm::vec3& point) const {
            return doComputeHitPoint(inputState, point);
        }

        PlaneDragRestricter::PlaneDragRestricter(const vm::plane3& plane) :
        m_plane(plane) {}

        bool PlaneDragRestricter::doComputeHitPoint(const InputState& inputState, vm::vec3& point) const {
            const auto distance = vm::intersect_ray_plane(inputState.pickRay(), m_plane);
            if (vm::is_nan(distance)) {
                return false;
            } else {
                point = vm::point_at_distance(inputState.pickRay(), distance);
                return true;
            }
        }

        LineDragRestricter::LineDragRestricter(const vm::line3& line) :
        m_line(line) {}

        bool LineDragRestricter::doComputeHitPoint(const InputState& inputState, vm::vec3& point) const {
            const auto dist = vm::distance(inputState.pickRay(), m_line);
            if (dist.parallel) {
                return false;
            } else {
                point = m_line.point + m_line.direction * dist.position2;
                return true;
            }
        }

        CircleDragRestricter::CircleDragRestricter(const vm::vec3& center, const vm::vec3& normal, const FloatType radius) :
        m_center(center),
        m_normal(normal),
        m_radius(radius) {
            assert(m_radius > 0.0);
        }

        bool CircleDragRestricter::doComputeHitPoint(const InputState& inputState, vm::vec3& point) const {
            const auto plane = vm::plane3(m_center, m_normal);
            const auto distance = vm::intersect_ray_plane(inputState.pickRay(), plane);
            if (vm::is_nan(distance)) {
                return false;
            } else {
                const auto hitPoint = vm::point_at_distance(inputState.pickRay(), distance);
                const auto direction = normalize(hitPoint - m_center);
                point = m_center + m_radius * direction;
                return true;
            }
        }

        SurfaceDragHelper::SurfaceDragHelper(Model::HitFilter filter) :
        m_filter{filter} {}

        SurfaceDragHelper::~SurfaceDragHelper() = default;

        bool SurfaceDragRestricter::doComputeHitPoint(const InputState& inputState, vm::vec3& point) const {
            const Model::Hit& hit = inputState.pickResult().first(m_filter);
            if (!hit.isMatch())
                return false;
            point = hit.hitPoint();
            return true;
        }

        DragSnapper::~DragSnapper() = default;

        bool DragSnapper::snap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const {
            return doSnap(inputState, initialPoint, lastPoint, curPoint);
        }

        void MultiDragSnapper::addDelegates() {}

        bool MultiDragSnapper::doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& originalCurPoint) const {
            if (m_delegates.empty())
                return false;

            vm::vec3 bestPoint;
            bool anySnapped = false;
            for (const std::unique_ptr<DragSnapper>& delegate : m_delegates) {
                vm::vec3 curPoint = originalCurPoint;
                if (delegate->snap(inputState, initialPoint, lastPoint, curPoint)) {
                    if (anySnapped) {
                        if (vm::squared_distance(curPoint, originalCurPoint) < vm::squared_distance(bestPoint, originalCurPoint)) {
                            bestPoint = curPoint;
                        }
                    } else {
                        bestPoint = curPoint;
                        anySnapped = true;
                    }
                }
            }

            if (anySnapped) {
                originalCurPoint = bestPoint;
            }
            return anySnapped;
        }

        bool NoDragSnapper::doSnap(const InputState&, const vm::vec3& /* initialPoint */, const vm::vec3& /* lastPoint */, vm::vec3& /* curPoint */) const {
            return true;
        }

        AbsoluteDragSnapper::AbsoluteDragSnapper(const Grid& grid, const vm::vec3& offset) :
        m_grid(grid),
        m_offset(offset) {}

        bool AbsoluteDragSnapper::doSnap(const InputState&, const vm::vec3& /* initialPoint */, const vm::vec3& /* lastPoint */, vm::vec3& curPoint) const {
            curPoint = m_grid.snap(curPoint) - m_offset;
            return true;
        }

        DeltaDragSnapper::DeltaDragSnapper(const Grid& grid) :
        m_grid(grid) {}

        bool DeltaDragSnapper::doSnap(const InputState&, const vm::vec3& initialPoint, const vm::vec3& /* lastPoint */, vm::vec3& curPoint) const {
            curPoint = initialPoint + m_grid.snap(curPoint - initialPoint);
            return true;
        }

        LineDragSnapper::LineDragSnapper(const Grid& grid, const vm::line3& line) :
        m_grid(grid),
        m_line(line) {}

        bool LineDragSnapper::doSnap(const InputState&, const vm::vec3& /* initialPoint */, const vm::vec3& /* lastPoint */, vm::vec3& curPoint) const {
            curPoint = m_grid.snap(curPoint, m_line);
            return true;
        }

        CircleDragSnapper::CircleDragSnapper(const Grid& grid, const FloatType snapAngle, const vm::vec3& start, const vm::vec3& center, const vm::vec3& normal, const FloatType radius) :
        m_grid(grid),
        m_snapAngle(snapAngle),
        m_start(start),
        m_center(center),
        m_normal(normal),
        m_radius(radius) {
            assert(m_start != m_center);
            assert(vm::is_unit(m_normal, vm::C::almost_zero()));
            assert(m_radius > 0.0);
        }

        bool CircleDragSnapper::doSnap(const InputState&, const vm::vec3& /* initialPoint */, const vm::vec3& /* lastPoint */, vm::vec3& curPoint) const {
            if (curPoint == m_center) {
                return false;
            }

            const vm::vec3 ref = vm::normalize(m_start - m_center);
            const vm::vec3 vec = vm::normalize(curPoint - m_center);
            const FloatType angle = vm::measure_angle(vec, ref, m_normal);
            const FloatType snapped = m_grid.snapAngle(angle, vm::abs(m_snapAngle));
            const FloatType canonical = snapped - vm::snapDown(snapped, vm::C::two_pi());
            const vm::quat3 rotation(m_normal, canonical);
            const vm::vec3 rot = rotation * ref;
            curPoint = m_center + m_radius * rot;
            return true;
        }

        SurfaceDragSnapper::SurfaceDragSnapper(Model::HitFilter filter, const Grid& grid) :
        SurfaceDragHelper{filter},
        m_grid(grid) {}

        bool SurfaceDragSnapper::doSnap(const InputState& inputState, const vm::vec3& /* initialPoint */, const vm::vec3& /* lastPoint */, vm::vec3& curPoint) const {
            const Model::Hit& hit = inputState.pickResult().first(m_filter);
            if (!hit.isMatch()) {
                return false;
            }

            const vm::plane3& plane = doGetPlane(inputState, hit);
            curPoint = m_grid.snap(hit.hitPoint(), plane);
            return true;
        }

        RestrictedDragPolicy::DragInfo::DragInfo() :
        restricter(nullptr),
        snapper(nullptr),
        computeInitialHandlePosition(false) {}

        RestrictedDragPolicy::DragInfo::DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper) :
        restricter(i_restricter),
        snapper(i_snapper),
        computeInitialHandlePosition(true) {
            ensure(restricter != nullptr, "restricter is null");
            ensure(snapper != nullptr, "snapper is null");
        }

        RestrictedDragPolicy::DragInfo::DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper, const vm::vec3& i_initialHandlePosition) :
        restricter(i_restricter),
        snapper(i_snapper),
        initialHandlePosition(i_initialHandlePosition),
        computeInitialHandlePosition(false) {
            ensure(restricter != nullptr, "restricter is null");
            ensure(snapper != nullptr, "snapper is null");
        }

        bool RestrictedDragPolicy::DragInfo::skip() const {
            return restricter == nullptr;
        }

        RestrictedDragPolicy::RestrictedDragPolicy() :
        m_restricter(nullptr),
        m_snapper(nullptr) {}


        RestrictedDragPolicy::~RestrictedDragPolicy() {
            deleteRestricter();
            deleteSnapper();
        }

        bool RestrictedDragPolicy::dragging() const {
            return m_restricter != nullptr;
        }

        void RestrictedDragPolicy::deleteRestricter() {
            delete m_restricter;
            m_restricter = nullptr;
        }

        void RestrictedDragPolicy::deleteSnapper() {
            delete m_snapper;
            m_snapper = nullptr;
        }

        const vm::vec3& RestrictedDragPolicy::initialHandlePosition() const {
            assert(dragging());
            return m_initialHandlePosition;
        }

        const vm::vec3& RestrictedDragPolicy::currentHandlePosition() const {
            assert(dragging());
            return m_currentHandlePosition;
        }

        const vm::vec3& RestrictedDragPolicy::initialMousePosition() const {
            assert(dragging());
            return m_initialMousePosition;

        }

        const vm::vec3& RestrictedDragPolicy::currentMousePosition() const {
            assert(dragging());
            return m_currentMousePosition;
        }

        bool RestrictedDragPolicy::hitPoint(const InputState& inputState, vm::vec3& result) const {
            assert(dragging());
            return m_restricter->hitPoint(inputState, result);
        }

        bool RestrictedDragPolicy::doStartMouseDrag(const InputState& inputState) {
            const DragInfo info = doStartDrag(inputState);
            if (info.skip())
                return false;

            m_restricter = info.restricter;
            m_snapper = info.snapper;

            if (!hitPoint(inputState, m_initialMousePosition)) {
                doCancelMouseDrag();
                return false;
            }

            m_currentMousePosition = m_initialHandlePosition;
            if (info.computeInitialHandlePosition)
                m_initialHandlePosition = m_currentHandlePosition = m_initialMousePosition;
            else
                m_initialHandlePosition = m_currentHandlePosition = info.initialHandlePosition;

            return true;
        }

        bool RestrictedDragPolicy::doMouseDrag(const InputState& inputState) {
            ensure(m_restricter != nullptr, "restricter is null");

            vm::vec3 newMousePosition;
            if (!hitPoint(inputState, newMousePosition))
                return true;

            m_currentMousePosition = newMousePosition;

            vm::vec3 newHandlePosition = m_currentMousePosition;
            if (!snapPoint(inputState, newHandlePosition) || newHandlePosition == m_currentHandlePosition)
                return true;

            const DragResult result = doDrag(inputState, m_currentHandlePosition, newHandlePosition);
            if (result == DR_Cancel)
                return false;

            if (result == DR_Continue)
                m_currentHandlePosition = newHandlePosition;
            return true;
        }

        void RestrictedDragPolicy::doEndMouseDrag(const InputState& inputState) {
            ensure(m_restricter != nullptr, "restricter is null");
            doEndDrag(inputState);
            deleteRestricter();
            deleteSnapper();
        }

        void RestrictedDragPolicy::doCancelMouseDrag() {
            ensure(m_restricter != nullptr, "restricter is null");
            doCancelDrag();
            deleteRestricter();
            deleteSnapper();
        }

        void RestrictedDragPolicy::setRestricter(const InputState& inputState, DragRestricter* restricter, const bool resetInitialPoint) {
            assert(dragging());
            ensure(restricter != nullptr, "restricter is null");

            deleteRestricter();
            m_restricter = restricter;

            if (resetInitialPoint) {
                this->resetInitialPoint(inputState);
            }

            doMouseDrag(inputState);
        }

        void RestrictedDragPolicy::setSnapper(const InputState& inputState, DragSnapper* snapper, const bool resetCurrentHandlePosition) {
            assert(dragging());
            ensure(snapper != nullptr, "snapper is null");

            deleteSnapper();
            m_snapper = snapper;

            if (resetCurrentHandlePosition) {
                vm::vec3 newHandlePosition = m_currentMousePosition;
                assertResult(snapPoint(inputState, newHandlePosition))
                m_currentHandlePosition = newHandlePosition;
            }

            doMouseDrag(inputState);
        }

        bool RestrictedDragPolicy::snapPoint(const InputState& inputState, vm::vec3& point) const {
            assert(dragging());
            return m_snapper->snap(inputState, m_initialHandlePosition, m_currentHandlePosition, point);
        }

        void RestrictedDragPolicy::resetInitialPoint(const InputState& inputState) {
            assertResult(hitPoint(inputState, m_initialMousePosition))
            m_currentMousePosition = m_initialHandlePosition = m_initialMousePosition;

            assertResult(snapPoint(inputState, m_initialHandlePosition))
            m_currentHandlePosition = m_initialHandlePosition;
        }

        RenderPolicy::~RenderPolicy() = default;
        void RenderPolicy::doSetRenderOptions(const InputState&, Renderer::RenderContext&) const {}
        void RenderPolicy::doRender(const InputState&, Renderer::RenderContext&, Renderer::RenderBatch&) {}

        DropPolicy::~DropPolicy() = default;

        NoDropPolicy::~NoDropPolicy() = default;

        bool NoDropPolicy::doDragEnter(const InputState&, const std::string& /* payload */) { return false; }
        bool NoDropPolicy::doDragMove(const InputState&) { return false; }
        void NoDropPolicy::doDragLeave(const InputState&) {}
        bool NoDropPolicy::doDragDrop(const InputState&) { return false; }

        ToolController::~ToolController() = default;
        Tool* ToolController::tool() { return doGetTool(); }
        const Tool* ToolController::tool() const { return doGetTool(); }
        bool ToolController::toolActive() const { return tool()->active(); }
        void ToolController::refreshViews() { tool()->refreshViews(); }

        ToolControllerGroup::ToolControllerGroup() :
        m_dragReceiver(nullptr),
        m_dropReceiver(nullptr) {}

        ToolControllerGroup::~ToolControllerGroup() = default;

        void ToolControllerGroup::addController(std::unique_ptr<ToolController> controller) {
            ensure(controller != nullptr, "controller is null");
            m_chain.append(std::move(controller));
        }

        void ToolControllerGroup::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_chain.pick(inputState, pickResult);
        }

        void ToolControllerGroup::doModifierKeyChange(const InputState& inputState) {
            m_chain.modifierKeyChange(inputState);
        }

        void ToolControllerGroup::doMouseDown(const InputState& inputState) {
            m_chain.mouseDown(inputState);
        }

        void ToolControllerGroup::doMouseUp(const InputState& inputState) {
            m_chain.mouseUp(inputState);
        }

        bool ToolControllerGroup::doMouseClick(const InputState& inputState) {
            return m_chain.mouseClick(inputState);
        }

        bool ToolControllerGroup::doMouseDoubleClick(const InputState& inputState) {
            return m_chain.mouseDoubleClick(inputState);
        }

        void ToolControllerGroup::doMouseMove(const InputState& inputState) {
            m_chain.mouseMove(inputState);
        }

        void ToolControllerGroup::doMouseScroll(const InputState& inputState) {
            m_chain.mouseScroll(inputState);
        }

        bool ToolControllerGroup::doStartMouseDrag(const InputState& inputState) {
            assert(m_dragReceiver == nullptr);
            if (!doShouldHandleMouseDrag(inputState))
                return false;
            m_dragReceiver = m_chain.startMouseDrag(inputState);
            if (m_dragReceiver != nullptr)
                doMouseDragStarted(inputState);
            return m_dragReceiver != nullptr;
        }

        bool ToolControllerGroup::doMouseDrag(const InputState& inputState) {
            ensure(m_dragReceiver != nullptr, "dragReceiver is null");
            if (m_dragReceiver->mouseDrag(inputState)) {
                doMouseDragged(inputState);
                return true;
            }
            return false;
        }

        void ToolControllerGroup::doEndMouseDrag(const InputState& inputState) {
            ensure(m_dragReceiver != nullptr, "dragReceiver is null");
            m_dragReceiver->endMouseDrag(inputState);
            m_dragReceiver = nullptr;
            doMouseDragEnded(inputState);
        }

        void ToolControllerGroup::doCancelMouseDrag() {
            ensure(m_dragReceiver != nullptr, "dragReceiver is null");
            m_dragReceiver->cancelMouseDrag();
            m_dragReceiver = nullptr;
            doMouseDragCancelled();
        }

        void ToolControllerGroup::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            m_chain.setRenderOptions(inputState, renderContext);
        }

        void ToolControllerGroup::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_chain.render(inputState, renderContext, renderBatch);
        }

        bool ToolControllerGroup::doDragEnter(const InputState& inputState, const std::string& payload) {
            assert(m_dropReceiver == nullptr);
            if (!doShouldHandleDrop(inputState, payload))
                return false;
            m_dropReceiver = m_chain.dragEnter(inputState, payload);
            return m_dropReceiver != nullptr;
        }

        bool ToolControllerGroup::doDragMove(const InputState& inputState) {
            ensure(m_dropReceiver != nullptr, "dropReceiver is null");
            return m_dropReceiver->dragMove(inputState);
        }

        void ToolControllerGroup::doDragLeave(const InputState& inputState) {
            ensure(m_dropReceiver != nullptr, "dropReceiver is null");
            m_dropReceiver->dragLeave(inputState);
            m_dropReceiver = nullptr;
        }

        bool ToolControllerGroup::doDragDrop(const InputState& inputState) {
            ensure(m_dropReceiver != nullptr, "dropReceiver is null");
            const bool result = m_dropReceiver->dragDrop(inputState);
            m_dropReceiver = nullptr;
            return result;
        }

        bool ToolControllerGroup::doCancel() {
            return m_chain.cancel();
        }

        bool ToolControllerGroup::doShouldHandleMouseDrag(const InputState&) const {
            return true;
        }

        void ToolControllerGroup::doMouseDragStarted(const InputState&) {}
        void ToolControllerGroup::doMouseDragged(const InputState&) {}
        void ToolControllerGroup::doMouseDragEnded(const InputState&) {}
        void ToolControllerGroup::doMouseDragCancelled() {}

        bool ToolControllerGroup::doShouldHandleDrop(const InputState&, const std::string& /* payload */) const {
            return true;
        }
    }
}
