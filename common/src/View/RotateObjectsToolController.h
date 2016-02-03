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

#ifndef TrenchBroom_RotateObjectsToolController
#define TrenchBroom_RotateObjectsToolController

#include "Model/Hit.h"
#include "View/RotateObjectsHandle.h"
#include "View/MoveToolDelegator.h"
#include "View/RotateObjectsHandle.h"
#include "View/RotateToolDelegator.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace View {
        class MovementRestriction;
        class RotateObjectsTool;
        
        class RotateObjectsToolController : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, public MoveToolDelegate, public RotateToolDelegate {
        protected:
            RotateObjectsTool* m_tool;
        private:
            MoveToolDelegator* m_moveDelegator;
            RotateToolDelegator* m_rotateDelegator;
            RestrictedDragPolicy* m_delegator;
        protected:
            RotateObjectsToolController(RotateObjectsTool* tool, MoveToolDelegator* moveDelegator);
        public:
            virtual ~RotateObjectsToolController();
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);

            void doModifierKeyChange(const InputState& inputState);
            bool doMouseClick(const InputState& inputState);
            
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            RotateObjectsHandle::HitArea highlightHandleArea(const InputState& inputState) const;

            bool doCancel();
        private: // MoveDelegate protocol
            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const InputState& inputState, const Vec3& delta);
            void doEndMove(const InputState& inputState);
            void doCancelMove();
        private:// RotateDelegate protocol
            bool doHandleRotate(const InputState& inputState) const;
            RotateInfo doGetRotateInfo(const InputState& inputState) const;
            bool doStartRotate(const InputState& inputState);
            FloatType doGetAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const;
            bool doRotate(const Vec3& center, const Vec3& axis, FloatType angle);
            void doEndRotate(const InputState& inputState);
            void doCancelRotate();
        private: // subclassing interface
            virtual Model::Hit doPick(const InputState& inputState) = 0;
            virtual Vec3 doGetRotationAxis(const InputState& inputState, RotateObjectsHandle::HitArea area) const = 0;
            virtual Vec3 doGetRotationAxisHandle(const InputState& inputState, RotateObjectsHandle::HitArea area) const = 0;
            virtual void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea highlight) = 0;
        };
        
        class RotateObjectsToolController2D : public RotateObjectsToolController {
        public:
            RotateObjectsToolController2D(RotateObjectsTool* tool);
        private:
            Model::Hit doPick(const InputState& inputState);
            
            Vec3 doGetRotationAxis(const InputState& inputState, RotateObjectsHandle::HitArea area) const;
            Vec3 doGetRotationAxisHandle(const InputState& inputState, RotateObjectsHandle::HitArea area) const;
            void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea highlight);
        };
        
        class RotateObjectsToolController3D : public RotateObjectsToolController {
        public:
            RotateObjectsToolController3D(RotateObjectsTool* tool, MovementRestriction& movementRestriction);
        private:
            Model::Hit doPick(const InputState& inputState);
            
            Vec3 doGetRotationAxis(const InputState& inputState, RotateObjectsHandle::HitArea area) const;
            Vec3 doGetRotationAxisHandle(const InputState& inputState, RotateObjectsHandle::HitArea area) const;
            void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea highlight);
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsToolController) */
