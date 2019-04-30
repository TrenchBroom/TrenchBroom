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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Node.h"
#include "Renderer/RenderContext.h"
#include "View/MoveObjectsTool.h"
#include "View/SelectionTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MoveObjectsToolController::MoveObjectsToolController(MoveObjectsTool* tool) :
        MoveToolController(tool->grid()),
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
        }

        MoveObjectsToolController::~MoveObjectsToolController() {}

        Tool* MoveObjectsToolController::doGetTool() {
            return m_tool;
        }

        const Tool* MoveObjectsToolController::doGetTool() const {
            return m_tool;
        }

        MoveObjectsToolController::MoveInfo MoveObjectsToolController::doStartMove(const InputState& inputState) {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKAlt) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt))
                return MoveInfo();

            const Model::PickResult& pickResult = inputState.pickResult();
            auto hits = pickResult.query().pickable().type(Model::Entity::EntityHit | Model::Brush::BrushHit).all();

            // We can't use Model::PickResult::selected() because it's not aware of groups
            // (when trying to move a selected group, the unselected entities/brushes inside
            // are what actually get picked, and the .selected() query would reject them.)
            // TODO: Implement a .transitivelySelected() hit query, and restore the query above to:
            // .query().pickable().type(Model::Entity::EntityHit | Model::Brush::BrushHit).transitivelySelected().occluded().first();
            Model::Hit* foundHit = nullptr;
            for (Model::Hit& hit : hits) {
                Model::Node* node = outermostClosedGroupOrNode(Model::hitToNode(hit));
                if (node->selected()) {
                    foundHit = &hit;
                }
            }

            if (foundHit == nullptr)
                return MoveInfo();

            if (!m_tool->startMove(inputState))
                return MoveInfo();

            return MoveInfo(foundHit->hitPoint());
        }

        RestrictedDragPolicy::DragResult MoveObjectsToolController::doMove(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) {
            switch (m_tool->move(inputState, nextHandlePosition - lastHandlePosition)) {
                case MoveObjectsTool::MR_Continue:
                    return DR_Continue;
                case MoveObjectsTool::MR_Deny:
                    return DR_Deny;
                case MoveObjectsTool::MR_Cancel:
                    return DR_Cancel;
                switchDefault();
            }
        }

        void MoveObjectsToolController::doEndMove(const InputState& inputState) {
            m_tool->endMove(inputState);
        }

        void MoveObjectsToolController::doCancelMove() {
            m_tool->cancelMove();
        }

        void MoveObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            if (thisToolDragging())
                renderContext.setForceShowSelectionGuide();
        }

        bool MoveObjectsToolController::doCancel() {
            return false;
        }
    }
}
