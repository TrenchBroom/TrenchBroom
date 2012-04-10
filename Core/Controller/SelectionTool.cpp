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
#include "Editor.h"
#include "Selection.h"
#include "Picker.h"
#include "Entity.h"
#include "Brush.h"
#include "Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool SelectionTool::leftMouseUp(ToolEvent& event) {
            Model::Selection& selection = m_editor.map().selection();
            Model::Hit* hit = event.hits->first(Model::HT_ENTITY | Model::HT_FACE, true);
            if (hit != NULL) {
                if (hit->type == Model::HT_ENTITY) {
                    Model::Entity& entity = hit->entity();
                    if (entity.selected()) {
                        if (multiSelectionModiferPressed(event)) {
                            selection.removeEntity(entity);
                        } else {
                            selection.removeAll();
                            selection.addEntity(entity);
                        }
                    } else {
                        if (!multiSelectionModiferPressed(event))
                            selection.removeAll();
                        selection.addEntity(entity);
                    }
                } else {
                    Model::Face& face = hit->face();
                    Model::Brush& brush = *face.brush();
                    if (selection.mode() == Model::SM_FACES) {
                        if (face.selected()) {
                            if (multiSelectionModiferPressed(event))
                                selection.removeFace(face);
                            else
                                selection.addBrush(brush);
                        } else {
                            if (multiSelectionModiferPressed(event)) {
                                selection.addFace(face);
                            } else if (noModifierPressed(event)) {
                                if (selection.isPartial(brush)) {
                                    selection.removeAll();
                                    selection.addFace(face);
                                } else {
                                    selection.addBrush(brush);
                                }
                            }
                        }
                    } else {
                        if (multiSelectionModiferPressed(event)) {
                            if (brush.selected())
                                selection.removeBrush(brush);
                            else
                                selection.addBrush(brush);
                        } else if (noModifierPressed(event)) {
                            if (brush.selected()) {
                                selection.addFace(face);
                            } else {
                                selection.removeAll();
                                selection.addBrush(brush);
                            }
                        }
                    }
                }
            } else {
                selection.removeAll();
            }
            
            return true;
        }
        
        bool SelectionTool::beginLeftDrag(ToolEvent& event) {
            return multiSelectionModiferPressed(event);
        }
        
        void SelectionTool::leftDrag(ToolEvent& event) {
            Model::Selection& selection = m_editor.map().selection();
            Model::Hit* hit = event.hits->first(Model::HT_ENTITY | Model::HT_FACE, true);
            if (hit != NULL) {
                if (hit->type == Model::HT_ENTITY) {
                    Model::Entity& entity = hit->entity();
                    if (!entity.selected())
                        selection.addEntity(entity);
                } else {
                    Model::Face& face = hit->face();
                    Model::Brush& brush = *face.brush();
                    if (selection.mode() == Model::SM_FACES) {
                        if (!face.selected())
                            selection.addFace(face);
                    } else {
                        if (!brush.selected())
                            selection.addBrush(brush);
                    }
                }
            }
        }
        
        bool SelectionTool::multiSelectionModiferPressed(ToolEvent& event) {
            return event.modifierKeys == MK_CMD;
        }
        
        bool SelectionTool::gridSizeModifierPressed(ToolEvent& event) {
            return event.modifierKeys == MK_ALT;
        }
   }
}
