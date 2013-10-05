/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MoveObjectsTool.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MoveObjectsTool::MoveObjectsTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller, MovementRestriction& movementRestriction) :
        MoveTool(next, document, controller, movementRestriction) {}

        bool MoveObjectsTool::doHandleEvent(const InputState& inputState) const {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKAlt) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt))
                return false;
            
            if (!document()->hasSelectedObjects())
                return false;
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Entity::EntityHit | Model::Brush::BrushHit, document()->filter(), false);
            if (!first.matches)
                return false;
            const Model::Object* object = Model::hitAsObject(first.hit);
            return object->selected();
        }
        
        Vec3 MoveObjectsTool::doGetInitialPoint(const InputState& inputState) const {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Entity::EntityHit | Model::Brush::BrushHit, document()->filter(), false);
            assert(first.matches);
            return first.hit.hitPoint();
        }

        String MoveObjectsTool::doGetActionName(const InputState& inputState) const {
            return duplicateObjects(inputState) ? "Duplicate" : "Move";
        }
        
        bool MoveObjectsTool::doStartMove(const InputState& inputState) {
            m_duplicateObjects = duplicateObjects(inputState);
            return true;
        }
        
        Vec3 MoveObjectsTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            Grid& grid = document()->grid();
            return grid.snap(delta);
        }
        
        MoveObjectsTool::MoveResult MoveObjectsTool::doMove(const Vec3& delta) {
            if (m_duplicateObjects) {
                const Model::ObjectList& duplicates = controller()->duplicateObjects(document()->selectedObjects(), document()->worldBounds());
                if (duplicates.empty())
                    return Conclude;
                
                controller()->deselectAll();
                controller()->selectObjects(duplicates);
                m_duplicateObjects = false;
            }
            
            if (!controller()->moveObjects(document()->selectedObjects(), delta, document()->textureLock()))
                return Deny;
            return Continue;
        }
        
        void MoveObjectsTool::doEndMove(const InputState& inputState) {}
        
        void MoveObjectsTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            renderMoveIndicator(inputState, renderContext);
        }

        bool MoveObjectsTool::duplicateObjects(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }
    }
}
