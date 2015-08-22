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

#include "MoveObjectsToolAdapter.h"

#include "Renderer/RenderContext.h"
#include "View/MoveObjectsTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MoveObjectsToolAdapter::MoveObjectsToolAdapter(MoveObjectsTool* tool, MoveToolHelper* helper) :
        MoveToolAdapter(helper),
        m_tool(tool) {
            assert(m_tool != NULL);
        }
        
        MoveObjectsToolAdapter::~MoveObjectsToolAdapter() {}

        Tool* MoveObjectsToolAdapter::doGetTool() {
            return m_tool;
        }

        void MoveObjectsToolAdapter::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            if (dragging())
                renderContext.setForceShowSelectionGuide();
        }
        
        void MoveObjectsToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging() || m_tool->handleMove(inputState))
                renderMoveIndicator(inputState, renderContext, renderBatch);
        }
        
        bool MoveObjectsToolAdapter::doCancel() {
            return false;
        }
        
        MoveObjectsToolAdapter2D::MoveObjectsToolAdapter2D(MoveObjectsTool* tool) :
        MoveObjectsToolAdapter(tool, new MoveToolHelper2D(this, tool)) {}
        
        MoveObjectsToolAdapter3D::MoveObjectsToolAdapter3D(MoveObjectsTool* tool, MovementRestriction& movementRestriction) :
        MoveObjectsToolAdapter(tool, new MoveToolHelper3D(this, tool, movementRestriction)) {}
    }
}
