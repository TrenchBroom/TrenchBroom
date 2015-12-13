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

#include "ToolAdapter.h"

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

        PlaneDragPolicy::PlaneDragPolicy() : m_dragging(false) {}
        PlaneDragPolicy::~PlaneDragPolicy() {}
        
        bool PlaneDragPolicy::doStartMouseDrag(const InputState& inputState) {
            if (doStartPlaneDrag(inputState, m_plane, m_lastPoint)) {
                m_dragging = true;
                m_refPoint = m_lastPoint;
                return true;
            }
            return false;
        }
        
        bool PlaneDragPolicy::doMouseDrag(const InputState& inputState) {
            const FloatType distance = m_plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return true;
            
            const Vec3 curPoint = inputState.pickRay().pointAtDistance(distance);
            if (curPoint.equals(m_lastPoint))
                return true;
            
            const bool result = doPlaneDrag(inputState, m_lastPoint, curPoint, m_refPoint);
            m_lastPoint = curPoint;
            return result;
        }
        
        void PlaneDragPolicy::doEndMouseDrag(const InputState& inputState) {
            doEndPlaneDrag(inputState);
            m_dragging = false;
        }
        
        void PlaneDragPolicy::doCancelMouseDrag() {
            doCancelPlaneDrag();
            m_dragging = false;
        }
        
        bool PlaneDragPolicy::dragging() const {
            return m_dragging;
        }
        
        void PlaneDragPolicy::resetPlane(const InputState& inputState) {
            doResetPlane(inputState, m_plane, m_lastPoint);
        }
        
        PlaneDragHelper::PlaneDragHelper(PlaneDragPolicy* policy) : m_policy(policy) { assert(m_policy != NULL); }
        PlaneDragHelper::~PlaneDragHelper() {}
        
        bool PlaneDragHelper::startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            return doStartPlaneDrag(inputState, plane, initialPoint);
        }
        
        bool PlaneDragHelper::planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            return doPlaneDrag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void PlaneDragHelper::endPlaneDrag(const InputState& inputState) {
            doEndPlaneDrag(inputState);
        }
        
        void PlaneDragHelper::cancelPlaneDrag() {
            doCancelPlaneDrag();
        }
        
        void PlaneDragHelper::resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            doResetPlane(inputState, plane, initialPoint);
        }
        
        void PlaneDragHelper::render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            doRender(inputState, renderContext, renderBatch);
        }

        bool PlaneDragHelper::dragging() const {
            return m_policy->dragging();
        }
        
        void PlaneDragHelper::resetPlane(const InputState& inputState) {
            m_policy->resetPlane(inputState);
        }

        LineDragPolicy::LineDragPolicy() : m_dragging(false) {}
        LineDragPolicy::~LineDragPolicy() {}

        bool LineDragPolicy::doStartMouseDrag(const InputState& inputState) {
            if (doStartLineDrag(inputState, m_line, m_lastDist)) {
                m_dragging = true;
                m_refDist = m_lastDist;
                return true;
            }
            return false;
        }

        bool LineDragPolicy::doMouseDrag(const InputState& inputState) {
            const Ray3::LineDistance lineDist = inputState.pickRay().distanceToLine(m_line.point, m_line.direction);
            if (lineDist.parallel)
                return true;
            
            const FloatType curDist = lineDist.lineDistance;
            if (curDist == m_lastDist)
                return true;
            
            const bool result = doLineDrag(inputState, m_lastDist, curDist, m_refDist);
            m_lastDist = curDist;
            return result;
        }
        
        void LineDragPolicy::doEndMouseDrag(const InputState& inputState) {
            doEndLineDrag(inputState);
            m_dragging = false;
        }
        
        void LineDragPolicy::doCancelMouseDrag() {
            doCancelLineDrag();
            m_dragging = false;
        }
        
        bool LineDragPolicy::dragging() const {
            return m_dragging;
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

        ToolAdapter::~ToolAdapter() {}
        Tool* ToolAdapter::tool() { return doGetTool(); }
        bool ToolAdapter::toolActive() { return tool()->active(); }
        void ToolAdapter::refreshViews() { tool()->refreshViews(); }
    }
}
