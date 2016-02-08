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
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        class ToolActivationDelegate;
        
        template <class PickingPolicyType, class MousePolicyType>
        class MoveToolController : public ToolControllerBase<PickingPolicyType, KeyPolicy, MousePolicyType, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            typedef ToolControllerBase<PickingPolicyType, KeyPolicy, MousePolicyType, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> Super;
            typedef enum {
                MT_Default,
                MT_Vertical,
                MT_Restricted
            } MoveType;
            
            MoveType m_lastMoveType;
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
            void doModifierKeyChange(const InputState& inputState) {
                if (Super::dragging(inputState)) {
                    const Vec3& initialPoint = RestrictedDragPolicy::initialPoint();
                    const Vec3& curPoint = RestrictedDragPolicy::curPoint();
                    
                    const MoveType newMoveType = moveType(inputState);
                    switch (newMoveType) {
                        case MT_Default:
                            RestrictedDragPolicy::setRestricter(inputState, doCreateDefaultDragRestricter(inputState, curPoint), m_lastMoveType == MT_Vertical);
                            break;
                        case MT_Vertical:
                            RestrictedDragPolicy::setRestricter(inputState, doCreateVerticalDragRestricter(inputState, curPoint), true);
                            break;
                        case MT_Restricted:
                            RestrictedDragPolicy::setRestricter(inputState, doCreateRestrictedDragRestricter(inputState, initialPoint, curPoint), false);
                            break;
                        switchDefault()
                    }
                    m_lastMoveType = newMoveType;
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
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
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
                } else {
                    restricter = doCreateDefaultDragRestricter(inputState, info.initialPoint);
                    m_lastMoveType = MT_Default;
                }
                
                DragSnapper* snapper = doCreateDragSnapper(inputState);
                return RestrictedDragPolicy::DragInfo(restricter, snapper, info.initialPoint);
            }
            
            RestrictedDragPolicy::DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                return doMove(inputState, lastPoint, curPoint);
            }
            
            void doEndDrag(const InputState& inputState) {
                return doEndMove(inputState);
            }
            
            void doCancelDrag() {
                return doCancelMove();
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                if (Super::dragging(inputState))
                    renderMoveTrace(renderContext, renderBatch);
            }
            
            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                const Vec3 start = RestrictedDragPolicy::dragOrigin();
                const Vec3 end   = RestrictedDragPolicy::lastPoint();
                if (end != start) {
                    typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
                    
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* defined(TrenchBroom_MoveToolController) */
