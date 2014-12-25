/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MoveToolHelper__
#define __TrenchBroom__MoveToolHelper__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/MoveIndicatorRenderer.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class MovementRestriction;
        
        typedef enum {
            MoveResult_Conclude,
            MoveResult_Deny,
            MoveResult_Continue
        } MoveResult;
        
        class MoveToolDelegate {
        public:
            virtual ~MoveToolDelegate();
            
            bool handleMove(const InputState& inputState) const;
            Vec3 getMoveOrigin(const InputState& inputState) const;
            bool startMove(const InputState& inputState);
            Vec3 snapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult move(const InputState& inputState, const Vec3& delta);
            void endMove(const InputState& inputState);
            void cancelMove();
        private:
            virtual bool doHandleMove(const InputState& inputState) const = 0;
            virtual Vec3 doGetMoveOrigin(const InputState& inputState) const = 0;
            virtual bool doStartMove(const InputState& inputState) = 0;
            virtual Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const = 0;
            virtual MoveResult doMove(const InputState& inputState, const Vec3& delta) = 0;
            virtual void doEndMove(const InputState& inputState) = 0;
            virtual void doCancelMove() = 0;
        };
        
        class MoveToolHelper : public PlaneDragHelper {
        private:
            MovementRestriction& m_movementRestriction;
            MoveToolDelegate& m_delegate;
            Vec3f::List m_trace;
            Renderer::EdgeRenderer m_traceRenderer;
        public:
            MoveToolHelper(MovementRestriction& movementRestriction, MoveToolDelegate& delegate);
            
            bool startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void endPlaneDrag(const InputState& inputState);
            void cancelPlaneDrag();
            void resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            void render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            Plane3 dragPlane(const InputState& inputState, const Vec3& initialPoint) const;
            void addTracePoint(const Vec3& point);
            
            void renderMoveIndicator(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            Renderer::MoveIndicatorRenderer::Direction getDirection() const;
            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveToolHelper__) */
