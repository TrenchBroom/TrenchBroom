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
            
            Vec3 m_initialPoint;
            Vec3 m_lastPoint;
        public:
            virtual ~MoveToolController() {}
        private:
            void doModifierKeyChange(const InputState& inputState) {
                if (Super::dragging())
                    Super::resetRestricter(inputState);
            }
            
            bool doShouldStartDrag(const InputState& inputState, Vec3& initialPoint) const {
                return doShouldStartMove(inputState, initialPoint);
            }
            
            void doDragStarted(const InputState& inputState, const Vec3& initialPoint) {
                doStartMove(inputState, initialPoint);
                m_initialPoint = m_lastPoint = initialPoint;
            }
            
            bool doDragged(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                if (doMove(inputState, lastPoint, curPoint)) {
                    m_lastPoint = curPoint;
                    return true;
                }
                return false;
            }
            
            void doDragEnded(const InputState& inputState) {
                doEndMove(inputState);
            }
            
            void doDragCancelled() {
                doCancelMove();
            }
            
            bool doSnapPoint(const InputState& inputState, const Vec3& lastPoint, Vec3& point) const {
                return doSnapMove(inputState, lastPoint, point);
            }
            
            DragRestricter* doCreateDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint, bool& resetInitialPoint) {
                if (isVerticalMove(inputState)) {
                    resetInitialPoint = true;
                    return doCreateVerticalDragRestricter(inputState, curPoint);
                }
                
                if (isRestrictedMove(inputState))
                    return doCreateRestrictedDragRestricter(inputState, initialPoint, curPoint);
                
                return doCreateDefaultDragRestricter(inputState, curPoint);
            }
            
            virtual bool isVerticalMove(const InputState& inputState) const {
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
            }
            
            virtual bool isRestrictedMove(const InputState& inputState) const {
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKShift);
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                if (Super::dragging())
                    renderMoveTrace(renderContext, renderBatch);
            }
            
            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
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
        private: // subclassing interface
            virtual bool doShouldStartMove(const InputState& inputState, Vec3& initialPoint) const = 0;
            virtual void doStartMove(const InputState& inputState, const Vec3& initialPoint) = 0;
            virtual bool doMove(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) = 0;
            virtual void doEndMove(const InputState& inputState) = 0;
            virtual void doCancelMove(const InputState& inputState) = 0;
            virtual bool doSnapMove(const InputState& inputState, const Vec3& lastPoint, Vec3& point) const = 0;
            
            virtual DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const = 0;
            virtual DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const = 0;
            virtual DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const = 0;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* defined(TrenchBroom_MoveToolController) */
