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

#include "TexturingViewOffsetTool.h"

#include "Hit.h"
#include "Model/BrushFace.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TexturingView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TexturingViewOffsetTool::TexturingViewOffsetTool(MapDocumentWPtr document, ControllerWPtr controller, const TexturingViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}
        
        bool TexturingViewOffsetTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Vec3 texPoint = m_helper.computeTexPoint(inputState.pickRay());
            m_lastPoint = Vec2f(texPoint);
            
            controller()->beginUndoableGroup("Move Texture");
            return true;
        }
        
        bool TexturingViewOffsetTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            const Vec3 texPoint = m_helper.computeTexPoint(inputState.pickRay());
            const Vec2f curPoint(texPoint);
            
            const Vec2f delta   = curPoint - m_lastPoint;
            const Vec2f snapped = m_helper.snapOffset(delta);
            
            if (snapped.null())
                return true;
            
            const Model::BrushFaceList applyTo(1, m_helper.face());
            if (snapped.x() != 0.0)
                controller()->setFaceXOffset(applyTo, -snapped.x(), true);
            if (snapped.y() != 0.0)
                controller()->setFaceYOffset(applyTo, -snapped.y(), true);
            
            m_lastPoint += snapped;
            return true;
        }
        
        void TexturingViewOffsetTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void TexturingViewOffsetTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
        }
    }
}
