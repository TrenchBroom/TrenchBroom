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

#include "SetFaceAttributesTool.h"

#include "Controller/ChangeEditStateCommand.h"
#include "Controller/SetFaceAttributesCommand.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Face.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Controller {
        bool SetFaceAttributesTool::handle(InputState& inputState, bool dclick) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft)
                return false;
            
            bool altPressed = inputState.modifierKeys() == ModifierKeys::MKAlt;
            bool altCtrlPressed = inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd);
            if (!altPressed && !altCtrlPressed)
                return false;
            
            bool copyAttributes = altPressed;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::FaceList& faces = editStateManager.selectedFaces();
            if (faces.size() != 1)
                return false;
            
            const Model::Face& sourceFace = *faces.front();
            
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
            if (hit == NULL)
                return false;
            
            Model::Face& targetFace = hit->face();
            if (targetFace.selected())
                return false;
            
            Model::FaceList targetFaces;
            if (!dclick)
                targetFaces.push_back(&targetFace);
            else
                targetFaces = targetFace.brush()->faces();
            
            wxString name = copyAttributes ? wxT("Copy Face Attributes") : wxT("Copy Texture");
            
            ChangeEditStateCommand* select = ChangeEditStateCommand::select(document(), targetFaces);
            ChangeEditStateCommand* deselect = ChangeEditStateCommand::deselect(document(), targetFaces);
            SetFaceAttributesCommand* command = new SetFaceAttributesCommand(document(), targetFaces, wxT("Copy Face Attributes"));
            command->setTexture(sourceFace.texture());
            if (copyAttributes) {
                command->setXOffset(sourceFace.xOffset());
                command->setYOffset(sourceFace.yOffset());
                command->setXScale(sourceFace.xScale());
                command->setYScale(sourceFace.yScale());
                command->setRotation(sourceFace.rotation());
            }
            
            beginCommandGroup(name);
            submitCommand(select);
            submitCommand(command);
            submitCommand(deselect);
            endCommandGroup();
            return true;
        }

        bool SetFaceAttributesTool::handleMouseUp(InputState& inputState) {
            return handle(inputState, false);
        }
        
        bool SetFaceAttributesTool::handleMouseDClick(InputState& inputState) {
            return handle(inputState, true);
        }

        
        SetFaceAttributesTool::SetFaceAttributesTool(View::DocumentViewHolder& documentViewHolder) :
        Tool(documentViewHolder, false) {}
    }
}
