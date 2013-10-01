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

#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        MoveObjectsTool::MoveObjectsTool(BaseTool* next, MapDocumentPtr document, ControllerFacade& controller, MovementRestriction& movementRestriction) :
        MoveTool(next, document, controller, movementRestriction) {}

        bool MoveObjectsTool::doHandleEvent(const InputState& inputState) const {
            return (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt));
        }
        
        String MoveObjectsTool::doGetActionName(const InputState& inputState) const {
            return "Move";
        }
        
        void MoveObjectsTool::doStartMove(const InputState& inputState) {
        }
        
        Vec3 MoveObjectsTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            Grid& grid = document()->grid();
            return grid.snap(delta);
        }
        
        MoveObjectsTool::MoveResult MoveObjectsTool::doMove(const Vec3& delta) {
            if (!controller().moveObjects(document()->selectedObjects(), delta, document()->textureLock()))
                return Deny;
            return Continue;
        }
        
        void MoveObjectsTool::doEndMove(const InputState& inputState) {}
    }
}
