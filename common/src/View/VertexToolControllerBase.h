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

#pragma once

#include "Model/HitType.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/HandleDragTracker.h"
#include "View/Lasso.h"
#include "View/MoveHandleDragTracker.h"
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
                m_tool{tool},
                m_hitType{hitType} {}
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
                    using namespace Model::HitFilters;

                    const auto hits = inputState.pickResult().all(type(hitType));
                    if (!hits.empty()) {
                        for (const auto& hit : hits) {
                            if (m_tool->selected(hit)) {
                                return hit;
                            }
                        }
                        return inputState.pickResult().first(type(hitType));
                    }
                    return Model::Hit::NoHit;
                }

                std::vector<Model::Hit> findDraggableHandles(const InputState& inputState, const Model::HitType::Type hitType) const {
                    using namespace Model::HitFilters;
                    return inputState.pickResult().all(type(hitType));
                }
            };

            class LassoDragDelegate : public HandleDragTrackerDelegate {
            public:
                static constexpr auto LassoDistance = 64.0;
            private:
                T& m_tool;
                std::unique_ptr<Lasso> m_lasso;
            public:
                LassoDragDelegate(T& tool) :
                m_tool{tool} {}

                HandlePositionProposer start(const InputState& inputState, const vm::vec3& initialHandlePosition, const vm::vec3& handleOffset) override {
                    const auto& camera = inputState.camera();
                    m_lasso = std::make_unique<Lasso>(camera, LassoDistance, initialHandlePosition);

                    const auto plane = vm::orthogonal_plane(initialHandlePosition, vm::vec3{camera.direction()});
                    return makeHandlePositionProposer(
                        makePlaneHandlePicker(plane, handleOffset),
                        makeIdentityHandleSnapper()
                    );
                }

                DragStatus drag(const InputState&, const DragState&, const vm::vec3& proposedHandlePosition) override {
                    m_lasso->update(proposedHandlePosition);
                    return DragStatus::Continue;
                }

                void end(const InputState& inputState, const DragState&) override {
                    m_tool.select(*m_lasso, inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
                }
                
                void cancel(const DragState&) override {}

                void render(const InputState&, const DragState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const override {
                    m_lasso->render(renderContext, renderBatch);
                }
            };

            template <typename H>
            class SelectPartBase : public ToolControllerBase<PickingPolicy, NoKeyPolicy, MousePolicy, RenderPolicy>, public PartBase {
            protected:
                SelectPartBase(T* tool, const Model::HitType::Type hitType) :
                PartBase{tool, hitType} {}
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
                    }
                    
                    return m_tool->select(hits, inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd));
                }

                std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No)) {
                        return nullptr;
                    }

                    const auto hits = firstHits(inputState.pickResult());
                    if (!hits.empty()) {
                        return nullptr;
                    }

                    const auto& camera = inputState.camera();
                    const auto plane = vm::orthogonal_plane(vm::vec3{camera.defaultPoint(static_cast<float>(LassoDragDelegate::LassoDistance))}, vm::vec3{camera.direction()});
                    const auto initialPoint = vm::point_at_distance(inputState.pickRay(), vm::intersect_ray_plane(inputState.pickRay(), plane));
                    
                    return createHandleDragTracker(LassoDragDelegate{*m_tool}, inputState, initialPoint, vm::vec3::zero());
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
            protected:
                std::vector<Model::Hit> firstHits(const Model::PickResult& pickResult) const {
                    using namespace Model::HitFilters;

                    auto result = std::vector<Model::Hit>{};
                    auto visitedBrushes = std::unordered_set<Model::BrushNode*>{};

                    const Model::Hit& first = pickResult.first(type(m_hitType));
                    if (first.isMatch()) {
                        const H& firstHandle = first.target<const H&>();

                        const auto matches = pickResult.all(type(m_hitType));
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


            class MoveDragDelegate : public MoveHandleDragTrackerDelegate {
            public:
                static constexpr auto LassoDistance = 64.0;
            private:
                T& m_tool;
            public:
                MoveDragDelegate(T& tool) :
                m_tool{tool} {}

                DragStatus move(const InputState&, const DragState& dragState, const vm::vec3& proposedHandlePosition) override {
                    switch (m_tool.move(proposedHandlePosition - dragState.currentHandlePosition)) {
                        case T::MoveResult::Continue:
                            return DragStatus::Continue;
                        case T::MoveResult::Deny:
                            return DragStatus::Deny;
                        case T::MoveResult::Cancel:
                            return DragStatus::End;
                        switchDefault()
                    }
                }

                void end(const InputState&, const DragState&) override {
                    m_tool.endMove();
                }

                void cancel(const DragState&) override {
                    m_tool.cancelMove();
                }

                void render(const InputState&, const DragState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const override {
                    m_tool.renderDragHandle(renderContext, renderBatch);
                    m_tool.renderDragHighlight(renderContext, renderBatch);
                    m_tool.renderDragGuide(renderContext, renderBatch);
                }

                DragHandleSnapper makeDragHandleSnapper(const InputState&, const SnapMode snapMode) const override {
                    if (m_tool.allowAbsoluteSnapping()) {
                        return makeDragHandleSnapperFromSnapMode(m_tool.grid(), snapMode);
                    }
                    return makeRelativeHandleSnapper(m_tool.grid());
                }
            };

            class MovePartBase : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, RenderPolicy>, public PartBase {
            protected:
                MovePartBase(T* tool, const Model::HitType::Type hitType) :
                PartBase{tool, hitType} {}
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

                std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override {
                    if (!shouldStartMove(inputState)) {
                        return nullptr;
                    }

                    const auto hits = findDraggableHandles(inputState);
                    if (hits.empty()) {
                        return nullptr;
                    }

                    if (!m_tool->startMove(hits)) {
                        return nullptr;
                    }

                    const auto [initialHandlePosition, handleOffset] = m_tool->handlePositionAndOffset(hits);

                    return createMoveHandleDragTracker(MoveDragDelegate{*m_tool}, inputState, initialHandlePosition, handleOffset);
                }

                // Overridden in vertex tool controller to handle special cases for vertex moving.
                virtual bool shouldStartMove(const InputState& inputState) const {
                    return inputState.mouseButtonsPressed(MouseButtons::MBLeft)
                           && (inputState.modifierKeysPressed(ModifierKeys::MKNone) // horizontal movement
                               || inputState.modifierKeysPressed(ModifierKeys::MKAlt));  // vertical movement
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

