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

#ifndef TrenchBroom_ResizeBrushTool_h
#define TrenchBroom_ResizeBrushTool_h

#include "Controller/DragTool.h"

namespace TrenchBroom {
    namespace Model {
        class Face;
    }
    
    namespace Renderer {
        class BoundsGuideFigure;
    }
    
    namespace Controller {
        class Editor;
        
        class ResizeBrushTool : public DragTool {
        protected:
            Model::Face* m_referenceFace;
            Renderer::BoundsGuideFigure* m_guideFigure;
            
            virtual void updateDragPlane(ToolEvent& event);

            virtual bool leftMouseDown(ToolEvent& event);
            virtual bool doBeginLeftDrag(ToolEvent& event, Vec3f& initialPoint);
            virtual bool doLeftDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            virtual void doEndLeftDrag(ToolEvent& event);
        public:
            ResizeBrushTool(Editor& editor);
            ~ResizeBrushTool();

            static bool resizeBrushModiferPressed(ToolEvent& event);
        };
    }
}

#endif
