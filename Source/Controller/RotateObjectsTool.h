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

#ifndef __TrenchBroom__RotateObjectsTool__
#define __TrenchBroom__RotateObjectsTool__

#include "Controller/DragTool.h"
#include "Controller/RotateObjectsHandle.h"

namespace TrenchBroom {
    namespace Renderer {
        class RotateObjectsHandleFigure;
    }
    
    namespace Controller {
        class RotateObjectsTool : public DragTool {
        protected:
            RotateObjectsHandle m_handle = RotateObjectsHandle(34.0f, 2.0f);
            Renderer::RotateObjectsHandleFigure* m_handleFigure;
            
            void updateHits(InputEvent& event);
            bool handleMouseMoved(InputEvent& event);
            
            bool handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint);
            bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            void handleEndPlaneDrag(InputEvent& event);
            
            void handleChangeEditState(const Model::EditStateChangeSet& changeSet);
        public:
            RotateObjectsTool(View::DocumentViewHolder& documentViewHolder);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsTool__) */
