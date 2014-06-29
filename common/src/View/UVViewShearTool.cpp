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
        UVViewShearTool::UVViewShearTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        UVViewTextureGridTool(document, controller, helper) {}

        bool UVViewShearTool::checkIfDragApplies(const InputState& inputState, const Hit& xHit, const Hit& yHit) const {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            if (!xHit.isMatch() && !yHit.isMatch())
                return false;
            
            return true;
        }
        
        String UVViewShearTool::getActionName() const {
            return "Shear Texture";
        }
        
        void UVViewShearTool::startDrag(const Vec2f& pos) {
            const Model::BrushFace* face = m_helper.face();
            m_axisLength[0] = face->textureXAxis().length();
            m_axisLength[1] = face->textureYAxis().length();
        }

        Vec2f UVViewShearTool::performDrag(const Vec2f& delta) {
            // all positions in unscaled and untranslated texture coordinates
            const Vec2f originPos = m_helper.originInFaceCoords();
            const Vec2f curHandlePos = getHandlePos();
            const Vec2f newHandlePos = curHandlePos + delta;
            return Vec2f::Null;
        }
        
        Vec2f UVViewShearTool::snap(const Vec2f& position) const {
            return position;
        }
        
        void UVViewShearTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
