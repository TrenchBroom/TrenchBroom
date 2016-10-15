/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Model/Brush.h"
#include "View/Grid.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace View {
        PickingPolicy::~PickingPolicy() {}

        NoPickingPolicy::~NoPickingPolicy() {}
        void NoPickingPolicy::doPick(const InputState& inputState, Model::PickResult& pickResult) {}

        KeyPolicy::~KeyPolicy() {}

        NoKeyPolicy::~NoKeyPolicy() {}
        void NoKeyPolicy::doModifierKeyChange(const InputState& inputState) {}
        
        MousePolicy::~MousePolicy() {}

        void MousePolicy::doMouseDown(const InputState& inputState) {}
        void MousePolicy::doMouseUp(const InputState& inputState) {}
        bool MousePolicy::doMouseClick(const InputState& inputState) { return false; }
        bool MousePolicy::doMouseDoubleClick(const InputState& inputState) { return false; }
        void MousePolicy::doMouseMove(const InputState& inputState) {}
        void MousePolicy::doMouseScroll(const InputState& inputState) {}

        MouseDragPolicy::~MouseDragPolicy() {}

        NoMouseDragPolicy::~NoMouseDragPolicy() {}

        bool NoMouseDragPolicy::doStartMouseDrag(const InputState& inputState) { return false; }
        bool NoMouseDragPolicy::doMouseDrag(const InputState& inputState) { return false; }
        void NoMouseDragPolicy::doEndMouseDrag(const InputState& inputState) {}
        void NoMouseDragPolicy::doCancelMouseDrag() {}
        
        DragRestricter::~DragRestricter() {}

        bool DragRestricter::hitPoint(const InputState& inputState, Vec3& point) const {
            return doComputeHitPoint(inputState, point);
        }

        PlaneDragRestricter::PlaneDragRestricter(const Plane3& plane) :
        m_plane(plane) {}
        
        bool PlaneDragRestricter::doComputeHitPoint(const InputState& inputState, Vec3& point) const {
            const FloatType distance = m_plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return false;
            point = inputState.pickRay().pointAtDistance(distance);
            return true;
        }

        LineDragRestricter::LineDragRestricter(const Line3& line) :
        m_line(line) {}

        bool LineDragRestricter::doComputeHitPoint(const InputState& inputState, Vec3& point) const {
            const Ray3::LineDistance lineDist = inputState.pickRay().distanceToLine(m_line.point, m_line.direction);
            if (lineDist.parallel)
                return false;
            point = m_line.point + m_line.direction * lineDist.lineDistance;
            return true;
        }
        
        CircleDragRestricter::CircleDragRestricter(const Vec3& center, const Vec3& normal, const FloatType radius) :
        m_center(center),
        m_normal(normal),
        m_radius(radius) {
            assert(m_radius > 0.0);
        }
        
        bool CircleDragRestricter::doComputeHitPoint(const InputState& inputState, Vec3& point) const {
            const Plane3 plane(m_center, m_normal);
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return false;
            
            const Vec3 hitPoint = inputState.pickRay().pointAtDistance(distance);
            const Vec3 direction = (hitPoint - m_center).normalized();
            point = m_center + m_radius * direction;
            return true;
        }

        SurfaceDragHelper::SurfaceDragHelper() :
        m_hitTypeSet(false),
        m_occludedTypeSet(false),
        m_minDistanceSet(false),
        m_pickable(false),
        m_selected(false) {}
        
        SurfaceDragHelper::~SurfaceDragHelper() {}

        void SurfaceDragHelper::setPickable(const bool pickable) {
            m_pickable = pickable;
        }
        
        void SurfaceDragHelper::setSelected(const bool selected) {
            m_selected = selected;
        }
        
        void SurfaceDragHelper::setType(const Model::Hit::HitType type) {
            m_hitTypeSet = true;
            m_hitTypeValue = type;
        }
        
        void SurfaceDragHelper::setOccluded(const Model::Hit::HitType type) {
            m_occludedTypeSet = true;
            m_occludedTypeValue = type;
        }
        
        void SurfaceDragHelper::setMinDistance(const FloatType minDistance) {
            m_minDistanceSet = true;
            m_minDistanceValue = minDistance;
        }

        Model::HitQuery SurfaceDragHelper::query(const InputState& inputState) const {
            Model::HitQuery query = inputState.pickResult().query();
            if (m_pickable)
                query.pickable();
            if (m_hitTypeSet)
                query.type(m_hitTypeValue);
            if (m_occludedTypeSet)
                query.occluded(m_occludedTypeValue);
            if (m_selected)
                query.selected();
            if (m_minDistanceSet)
                query.minDistance(m_minDistanceValue);
            return query;
        }

        bool SurfaceDragRestricter::doComputeHitPoint(const InputState& inputState, Vec3& point) const {
            const Model::Hit& hit = query(inputState).first();
            if (!hit.isMatch())
                return false;
            point = hit.hitPoint();
            return true;
        }

        DragSnapper::~DragSnapper() {}
        
        bool DragSnapper::snap(const InputState& inputState, const Vec3& lastPoint, Vec3& curPoint) const {
            return doSnap(inputState, lastPoint, curPoint);
        }
        
        bool NoDragSnapper::doSnap(const InputState& inputState, const Vec3& lastPoint, Vec3& curPoint) const {
            return true;
        }

        AbsoluteDragSnapper::AbsoluteDragSnapper(const Grid& grid) :
        m_grid(grid) {}

        bool AbsoluteDragSnapper::doSnap(const InputState& inputState, const Vec3& lastPoint, Vec3& curPoint) const {
            curPoint = m_grid.snap(curPoint);
            return true;
        }

        DeltaDragSnapper::DeltaDragSnapper(const Grid& grid) :
        m_grid(grid) {}

        bool DeltaDragSnapper::doSnap(const InputState& inputState, const Vec3& lastPoint, Vec3& curPoint) const {
            curPoint = lastPoint + m_grid.snap(curPoint - lastPoint);
            return true;
        }

        CircleDragSnapper::CircleDragSnapper(const Grid& grid, const Vec3& start, const Vec3& center, const Vec3& normal, const FloatType radius) :
        m_grid(grid),
        m_start(start),
        m_center(center),
        m_normal(normal),
        m_radius(radius) {
            assert(m_start != m_center);
            assert(m_normal.isNormalized());
            assert(m_radius > 0.0);
        }

        bool CircleDragSnapper::doSnap(const InputState& inputState, const Vec3& lastPoint, Vec3& curPoint) const {
            if (curPoint == m_center)
                return false;
            
            const Vec3 ref = (m_start - m_center).normalized();
            const Vec3 vec = (curPoint - m_center).normalized();
            const FloatType angle = angleBetween(vec, ref, m_normal);
            const FloatType snapped = m_grid.snapAngle(angle);
            const FloatType canonical = snapped - Math::roundDownToMultiple(snapped, Math::C::twoPi());
            const Quat3 rotation(m_normal, canonical);
            const Vec3 rot = rotation * ref;
            curPoint = m_center + m_radius * rot;
            return true;
        }

        SurfaceDragSnapper::SurfaceDragSnapper(const Grid& grid) :
        m_grid(grid) {}

        bool SurfaceDragSnapper::doSnap(const InputState& inputState, const Vec3& lastPoint, Vec3& curPoint) const {
            const Model::Hit& hit = query(inputState).first();
            if (!hit.isMatch())
                return false;

            const Plane3& plane = doGetPlane(inputState, hit);
            curPoint = m_grid.snap(hit.hitPoint(), plane);
            return true;
        }

        RestrictedDragPolicy::DragInfo::DragInfo() :
        restricter(NULL) {}
        
        RestrictedDragPolicy::DragInfo::DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper) :
        restricter(i_restricter),
        snapper(i_snapper),
        setInitialPoint(false) {
            assert(restricter != NULL);
            assert(snapper != NULL);
        }

        RestrictedDragPolicy::DragInfo::DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper, const Vec3& i_initialPoint) :
        restricter(i_restricter),
        snapper(i_snapper),
        setInitialPoint(true),
        initialPoint(i_initialPoint) {
            assert(restricter != NULL);
            assert(snapper != NULL);
        }

        bool RestrictedDragPolicy::DragInfo::skip() const {
            return restricter == NULL;
        }

        RestrictedDragPolicy::RestrictedDragPolicy() :
        m_restricter(NULL),
        m_snapper(NULL) {}
        

        RestrictedDragPolicy::~RestrictedDragPolicy() {
            deleteRestricter();
            deleteSnapper();
        }

        bool RestrictedDragPolicy::dragging() const {
            return m_restricter != NULL;
        }

        void RestrictedDragPolicy::deleteRestricter() {
            delete m_restricter;
            m_restricter = NULL;
        }

        void RestrictedDragPolicy::deleteSnapper() {
            delete m_snapper;
            m_snapper = NULL;
        }

        const Vec3& RestrictedDragPolicy::dragOrigin() const {
            assert(dragging());
            return m_dragOrigin;
        }

        const Vec3& RestrictedDragPolicy::initialPoint() const {
            assert(dragging());
            return m_initialPoint;
        }

        const Vec3& RestrictedDragPolicy::lastPoint() const {
            assert(dragging());
            return m_lastPoint;
        }

        const Vec3& RestrictedDragPolicy::curPoint() const {
            assert(dragging());
            return m_curPoint;
        }

        bool RestrictedDragPolicy::hitPoint(const InputState& inputState, Vec3& result) const {
            assert(dragging());
            return m_restricter->hitPoint(inputState, result);
        }

        bool RestrictedDragPolicy::doStartMouseDrag(const InputState& inputState) {
            const DragInfo info = doStartDrag(inputState);
            if (info.skip())
                return false;
            
            m_restricter = info.restricter;
            m_snapper = info.snapper;
            
            if (info.setInitialPoint) {
                m_dragOrigin = info.initialPoint;
            } else {
                if (!hitPoint(inputState, m_dragOrigin)) {
                    deleteRestricter();
                    deleteSnapper();
                    return false;
                }
            }
            
            m_initialPoint = m_lastPoint = m_curPoint = m_dragOrigin;
            return true;
        }
        
        bool RestrictedDragPolicy::doMouseDrag(const InputState& inputState) {
            assert(m_restricter != NULL);
            
            if (!hitPoint(inputState, m_curPoint))
                return true;
            
            Vec3 snappedPoint = m_curPoint;
            if (!snapPoint(inputState, m_lastPoint, snappedPoint) ||
                snappedPoint.equals(m_lastPoint))
                return true;
            
            const DragResult result = doDrag(inputState, m_lastPoint, snappedPoint);
            if (result == DR_Cancel)
                return false;
            
            if (result == DR_Continue)
                m_lastPoint = snappedPoint;
            return true;
        }
        
        void RestrictedDragPolicy::doEndMouseDrag(const InputState& inputState) {
            assert(m_restricter != NULL);
            doEndDrag(inputState);
            deleteRestricter();
            deleteSnapper();
        }
        
        void RestrictedDragPolicy::doCancelMouseDrag() {
            assert(m_restricter != NULL);
            doCancelDrag();
            deleteRestricter();
            deleteSnapper();
        }
        
        void RestrictedDragPolicy::setRestricter(const InputState& inputState, DragRestricter* restricter, const bool resetInitialPoint) {
            assert(dragging());
            assert(restricter != NULL);
            
            deleteRestricter();
            m_restricter = restricter;
            if (resetInitialPoint) {
                assertResult(m_restricter->hitPoint(inputState, m_initialPoint));
            } else {
                doMouseDrag(inputState);
            }
        }
        
        void RestrictedDragPolicy::setSnapper(const InputState& inputState, DragSnapper* snapper) {
            assert(dragging());
            assert(snapper != NULL);
            
            deleteSnapper();
            m_snapper = snapper;
        }

        bool RestrictedDragPolicy::snapPoint(const InputState& inputState, const Vec3& lastPoint, Vec3& point) const {
            assert(dragging());
            return m_snapper->snap(inputState, lastPoint, point);
        }

        RenderPolicy::~RenderPolicy() {}
        void RenderPolicy::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
        void RenderPolicy::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        DropPolicy::~DropPolicy() {}
        
        NoDropPolicy::~NoDropPolicy() {}

        bool NoDropPolicy::doDragEnter(const InputState& inputState, const String& payload) { return false; }
        bool NoDropPolicy::doDragMove(const InputState& inputState) { return false; }
        void NoDropPolicy::doDragLeave(const InputState& inputState) {}
        bool NoDropPolicy::doDragDrop(const InputState& inputState) { return false; }

        ToolController::~ToolController() {}
        Tool* ToolController::tool() { return doGetTool(); }
        bool ToolController::toolActive() { return tool()->active(); }
        void ToolController::refreshViews() { tool()->refreshViews(); }

        ToolControllerGroup::ToolControllerGroup() :
        m_dragReceiver(NULL),
        m_dropReceiver(NULL) {}
        
        ToolControllerGroup::~ToolControllerGroup() {}

        void ToolControllerGroup::addController(ToolController* controller) {
            assert(controller != NULL);
            m_chain.append(controller);
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
            assert(m_dragReceiver == NULL);
            if (!doShouldHandleMouseDrag(inputState))
                return false;
            m_dragReceiver = m_chain.startMouseDrag(inputState);
            if (m_dragReceiver != NULL)
                doMouseDragStarted(inputState);
            return m_dragReceiver != NULL;
        }
        
        bool ToolControllerGroup::doMouseDrag(const InputState& inputState) {
            assert(m_dragReceiver != NULL);
            if (m_dragReceiver->mouseDrag(inputState)) {
                doMouseDragged(inputState);
                return true;
            }
            return false;
        }
        
        void ToolControllerGroup::doEndMouseDrag(const InputState& inputState) {
            assert(m_dragReceiver != NULL);
            m_dragReceiver->endMouseDrag(inputState);
            m_dragReceiver = NULL;
            doMouseDragEnded(inputState);
        }
        
        void ToolControllerGroup::doCancelMouseDrag() {
            assert(m_dragReceiver != NULL);
            m_dragReceiver->cancelMouseDrag();
            m_dragReceiver = NULL;
            doMouseDragCancelled();
        }
        
        void ToolControllerGroup::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            m_chain.setRenderOptions(inputState, renderContext);
        }
        
        void ToolControllerGroup::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_chain.render(inputState, renderContext, renderBatch);
        }
        
        bool ToolControllerGroup::doDragEnter(const InputState& inputState, const String& payload) {
            assert(m_dropReceiver == NULL);
            if (!doShouldHandleDrop(inputState, payload))
                return false;
            m_dropReceiver = m_chain.dragEnter(inputState, payload);
            return m_dropReceiver != NULL;
        }
        
        bool ToolControllerGroup::doDragMove(const InputState& inputState) {
            assert(m_dropReceiver != NULL);
            return m_dropReceiver->dragMove(inputState);
        }
        
        void ToolControllerGroup::doDragLeave(const InputState& inputState) {
            assert(m_dropReceiver != NULL);
            m_dropReceiver->dragLeave(inputState);
            m_dropReceiver = NULL;
        }
        
        bool ToolControllerGroup::doDragDrop(const InputState& inputState) {
            assert(m_dropReceiver != NULL);
            const bool result = m_dropReceiver->dragDrop(inputState);
            m_dropReceiver = NULL;
            return result;
        }

        bool ToolControllerGroup::doCancel() {
            return m_chain.cancel();
        }

        bool ToolControllerGroup::doShouldHandleMouseDrag(const InputState& inputState) const {
            return true;
        }
        
        void ToolControllerGroup::doMouseDragStarted(const InputState& inputState) {}
        void ToolControllerGroup::doMouseDragged(const InputState& inputState) {}
        void ToolControllerGroup::doMouseDragEnded(const InputState& inputState) {}
        void ToolControllerGroup::doMouseDragCancelled() {}

        bool ToolControllerGroup::doShouldHandleDrop(const InputState& inputState, const String& payload) const {
            return true;
        }
    }
}
