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
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "Utility/List.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        void SelectionTool::handleRenderOverlay(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if ((inputState.modifierKeys() & ModifierKeys::MKShift) == 0)
                return;
            
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
            if (hit == NULL)
                return;
            
            Model::Face& face = hit->face();
            
            Model::FaceList::const_iterator faceIt, faceEnd;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::VertexArray edgeArray(vbo, GL_LINES, static_cast<unsigned int>(2 * face.edges().size()), Renderer::Attribute::position3f());
            Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);
            
            const Model::EdgeList& edges = face.edges();
            Model::EdgeList::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                const Model::Edge& edge = **eIt;
                edgeArray.addAttribute(edge.start->position);
                edgeArray.addAttribute(edge.end->position);
            }
            
            Renderer::glSetEdgeOffset(0.3f);
            
            Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);
            Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::EdgeShader);
            
            glDisable(GL_DEPTH_TEST);
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::ResizeBrushFaceColor));
            edgeArray.render();
            glEnable(GL_DEPTH_TEST);
            
            Renderer::glResetEdgeOffset();
        }

        bool SelectionTool::handleMouseDClick(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft)
                return false;
            
            if ((inputState.modifierKeys() & ModifierKeys::MKShift) == 0)
                return false;
            
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
            if (hit == NULL)
                return false;
            
            Command* command = NULL;
            bool multi = (inputState.modifierKeys() & ModifierKeys::MKCtrlCmd) != 0;

            Model::Face& face = hit->face();
            Model::Brush& brush = *face.brush();
            Model::FaceList selectFaces;
            
            const Model::FaceList& brushFaces = brush.faces();
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = brushFaces.begin(), faceEnd = brushFaces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face& brushFace = **faceIt;
                if (!brushFace.selected())
                    selectFaces.push_back(&brushFace);
            }
            
            if (multi)
                command = ChangeEditStateCommand::select(document(), brush.faces());
            else
                command = ChangeEditStateCommand::replace(document(), brush.faces());

            submitCommand(command);
            return true;
        }

        bool SelectionTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft)
                return false;
            
            Command* command = NULL;
            bool multi = (inputState.modifierKeys() & ModifierKeys::MKCtrlCmd) != 0;
            bool faceMode = (inputState.modifierKeys() & ModifierKeys::MKShift) != 0;

            if (faceMode) {
                Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
                if (hit != NULL) {
                    Model::Face& face = hit->face();
                    if (multi) {
                        if (face.selected()) {
                            command = ChangeEditStateCommand::deselect(document(), face);
                        } else {
                            command = ChangeEditStateCommand::select(document(), face);
                        }
                    } else {
                        command = ChangeEditStateCommand::replace(document(), face);
                    }
                } else {
                    command = ChangeEditStateCommand::deselectAll(document());
                }
            } else {
                Model::ObjectHit* hit = static_cast<Model::ObjectHit*>(inputState.pickResult().first(Model::HitType::ObjectHit, false, view().filter()));
                if (hit != NULL) {
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
                        
                        if (multi) {
                            if (brush.selected())
                                command = ChangeEditStateCommand::deselect(document(), brush);
                            else
                                command = ChangeEditStateCommand::select(document(), brush);
                        } else {
                            command = ChangeEditStateCommand::replace(document(), brush);
                        }
                    }
                } else {
                    command = ChangeEditStateCommand::deselectAll(document());
                }
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

        SelectionTool::SelectionTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        Tool(documentViewHolder, inputController, false) {}
    }
}
