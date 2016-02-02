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

#ifndef TrenchBroom_MoveToolDelegator
#define TrenchBroom_MoveToolDelegator

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
            void startMove(const InputState& inputState);
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
        
        class MoveToolDelegator : public RestrictedDragPolicy {
        private:
            MoveToolDelegate* m_delegate;
            Vec3 m_initialPoint;
            Vec3 m_lastPoint;
            bool m_moving;
        protected:
            MoveToolDelegator(MoveToolDelegate* delegate);
        public:
            virtual ~MoveToolDelegator();
        private:
            bool doShouldStartDrag(const InputState& inputState, Vec3& initialPoint);
            void doDragStarted(const InputState& inputState, const Vec3& initialPoint);
            bool doDragged(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
            void doDragEnded(const InputState& inputState);
            void doDragCancelled();
            bool doSnapPoint(const InputState& inputState, const Vec3& lastPoint, Vec3& point);
            
            DragRestricter* doCreateDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint, bool& resetInitialPoint);
            bool isVerticalMove(const InputState& inputState) const;
            bool isRestrictedMove(const InputState& inputState) const;
            
            virtual DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const = 0;
            virtual DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const = 0;
            virtual DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const = 0;

        public:
            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
        
        class MoveToolDelegator2D : public MoveToolDelegator {
        public:
            MoveToolDelegator2D(MoveToolDelegate* delegate);
        private:
            DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const;
            DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const;
            DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const;
        };

        class MoveToolDelegator3D : public MoveToolDelegator {
        public:
            MoveToolDelegator3D(MoveToolDelegate* delegate);
        private:
            DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const ;
            DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const;
            DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const;
        };
    }
}

#endif /* defined(TrenchBroom_MoveToolDelegator) */
