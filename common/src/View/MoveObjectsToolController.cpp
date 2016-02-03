/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Renderer/RenderContext.h"
#include "View/MoveObjectsTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MoveObjectsToolController::MoveObjectsToolController(MoveObjectsTool* tool, MoveToolDelegator* delegator) :
        MoveToolController(delegator),
        m_tool(tool) {
            assert(m_tool != NULL);
        }
        
        MoveObjectsToolController::~MoveObjectsToolController() {}

        Tool* MoveObjectsToolController::doGetTool() {
            return m_tool;
        }

        void MoveObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            if (dragging())
                renderContext.setForceShowSelectionGuide();
        }
        
        bool MoveObjectsToolController::doCancel() {
            return false;
        }
        
        MoveObjectsToolController2D::MoveObjectsToolController2D(MoveObjectsTool* tool) :
        MoveObjectsToolController(tool, new MoveToolDelegator2D(tool)) {}
        
        MoveObjectsToolController3D::MoveObjectsToolController3D(MoveObjectsTool* tool, MovementRestriction& movementRestriction) :
        MoveObjectsToolController(tool, new MoveToolDelegator3D(tool)) {}
    }
}
