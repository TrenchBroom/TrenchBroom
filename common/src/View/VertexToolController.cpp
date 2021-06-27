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

#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "View/Grid.h"
#include "View/VertexTool.h"

#include <vecmath/polygon.h>

#include <memory>

namespace TrenchBroom {
    namespace View {
        /*
         * This is a bit awkward, but I'd rather not duplicate this logic into the two part classes, and I can't move
         * it up the inheritance hierarchy either. Nor can I introduce a separate common base class for the two parts
         * to contain this method due to the call to the inherited findDraggableHandle method.
         */
        Model::Hit VertexToolController::findHandleHit(const InputState& inputState, const VertexToolController::PartBase& base) {
            using namespace Model::HitFilters;

            const auto vertexHit = base.findDraggableHandle(inputState, VertexHandleManager::HandleHitType);
            if (vertexHit.isMatch())
                return vertexHit;
            if (inputState.modifierKeysDown(ModifierKeys::MKShift) && !inputState.pickResult().empty()) {
                const auto& anyHit = inputState.pickResult().all().front();
                if (anyHit.hasType(EdgeHandleManager::HandleHitType | FaceHandleManager::HandleHitType))
                    return anyHit;
            }
            return Model::Hit::NoHit;
        }


        std::vector<Model::Hit> VertexToolController::findHandleHits(const InputState& inputState, const VertexToolController::PartBase& base) {
            using namespace Model::HitFilters;

            const auto vertexHits = base.findDraggableHandles(inputState, VertexHandleManager::HandleHitType);
            if (!vertexHits.empty())
                return vertexHits;
            if (inputState.modifierKeysDown(ModifierKeys::MKShift) && !inputState.pickResult().empty()) {
                const auto& anyHit = inputState.pickResult().all().front();
                if (anyHit.hasType(EdgeHandleManager::HandleHitType)) {
                    const auto edgeHits = inputState.pickResult().all(type(EdgeHandleManager::HandleHitType));
                    if (!edgeHits.empty())
                        return edgeHits;
                } else if (anyHit.hasType(FaceHandleManager::HandleHitType)) {
                    const auto faceHits = inputState.pickResult().all(type(FaceHandleManager::HandleHitType));
                    if (!faceHits.empty())
                        return faceHits;
                }
            }
            return std::vector<Model::Hit>();
        }

        class VertexToolController::SelectVertexPart : public SelectPartBase<vm::vec3> {
        public:
            explicit SelectVertexPart(VertexTool& tool) :
            SelectPartBase(tool, VertexHandleManager::HandleHitType) {}
        private:
            Model::Hit doFindDraggableHandle(const InputState& inputState) const override {
                return VertexToolController::findHandleHit(inputState, *this);
            }

            std::vector<Model::Hit> doFindDraggableHandles(const InputState& inputState) const override {
                return VertexToolController::findHandleHits(inputState, *this);
            }

            bool equalHandles(const vm::vec3& lhs, const vm::vec3& rhs) const override {
                return vm::squared_distance(lhs, rhs) < MaxHandleDistance * MaxHandleDistance;
            }
        };

        class VertexToolController::MoveVertexPart : public MovePartBase {
        public:
            explicit MoveVertexPart(VertexTool& tool) :
            MovePartBase(tool, VertexHandleManager::HandleHitType) {}
        private:
            bool mouseClick(const InputState& inputState) override {
                if (inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift) &&
                    m_tool.handleManager().selectedHandleCount() == 1) {

                    const Model::Hit hit = VertexToolController::findHandleHit(inputState, *this);
                    if (hit.hasType(VertexHandleManager::HandleHitType)) {
                        const vm::vec3 sourcePos = m_tool.handleManager().selectedHandles().front();
                        const vm::vec3 targetPos = hit.target<vm::vec3>();
                        const vm::vec3 delta = targetPos - sourcePos;
                        m_tool.moveSelection(delta);
                        return true;
                    }
                }

                return false;
            }

            bool shouldStartMove(const InputState& inputState) const override {
                return (inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                        (inputState.modifierKeysPressed(ModifierKeys::MKNone)                            || // horizontal movement
                         inputState.modifierKeysPressed(ModifierKeys::MKAlt)                             || // vertical movement
                         inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd)                         || // horizontal absolute snap
                         inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt)   || // vertical absolute snap
                         inputState.modifierKeysPressed(ModifierKeys::MKShift)                           || // add new vertex and horizontal movement
                         inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKAlt)     || // add new vertex and vertical movement
                         inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd) || // add new vertex and horizontal movement with absolute snap
                         inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt)  // add new vertex and vertical movement with absolute snap
                        ));
            }

            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                MovePartBase::render(inputState, renderContext, renderBatch);

                if (!anyToolDragging(inputState)) {
                    const Model::Hit hit = findDraggableHandle(inputState);
                    if (hit.hasType(EdgeHandleManager::HandleHitType | FaceHandleManager::HandleHitType)) {
                        const vm::vec3 handle = m_tool.getHandlePosition(hit);
                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool.renderHandle(renderContext, renderBatch, handle, pref(Preferences::SelectedHandleColor));
                        else
                            m_tool.renderHandle(renderContext, renderBatch, handle);
                        m_tool.renderHighlight(renderContext, renderBatch, handle);
                    }
                }
            }
        private:
            Model::Hit doFindDraggableHandle(const InputState& inputState) const override {
                return VertexToolController::findHandleHit(inputState, *this);
            }

            std::vector<Model::Hit> doFindDraggableHandles(const InputState& inputState) const override {
                return VertexToolController::findHandleHits(inputState, *this);
            }
        };

        VertexToolController::VertexToolController(VertexTool& tool) :
        VertexToolControllerBase(tool) {
            addController(std::make_unique<MoveVertexPart>(tool));
            addController(std::make_unique<SelectVertexPart>(tool));
        }
    }
}
