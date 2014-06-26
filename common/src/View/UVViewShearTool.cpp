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

#include "UVViewShearTool.h"

#include "Model/BrushFace.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType UVViewShearTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType UVViewShearTool::YHandleHit = Hit::freeHitType();

        UVViewShearTool::UVViewShearTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}
        
        void UVViewShearTool::doPick(const InputState& inputState, Hits& hits) {
            if (m_helper.valid()) {
                static const Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
                if (m_helper.valid())
                    m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, hits);
            }
        }
        
        bool UVViewShearTool::doStartMouseDrag(const InputState& inputState) {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit& xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit& yHandleHit = hits.findFirst(YHandleHit, true);
            
            if (!xHandleHit.isMatch() && !yHandleHit.isMatch())
                return false;
        }
        
        bool UVViewShearTool::doMouseDrag(const InputState& inputState) {
        }
        
        void UVViewShearTool::doEndMouseDrag(const InputState& inputState) {
        }
        
        void UVViewShearTool::doCancelMouseDrag(const InputState& inputState) {
        }
        
        void UVViewShearTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
