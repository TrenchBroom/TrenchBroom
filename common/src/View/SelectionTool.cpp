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

#include "SelectionTool.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/Entity.h"
#include "Model/ModelFilter.h"
#include "Model/Object.h"
#include "Model/Picker.h"
#include "Renderer/RenderContext.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SelectionTool::SelectionTool(MapDocumentWPtr document, ControllerWPtr controller) :
        ToolImpl(document, controller) {}

        bool SelectionTool::doMouseUp(const InputState& inputState) {
            if (!handleClick(inputState))
                return false;
            
            if (isFaceClick(inputState)) {
                const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitAsFace(hit);
                    if (isMultiClick(inputState)) {
                        const bool objects = !document()->selectedObjects().empty();
                        if (objects) {
                            const Model::Brush* brush = face->parent();
                            if (brush->selected())
                                controller()->deselectFace(face);
                            else
                                controller()->selectFaceAndKeepBrushes(face);
                        } else {
                            if (face->selected())
                                controller()->deselectFace(face);
                            else
                                controller()->selectFace(face);
                        }
                    } else {
                        controller()->deselectAllAndSelectFace(face);
                    }
                } else {
                    controller()->deselectAll();
                }
            } else {
                const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Entity::EntityHit | Model::Brush::BrushHit, document()->filter(), true);
                if (hit.isMatch()) {
                    Model::Object* object = Model::hitAsObject(hit);
                    if (isMultiClick(inputState)) {
                        if (object->selected())
                            controller()->deselectObject(object);
                        else
                            controller()->selectObject(object);
                    } else {
                        controller()->deselectAllAndSelectObject(object);
                    }
                } else {
                    controller()->deselectAll();
                }
            }
            
            return true;
        }
        
        bool SelectionTool::doMouseDoubleClick(const InputState& inputState) {
            if (!handleClick(inputState))
                return false;
            
            if (isFaceClick(inputState)) {
                const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitAsFace(hit);
                    const Model::Brush* brush = face->parent();
                    if (isMultiClick(inputState))
                        controller()->selectFaces(brush->faces());
                    else
                        controller()->deselectAllAndSelectFaces(brush->faces());
                }
            } else {
                const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
                if (hit.isMatch()) {
                    Model::Brush* brush = Model::hitAsBrush(hit);
                    Model::Entity* parent = brush->entity();
                    if (!parent->worldspawn()) {
                        const Model::ObjectList objects = VectorUtils::cast<Model::Object*>(parent->brushes());
                        if (isMultiClick(inputState))
                            controller()->selectObjects(objects);
                        else
                            controller()->deselectAllAndSelectObjects(objects);
                    }
                }
            }
            
            return true;
        }
        
        bool SelectionTool::handleClick(const InputState& inputState) const {
            return inputState.mouseButtonsPressed(MouseButtons::MBLeft);
        }
        
        bool SelectionTool::isFaceClick(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKShift);
        }

        bool SelectionTool::isMultiClick(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
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

        void SelectionTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Entity::EntityHit | Model::Brush::BrushHit, document()->filter(), true);
            if (hit.isMatch() && Model::hitAsObject(hit)->selected())
                renderContext.setShowSelectionGuide();
        }
    }
}
