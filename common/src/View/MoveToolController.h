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
        public:
            virtual ~MoveToolController() {}
        protected:
            void doModifierKeyChange(const InputState& inputState) {
                if (Super::dragging()) {
                    const Vec3& initialPoint = RestrictedDragPolicy::initialPoint();
                    const Vec3& curPoint = RestrictedDragPolicy::curPoint();
                    if (isVerticalMove(inputState)) {
                        RestrictedDragPolicy::setRestricter(inputState, doCreateVerticalDragRestricter(inputState, curPoint), true);
                    } else if (isRestrictedMove(inputState)) {
                        RestrictedDragPolicy::setRestricter(inputState, doCreateRestrictedDragRestricter(inputState, initialPoint, curPoint), false);
                    } else {
                        RestrictedDragPolicy::setRestricter(inputState, doCreateDefaultDragRestricter(inputState, curPoint), false);
                    }
                }
            }
            
            virtual bool isVerticalMove(const InputState& inputState) const {
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
            }
            
            virtual bool isRestrictedMove(const InputState& inputState) const {
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKShift);
            }
            
            RestrictedDragPolicy::DragInfo doStartDrag(const InputState& inputState) {
                const MoveInfo info = doStartMove(inputState);
                if (!info.move)
                    return RestrictedDragPolicy::DragInfo();
                
                DragRestricter* restricter = NULL;
                if (isVerticalMove(inputState))
                    restricter = doCreateVerticalDragRestricter(inputState, info.initialPoint);
                else
                    restricter = doCreateDefaultDragRestricter(inputState, info.initialPoint);
                
                DragSnapper* snapper = doCreateDragSnapper(inputState);
                return RestrictedDragPolicy::DragInfo(restricter, snapper, info.initialPoint);
            }
            
            bool doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                return doMove(inputState, lastPoint, curPoint);
            }
            
            void doEndDrag(const InputState& inputState) {
                return doEndMove(inputState);
            }
            
            void doCancelDrag() {
                return doCancelMove();
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                if (Super::dragging())
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
            virtual bool doMove(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) = 0;
            virtual void doEndMove(const InputState& inputState) = 0;
            virtual void doCancelMove() = 0;
            
            virtual DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const = 0;
            virtual DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const = 0;
            virtual DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const = 0;
            virtual DragSnapper* doCreateDragSnapper(const InputState& inputState) const = 0;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* defined(TrenchBroom_MoveToolController) */
