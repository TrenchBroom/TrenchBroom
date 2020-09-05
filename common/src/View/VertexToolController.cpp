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

#include "View/Grid.h"
#include "View/VertexTool.h"

#include <vecmath/polygon.h>

namespace TrenchBroom {
    namespace View {
        /*
         * This is a bit awkward, but I'd rather not duplicate this logic into the two part classes, and I can't move
         * it up the inheritance hierarchy either. Nor can I introduce a separate common base class for the two parts
         * to contain this method due to the call to the inherited findDraggableHandle method.
         */
        Model::Hit VertexToolController::findHandleHit(const InputState& inputState, const VertexToolController::PartBase& base) {
            const auto vertexHit = base.findDraggableHandle(inputState, VertexHandleManager::HandleHitType);
            if (vertexHit.isMatch())
                return vertexHit;
            if (inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                const auto &firstHit = inputState.pickResult().query().first();
                if (firstHit.hasType(EdgeHandleManager::HandleHitType | FaceHandleManager::HandleHitType))
                    return firstHit;
            }
            return Model::Hit::NoHit;
        }


        std::vector<Model::Hit> VertexToolController::findHandleHits(const InputState& inputState, const VertexToolController::PartBase& base) {
            const auto vertexHits = base.findDraggableHandles(inputState, VertexHandleManager::HandleHitType);
            if (!vertexHits.empty())
                return vertexHits;
            if (inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                const auto& firstHit = inputState.pickResult().query().first();
                if (firstHit.hasType(EdgeHandleManager::HandleHitType)) {
                    const std::vector<Model::Hit> edgeHits = inputState.pickResult().query().type(EdgeHandleManager::HandleHitType).all();
                    if (!edgeHits.empty())
                        return edgeHits;
                } else if (firstHit.hasType(FaceHandleManager::HandleHitType)) {
                    const std::vector<Model::Hit> faceHits = inputState.pickResult().query().type(FaceHandleManager::HandleHitType).all();
                    if (!faceHits.empty())
                        return faceHits;
                }
            }
            return std::vector<Model::Hit>();
        }

        class VertexToolController::SelectVertexPart : public SelectPartBase<vm::vec3> {
        public:
            explicit SelectVertexPart(VertexTool* tool) :
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
        private:
            enum class SnapType {
                Relative,
                Absolute
            };

            SnapType m_lastSnapType;
            vm::vec3 m_handleOffset;
        public:
            explicit MoveVertexPart(VertexTool* tool) :
            MovePartBase(tool, VertexHandleManager::HandleHitType),
            m_lastSnapType(SnapType::Relative) {}
        private:
            void doModifierKeyChange(const InputState& inputState) override {
                MoveToolController::doModifierKeyChange(inputState);

                if (Super::thisToolDragging()) {
                    const SnapType currentSnapType = snapType(inputState);
                    if (currentSnapType != m_lastSnapType) {
                        setSnapper(inputState, doCreateDragSnapper(inputState), false);
                        m_lastSnapType = currentSnapType;
                    }
                }
            }

            bool doMouseClick(const InputState& inputState) override {
                if (inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift) &&
                    m_tool->handleManager().selectedHandleCount() == 1) {

                    const Model::Hit hit = VertexToolController::findHandleHit(inputState, *this);
                    if (hit.hasType(VertexHandleManager::HandleHitType)) {
                        const vm::vec3 sourcePos = m_tool->handleManager().selectedHandles().front();
                        const vm::vec3 targetPos = hit.target<vm::vec3>();
                        const vm::vec3 delta = targetPos - sourcePos;
                        m_tool->moveSelection(delta);
                        return true;
                    }
                }

                return false;
            }

            MoveInfo doStartMove(const InputState& inputState) override {
                const MoveInfo info = MovePartBase::doStartMove(inputState);
                if (info.move) {
                    m_lastSnapType = snapType(inputState);
                    const Model::Hit hit = findDraggableHandle(inputState);
                    const vm::vec3 handlePos = m_tool->getHandlePosition(hit);
                    m_handleOffset = handlePos - hit.hitPoint();
                }
                return info;
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

            DragSnapper* doCreateDragSnapper(const InputState& inputState) const override {
                switch (snapType(inputState)) {
                    case SnapType::Absolute:
                        return new AbsoluteDragSnapper(m_tool->grid(), m_handleOffset);
                    case SnapType::Relative:
                        return new DeltaDragSnapper(m_tool->grid());
                    switchDefault();
                }
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                MovePartBase::doRender(inputState, renderContext, renderBatch);

                if (!thisToolDragging()) {
                    const Model::Hit hit = findDraggableHandle(inputState);
                    if (hit.hasType(EdgeHandleManager::HandleHitType | FaceHandleManager::HandleHitType)) {
                        const vm::vec3 handle = m_tool->getHandlePosition(hit);
                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool->renderHandle(renderContext, renderBatch, handle, pref(Preferences::SelectedHandleColor));
                        else
                            m_tool->renderHandle(renderContext, renderBatch, handle);
                        m_tool->renderHighlight(renderContext, renderBatch, handle);
                    }
                }
            }
        private:
            SnapType snapType(const InputState& inputState) const {
                if (inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd)) {
                    return SnapType::Absolute;
                } else {
                    return SnapType::Relative;
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

        VertexToolController::VertexToolController(VertexTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveVertexPart(tool));
            addController(new SelectVertexPart(tool));
        }
    }
}
