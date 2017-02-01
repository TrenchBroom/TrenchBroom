/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_MoveToolController
#define TrenchBroom_MoveToolController

#include "View/ToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Macros.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        class ToolActivationDelegate;
        
        template <class PickingPolicyType, class MousePolicyType>
        class MoveToolController : public ToolControllerBase<PickingPolicyType, KeyPolicy, MousePolicyType, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            typedef ToolControllerBase<PickingPolicyType, KeyPolicy, MousePolicyType, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> Super;
        private:
            typedef enum {
                MT_Default,
                MT_Vertical,
                MT_Restricted
            } MoveType;
            
            MoveType m_lastMoveType;
            Vec3 m_moveTraceOrigin;
            Vec3 m_moveTraceCurPoint;
            bool m_restricted;
        protected:
            struct MoveInfo {
                bool move;
                Vec3 initialPoint;
                
                MoveInfo() :
                move(false) {}
                
                MoveInfo(const Vec3& i_initialPoint) :
                move(true),
                initialPoint(i_initialPoint) {}
            };
        protected:
            const Grid& m_grid;
        public:
            MoveToolController(const Grid& grid) : m_grid(grid) {}
            virtual ~MoveToolController() {}
        protected:
            virtual void doModifierKeyChange(const InputState& inputState) {
                if (Super::thisToolDragging()) {
                    const Vec3& initialPoint = RestrictedDragPolicy::initialPoint();
                    const Vec3& curPoint = RestrictedDragPolicy::curPoint();
                    
                    const MoveType nextMoveType = moveType(inputState);
                    if (nextMoveType != m_lastMoveType) {
                        if (m_lastMoveType != MT_Default) {
                            RestrictedDragPolicy::setRestricter(inputState, doCreateDefaultDragRestricter(inputState, curPoint), m_lastMoveType == MT_Vertical);
                            if (m_lastMoveType == MT_Vertical)
                                m_moveTraceOrigin = m_moveTraceCurPoint = curPoint;
                        }
                        if (nextMoveType == MT_Vertical) {
                            RestrictedDragPolicy::setRestricter(inputState, doCreateVerticalDragRestricter(inputState, curPoint), false);
                            m_moveTraceOrigin = m_moveTraceCurPoint = curPoint;
                            m_restricted = true;
                        } else if (nextMoveType == MT_Restricted) {
                            RestrictedDragPolicy::setRestricter(inputState, doCreateRestrictedDragRestricter(inputState, initialPoint, curPoint), false);
                            m_restricted = true;
                        }
                        m_lastMoveType = nextMoveType;
                    }
                }
            }
            
        private:
            MoveType moveType(const InputState& inputState) const {
                if (isVerticalMove(inputState))
                    return MT_Vertical;
                if (isRestrictedMove(inputState))
                    return MT_Restricted;
                return MT_Default;
            }
            
            virtual bool isVerticalMove(const InputState& inputState) const {
                const Renderer::Camera& camera = inputState.camera();
                return camera.perspectiveProjection() && inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
            }
            
            virtual bool isRestrictedMove(const InputState& inputState) const {
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKShift);
            }
        protected:
            RestrictedDragPolicy::DragInfo doStartDrag(const InputState& inputState) {
                const MoveInfo info = doStartMove(inputState);
                if (!info.move)
                    return RestrictedDragPolicy::DragInfo();
                
                DragRestricter* restricter = NULL;
                if (isVerticalMove(inputState)) {
                    restricter = doCreateVerticalDragRestricter(inputState, info.initialPoint);
                    m_lastMoveType = MT_Vertical;
                    m_restricted = true;
                } else {
                    restricter = doCreateDefaultDragRestricter(inputState, info.initialPoint);
                    m_lastMoveType = MT_Default;
                    m_restricted = false;
                }
                
                m_moveTraceOrigin = m_moveTraceCurPoint = info.initialPoint;
                DragSnapper* snapper = doCreateDragSnapper(inputState);
                return RestrictedDragPolicy::DragInfo(restricter, snapper, info.initialPoint);
            }
            
            RestrictedDragPolicy::DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                const RestrictedDragPolicy::DragResult result = doMove(inputState, lastPoint, curPoint);
                if (result == RestrictedDragPolicy::DR_Continue)
                    m_moveTraceCurPoint += (curPoint - lastPoint);
                return result;
            }
            
            void doEndDrag(const InputState& inputState) {
                doEndMove(inputState);
            }
            
            void doCancelDrag() {
                doCancelMove();
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                if (Super::thisToolDragging())
                    renderMoveTrace(renderContext, renderBatch);
            }
            
            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                const Vec3& start = m_moveTraceOrigin;
                const Vec3& end = m_moveTraceCurPoint;
                if (end != start) {
                    const Vec3 vec = end - start;
                    
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setShowOccludedObjects();
                    if (m_restricted)
                        renderService.setLineWidth(2.0f);
                    
                    Vec3::Array stages(3);
                    stages[0] = vec * Vec3::PosX;
                    stages[1] = vec * Vec3::PosY;
                    stages[2] = vec * Vec3::PosZ;

                    Color::Array colors(3);
                    colors[0] = pref(Preferences::XAxisColor);
                    colors[1] = pref(Preferences::YAxisColor);
                    colors[2] = pref(Preferences::ZAxisColor);
                    
                    Vec3 lastPos = start;
                    for (size_t i = 0; i < 3; ++i) {
                        const Vec3& stage = stages[i];
                        const Vec3 curPos = lastPos + stage;
                        
                        renderService.setForegroundColor(colors[i]);
                        renderService.renderLine(lastPos, curPos);
                        lastPos = curPos;
                    }
                    
                }
            }
        protected: // subclassing interface
            virtual MoveInfo doStartMove(const InputState& inputState) = 0;
            virtual RestrictedDragPolicy::DragResult doMove(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) = 0;
            virtual void doEndMove(const InputState& inputState) = 0;
            virtual void doCancelMove() = 0;
            
            virtual DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const {
                const Renderer::Camera& camera = inputState.camera();
                if (camera.perspectiveProjection())
                    return new PlaneDragRestricter(Plane3(curPoint, Vec3::PosZ));
                return new PlaneDragRestricter(Plane3(curPoint, Vec3(camera.direction().firstAxis())));
            }
            
            virtual DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const {
                const Renderer::Camera& camera = inputState.camera();
                if (camera.perspectiveProjection())
                    return new LineDragRestricter(Line3(curPoint, Vec3::PosZ));
                return new PlaneDragRestricter(Plane3(curPoint, Vec3(camera.direction().firstAxis())));
            }
            
            virtual DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const {
                const Vec3 delta = curPoint - initialPoint;
                const Vec3 axis = delta.firstAxis();
                return new LineDragRestricter(Line3(initialPoint, axis));
            }
            
            virtual DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                return new DeltaDragSnapper(m_grid);
            }
        };
    }
}

#endif /* defined(TrenchBroom_MoveToolController) */
