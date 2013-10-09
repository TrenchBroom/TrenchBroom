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

#include "MoveToolHelper.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/MovementRestriction.h"

namespace TrenchBroom {
    namespace View {
        MoveDelegate::~MoveDelegate() {}
        
        bool MoveDelegate::handleMove(const InputState& inputState) const {
            return doHandleMove(inputState);
        }
        
        Vec3 MoveDelegate::getMoveOrigin(const InputState& inputState) const {
            return doGetMoveOrigin(inputState);
        }
        
        bool MoveDelegate::startMove(const InputState& inputState) {
            return doStartMove(inputState);
        }
        
        Vec3 MoveDelegate::snapDelta(const InputState& inputState, const Vec3& delta) const {
            return doSnapDelta(inputState, delta);
        }
        
        MoveResult MoveDelegate::move(const Vec3& delta) {
            return doMove(delta);
        }
        
        void MoveDelegate::endMove(const InputState& inputState) {
            doEndMove(inputState);
        }
        
        void MoveDelegate::cancelMove(const InputState& inputState) {
            doCancelMove(inputState);
        }

        MoveHelper::MoveHelper(MovementRestriction& movementRestriction, MoveDelegate& delegate) :
        m_movementRestriction(movementRestriction),
        m_delegate(delegate) {}
        
        bool MoveHelper::startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!m_delegate.handleMove(inputState))
                return false;
            initialPoint = m_delegate.getMoveOrigin(inputState);
            plane = dragPlane(inputState, initialPoint);
            
            if (!m_delegate.startMove(inputState))
                return false;
            return true;
        }
        
        bool MoveHelper::planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            const Vec3 delta = m_delegate.snapDelta(inputState, m_movementRestriction.apply(curPoint - refPoint));
            if (delta.null())
                return true;
            
            const MoveResult result = m_delegate.move(delta);
            if (result == Conclude)
                return false;
            if (result == Continue)
                refPoint += delta;
            return true;
        }
        
        void MoveHelper::endPlaneDrag(const InputState& inputState) {
            m_delegate.endMove(inputState);
        }
        
        void MoveHelper::cancelPlaneDrag(const InputState& inputState) {
            m_delegate.cancelMove(inputState);
        }
        
        void MoveHelper::resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return;
            initialPoint = inputState.pickRay().pointAtDistance(distance);
            plane = dragPlane(inputState, initialPoint);
        }
        
        void MoveHelper::render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) {
            if (!dragging && !m_delegate.handleMove(inputState))
                return;
            
            const Vec3f position = renderContext.camera().defaultPoint(inputState.mouseX() + 20, inputState.mouseY() + 20);
            const Renderer::MoveIndicatorRenderer::Direction direction = getDirection();
            
            Renderer::MoveIndicatorRenderer indicatorRenderer;
            indicatorRenderer.render(renderContext, position, direction);
        }

        Plane3 MoveHelper::dragPlane(const InputState& inputState, const Vec3& initialPoint) const {
            if (m_movementRestriction.isRestricted(Math::Axis::AZ)) {
                Vec3 planeNorm = inputState.pickRay().direction;
                planeNorm[2] = 0.0;
                planeNorm.normalize();
                return Plane3(initialPoint, planeNorm);
            }
            return horizontalDragPlane(initialPoint);
        }
        
        Renderer::MoveIndicatorRenderer::Direction MoveHelper::getDirection() const {
            if (m_movementRestriction.isRestricted(Math::Axis::AZ))
                return Renderer::MoveIndicatorRenderer::Vertical;
            if (m_movementRestriction.isRestricted(Math::Axis::AX))
                return Renderer::MoveIndicatorRenderer::HorizontalX;
            if (m_movementRestriction.isRestricted(Math::Axis::AY))
                return Renderer::MoveIndicatorRenderer::HorizontalY;
            return Renderer::MoveIndicatorRenderer::HorizontalXY;
        }
    }
}
