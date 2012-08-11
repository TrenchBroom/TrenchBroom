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
#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"

namespace TrenchBroom {
    namespace Controller {

        bool SelectionTool::handleMouseUp(InputEvent& event) {
            editor().map().undoManager().addSelection(editor().map());

            Model::Selection& selection = editor().map().selection();
            Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
            if (hit != NULL) {
                if (hit->type == Model::TB_HT_ENTITY) {
                    Model::Entity& entity = hit->entity();
                    if (entity.selected()) {
                        if (multiSelectionModiferPressed(event)) {
                            selection.deselectEntity(entity);
                        } else {
                            selection.replaceSelection(entity);
                        }
                    } else {
                        if (!multiSelectionModiferPressed(event))
                            selection.replaceSelection(entity);
                        else
                            selection.selectEntity(entity);
                    }
                } else {
                    Model::Face& face = hit->face();
                    Model::Brush& brush = *face.brush;
                    if (selection.selectionMode() == Model::TB_SM_FACES) {
                        if (face.selected) {
                            if (multiSelectionModiferPressed(event))
                                selection.deselectFace(face);
                            else
                                selection.replaceSelection(brush);
                        } else {
                            if (multiSelectionModiferPressed(event)) {
                                selection.selectFace(face);
                            } else if (noModifierPressed(event)) {
                                if (brush.partiallySelected) {
                                    selection.replaceSelection(face);
                                } else {
                                    selection.replaceSelection(brush);
                                }
                            }
                        }
                    } else {
                        if (multiSelectionModiferPressed(event)) {
                            if (brush.selected)
                                selection.deselectBrush(brush);
                            else
                                selection.selectBrush(brush);
                        } else if (noModifierPressed(event)) {
                            if (brush.selected) {
                                selection.replaceSelection(face);
                            } else {
                                selection.replaceSelection(brush);
                            }
                        }
                    }
                }
            } else {
                selection.deselectAll();
            }
            
            return true;
        }
        
        bool SelectionTool::handleScrolled(InputEvent& event) {
            if (!gridSizeModifierPressed(event))
                return false;
            if (event.scrollX > 0)
                editor().grid().setSize(editor().grid().size() + 1);
            else if (editor().grid().size() > 0)
                editor().grid().setSize(editor().grid().size() - 1);
            return true;
        }
        
        bool SelectionTool::handleBeginDrag(InputEvent& event) {
            if (event.mouseButton != TB_MB_LEFT || !multiSelectionModiferPressed(event))
                return false;
            
            editor().map().undoManager().begin("Selection");
            return true;
        }
        
        bool SelectionTool::handleDrag(InputEvent& event) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            Model::Selection& selection = editor().map().selection();
            Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;
            
            if (hit->type == Model::TB_HT_ENTITY) {
                Model::Entity& entity = hit->entity();
                if (!entity.selected()) {
                    editor().map().undoManager().addSelection(editor().map());
                    selection.selectEntity(entity);
                }
            } else {
                Model::Face& face = hit->face();
                Model::Brush& brush = *face.brush;
                if (selection.selectionMode() == Model::TB_SM_FACES) {
                    if (!face.selected) {
                        editor().map().undoManager().addSelection(editor().map());
                        selection.selectFace(face);
                    }
                } else {
                    if (!brush.selected) {
                        editor().map().undoManager().addSelection(editor().map());
                        selection.selectBrush(brush);
                    }
                }
            }
            
            return true;
        }
        
        void SelectionTool::handleEndDrag(InputEvent& event) {
            editor().map().undoManager().end();
        }

        bool SelectionTool::multiSelectionModiferPressed(InputEvent& event) {
			return event.modifierKeys == Model::Preferences::sharedPreferences->selectionToolMultiKey();
        }
        
        bool SelectionTool::gridSizeModifierPressed(InputEvent& event) {
			return event.modifierKeys == Model::Preferences::sharedPreferences->selectionToolGridKey();
        }
   }
}
