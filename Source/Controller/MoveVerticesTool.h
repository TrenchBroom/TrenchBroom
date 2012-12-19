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

#ifndef __TrenchBroom__MoveVerticesTool__
#define __TrenchBroom__MoveVerticesTool__

#include "Controller/Tool.h"
#include "Controller/HandleManager.h"
#include "Controller/MoveHandle.h"
#include "Renderer/RenderTypes.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class MoveVerticesTool : public PlaneDragTool {
        protected:
            typedef enum {
                VMMove,
                VMSplit
            } VertexToolMode;
            
            HandleManager m_handleManager;
            MoveHandle m_moveHandle;
            MoveHandle::RestrictToAxis m_restrictToAxis;
            VertexToolMode m_mode;
            
            void updateMoveHandle(InputState& inputState);
            
            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            
            bool handleMouseUp(InputState& inputState);
            bool handleMouseDClick(InputState& inputState);
            void handleMouseMove(InputState& inputState);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            bool handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) ;
            void handleEndPlaneDrag(InputState& inputState);

            void handleObjectsChange(InputState& inputState);
            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
            void handleCameraChange(InputState& inputState);
        public:
            MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVerticesTool__) */
