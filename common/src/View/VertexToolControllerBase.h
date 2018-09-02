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

#include <algorithm>

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
                const Model::Hit findDraggableHandle(const InputState& inputState) const {
                    return doFindDraggableHandle(inputState);
                }

                const Model::Hit::List findDraggableHandles(const InputState& inputState) const {
                    return doFindDraggableHandles(inputState);
                }
            private:
                virtual const Model::Hit doFindDraggableHandle(const InputState& inputState) const {
                    return findDraggableHandle(inputState, m_hitType);
                }

                virtual const Model::Hit::List doFindDraggableHandles(const InputState& inputState) const {
                    return findDraggableHandles(inputState, m_hitType);
                }
            public:
                const Model::Hit findDraggableHandle(const InputState& inputState, const Model::Hit::HitType hitType) const {
                    const auto query = inputState.pickResult().query().type(hitType).occluded();
                    if (!query.empty()) {
                        const auto hits = query.all();
                        const auto it = std::find_if(std::begin(hits), std::end(hits), [this](const auto& hit){ return this->selected(hit); });
                        if (it != std::end(hits)) {
                            return *it;
                        } else {
                            return query.first();
                        }
                    }
                    return Model::Hit::NoHit;
                }

                const Model::Hit::List findDraggableHandles(const InputState& inputState, const Model::Hit::HitType hitType) const {
                    return inputState.pickResult().query().type(hitType).occluded().all();
                }
            private:
                bool selected(const Model::Hit& hit) const {
                    return m_tool->selected(hit);
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
                virtual ~SelectPartBase() override {
                    delete m_lasso;
                }
            protected:
                using PartBase::m_tool;
                using PartBase::m_hitType;
                using PartBase::findDraggableHandle;
            private:
                Tool* doGetTool() override {
                    return m_tool;
                }
                
                void doPick(const InputState& inputState, Model::PickResult& pickResult) override {
                    m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
                }
                
                bool doMouseClick(const InputState& inputState) override {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No)) {
                        return false;
                    }

                    const auto hits = firstHits(inputState.pickResult());
                    if (hits.empty()) {
                        return m_tool->deselectAll();
                    } else {
                        return m_tool->select(hits, inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd));
                    }
                }

                DragInfo doStartDrag(const InputState& inputState) override {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No)) {
                        return DragInfo();
                    }

                    const auto hits = firstHits(inputState.pickResult());
                    if (!hits.empty()) {
                        return DragInfo();
                    }

                    const auto& camera = inputState.camera();
                    const auto distance = 64.0f;
                    const auto plane = orthogonalPlane(vec3(camera.defaultPoint(distance)), vec3(camera.direction()));
                    const auto initialPoint = inputState.pickRay().pointAtDistance(intersect(inputState.pickRay(), plane));
                    
                    m_lasso = new Lasso(camera, distance, initialPoint);
                    return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), initialPoint);
                }
                
                DragResult doDrag(const InputState& inputState, const vec3& lastHandlePosition, const vec3& nextHandlePosition) override {
                    ensure(m_lasso != nullptr, "lasso is null");
                    m_lasso->update(nextHandlePosition);
                    return DR_Continue;
                }
                
                void doEndDrag(const InputState& inputState) override {
                    ensure(m_lasso != nullptr, "lasso is null");
                    m_tool->select(*m_lasso, inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
                    delete m_lasso;
                    m_lasso = nullptr;
                }
                
                void doCancelDrag() override {
                    ensure(m_lasso != nullptr, "lasso is null");
                    delete m_lasso;
                    m_lasso = nullptr;
                }

                bool doCancel() override {
                    return m_tool->deselectAll();
                }
            protected:
                virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override {
                    renderContext.setForceHideSelectionGuide();
                }
                
                virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                    m_tool->renderHandles(renderContext, renderBatch);
                    if (m_lasso != nullptr) {
                        m_lasso->render(renderContext, renderBatch);
                    } else {
                        if (!anyToolDragging(inputState)) {
                            const auto hit = findDraggableHandle(inputState);
                            if (hit.hasType(m_hitType)) {
                                const auto& handle = m_tool->getHandlePosition(hit);
                                m_tool->renderHighlight(renderContext, renderBatch, handle);
                                
                                if (inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                                    m_tool->renderGuide(renderContext, renderBatch, handle);
                                }
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

                            if (equalHandles(handle, firstHandle)) {
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
            private:
                virtual bool equalHandles(const H& lhs, const H& rhs) const = 0;
            };
            
            class MovePartBase : public MoveToolController<NoPickingPolicy, MousePolicy>, public PartBase {
            protected:
                MovePartBase(T* tool, const Model::Hit::HitType hitType) :
                MoveToolController(tool->grid()),
                PartBase(tool, hitType) {}
            public:
                virtual ~MovePartBase() override {}
            protected:
                using PartBase::m_tool;
                using PartBase::findDraggableHandle;
                using PartBase::findDraggableHandles;
            protected:
                Tool* doGetTool() override {
                    return m_tool;
                }
                
                bool doCancel() override {
                    return m_tool->deselectAll();
                }

                MoveInfo doStartMove(const InputState& inputState) override {
                    if (!shouldStartMove(inputState)) {
                        return MoveInfo();
                    }

                    const Model::Hit::List hits = findDraggableHandles(inputState);
                    if (hits.empty()) {
                        return MoveInfo();
                    }

                    if (!m_tool->startMove(hits)) {
                        return MoveInfo();
                    } else {
                        return MoveInfo(hits.front().hitPoint());
                    }
                }

                // Overridden in vertex tool controller to handle special cases for vertex moving.
                virtual bool shouldStartMove(const InputState& inputState) const {
                    return (inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                            (inputState.modifierKeysPressed(ModifierKeys::MKNone) || // horizontal movement
                             inputState.modifierKeysPressed(ModifierKeys::MKAlt)     // vertical movement
                            ));
                }

                DragResult doMove(const InputState& inputState, const vec3& lastHandlePosition, const vec3& nextHandlePosition) override {
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
                
                void doEndMove(const InputState& inputState) override {
                    m_tool->endMove();
                }
                
                void doCancelMove() override {
                    m_tool->cancelMove();
                }
                
                DragSnapper* doCreateDragSnapper(const InputState& inputState) const  override {
                    return new DeltaDragSnapper(m_tool->grid());
                }
                
                void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
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
            virtual ~VertexToolControllerBase() override {}
        private:
            Tool* doGetTool() override {
                return m_tool;
            }
        };
    }
}

#endif /* VertexToolControllerBase_h */
