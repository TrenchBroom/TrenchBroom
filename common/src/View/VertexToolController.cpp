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

#include "VertexToolController.h"

#include "Renderer/RenderContext.h"
#include "View/Lasso.h"
#include "View/VertexTool.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexToolController::MaxVertexDistance = 0.25;
        
        class VertexToolController::VertexPartBase {
        protected:
            VertexTool* m_tool;
        public:
            VertexPartBase(VertexTool* tool) :
            m_tool(tool) {
                ensure(m_tool != nullptr, "tool is null");
            }
        protected:
            Model::Hit::List firstHits(const Model::PickResult& pickResult) const {
                Model::Hit::List result;
                Model::BrushSet visitedBrushes;
                
                static const Model::Hit::HitType any = VertexTool::AnyHandleHit;
                const Model::Hit& first = pickResult.query().type(any).occluded().first();
                if (first.isMatch()) {
                    const Vec3 firstHandle = first.target<Vec3>();
                    
                    const Model::Hit::List matches = pickResult.query().type(any).all();
                    for (const Model::Hit& match : matches) {
                        const Vec3 handle = match.target<Vec3>();
                        
                        if (handle.squaredDistanceTo(firstHandle) < MaxVertexDistance * MaxVertexDistance) {
                            if (allIncidentBrushesVisited(handle, visitedBrushes))
                                result.push_back(match);
                        }
                    }
                }
                
                return result;
            }
        private:
            bool allIncidentBrushesVisited(const Vec3& handle, Model::BrushSet& visitedBrushes) const {
                bool result = true;
                for (auto brush : m_tool->findIncidentBrushes(handle)) {
                    const bool unvisited = visitedBrushes.insert(brush).second;
                    result &= unvisited;
                }
                return result;
            }
        };

        class VertexToolController::SelectVertexPart : public ToolControllerBase<PickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, public VertexPartBase {
        private:
            Lasso* m_lasso;
        public:
            SelectVertexPart(VertexTool* tool) :
            VertexPartBase(tool),
            m_lasso(nullptr) {}
            
            ~SelectVertexPart() {
                delete m_lasso;
            }
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
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                renderContext.setForceHideSelectionGuide();
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_tool->renderHandles(renderContext, renderBatch);
                if (m_lasso != nullptr)
                    m_lasso->render(renderContext, renderBatch);
            }
            
            bool doCancel() {
                return m_tool->deselectAll();
            }
        };
    }
}
