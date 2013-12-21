/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "SelectionTool.h"

#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/Entity.h"
#include "Model/ModelFilter.h"
#include "Model/Object.h"
#include "Model/Picker.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SelectionTool::SelectionTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller) :
        Tool(next, document, controller) {}

        bool SelectionTool::doMouseUp(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const bool multi = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
            const bool faces = inputState.modifierKeysDown(ModifierKeys::MKShift);
            
            if (faces) {
                const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
                if (first.matches) {
                    Model::BrushFace* face = hitAsFace(first.hit);
                    if (multi) {
                        if (face->selected()) {
                            controller()->deselectFace(*face);
                        } else {
                            controller()->selectFace(*face);
                        }
                    } else {
                        controller()->deselectAllAndSelectFace(*face);
                    }
                } else {
                    controller()->deselectAll();
                }
            } else {
                const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Entity::EntityHit | Model::Brush::BrushHit, document()->filter(), true);
                if (first.matches) {
                    Model::Object* object = hitAsObject(first.hit);
                    if (multi) {
                        if (object->selected()) {
                            controller()->deselectObject(*object);
                        } else {
                            controller()->selectObject(*object);
                        }
                    } else {
                        controller()->deselectAllAndSelectObject(*object);
                    }
                } else {
                    controller()->deselectAll();
                }
            }
            
            return true;
        }
        
        bool SelectionTool::doMouseDoubleClick(const InputState& inputState) {
            return false;
        }
        
        bool SelectionTool::doStartMouseDrag(const InputState& inputState) {
            return false;
        }
        
        bool SelectionTool::doMouseDrag(const InputState& inputState) {
            return false;
        }
        
        void SelectionTool::doEndMouseDrag(const InputState& inputState) {
        }
        
        void SelectionTool::doCancelMouseDrag(const InputState& inputState) {
        }
    }
}
