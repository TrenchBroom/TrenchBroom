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

#ifndef __TrenchBroom__DragTool__
#define __TrenchBroom__DragTool__

#include "Controller/Tool.h"

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class DragTool : public Tool {
        private:
            Plane m_dragPlane;
            Vec3f m_lastDragPoint;
            Vec3f m_lastReferencePoint;
        protected:
            virtual bool handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint) = 0;
            virtual bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) = 0;
            virtual void handleEndPlaneDrag(InputEvent& event) = 0;

            bool handleBeginDrag(InputEvent& event);
            bool handleDrag(InputEvent& event);
            void handleEndDrag(InputEvent& event);
        public:
            DragTool(View::DocumentViewHolder& documentViewHolder);
        };
    }
}

#endif /* defined(__TrenchBroom__DragTool__) */
