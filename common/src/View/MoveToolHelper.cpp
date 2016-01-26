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

        MoveToolHelper::MoveToolHelper(PlaneDragPolicy* policy, MoveToolDelegate* delegate) :
        PlaneDragHelper(policy),
        m_delegate(delegate) {}
        
        MoveToolHelper::~MoveToolHelper() {}

        bool MoveToolHelper::handleMove(const InputState& inputState) const {
            return m_delegate->handleMove(inputState);
        }

        bool MoveToolHelper::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
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
        
        bool MoveToolHelper::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
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
        
        void MoveToolHelper::doEndPlaneDrag(const InputState& inputState) {
            m_delegate->endMove(inputState);
            m_trace.clear();
        }
        
        void MoveToolHelper::doCancelPlaneDrag() {
            m_delegate->cancelMove();
            m_trace.clear();
        }
        
        void MoveToolHelper::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return;
            initialPoint = inputState.pickRay().pointAtDistance(distance);
            plane = dragPlane(inputState, initialPoint);
        }
        
        void MoveToolHelper::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
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

        void MoveToolHelper::renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_trace.size() > 1) {
                typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
                
                const Vec3 start = m_trace.front();
                const Vec3 end   = m_trace.back();
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

        MoveToolHelper2D::MoveToolHelper2D(PlaneDragPolicy* policy, MoveToolDelegate* delegate) :
        MoveToolHelper(policy, delegate) {}

        Plane3 MoveToolHelper2D::doGetDragPlane(const InputState& inputState, const Vec3& initialPoint) const {
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 planeNorm(camera.direction().firstAxis());
            return Plane3(initialPoint, planeNorm);
        }
        
        Vec3 MoveToolHelper2D::doGetDelta(const Vec3& delta) const {
            return delta;
        }
        
        MoveToolHelper3D::MoveToolHelper3D(PlaneDragPolicy* policy, MoveToolDelegate* delegate, MovementRestriction& movementRestriction) :
        MoveToolHelper(policy, delegate),
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
    }
}
