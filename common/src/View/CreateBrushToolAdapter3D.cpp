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

#include "CreateBrushToolAdapter3D.h"

#include "View/CreateBrushTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateBrushToolAdapter3D::CreateBrushToolAdapter3D(CreateBrushTool* tool) :
        m_tool(tool) {
            assert(tool != NULL);
        }

        CreateBrushToolAdapter3D::~CreateBrushToolAdapter3D() {}

        Tool* CreateBrushToolAdapter3D::doGetTool() {
            return m_tool;
        }
        
        void CreateBrushToolAdapter3D::doModifierKeyChange(const InputState& inputState) {
        }
        
        bool CreateBrushToolAdapter3D::doMouseClick(const InputState& inputState) {
            return false;
        }
        
        void CreateBrushToolAdapter3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
        }
        
        void CreateBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
        }

        bool CreateBrushToolAdapter3D::doCancel() {
            return false;
        }
    }
}
