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

#include "UVShearTool.h"

#include "Model/BrushFace.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        UVShearTool::UVShearTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        UVGridTool(document, controller, helper) {}

        bool UVShearTool::checkIfDragApplies(const InputState& inputState, const Hit& xHit, const Hit& yHit) const {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            if (!(xHit.isMatch() ^ yHit.isMatch()))
                return false;
            
            return true;
        }
        
        String UVShearTool::getActionName() const {
            return "Shear Texture";
        }
        
        Vec2f UVShearTool::performDrag(const Vec2f& delta) {
            Model::BrushFace* face = m_helper.face();
            const Mat4x4 toWorld = face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            const Vec3 origin = toWorld * Vec3(m_helper.originInFaceCoords());
            const Vec2f oldCoords = face->textureCoords(origin) * face->textureSize();

            const UndoableCommandGroup group(controller());
            const Model::BrushFaceList applyTo(1, face);
            const Vec2f factors = shearFactors(delta);
            controller()->shearTextures(applyTo, factors);
            
            const Vec2f newCoords = face->textureCoords(origin) * face->textureSize();
            const Vec2f newOffset = face->modOffset(oldCoords - newCoords).corrected(4);
            controller()->setFaceOffset(applyTo, newOffset, false);
            
            return delta;
        }
        
        Vec2f UVShearTool::shearFactors(const Vec2f& delta) const {
            Vec2f factors = delta;
            
            const Vec2f lastVec  = m_lastHitPoint - m_helper.originInFaceCoords();
            const Vec2f curVec   = m_lastHitPoint + delta - m_helper.originInFaceCoords();
            
            assert(m_selector[0] ^ m_selector[1]);
            if (m_selector[0]) {
                factors[0] = 0.0f;
                factors[1] = curVec.at(0, 1.0f)[0] - lastVec.at(0, 1.0f)[0];
            } else {
                factors[0] = curVec.at(1, 1.0f)[0] - lastVec.at(1, 1.0f)[0];
                factors[1] = 0.0f;
            }
            return factors;
        }

        Vec2f UVShearTool::snap(const Vec2f& position) const {
            return position;
        }
        
        void UVShearTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
