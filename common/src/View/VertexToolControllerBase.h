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

#include "Model/HitQuery.h"
#include "Model/HitType.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/Lasso.h"
#include "View/MoveToolController.h"
#include "View/ToolController.h"

#include <vecmath/intersection.h>

#include <unordered_set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
    }

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
                Model::HitType::Type m_hitType;
            protected:
                PartBase(T* tool, const Model::HitType::Type hitType) :
                m_tool(tool),
                m_hitType(hitType) {}
            public:
                virtual ~PartBase() = default;
            protected:
                Model::Hit findDraggableHandle(const InputState& inputState) const {
                    return doFindDraggableHandle(inputState);
                }

                std::vector<Model::Hit> findDraggableHandles(const InputState& inputState) const {
                    return doFindDraggableHandles(inputState);
                }
            private:
                virtual Model::Hit doFindDraggableHandle(const InputState& inputState) const {
                    return findDraggableHandle(inputState, m_hitType);
                }

                virtual std::vector<Model::Hit> doFindDraggableHandles(const InputState& inputState) const {
                    return findDraggableHandles(inputState, m_hitType);
                }
            public:
                Model::Hit findDraggableHandle(const InputState& inputState, const Model::HitType::Type hitType) const {
                    const auto query = inputState.pickResult().query().type(hitType).occluded();
                    if (!query.empty()) {
                        const auto hits = query.all();
                        for (const auto& hit : hits) {
                            if (this->selected(hit)) {
                                return hit;
                            }
                        }
                        return query.first();
                    }
                    return Model::Hit::NoHit;
                }

                std::vector<Model::Hit> findDraggableHandles(const InputState& inputState, const Model::HitType::Type hitType) const {
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
                SelectPartBase(T* tool, const Model::HitType::Type hitType) :
                PartBase(tool, hitType),
                m_lasso(nullptr) {}
            public:
                ~SelectPartBase() override {
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

                const Tool* doGetTool() const override {
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
                    const auto plane = vm::orthogonal_plane(vm::vec3(camera.defaultPoint(distance)), vm::vec3(camera.direction()));
                    const auto initialPoint = vm::point_at_distance(inputState.pickRay(), vm::intersect_ray_plane(inputState.pickRay(), plane));

                    m_lasso = new Lasso(camera, static_cast<FloatType>(distance), initialPoint);
                    return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), initialPoint);
                }

                DragResult doDrag(const InputState&, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
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
                void doSetRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
                    renderContext.setForceHideSelectionGuide();
                }

                void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                    m_tool->renderHandles(renderContext, renderBatch);
                    if (m_lasso != nullptr) {
                        m_lasso->render(renderContext, renderBatch);
                    } else {
                        if (!anyToolDragging(inputState)) {
                            const auto hit = findDraggableHandle(inputState);
                            if (hit.hasType(m_hitType)) {
                                const auto handle = m_tool->getHandlePosition(hit);
                                m_tool->renderHighlight(renderContext, renderBatch, handle);

                                if (inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                                    m_tool->renderGuide(renderContext, renderBatch, handle);
                                }
                            }
                        }
                    }
                }
            protected:
                std::vector<Model::Hit> firstHits(const Model::PickResult& pickResult) const {
                    std::vector<Model::Hit> result;
                    std::unordered_set<Model::BrushNode*> visitedBrushes;

                    const Model::Hit& first = pickResult.query().type(m_hitType).occluded().first();
                    if (first.isMatch()) {
                        const H& firstHandle = first.target<const H&>();

                        const std::vector<Model::Hit> matches = pickResult.query().type(m_hitType).all();
                        for (const Model::Hit& match : matches) {
                            const H& handle = match.target<const H&>();

                            if (equalHandles(handle, firstHandle)) {
                                if (allIncidentBrushesVisited(handle, visitedBrushes)) {
                                    result.push_back(match);
                                }
                            }
                        }
                    }

                    return result;
                }

                bool allIncidentBrushesVisited(const H& handle, std::unordered_set<Model::BrushNode*>& visitedBrushes) const {
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
                MovePartBase(T* tool, const Model::HitType::Type hitType) :
                MoveToolController(tool->grid()),
                PartBase(tool, hitType) {}
            public:
                ~MovePartBase() override = default;
            protected:
                using PartBase::m_tool;
                using PartBase::findDraggableHandle;
                using PartBase::findDraggableHandles;
            protected:
                Tool* doGetTool() override {
                    return m_tool;
                }

                const Tool* doGetTool() const override {
                    return m_tool;
                }

                bool doCancel() override {
                    return m_tool->deselectAll();
                }

                MoveInfo doStartMove(const InputState& inputState) override {
                    if (!shouldStartMove(inputState)) {
                        return MoveInfo();
                    }

                    const std::vector<Model::Hit> hits = findDraggableHandles(inputState);
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

                DragResult doMove(const InputState&, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) override {
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

                void doEndMove(const InputState&) override {
                    m_tool->endMove();
                }

                void doCancelMove() override {
                    m_tool->cancelMove();
                }

                DragSnapper* doCreateDragSnapper(const InputState&) const  override {
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
            explicit VertexToolControllerBase(T* tool) :
            m_tool(tool) {}
        public:
            ~VertexToolControllerBase() override = default;
        private:
            Tool* doGetTool() override {
                return m_tool;
            }

            const Tool* doGetTool() const override {
                return m_tool;
            }
        };
    }
}

#endif /* VertexToolControllerBase_h */
