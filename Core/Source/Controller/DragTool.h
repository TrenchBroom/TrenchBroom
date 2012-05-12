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
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Controller {
        class DragPlane;
        class Editor;
        
        class DragTool : public Tool {
        private:
            void updateDragPlane(ToolEvent& event);
        protected:
            DragPlane* m_dragPlane;
            Vec3f m_dragPlanePosition;
            Vec3f m_lastPoint;
            bool m_drag;
            
            virtual bool doBeginLeftDrag(ToolEvent& event, Vec3f& lastPoint);
            virtual bool doLeftDrag(ToolEvent& event, Vec3f& delta, Vec3f& lastPoint);
            virtual void doEndLeftDrag(ToolEvent& event);
            
            virtual bool doBeginRightDrag(ToolEvent& event, Vec3f& lastPoint) ;
            virtual bool doRightDrag(ToolEvent& event, Vec3f& delta, Vec3f& lastPoint);
            virtual void doEndRightDrag(ToolEvent& event);
        public:
            static bool altPlaneModifierPressed(ToolEvent& event);

            DragTool(Editor& editor);
            virtual ~DragTool();
            
            bool beginLeftDrag(ToolEvent& event);
            void leftDrag(ToolEvent& event);
            void endLeftDrag(ToolEvent& event);
        };
    }
}

#endif
