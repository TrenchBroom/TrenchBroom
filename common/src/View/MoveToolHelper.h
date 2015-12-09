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

#ifndef TrenchBroom_MoveToolHelper
#define TrenchBroom_MoveToolHelper

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/EdgeRenderer.h"
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
            MoveToolDelegate* m_delegate;
            Vec3f::List m_trace;
        protected:
            MoveToolHelper(PlaneDragPolicy* policy, MoveToolDelegate* delegate);
        public:
            virtual ~MoveToolHelper();
            
            bool handleMove(const InputState& inputState) const;
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            Plane3 dragPlane(const InputState& inputState, const Vec3& initialPoint) const;
            void addTracePoint(const Vec3& point);
            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            virtual Plane3 doGetDragPlane(const InputState& inputState, const Vec3& initialPoint) const = 0;
            virtual Vec3 doGetDelta(const Vec3& delta) const = 0;
        };
        
        class MoveToolHelper2D : public MoveToolHelper {
        public:
            MoveToolHelper2D(PlaneDragPolicy* policy, MoveToolDelegate* delegate);
        private:
            Plane3 doGetDragPlane(const InputState& inputState, const Vec3& initialPoint) const;
            Vec3 doGetDelta(const Vec3& delta) const;
        };

        class MoveToolHelper3D : public MoveToolHelper {
        private:
            MovementRestriction& m_movementRestriction;
        public:
            MoveToolHelper3D(PlaneDragPolicy* policy, MoveToolDelegate* delegate, MovementRestriction& movementRestriction);
        private:
            Plane3 doGetDragPlane(const InputState& inputState, const Vec3& initialPoint) const;
            Vec3 doGetDelta(const Vec3& delta) const;
        };
    }
}

#endif /* defined(TrenchBroom_MoveToolHelper) */
