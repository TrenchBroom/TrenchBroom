/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "DragTool.h"

namespace TrenchBroom {
    namespace Controller {
        void DragTool::updateDragPlane(ToolEvent& event) {
            if (altPlaneModifierPressed(event))
                m_dragPlane = DragPlane::vertical(event.ray.direction * -1.0f);
            else
                m_dragPlane = DragPlane::horizontal();
        }

        bool DragTool::doBeginLeftDrag(ToolEvent& event, Vec3f& initialPoint) { return false; }
        bool DragTool::doLeftDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) { return false; }
        void DragTool::doEndLeftDrag(ToolEvent& event) {}
        
        bool DragTool::doBeginRightDrag(ToolEvent& event, Vec3f& initialPoint) { return false; }
        bool DragTool::doRightDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) { return false; }
        void DragTool::doEndRightDrag(ToolEvent& event) {}

        bool DragTool::altPlaneModifierPressed(ToolEvent& event) {
            return event.modifierKeys == TB_MK_ALT;
        }

        DragTool::DragTool(Editor& editor) : Tool(editor), m_dragPlane(DragPlane::horizontal()), m_dragPlanePosition(Vec3f::Null), m_lastMousePoint(Vec3f::Null), m_lastRefPoint(Vec3f::Null), m_drag(false) {};
        
        bool DragTool::beginLeftDrag(ToolEvent& event) {
            m_drag = doBeginLeftDrag(event, m_lastMousePoint);
            if (m_drag) {
                updateDragPlane(event);
                m_lastRefPoint = m_lastMousePoint;
                m_dragPlanePosition = m_lastMousePoint;
            }
            return m_drag;
        }
        
        void DragTool::leftDrag(ToolEvent& event) {
            if (!m_drag)
                return;
            
            float dist = m_dragPlane.intersect(event.ray, m_dragPlanePosition);
            if (Math::isnan(dist))
                return;
            
            Vec3f currentMousePoint = event.ray.pointAtDistance(dist);
            if (currentMousePoint.equals(m_lastMousePoint))
                return;

            Vec3f delta = currentMousePoint - m_lastRefPoint;
            if (delta.null())
                return;
            
            if (!doLeftDrag(event, m_lastMousePoint, currentMousePoint, m_lastRefPoint))
                endLeftDrag(event);
            
            m_lastMousePoint = currentMousePoint;
        }

        void DragTool::endLeftDrag(ToolEvent& event) {
            if (!m_drag)
                return;
            
            doEndLeftDrag(event);
            m_drag = false;
        }
    }
}