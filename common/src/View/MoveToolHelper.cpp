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
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/MovementRestriction.h"

namespace TrenchBroom {
    namespace View {
        MoveToolDelegate::~MoveToolDelegate() {}
        
        bool MoveToolDelegate::handleMove(const InputState& inputState) const {
            return doHandleMove(inputState);
        }
        
        Vec3 MoveToolDelegate::getMoveOrigin(const InputState& inputState) const {
            return doGetMoveOrigin(inputState);
        }
        
        bool MoveToolDelegate::startMove(const InputState& inputState) {
            return doStartMove(inputState);
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

        MoveToolHelper::MoveToolHelper(MoveToolDelegate* delegate) :
        m_delegate(delegate) {}
        
        MoveToolHelper::~MoveToolHelper() {}

        bool MoveToolHelper::handleMove(const InputState& inputState) const {
            return m_delegate->handleMove(inputState);
        }

        bool MoveToolHelper::startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!handleMove(inputState))
                return false;
            initialPoint = m_delegate->getMoveOrigin(inputState);
            plane = dragPlane(inputState, initialPoint);
            
            if (!m_delegate->startMove(inputState))
                return false;
            
            addTracePoint(initialPoint);
            return true;
        }
        
        bool MoveToolHelper::planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            const Vec3 delta = m_delegate->snapDelta(inputState, doGetDelta(curPoint - refPoint));
            if (delta.null())
                return true;
            
            const MoveResult result = m_delegate->move(inputState, delta);
            if (result == MoveResult_Conclude)
                return false;
            if (result == MoveResult_Continue) {
                refPoint += delta;
                addTracePoint(refPoint);
            }
            return true;
        }
        
        void MoveToolHelper::endPlaneDrag(const InputState& inputState) {
            m_delegate->endMove(inputState);
            m_trace.clear();
        }
        
        void MoveToolHelper::cancelPlaneDrag() {
            m_delegate->cancelMove();
            m_trace.clear();
        }
        
        void MoveToolHelper::resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return;
            initialPoint = inputState.pickRay().pointAtDistance(distance);
            plane = dragPlane(inputState, initialPoint);
        }
        
        void MoveToolHelper::render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderMoveIndicator(inputState, renderContext, renderBatch);
            renderMoveTrace(renderContext, renderBatch);
        }

        Plane3 MoveToolHelper::dragPlane(const InputState& inputState, const Vec3& initialPoint) const {
            return doGetDragPlane(inputState, initialPoint);
        }
        
        void MoveToolHelper::addTracePoint(const Vec3& point) {
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

        void MoveToolHelper::renderMoveIndicator(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (renderContext.showMouseIndicators())
                doRenderMoveIndicator(inputState, renderContext, renderBatch);
        }

        void MoveToolHelper::renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_trace.size() > 1) {
                typedef Renderer::VertexSpecs::P3::Vertex Vertex;
                Vertex::List vertices = Vertex::fromLists(m_trace, m_trace.size());
                m_traceRenderer = Renderer::EdgeRenderer(Renderer::VertexArray::swap(GL_LINE_STRIP, vertices));

                Renderer::RenderEdges* renderOccludedEdges = new Renderer::RenderEdges(Reference::ref(m_traceRenderer));
                renderOccludedEdges->setColor(pref(Preferences::OccludedMoveTraceColor));
                renderBatch.addOneShot(renderOccludedEdges);
                
                Renderer::RenderEdges* renderUnoccludedEdges = new Renderer::RenderEdges(Reference::ref(m_traceRenderer));
                renderUnoccludedEdges->setColor(pref(Preferences::MoveTraceColor));
                renderBatch.addOneShot(renderUnoccludedEdges);
            }
        }

        MoveToolHelper2D::MoveToolHelper2D(MoveToolDelegate* delegate) :
        MoveToolHelper(delegate) {}

        Plane3 MoveToolHelper2D::doGetDragPlane(const InputState& inputState, const Vec3& initialPoint) const {
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 planeNorm(camera.direction().firstAxis());
            return Plane3(initialPoint, planeNorm);
        }
        
        Vec3 MoveToolHelper2D::doGetDelta(const Vec3& delta) const {
            return delta;
        }

        void MoveToolHelper2D::doRenderMoveIndicator(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const Vec3f position = renderContext.camera().defaultPoint(inputState.mouseX() + 20, inputState.mouseY() + 20);
            const Renderer::MoveIndicatorRenderer::Direction direction = Renderer::MoveIndicatorRenderer::Direction_XY;
            renderBatch.addOneShot(new Renderer::MoveIndicatorRenderer(position, direction));
        }
        
        MoveToolHelper3D::MoveToolHelper3D(MoveToolDelegate* delegate, MovementRestriction& movementRestriction) :
        MoveToolHelper(delegate),
        m_movementRestriction(movementRestriction) {}
        
        Plane3 MoveToolHelper3D::doGetDragPlane(const InputState& inputState, const Vec3& initialPoint) const {
            if (m_movementRestriction.isRestricted(Math::Axis::AZ)) {
                Vec3 planeNorm = inputState.pickRay().direction;
                planeNorm[2] = 0.0;
                planeNorm.normalize();
                return Plane3(initialPoint, planeNorm);
            }
            return horizontalDragPlane(initialPoint);
        }
        
        Vec3 MoveToolHelper3D::doGetDelta(const Vec3& delta) const {
            return m_movementRestriction.apply(delta);
        }

        void MoveToolHelper3D::doRenderMoveIndicator(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const Vec3f position = renderContext.camera().defaultPoint(inputState.mouseX() + 20, inputState.mouseY() + 20);
            const Renderer::MoveIndicatorRenderer::Direction direction = getDirection();
            renderBatch.addOneShot(new Renderer::MoveIndicatorRenderer(position, direction));
        }
        
        Renderer::MoveIndicatorRenderer::Direction MoveToolHelper3D::getDirection() const {
            if (m_movementRestriction.isRestricted(Math::Axis::AZ))
                return Renderer::MoveIndicatorRenderer::Direction_Z;
            if (m_movementRestriction.isRestricted(Math::Axis::AX))
                return Renderer::MoveIndicatorRenderer::Direction_X;
            if (m_movementRestriction.isRestricted(Math::Axis::AY))
                return Renderer::MoveIndicatorRenderer::Direction_Y;
            return Renderer::MoveIndicatorRenderer::Direction_XY;
        }
    }
}
