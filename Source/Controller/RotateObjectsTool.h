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

#include "Controller/Tool.h"
#include "Controller/RotateHandle.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class RenderContext;
    }

    namespace Controller {
        class RotateObjectsTool : public Tool {
        protected:
            Vec3f m_axis;
            int m_startX;
            int m_startY;
            bool m_invert;
            float m_angle;
            Vec3f m_center;
            bool m_ignoreObjectsChange;
            
            RotateHandle m_rotateHandle;
            
            void updateHandlePosition(InputState& inputState);

            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            
            bool handleStartDrag(InputState& inputState);
            bool handleDrag(InputState& inputState);
            void handleEndDrag(InputState& inputState);
            
            void handleObjectsChange(InputState& inputState);
            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
            void handleGridChange(InputState& inputState);
        public:
            RotateObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, float axisLength, float ringRadius, float ringThickness);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsTool__) */
