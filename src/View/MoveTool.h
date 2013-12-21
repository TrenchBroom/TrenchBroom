/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__MoveTool__
#define __TrenchBroom__MoveTool__

#include "View/Tool.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ControllerFacade.h"
#include "View/MoveToolHelper.h"

#ifdef _MSC_VER
// We get a warning here because we pass 'this' to the member initializer, but it's okay because we don't use it in the member's initializer.
#pragma warning(push)
#pragma warning(disable : 4355)
#endif

namespace TrenchBroom {
    namespace View {
        class ControllerFacade;

        template <class ActivationPolicyType, class PickingPolicyType, class MousePolicyType, class RenderPolicyType>
        class MoveTool : public Tool<ActivationPolicyType, PickingPolicyType, MousePolicyType, PlaneDragPolicy, RenderPolicyType>, public MoveDelegate {
        private:
            typedef Tool<ActivationPolicyType, PickingPolicyType, MousePolicyType, PlaneDragPolicy, RenderPolicyType> Super;
            MoveHelper m_helper;
        public:
            MoveTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, MovementRestriction& movementRestriction) :
            Super(next, document, controller),
            m_helper(movementRestriction, *this) {}
        protected:
            void renderMoveIndicator(const InputState& inputState, Renderer::RenderContext& renderContext) {
                m_helper.render(inputState, Super::dragging(), renderContext);
            }
        private:
            void doModifierKeyChange(const InputState& inputState) {
                if (!handleMove(inputState))
                    return;
                if (Super::dragging())
                    PlaneDragPolicy::resetPlane(inputState);
            }

            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                if (!m_helper.startPlaneDrag(inputState, plane, initialPoint))
                    return false;
                Super::controller()->beginUndoableGroup(getActionName(inputState));
                return true;
            }
            
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
                return m_helper.planeDrag(inputState, lastPoint, curPoint, refPoint);
            }
            
            void doEndPlaneDrag(const InputState& inputState) {
                m_helper.endPlaneDrag(inputState);
                Super::controller()->closeGroup();
            }
            
            void doCancelPlaneDrag(const InputState& inputState) {
                m_helper.cancelPlaneDrag(inputState);
                Super::controller()->rollbackGroup();
                Super::controller()->closeGroup();
            }
            
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                m_helper.resetPlane(inputState, plane, initialPoint);
            }
            
            String getActionName(const InputState& inputState) const {
                return doGetActionName(inputState);
            }

            virtual String doGetActionName(const InputState& inputState) const = 0;
            
            // MoveDelegate protocol must be implemented by derived classes
            virtual bool doHandleMove(const InputState& inputState) const = 0;
            virtual Vec3 doGetMoveOrigin(const InputState& inputState) const = 0;
            virtual bool doStartMove(const InputState& inputState) = 0;
            virtual Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const = 0;
            virtual MoveResult doMove(const Vec3& delta) = 0;
            virtual void doEndMove(const InputState& inputState) {};
            virtual void doCancelMove(const InputState& inputState) {};
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* defined(__TrenchBroom__MoveTool__) */
