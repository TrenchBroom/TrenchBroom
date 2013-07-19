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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SelectionTool.h"

#include "Controller/ControllerFacade.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "Model/Picker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class BrushHitFilter : public Model::HitFilter {
        public:
            inline bool matches(const Model::Hit& hit) const {
                return hit.type() == Model::Brush::BrushHit;
            }
        };

        class ObjectHitFilter : public Model::HitFilter {
        public:
            inline bool matches(const Model::Hit& hit) const {
                return (hit.type() == Model::Entity::EntityHit ||
                        hit.type() == Model::Brush::BrushHit);
            }
        };

        SelectionTool::SelectionTool(BaseTool* next, Controller::ControllerFacade& controller) :
        Tool(next),
        m_controller(controller) {}

        bool SelectionTool::doMouseUp(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const bool multi = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
            const bool faces = inputState.modifierKeysDown(ModifierKeys::MKShift);
            
            if (faces) {
                const Model::PickResult::FirstHit first = inputState.pickResult().firstHit(BrushHitFilter(), true);
                if (first.matches) {
                    Model::BrushFace* face = first.hit.object<Model::BrushFace*>();
                    if (multi) {
                        if (face->selected()) {
                            m_controller.deselectFace(face);
                        } else {
                            m_controller.selectFace(face);
                        }
                    } else {
                        m_controller.deselectAllAndSelectFace(face);
                    }
                } else {
                    m_controller.deselectAll();
                }
            } else {
                const Model::PickResult::FirstHit first = inputState.pickResult().firstHit(ObjectHitFilter(), false);
                if (first.matches) {
                    Model::Object* object;
                    if (multi) {
                        if (object->selected()) {
                            m_controller.deselectObject(object);
                        } else {
                            m_controller.selectObject(object);
                        }
                    } else {
                        m_controller.deselectAllAndSelectObject(object);
                    }
                } else {
                    m_controller.deselectAll();
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
