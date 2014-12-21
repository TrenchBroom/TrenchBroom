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

#ifndef __TrenchBroom__RotateObjectsTool__
#define __TrenchBroom__RotateObjectsTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Hit.h"
#include "Renderer/PointGuideRenderer.h"
#include "View/MapViewToolPage.h"
#include "View/MoveToolHelper.h"
#include "View/RotateObjectsHandle.h"
#include "View/RotateToolHelper.h"
#include "View/Tool.h"
#include "View/ToolActivationDelegate.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class RotateObjectsToolPage;
        class RotateObjectsTool : public ToolActivationDelegate, public ToolImpl<ActivationPolicy, PickingPolicy, MousePolicy, PlaneDragPolicy, NoDropPolicy, RenderPolicy>, public MoveDelegate, public RotateDelegate, public MapViewToolPage {
        private:
            static const Hit::HitType HandleHit;
            
            RotateObjectsToolPage* m_toolPage;
            
            RotateObjectsHandle m_handle;
            PlaneDragHelper* m_helper;
            MoveHelper m_moveHelper;
            RotateHelper m_rotateHelper;
            double m_angle;
            
            Renderer::PointGuideRenderer m_centerGuideRenderer;
            
            bool m_firstActivation;
        public:
            RotateObjectsTool(MapDocumentWPtr document, MovementRestriction& movementRestriction);
            
            double angle() const;
            void setAngle(double angle);
            
            Vec3 center() const;
            void resetHandlePosition();

            void moveCenter(const Vec3& delta);
        private:
            bool doActivate();
            bool doDeactivate();
            
            void doPick(const InputState& inputState, Hits& hits);

            void doModifierKeyChange(const InputState& inputState);
            bool doMouseDown(const InputState& inputState);
            bool doMouseUp(const InputState& inputState);
            
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            RotateObjectsHandle::HitArea highlightHandleArea(const InputState& inputState) const;
            
            // MoveDelegate protocol
            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const InputState& inputState, const Vec3& delta);
            void doEndMove(const InputState& inputState);
            void doCancelMove();
            
            // RotateDelegate protocol
            bool doHandleRotate(const InputState& inputState) const;
            RotateInfo doGetRotateInfo(const InputState& inputState) const;
            bool doStartRotate(const InputState& inputState);
            FloatType doGetAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const;
            bool doRotate(const Vec3& center, const Vec3& axis, const FloatType angle);
            void doEndRotate(const InputState& inputState);
            void doCancelRotate();

            wxWindow* doCreatePage(wxWindow* parent);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsTool__) */
