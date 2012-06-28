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

#ifndef TrenchBroom_DragTool_h
#define TrenchBroom_DragTool_h

#include "Controller/Tool.h"
#include "Controller/DragPlane.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Controller {
        class DragPlane;
        class Editor;
        
        class DragTool : public Tool {
        protected:
            DragPlane m_dragPlane;
            Vec3f m_dragPlanePosition;
            Vec3f m_lastMousePoint;
            Vec3f m_lastRefPoint;
            bool m_drag;
            
            virtual void updateDragPlane(ToolEvent& event);

            virtual bool doBeginLeftDrag(ToolEvent& event, Vec3f& initialPoint);
            virtual bool doLeftDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            virtual void doEndLeftDrag(ToolEvent& event);
            
            virtual bool doBeginRightDrag(ToolEvent& event, Vec3f& initialPoint) ;
            virtual bool doRightDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            virtual void doEndRightDrag(ToolEvent& event);
        public:
            static bool altPlaneModifierPressed(ToolEvent& event);

            DragTool(Editor& editor);
            virtual ~DragTool() {}
            
            bool beginLeftDrag(ToolEvent& event);
            void leftDrag(ToolEvent& event);
            void endLeftDrag(ToolEvent& event);
        };
    }
}

#endif
