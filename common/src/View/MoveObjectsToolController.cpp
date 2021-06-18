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

#include "MoveObjectsToolController.h"

#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/ModelUtils.h"
#include "Renderer/RenderContext.h"
#include "View/DragTracker.h"
#include "View/MoveHandleDragTracker.h"
#include "View/MoveObjectsTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MoveObjectsToolController::MoveObjectsToolController(MoveObjectsTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
        }

        MoveObjectsToolController::~MoveObjectsToolController() {}

        Tool& MoveObjectsToolController::tool() {
            return *m_tool;
        }

        const Tool& MoveObjectsToolController::tool() const {
            return *m_tool;
        }

        namespace {
            class MoveObjectsDragDelegate : public MoveHandleDragTrackerDelegate {
            private:
                MoveObjectsTool& m_tool;
            public:
                MoveObjectsDragDelegate(MoveObjectsTool& tool) :
                m_tool{tool} {}

                DragStatus move(const InputState& inputState, const DragState& dragState, const vm::vec3& proposedHandlePosition) override {
                    switch (m_tool.move(inputState, proposedHandlePosition - dragState.currentHandlePosition)) {
                        case MoveObjectsTool::MR_Continue:
                            return DragStatus::Continue;
                        case MoveObjectsTool::MR_Deny:
                            return DragStatus::Deny;
                        case MoveObjectsTool::MR_Cancel:
                            return DragStatus::End;
                        switchDefault();
                    }
                }

                void end(const InputState& inputState, const DragState&) override {
                    m_tool.endMove(inputState);
                }

                void cancel(const DragState&) override {
                    m_tool.cancelMove();
                }

                void setRenderOptions(const InputState& , Renderer::RenderContext& renderContext) const override {
                    renderContext.setForceShowSelectionGuide();
                }

                DragHandleSnapper makeDragHandleSnapper(const InputState&, const SnapMode) const override {
                    return makeRelativeHandleSnapper(m_tool.grid());
                }
            };
        }

        std::unique_ptr<DragTracker> MoveObjectsToolController::acceptMouseDrag(const InputState& inputState) {
            using namespace Model::HitFilters;

            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKAlt) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt)) {
                return nullptr;
            }

            // The transitivelySelected() lets the hit query match entities/brushes inside a
            // selected group, even though the entities/brushes aren't selected themselves.

            const Model::Hit& hit = inputState.pickResult().first(type(Model::nodeHitType()) && transitivelySelected());
            if (!hit.isMatch()) {
                return nullptr;
            }

            if (!m_tool->startMove(inputState)) {
                return nullptr;
            }

            return createMoveHandleDragTracker(MoveObjectsDragDelegate{*m_tool}, inputState, hit.hitPoint(), vm::vec3::zero());
        }

        bool MoveObjectsToolController::cancel() {
            return false;
        }
    }
}
