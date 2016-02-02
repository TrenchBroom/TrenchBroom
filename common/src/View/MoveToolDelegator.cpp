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

#include "MoveToolDelegator.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/MovementRestriction.h"

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        MoveToolDelegate::~MoveToolDelegate() {}
        
        bool MoveToolDelegate::handleMove(const InputState& inputState) const {
            return doHandleMove(inputState);
        }
        
        Vec3 MoveToolDelegate::getMoveOrigin(const InputState& inputState) const {
            return doGetMoveOrigin(inputState);
        }
        
        void MoveToolDelegate::startMove(const InputState& inputState) {
            doStartMove(inputState);
        }
        
        Vec3 MoveToolDelegate::snapDelta(const InputState& inputState, const Vec3& delta) const {
            return doSnapDelta(inputState, delta);
        }
        
        MoveResult MoveToolDelegate::move(const InputState& inputState, const Vec3& delta) {
            return doMove(inputState, delta);
        }
        
        void MoveToolDelegate::endMove(const InputState& inputState) {
            doEndMove(inputState);
        }
        
        void MoveToolDelegate::cancelMove() {
            doCancelMove();
        }

        MoveToolDelegator::MoveToolDelegator(MoveToolDelegate* delegate) :
        m_delegate(delegate) {}
        
        MoveToolDelegator::~MoveToolDelegator() {}

        bool MoveToolDelegator::doShouldStartDrag(const InputState& inputState, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!m_delegate->handleMove(inputState))
                return false;
            initialPoint = m_delegate->getMoveOrigin(inputState);
            return true;
        }
        
        void MoveToolDelegator::doDragStarted(const InputState& inputState, const Vec3& initialPoint) {
            m_delegate->startMove(inputState);
            m_initialPoint = m_lastPoint = initialPoint;
        }
        
        bool MoveToolDelegator::doDragged(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
            const MoveResult result = m_delegate->move(inputState, curPoint - lastPoint);
            if (result == MoveResult_Conclude)
                return false;
            m_lastPoint = curPoint;
            return true;
        }
        
        void MoveToolDelegator::doDragEnded(const InputState& inputState) {
            m_delegate->endMove(inputState);
        }
        
        void MoveToolDelegator::doDragCancelled() {
            m_delegate->cancelMove();
        }
        
        bool MoveToolDelegator::doSnapPoint(const InputState& inputState, const Vec3& lastPoint, Vec3& point) {
            point = lastPoint + m_delegate->snapDelta(inputState, point - lastPoint);
            return true;
        }

        DragRestricter* MoveToolDelegator::doCreateDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint, bool& resetInitialPoint) {
            if (isVerticalMove(inputState)) {
                resetInitialPoint = true;
                return doCreateVerticalDragRestricter(inputState, curPoint);
            }
            
            if (isRestrictedMove(inputState))
                return doCreateRestrictedDragRestricter(inputState, initialPoint, curPoint);
            
            return doCreateDefaultDragRestricter(inputState, curPoint);
        }
        
        bool MoveToolDelegator::isVerticalMove(const InputState& inputState) const {
            return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
        }
        
        bool MoveToolDelegator::isRestrictedMove(const InputState& inputState) const {
            return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKShift);
        }

        void MoveToolDelegator::render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging())
                renderMoveTrace(renderContext, renderBatch);
        }

        void MoveToolDelegator::renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_lastPoint != m_initialPoint) {
                typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
                
                const Vec3 start = m_initialPoint;
                const Vec3 end   = m_lastPoint;
                const Vec3 vec   = end - start;

                Vec3::List stages(3);
                stages[0] = vec * Vec3::PosX;
                stages[1] = vec * Vec3::PosY;
                stages[2] = vec * Vec3::PosZ;

                Vec3 lastPos = start;
                Vertex::List vertices(6);
                for (size_t i = 0; i < 3; ++i) {
                    const Vec3& stage = stages[i];
                    const Vec3 curPos = lastPos + stage;
                    
                    const Color& color = (stage[0] != 0.0 ? pref(Preferences::XAxisColor) : (stage[1] != 0.0 ? pref(Preferences::YAxisColor) : pref(Preferences::ZAxisColor)));
                    vertices[2 * i + 0] = Vertex(lastPos, color);
                    vertices[2 * i + 1] = Vertex(curPos,  color);
                    lastPos = curPos;
                }
                
                Renderer::DirectEdgeRenderer traceRenderer(Renderer::VertexArray::swap(vertices), GL_LINES);
                traceRenderer.renderOnTop(renderBatch);
            }
        }

        MoveToolDelegator2D::MoveToolDelegator2D(MoveToolDelegate* delegate) :
        MoveToolDelegator(delegate) {}

        DragRestricter* MoveToolDelegator2D::doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const {
            const Renderer::Camera& camera = inputState.camera();
            return new PlaneDragRestricter(Plane3(curPoint, camera.direction()));
        }
        
        DragRestricter* MoveToolDelegator2D::doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const {
            const Renderer::Camera& camera = inputState.camera();
            return new PlaneDragRestricter(Plane3(curPoint, camera.direction()));
        }
        
        DragRestricter* MoveToolDelegator2D::doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const {
            const Vec3 direction = (curPoint - initialPoint).absFirstAxis();
            return new LineDragRestricter(Line3(curPoint, direction));
        }

        MoveToolDelegator3D::MoveToolDelegator3D(MoveToolDelegate* delegate) :
        MoveToolDelegator(delegate) {}
        
        DragRestricter* MoveToolDelegator3D::doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const {
            return new PlaneDragRestricter(Plane3(curPoint, Vec3::PosZ));
        }
        
        DragRestricter* MoveToolDelegator3D::doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const {
            return new LineDragRestricter(Line3(curPoint, Vec3::PosZ));
        }
        
        DragRestricter* MoveToolDelegator3D::doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const {
            const Vec3 direction = (curPoint - initialPoint).absFirstAxis();
            return new LineDragRestricter(Line3(curPoint, direction));
        }
    }
}
