/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef VertexToolControllerBase_h
#define VertexToolControllerBase_h

#include "Model/ModelTypes.h"
#include "Renderer/Camera.h"
#include "View/Lasso.h"
#include "View/MoveToolController.h"
#include "View/ToolController.h"

namespace TrenchBroom {
    namespace View {
        class Tool;
        
        template <typename T>
        class VertexToolControllerBase : public ToolControllerGroup {
        protected:
            class PartBase {
            protected:
                T* m_tool;
            protected:
                PartBase(T* tool) :
                m_tool(tool) {}
            };
            
            class SelectPartBase : public ToolControllerBase<PickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, public PartBase {
            private:
                Lasso* m_lasso;
            protected:
                SelectPartBase(T* tool) :
                PartBase(tool),
                m_lasso(nullptr) {}
            public:
                virtual ~SelectPartBase() {
                    delete m_lasso;
                }
            protected:
                using PartBase::m_tool;
            private:
                Tool* doGetTool() {
                    return m_tool;
                }
                
                void doPick(const InputState& inputState, Model::PickResult& pickResult) {
                    m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
                }
                
                bool doMouseClick(const InputState& inputState) {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No))
                        return false;
                    
                    const Model::Hit::List hits = firstHits(inputState.pickResult());
                    if (hits.empty())
                        return m_tool->deselectAll();
                    else
                        return m_tool->select(hits, inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd));
                }

                DragInfo doStartDrag(const InputState& inputState) {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No))
                        return DragInfo();
                    
                    const Model::Hit::List hits = firstHits(inputState.pickResult());
                    if (!hits.empty())
                        return DragInfo();
                    
                    const Renderer::Camera& camera = inputState.camera();
                    const FloatType distance = 64.0f;
                    const Plane3 plane = orthogonalDragPlane(camera.defaultPoint(distance), camera.direction());
                    const Vec3 initialPoint = inputState.pickRay().pointAtDistance(plane.intersectWithRay(inputState.pickRay()));
                    
                    m_lasso = new Lasso(camera, distance, initialPoint);
                    return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), initialPoint);
                }
                
                DragResult doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                    ensure(m_lasso != nullptr, "lasso is null");
                    m_lasso->setPoint(nextHandlePosition);
                    return DR_Continue;
                }
                
                void doEndDrag(const InputState& inputState) {
                    ensure(m_lasso != nullptr, "lasso is null");
                    m_tool->select(*m_lasso, inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
                    delete m_lasso;
                    m_lasso = nullptr;
                }
                
                void doCancelDrag() {
                    ensure(m_lasso != nullptr, "lasso is null");
                    delete m_lasso;
                    m_lasso = nullptr;
                }

                bool doCancel() {
                    return m_tool->deselectAll();
                }
            protected:
                virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
                
                virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                    if (m_lasso != nullptr)
                        m_lasso->render(renderContext, renderBatch);
                }
            private:
                virtual Model::Hit::List firstHits(const Model::PickResult& pickResult) const = 0;
            };
            
            class MovePartBase : public MoveToolController<NoPickingPolicy, MousePolicy>, public PartBase {
            protected:
                MovePartBase(T* tool) :
                MoveToolController(tool->grid()),
                PartBase(tool) {}
            public:
                virtual ~MovePartBase() {}
            protected:
                using PartBase::m_tool;
            private:
                Tool* doGetTool() {
                    return m_tool;
                }
                
                bool doCancel() {
                    return m_tool->deselectAll();
                }
            };
            
        protected:
            T* m_tool;
        protected:
            VertexToolControllerBase(T* tool) :
            m_tool(tool) {}
        public:
            virtual ~VertexToolControllerBase() {}
        private:
            Tool* doGetTool() {
                return m_tool;
            }
        };
    }
}

#endif /* VertexToolControllerBase_h */
