/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Tool.h"

#include "View/InputState.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        bool EmptyPolicy::doMouseDown(const InputState& inputState) {
            return false;
        }
        
        bool EmptyPolicy::doMouseUp(const InputState& inputState) {
            return false;
        }

        bool EmptyPolicy::doMouseDoubleClick(const InputState& inputState) {
            return false;
        }

        bool EmptyPolicy::doScroll(const InputState& inputState) {
            return false;
        }

        void EmptyPolicy::doMouseMove(const InputState& inputState) {
        }
        
        bool EmptyPolicy::doStartMouseDrag(const InputState& inputState) {
            return false;
        }

        bool EmptyPolicy::doMouseDrag(const InputState& inputState) {
            return true;
        }

        void EmptyPolicy::doEndMouseDrag(const InputState& inputState) {
        }

        void EmptyPolicy::doCancelMouseDrag(const InputState& inputState) {
        }


        MousePolicy::~MousePolicy() {}

        bool MousePolicy::doMouseDown(const InputState& inputState) {
            return false;
        }
        
        bool MousePolicy::doMouseUp(const InputState& inputState) {
            return false;
        }
        
        bool MousePolicy::doMouseDoubleClick(const InputState& inputState) {
            return false;
        }
        
        void MousePolicy::doScroll(const InputState& inputState) {
        }
        
        void MousePolicy::doMouseMove(const InputState& inputState) {}

        MouseDragPolicy::~MouseDragPolicy() {}
        
        PlaneDragPolicy::~PlaneDragPolicy() {}
        
        bool PlaneDragPolicy::doStartMouseDrag(const InputState& inputState) {
            if (doStartPlaneDrag(inputState, m_plane, m_lastPoint)) {
                m_refPoint = m_lastPoint;
                return true;
            }
            return false;
        }
        
        bool PlaneDragPolicy::doMouseDrag(const InputState& inputState) {
            const FloatType distance = m_plane.intersectWithRay(inputState.pickRay());
            if (Math<FloatType>::isnan(distance))
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
        }
        
        void PlaneDragPolicy::doCancelMouseDrag(const InputState& inputState) {
            doCancelPlaneDrag(inputState);
        }
        
        void PlaneDragPolicy::resetPlane(const InputState& inputState) {
            doResetPlane(inputState, m_plane, m_lastPoint);
            m_refPoint = m_lastPoint;
        }
        
        BaseTool::~BaseTool() {}
    }
}
