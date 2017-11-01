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

#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/Lasso.h"
#include "View/MoveToolController.h"
#include "View/ToolController.h"

namespace TrenchBroom {
    namespace View {
        class Tool;
        
        template <typename T>
        class VertexToolControllerBase : public ToolControllerGroup {
        protected:
            constexpr static const FloatType MaxHandleDistance = 0.25;
        protected:
            class PartBase {
            protected:
                T* m_tool;
                Model::Hit::HitType m_hitType;
            protected:
                PartBase(T* tool, const Model::Hit::HitType hitType) :
                m_tool(tool),
                m_hitType(hitType) {}
            public:
                virtual ~PartBase() {}
            protected:
                const Model::Hit& findDraggableHandle(const InputState& inputState) const {
                    return doFindDraggableHandle(inputState);
                }
            private:
                virtual const Model::Hit& doFindDraggableHandle(const InputState& inputState) const {
                    return inputState.pickResult().query().type(m_hitType).occluded().first();
                }
            };
            
            template <typename H>
            class SelectPartBase : public ToolControllerBase<PickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, public PartBase {
            private:
                Lasso* m_lasso;
            protected:
                SelectPartBase(T* tool, const Model::Hit::HitType hitType) :
                PartBase(tool, hitType),
                m_lasso(nullptr) {}
            public:
                virtual ~SelectPartBase() {
                    delete m_lasso;
                }
            protected:
                using PartBase::m_tool;
                using PartBase::m_hitType;
                using PartBase::findDraggableHandle;
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
                    m_lasso->update(nextHandlePosition);
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
                virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                    renderContext.setForceHideSelectionGuide();
                }
                
                virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                    m_tool->renderHandles(renderContext, renderBatch);
                    if (m_lasso != nullptr) {
                        m_lasso->render(renderContext, renderBatch);
                    } else {
                        if (!anyToolDragging(inputState)) {
                            const Model::Hit& hit = findDraggableHandle(inputState);
                            if (hit.hasType(m_hitType)) {
                                const H& handle = hit.target<H>();
                                m_tool->renderHighlight(renderContext, renderBatch, handle);
                                
                                if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                                    m_tool->renderGuide(renderContext, renderBatch, handle);
                            }
                        }
                    }
                }
            protected:
                Model::Hit::List firstHits(const Model::PickResult& pickResult) const {
                    Model::Hit::List result;
                    Model::BrushSet visitedBrushes;
                    
                    const Model::Hit& first = pickResult.query().type(m_hitType).occluded().first();
                    if (first.isMatch()) {
                        const H& firstHandle = first.target<H>();
                        
                        const Model::Hit::List matches = pickResult.query().type(m_hitType).all();
                        for (const Model::Hit& match : matches) {
                            const H& handle = match.target<H>();
                            
                            if (handle.squaredDistanceTo(firstHandle) < MaxHandleDistance * MaxHandleDistance) {
                                if (allIncidentBrushesVisited(handle, visitedBrushes))
                                    result.push_back(match);
                            }
                        }
                    }
                    
                    return result;
                }

                bool allIncidentBrushesVisited(const H& handle, Model::BrushSet& visitedBrushes) const {
                    bool result = true;
                    for (auto brush : m_tool->findIncidentBrushes(handle)) {
                        const bool unvisited = visitedBrushes.insert(brush).second;
                        result &= unvisited;
                    }
                    return result;
                }
            };
            
            class MovePartBase : public MoveToolController<NoPickingPolicy, MousePolicy>, public PartBase {
            protected:
                MovePartBase(T* tool, const Model::Hit::HitType hitType) :
                MoveToolController(tool->grid()),
                PartBase(tool, hitType) {}
            public:
                virtual ~MovePartBase() {}
            protected:
                using PartBase::m_tool;
                using PartBase::m_hitType;
                using PartBase::findDraggableHandle;
            protected:
                Tool* doGetTool() {
                    return m_tool;
                }
                
                bool doCancel() {
                    return m_tool->deselectAll();
                }

                MoveInfo doStartMove(const InputState& inputState) {
                    if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                          (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                           inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                           inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) ||
                           inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd) ||
                           inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                           inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift)
                           )))
                        return MoveInfo();
                    
                    const Model::Hit& hit = findDraggableHandle(inputState);
                    if (!hit.isMatch())
                        return MoveInfo();
                    
                    if (!m_tool->startMove(hit))
                        return MoveInfo();
                    
                    return MoveInfo(hit.hitPoint());
                }
                
                DragResult doMove(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                    switch (m_tool->move(nextHandlePosition - lastHandlePosition)) {
                        case T::MR_Continue:
                            return DR_Continue;
                        case T::MR_Deny:
                            return DR_Deny;
                        case T::MR_Cancel:
                            return DR_Cancel;
                            switchDefault()
                    }
                }
                
                void doEndMove(const InputState& inputState) {
                    m_tool->endMove();
                }
                
                void doCancelMove() {
                    m_tool->cancelMove();
                }
                
                DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                    return new DeltaDragSnapper(m_tool->grid());
                }
                
                void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                    MoveToolController::doRender(inputState, renderContext, renderBatch);
                    
                    if (thisToolDragging()) {
                        m_tool->renderDragHandle(renderContext, renderBatch);
                        m_tool->renderDragHighlight(renderContext, renderBatch);
                        m_tool->renderDragGuide(renderContext, renderBatch);
                    }
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
