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

#include "SetFaceAttribsTool.h"

#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SetFaceAttribsTool::SetFaceAttribsTool(MapDocumentWPtr document, ControllerWPtr controller) :
        ToolImpl(document, controller) {}
        
        bool SetFaceAttribsTool::doMouseUp(const InputState& inputState) {
            return performCopy(inputState, false);
        }
        
        bool SetFaceAttribsTool::doMouseDoubleClick(const InputState& inputState) {
            return performCopy(inputState, true);
        }

        bool SetFaceAttribsTool::applies(const InputState& inputState) const {
            return inputState.checkModifierKeys(MK_DontCare, MK_Yes, MK_No);
        }
        
        bool SetFaceAttribsTool::copyAttributes(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }

        bool SetFaceAttribsTool::performCopy(const InputState& inputState, const bool applyToBrush) {
            if (!applies(inputState))
                return false;
            
            const Model::BrushFaceList& selectedFaces = document()->selectedFaces();
            if (selectedFaces.size() != 1)
                return false;
            
            const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
            if (!hit.isMatch())
                return false;
            
            const Model::BrushFace* source = selectedFaces.front();
            Model::BrushFace* targetFace = Model::hitAsFace(hit);
            Model::Brush* targetBrush = targetFace->parent();
            const Model::BrushFaceList targetList = applyToBrush ? targetBrush->faces() : Model::BrushFaceList(1, targetFace);
            
            const UndoableCommandGroup group(controller());
            if (copyAttributes(inputState))
                controller()->setFaceAttributes(targetList, *source);
            else
                controller()->setTexture(targetList, source->texture());
            controller()->deselectAllAndSelectFace(targetFace);
            
            return true;
        }
    }
}
