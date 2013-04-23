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

#ifndef __TrenchBroom__MoveTool__
#define __TrenchBroom__MoveTool__

#include "Controller/Tool.h"

namespace TrenchBroom {
    namespace Renderer {
        class MovementIndicator;
        class Vbo;
        class RenderContext;
    }

    namespace Controller {
        class Command;
        
        class MoveTool : public PlaneDragTool {
        public:
            typedef enum {
                Conclude,
                Deny,
                Continue
            } MoveResult;
        protected:
            typedef enum {
                Horizontal,
                Vertical
            } MoveDirection;

            virtual bool isApplicable(InputState& inputState, Vec3f& hitPoint) = 0;
            virtual wxString actionName(InputState& inputState) = 0;
            virtual void startDrag(InputState& inputState) {}
            virtual void snapDragDelta(InputState& inputState, Vec3f& delta);
            virtual MoveResult performMove(const Vec3f& delta) = 0;
            virtual void endDrag(InputState& inputState) {}
            
            virtual void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            virtual void handleFreeRenderResources();
            
            virtual void handleModifierKeyChange(InputState& inputState);
            
            virtual bool handleStartPlaneDrag(InputState& inputState, Planef& plane, Vec3f& initialPoint);
            virtual void handleResetPlane(InputState& inputState, Planef& plane, Vec3f& initialPoint);
            virtual bool handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint);
            virtual void handleEndPlaneDrag(InputState& inputState);
        private:
            MoveDirection m_direction;
            Renderer::MovementIndicator* m_indicator;
        public:
            MoveTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, bool activatable);
            ~MoveTool() {}
        };
    }
}

#endif /* defined(__TrenchBroom__MoveTool__) */
