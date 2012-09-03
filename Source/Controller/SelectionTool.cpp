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


#include "SelectionTool.h"

#include "Controller/ChangeEditStateCommand.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/Face.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Controller {
        bool SelectionTool::handleMouseUp(InputEvent& event) {
            if (event.mouseButtons != MouseButtons::Left)
                return false;
            
            Model::Hit* hit = event.pickResult->first(Model::Hit::EntityHit | Model::Hit::FaceHit, false);
            Command* command = NULL;
            Model::EditStateManager& editStateManager = document().EditStateManager();
            
            if (hit != NULL) {
                bool multi = event.modifierKeys == ModifierKeys::Cmd;
                
                if (hit->type() == Model::Hit::EntityHit) {
                    Model::Entity& entity = hit->entity();
                    if (multi) {
                        if (entity.selected())
                            command = ChangeEditStateCommand::deselect(document(), entity);
                        else
                            command = ChangeEditStateCommand::select(document(), entity);
                    } else {
                        command = ChangeEditStateCommand::replace(document(), entity);
                    }
                } else {
                    Model::Face& face = hit->face();
                    Model::Brush& brush = *face.brush();
                    
                    if (brush.selected()) {
                        if (multi)
                            command = ChangeEditStateCommand::deselect(document(), brush);
                        else
                            command = ChangeEditStateCommand::select(document(), face);
                    } else if (face.selected()) {
                        if (multi)
                            command = ChangeEditStateCommand::deselect(document(), face);
                        else
                            command = ChangeEditStateCommand::select(document(), brush);
                    } else {
                        if (multi) {
                            if (editStateManager.selectionMode() == Model::EditStateManager::Faces)
                                command = ChangeEditStateCommand::select(document(), face);
                            else
                                command = ChangeEditStateCommand::select(document(), brush);
                        } else {
                            if (editStateManager.selectionMode() == Model::EditStateManager::Faces)
                                command = ChangeEditStateCommand::replace(document(), face);
                            else
                                command = ChangeEditStateCommand::replace(document(), brush);
                        }
                    }
                }
            } else {
                command = ChangeEditStateCommand::deselectAll(document());
            }

            if (command != NULL) {
                postCommand(command);
                return true;
            }
            
            return false;
        }
    }
}