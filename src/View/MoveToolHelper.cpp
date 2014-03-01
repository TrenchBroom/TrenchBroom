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

#include "MoveToolHelper.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
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
            
            addTracePoint(initialPoint);
            return true;
        }
        
        bool MoveHelper::planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            const Vec3 delta = m_delegate.snapDelta(inputState, m_movementRestriction.apply(curPoint - refPoint));
            if (delta.null())
                return true;
            
            const MoveResult result = m_delegate.move(delta);
            if (result == Conclude)
                return false;
            if (result == Continue) {
                refPoint += delta;
                addTracePoint(refPoint);
            }
            return true;
        }
        
        void MoveHelper::endPlaneDrag(const InputState& inputState) {
            m_delegate.endMove(inputState);
            m_trace.clear();
        }
        
        void MoveHelper::cancelPlaneDrag(const InputState& inputState) {
            m_delegate.cancelMove(inputState);
            m_trace.clear();
        }
        
        void MoveHelper::resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return;
            initialPoint = inputState.pickRay().pointAtDistance(distance);
            plane = dragPlane(inputState, initialPoint);
        }
        
        void MoveHelper::render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) {
            renderMoveIndicator(inputState, renderContext);
            renderMoveTrace(renderContext);
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
        
        void MoveHelper::addTracePoint(const Vec3& point) {
            if (m_trace.size() < 2) {
                m_trace.push_back(point);
            } else {
                const size_t c = m_trace.size();
                const Vec3 curVec = (point - m_trace[c-1]).normalized();
                const Vec3 lastVec = (m_trace[c-1] - m_trace[c-2]).normalized();
                if (Math::eq(Math::abs(curVec.dot(lastVec)), 1.0)) {
                    if (m_trace[c - 2].equals(point)) {
                        m_trace.pop_back();
                    } else {
                        m_trace[c-1] = Vec3f(point);
                    }
                } else {
                    m_trace.push_back(point);
                }
            }
        }

        void MoveHelper::renderMoveIndicator(const InputState& inputState, Renderer::RenderContext& renderContext) {
            const Vec3f position = renderContext.camera().defaultPoint(inputState.mouseX() + 20, inputState.mouseY() + 20);
            const Renderer::MoveIndicatorRenderer::Direction direction = getDirection();
            
            Renderer::MoveIndicatorRenderer indicatorRenderer;
            indicatorRenderer.render(renderContext, position, direction);
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
        
        void MoveHelper::renderMoveTrace(Renderer::RenderContext& renderContext) {
            if (m_trace.size() > 1) {
                typedef Renderer::VertexSpecs::P3::Vertex Vertex;
                Vertex::List vertices = Vertex::fromLists(m_trace, m_trace.size());
                
                PreferenceManager& prefs = PreferenceManager::instance();
                
                Renderer::EdgeRenderer renderer(Renderer::VertexArray::ref(GL_LINE_STRIP, vertices));
                renderer.setUseColor(true);

                glDisable(GL_DEPTH_TEST);
                renderer.setColor(prefs.get(Preferences::OccludedMoveTraceColor));
                renderer.render(renderContext);
                
                glEnable(GL_DEPTH_TEST);
                renderer.setColor(prefs.get(Preferences::MoveTraceColor));
                renderer.render(renderContext);
            }
        }
    }
}
