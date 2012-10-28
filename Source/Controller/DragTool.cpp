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
        bool DragTool::handleBeginDrag(InputEvent& event) {
            if (handleBeginPlaneDrag(event, m_dragPlane, m_lastDragPoint)) {
                m_lastReferencePoint = m_lastDragPoint;
                return true;
            }
            return false;
        }
        
        bool DragTool::handleDrag(InputEvent& event) {
            float dist = m_dragPlane.intersectWithRay(event.ray);
            if (Math::isnan(dist))
                return true;
            
            Vec3f currentDragPoint = event.ray.pointAtDistance(dist);
            if (currentDragPoint.equals(m_lastDragPoint))
                return true;
            
            if (!handlePlaneDrag(event, m_lastDragPoint, currentDragPoint, m_lastReferencePoint))
                return false;
            
            m_lastDragPoint = currentDragPoint;
            return true;
        }
        
        void DragTool::handleEndDrag(InputEvent& event) {
            handleEndPlaneDrag(event);
        }

        DragTool::DragTool(View::DocumentViewHolder& documentViewHolder) :
        Tool(documentViewHolder) {}
    }
}