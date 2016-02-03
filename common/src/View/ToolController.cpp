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

#include "ToolController.h"

#include "View/InputState.h"
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
        
        DelegatingMouseDragPolicy::DelegatingMouseDragPolicy() :
        m_delegate(NULL) {}
        
        DelegatingMouseDragPolicy::~DelegatingMouseDragPolicy() {}

        bool DelegatingMouseDragPolicy::doStartMouseDrag(const InputState& inputState) {
            assert(m_delegate == NULL);
            m_delegate = doCreateDelegate(inputState);
            if (m_delegate == NULL)
                return false;
            
            if (m_delegate->doStartMouseDrag(inputState)) {
                doMouseDragStarted();
                return true;
            }
            return false;
        }
        
        bool DelegatingMouseDragPolicy::doMouseDrag(const InputState& inputState) {
            assert(m_delegate != NULL);
            if (m_delegate->doMouseDrag(inputState)) {
                doMouseDragged();
                return true;
            }
            return false;
        }
        
        void DelegatingMouseDragPolicy::doEndMouseDrag(const InputState& inputState) {
            assert(m_delegate != NULL);
            m_delegate->doEndMouseDrag(inputState);
            doMouseDragEnded();
            doDeleteDelegate(m_delegate);
            m_delegate = NULL;
        }
        
        void DelegatingMouseDragPolicy::doCancelMouseDrag() {
            assert(m_delegate != NULL);
            m_delegate->doCancelMouseDrag();
            doMouseDragCancelled();
            doDeleteDelegate(m_delegate);
            m_delegate = NULL;
        }

        void DelegatingMouseDragPolicy::doMouseDragStarted() {}
        void DelegatingMouseDragPolicy::doMouseDragged() {}
        void DelegatingMouseDragPolicy::doMouseDragEnded() {}
        void DelegatingMouseDragPolicy::doMouseDragCancelled() {}

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
            point = inputState.pickRay().pointAtDistance(lineDist.rayDistance);
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
            point = m_radius * direction;
            return true;
        }

        /*
         bool m_pickableSet;
         bool m_selectedSet;
         bool m_hitTypeSet;
         bool m_occludedTypeSet;
         bool m_minDistanceSet;
         
         bool m_pickableValue;
         bool m_selectedValue;
         const Model::Hit::HitType m_hitTypeValue;
         const Model::Hit::HitType m_occludedTypeValue;
         FloatType m_minDistanceValue;
         */

        SurfaceDragRestricter::SurfaceDragRestricter() :
        m_hitTypeSet(false),
        m_occludedTypeSet(false),
        m_minDistanceSet(false),
        m_pickable(false),
        m_selected(false) {}

        void SurfaceDragRestricter::setPickable(const bool pickable) {
            m_pickable = pickable;
        }
        
        void SurfaceDragRestricter::setSelected(const bool selected) {
            m_selected = selected;
        }
        
        void SurfaceDragRestricter::setType(const Model::Hit::HitType type) {
            m_hitTypeSet = true;
            m_hitTypeValue = type;
        }
        
        void SurfaceDragRestricter::setOccluded(const Model::Hit::HitType type) {
            m_occludedTypeSet = true;
            m_occludedTypeValue = type;
        }
        
        void SurfaceDragRestricter::setMinDistance(const FloatType minDistance) {
            m_minDistanceSet = true;
            m_minDistanceValue = minDistance;
        }

        bool SurfaceDragRestricter::doComputeHitPoint(const InputState& inputState, Vec3& point) const {
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
            
            const Model::Hit& hit = query.first();
            if (!hit.isMatch())
                return false;
            point = hit.hitPoint();
            return true;
        }

        RestrictedDragPolicy::RestrictedDragPolicy() :
        m_restricter(NULL),
        m_restrictOnNextEvent(false) {}
        
        RestrictedDragPolicy::~RestrictedDragPolicy() {
            deleteRestricter();
        }

        void RestrictedDragPolicy::deleteRestricter() {
            delete m_restricter;
            m_restricter = NULL;
        }

        bool RestrictedDragPolicy::dragging() const {
            return m_restricter != NULL;
        }

        bool RestrictedDragPolicy::doStartMouseDrag(const InputState& inputState) {
            if (!doShouldStartDrag(inputState, m_initialPoint))
                return false;
            
            if (!updateRestricter(inputState, m_initialPoint, m_initialPoint))
                return false;

            m_lastPoint = m_initialPoint;
            doDragStarted(inputState, m_lastPoint);
            return true;
        }
        
        bool RestrictedDragPolicy::doMouseDrag(const InputState& inputState) {
            assert(m_restricter != NULL);
            
            if (m_restrictOnNextEvent)
                resetRestricter(inputState);
            
            Vec3 curPoint;
            if (!m_restricter->hitPoint(inputState, curPoint) || !doSnapPoint(inputState, m_lastPoint, curPoint) || curPoint.equals(m_lastPoint))
                return true;
            
            const bool result = doDragged(inputState, m_lastPoint, curPoint);
            m_lastPoint = curPoint;
            return result;
        }
        
        void RestrictedDragPolicy::doEndMouseDrag(const InputState& inputState) {
            assert(m_restricter != NULL);
            doDragEnded(inputState);
            deleteRestricter();
            m_restrictOnNextEvent = false;
        }
        
        void RestrictedDragPolicy::doCancelMouseDrag() {
            assert(m_restricter != NULL);
            doDragCancelled();
            deleteRestricter();
            m_restrictOnNextEvent = false;
        }
        
        void RestrictedDragPolicy::resetRestricter(const InputState& inputState) {
            assert(m_restricter != NULL);
            Vec3 curPoint;
            assertResult(m_restricter->hitPoint(inputState, curPoint));

            if (m_initialPoint == curPoint) {
                m_restrictOnNextEvent = true;
            } else if (updateRestricter(inputState, m_initialPoint, curPoint)) {
                m_restrictOnNextEvent = false;
                doMouseDrag(inputState);
            }
        }

        bool RestrictedDragPolicy::snapPoint(const InputState& inputState, const Vec3& lastPoint, Vec3& point) const {
            return doSnapPoint(inputState, lastPoint, point);
        }

        bool RestrictedDragPolicy::updateRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) {
            bool resetInitialPoint = false;
            DragRestricter* restricter = doCreateDragRestricter(inputState, m_initialPoint, m_initialPoint, resetInitialPoint);
            if (restricter == NULL)
                return false;
            
            if (resetInitialPoint) {
                if (!restricter->hitPoint(inputState, m_initialPoint))
                    return false;
            }

            deleteRestricter();
            m_restricter = restricter;
            
            return true;
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

        void ToolControllerGroup::addAdapter(ToolController* adapter) {
            assert(adapter != NULL);
            m_chain.append(adapter);
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
            m_dragReceiver = m_chain.startMouseDrag(inputState);
            return m_dragReceiver != NULL;
        }
        
        bool ToolControllerGroup::doMouseDrag(const InputState& inputState) {
            assert(m_dragReceiver != NULL);
            return m_dragReceiver->mouseDrag(inputState);
        }
        
        void ToolControllerGroup::doEndMouseDrag(const InputState& inputState) {
            assert(m_dragReceiver != NULL);
            m_dragReceiver->endMouseDrag(inputState);
            m_dragReceiver = NULL;
        }
        
        void ToolControllerGroup::doCancelMouseDrag() {
            assert(m_dragReceiver != NULL);
            m_dragReceiver->cancelMouseDrag();
            m_dragReceiver = NULL;
        }
        
        void ToolControllerGroup::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            m_chain.setRenderOptions(inputState, renderContext);
        }
        
        void ToolControllerGroup::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_chain.render(inputState, renderContext, renderBatch);
        }
        
        bool ToolControllerGroup::doDragEnter(const InputState& inputState, const String& payload) {
            assert(m_dropReceiver == NULL);
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
    }
}
