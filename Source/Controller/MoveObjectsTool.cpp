/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MoveObjectsTool.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectsTool::handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane) {
            if (event.mouseButtons != MouseButtons::MBLeft ||
                event.modifierKeys() != ModifierKeys::MKNone)
                return false;

            return true;
        }
        
        bool MoveObjectsTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            return true;
        }
        
        void MoveObjectsTool::handleEndPlaneDrag(InputEvent& event) {
        }
        
        void MoveObjectsTool::handleChangeEditState(const Model::EditStateChangeSet& changeSet) {
            
        }

        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder) :
        DragTool(documentViewHolder) {}
    }
}
