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
#include "Controller/InputController.h"
#include "Controller/MoveHandle.h"
#include "Controller/RotateObjectsTool.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/Face.h"
#include "Model/ModelUtils.h"
#include "Model/Picker.h"
#include "Renderer/BoxGuideRenderer.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Text/StringManager.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "Utility/List.h"

namespace TrenchBroom {
    namespace Controller {
        void SelectionTool::handleRenderFirst(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (controller().moveVerticesToolActive() || controller().clipToolActive())
                return;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;

            /*
            Model::Hit* moveHandleHit = inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter());
            Model::Hit* rotateHandleHit = inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter());
            Model::ObjectHit* objectHit = static_cast<Model::ObjectHit*>(inputState.pickResult().first(Model::HitType::ObjectHit, false, view().filter()));

            
            if (moveHandleHit != NULL || rotateHandleHit != NULL || (objectHit != NULL && objectHit->object().selected())) {
             */
                if (m_guideRenderer == NULL) {
                    BBox bounds = Model::objectBounds(entities, brushes);
                    m_guideRenderer = new Renderer::BoxGuideRenderer(bounds, document().picker(), view().filter(), document().sharedResources().stringManager());
                }
                m_guideRenderer->render(vbo, renderContext);
            //}
        }
        
        void SelectionTool::handleFreeRenderResources() {
            delete m_guideRenderer;
            m_guideRenderer = NULL;
        }

        bool SelectionTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft)
                return false;
            
            Model::Hit* hit = inputState.pickResult().first(Model::HitType::ObjectHit, false, view().filter());
            Command* command = NULL;
            Model::EditStateManager& editStateManager = document().editStateManager();
            
            if (hit != NULL) {
                bool multi = inputState.modifierKeys() == ModifierKeys::MKCtrlCmd;
                
                if (hit->type() == Model::HitType::EntityHit) {
                    Model::Entity& entity = static_cast<Model::EntityHit*>(hit)->entity();
                    if (multi) {
                        if (entity.selected())
                            command = ChangeEditStateCommand::deselect(document(), entity);
                        else
                            command = ChangeEditStateCommand::select(document(), entity);
                    } else {
                        command = ChangeEditStateCommand::replace(document(), entity);
                    }
                } else {
                    Model::Face& face = static_cast<Model::FaceHit*>(hit)->face();
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
                            if (editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                                command = ChangeEditStateCommand::select(document(), face);
                            else
                                command = ChangeEditStateCommand::select(document(), brush);
                        } else {
                            if (editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
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
                submitCommand(command);
                return true;
            }
            
            return false;
        }
        
        void SelectionTool::handleScroll(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd &&
                inputState.modifierKeys() != (ModifierKeys::MKCtrlCmd | ModifierKeys::MKShift))
                return;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                return;
            
            const Model::HitList hits = inputState.pickResult().hits(Model::HitType::ObjectHit, view().filter());
            if (hits.empty())
                return;
            
            bool appendSelection = inputState.modifierKeys() == (ModifierKeys::MKCtrlCmd | ModifierKeys::MKShift);
            size_t firstSelectionBlockStart = hits.size();
            size_t firstSelectionBlockEnd = hits.size();
            
            // find the index of the first selected hit
            for (size_t i = 0; i < hits.size() && firstSelectionBlockStart == hits.size(); i++) {
                Model::Hit* hit = hits[i];
                if (hit->type() == Model::HitType::EntityHit) {
                    Model::Entity& entity = static_cast<Model::EntityHit*>(hit)->entity();
                    if (entity.selected())
                        firstSelectionBlockStart = i;
                } else {
                    Model::Face& face = static_cast<Model::FaceHit*>(hit)->face();
                    Model::Brush& brush = *face.brush();
                    if (brush.selected())
                        firstSelectionBlockStart = i;
                }
            }
            
            // if we found a selected hit, find the index of the last selected hit in a contiguous block of selected
            // hits
            if (firstSelectionBlockStart < hits.size()) {
                for (size_t i = firstSelectionBlockStart; i < hits.size(); i++) {
                    Model::Hit* hit = hits[i];
                    if (hit->type() == Model::HitType::EntityHit) {
                        Model::Entity& entity = static_cast<Model::EntityHit*>(hit)->entity();
                        if (entity.selected())
                            firstSelectionBlockEnd = i;
                        else
                            break;
                    } else {
                        Model::Face& face = static_cast<Model::FaceHit*>(hit)->face();
                        Model::Brush& brush = *face.brush();
                        if (brush.selected())
                            firstSelectionBlockEnd = i;
                        else
                            break;
                    }
                }
            }
            
            Model::EntityList entities = appendSelection ? editStateManager.selectedEntities() : Model::EmptyEntityList;
            Model::BrushList brushes = appendSelection ? editStateManager.selectedBrushes() : Model::EmptyBrushList;
            
            // find the index of the object to select
            size_t selectionIndex = hits.size();
            if (inputState.scroll() > 0.0f) {
                if (firstSelectionBlockEnd < hits.size() - 1)
                    selectionIndex = firstSelectionBlockEnd + 1;
                else if (!appendSelection || (entities.empty() && brushes.empty()))
                    selectionIndex = 0;
            } else {
                if (firstSelectionBlockStart > 0)
                    selectionIndex = firstSelectionBlockStart - 1;
                else if (!appendSelection || (entities.empty() && brushes.empty()))
                    selectionIndex = hits.size() - 1;
            }
            
            if (selectionIndex < hits.size()) {
                Model::Hit* hit = hits[selectionIndex];
                if (hit->type() == Model::HitType::EntityHit) {
                    Model::Entity& entity = static_cast<Model::EntityHit*>(hit)->entity();
                    entities.push_back(&entity);
                } else {
                    Model::Face& face = static_cast<Model::FaceHit*>(hit)->face();
                    Model::Brush& brush = *face.brush();
                    brushes.push_back(&brush);
                }
            }

            assert(!entities.empty() || !brushes.empty());
            
            ChangeEditStateCommand* command = ChangeEditStateCommand::replace(document(), entities, brushes);
            submitCommand(command);
        }
        
        
        void SelectionTool::handleObjectsChange(InputState& inputState) {
            delete m_guideRenderer;
            m_guideRenderer = NULL;
        }
        
        void SelectionTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            delete m_guideRenderer;
            m_guideRenderer = NULL;
        }

        SelectionTool::SelectionTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        Tool(documentViewHolder, inputController, false),
        m_guideRenderer(NULL) {}
    }
}
