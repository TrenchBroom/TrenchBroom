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
        class Hit;
    }
    
    namespace Controller {
        class Editor;
        
        class ResizeBrushTool : public DragTool {
        protected:
            Model::Face* m_referenceFace;
            bool m_figureCreated;
            
            virtual void updateDragPlane(InputEvent& event);
            Model::Hit* selectReferenceHit(InputEvent& event);

            bool handleMouseDown(InputEvent& event);
            bool handleMouseUp(InputEvent& event);

            bool handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint);
            bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            void handleEndPlaneDrag(InputEvent& event);
        public:
            ResizeBrushTool(Editor& editor);
            ~ResizeBrushTool();

            virtual void updateHits(InputEvent& event);
            inline Model::Face* referenceFace() { return m_referenceFace; }
            
            static bool resizeBrushModiferPressed(InputEvent& event);
        };
    }
}

#endif
