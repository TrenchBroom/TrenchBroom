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

#include "MoveVerticesTool.h"

#include "Controller/MoveEdgesCommand.h"
#include "Controller/MoveFacesCommand.h"
#include "Controller/MoveVerticesCommand.h"
#include "Controller/SplitEdgesCommand.h"
#include "Controller/SplitFacesCommand.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Renderer/LinesRenderer.h"
#include "Renderer/PointHandleHighlightFigure.h"
#include "Renderer/PointGuideRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveVerticesTool::isApplicable(InputState& inputState, Vec3f& hitPoint) {
            if ((inputState.mouseButtons() != MouseButtons::MBNone &&
                 inputState.mouseButtons() != MouseButtons::MBLeft) ||
                (inputState.modifierKeys() != ModifierKeys::MKNone &&
                 inputState.modifierKeys() != ModifierKeys::MKAlt &&
                 inputState.modifierKeys() != ModifierKeys::MKShift &&
                 inputState.modifierKeys() != (ModifierKeys::MKAlt | ModifierKeys::MKShift)))
                return false;
            
            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;
            hitPoint = hit->hitPoint();
            return true;
        }
        
        wxString MoveVerticesTool::actionName() {
            if (m_mode == VMMove || m_mode == VMSnap) {
                assert((m_handleManager.selectedVertexHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedEdgeHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedFaceHandles().empty() ? 0 : 1) == 1);
                
                if (!m_handleManager.selectedVertexHandles().empty())
                    return m_handleManager.selectedVertexHandles().size() == 1 ? wxT("Move Vertex") : wxT("Move Vertices");
                if (!m_handleManager.selectedEdgeHandles().empty())
                    return m_handleManager.selectedEdgeHandles().size() == 1 ? wxT("Move Edge") : wxT("Move Edges");
                return m_handleManager.selectedFaceHandles().size() == 1 ? wxT("Move Face") : wxT("Move Faces");
            }
            
            assert(m_handleManager.selectedVertexHandles().size() == 0 &&
                   ((m_handleManager.selectedEdgeHandles().size() == 1) ^
                    (m_handleManager.selectedFaceHandles().size() == 1))
                   );
            
            if (!m_handleManager.selectedEdgeHandles().empty())
                return wxT("Split Edge");
            return wxT("Split Face");
        }
        
        void MoveVerticesTool::startDrag(InputState& inputState) {
            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            assert(hit != NULL);
            m_dragHandlePosition = hit->vertex();
        }

        void MoveVerticesTool::snapDragDelta(InputState& inputState, Vec3f& delta) {
            if (m_mode == VMSnap) {
                Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit, true, view().filter()));
                if (hit != NULL && !m_handleManager.vertexHandleSelected(hit->vertex()))
                    delta = hit->vertex() - m_dragHandlePosition;
                else
                    delta = Vec3f::Null;
            } else {
                if ((inputState.modifierKeys() & ModifierKeys::MKShift) == 0) {
                    MoveTool::snapDragDelta(inputState, delta);
                } else {
                    const Vec3f targetPos = document().grid().snap(m_dragHandlePosition + delta);
                    delta = targetPos - m_dragHandlePosition;
                }
            }
        }

        MoveTool::MoveResult MoveVerticesTool::performMove(const Vec3f& delta) {
            return moveVertices(delta);
        }
        
        bool MoveVerticesTool::handleActivate(InputState& inputState) {
            m_mode = VMMove;
            m_handleManager.clear();
            m_handleManager.add(document().editStateManager().selectedBrushes());
            
            return true;
        }
        
        bool MoveVerticesTool::handleDeactivate(InputState& inputState) {
            m_handleManager.clear();
            return true;
        }
        
        bool MoveVerticesTool::handleIsModal(InputState& inputState) {
            return true;
        }

        void MoveVerticesTool::handlePick(InputState& inputState) {
            m_handleManager.pick(inputState.pickRay(), inputState.pickResult(), m_mode == VMSplit);
        }
        
        void MoveVerticesTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            MoveTool::handleRender(inputState, vbo, renderContext);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            if (dragType() == DTDrag) {
                Renderer::PointGuideRenderer guideRenderer(m_dragHandlePosition, document().picker(), view().filter());
                guideRenderer.setColor(prefs.getColor(Preferences::GuideColor));
                guideRenderer.render(vbo, renderContext);
            }
            
            m_handleManager.render(vbo, renderContext, m_mode == VMSplit);
            
            if (m_textRenderer == NULL) {
                m_textRenderer = new Renderer::Text::TextRenderer<Vec3f, Vec3f::LexicographicOrder>(document().sharedResources().stringManager());
                m_textRenderer->setFadeDistance(10000.0f);
            }
            m_textRenderer->clear();
            
            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (hit != NULL) {
                const Color& color = hit->type() == Model::HitType::VertexHandleHit ? prefs.getColor(Preferences::VertexHandleColor) : (hit->type() == Model::HitType::EdgeHandleHit ? prefs.getColor(Preferences::EdgeHandleColor) : prefs.getColor(Preferences::FaceHandleColor));
                const float radius = prefs.getFloat(Preferences::HandleRadius);
                const float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
                
                const String fontName = prefs.getString(Preferences::RendererFontName);
                const int fontSize = prefs.getInt(Preferences::RendererFontSize);
                assert(fontSize >= 0);
                const Renderer::Text::FontDescriptor font(fontName, static_cast<unsigned int>(fontSize));

                glDisable(GL_DEPTH_TEST);
                Renderer::PointHandleHighlightFigure highlightFigure(hit->vertex(), color, radius, scalingFactor);
                highlightFigure.render(vbo, renderContext);
                glEnable(GL_DEPTH_TEST);
                
                if (hit->type() == Model::HitType::VertexHandleHit) {
                    Renderer::Text::TextAnchor* anchor = new Renderer::Text::SimpleTextAnchor(hit->vertex() + Vec3f(0.0f, 0.0f, radius + 2.0f), Renderer::Text::Alignment::Bottom);
                    m_textRenderer->addString(hit->vertex(), font, hit->vertex().asString(), anchor);
                } else if (hit->type() == Model::HitType::EdgeHandleHit) {
                    Renderer::LinesRenderer linesRenderer;
                    linesRenderer.setColor(prefs.getColor(Preferences::EdgeHandleColor), prefs.getColor(Preferences::OccludedEdgeHandleColor));
                    
                    const Model::EdgeList& edges = m_handleManager.edges(hit->vertex());
                    Model::EdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                        const Model::Edge& edge = **edgeIt;
                        linesRenderer.add(edge.start->position, edge.end->position);
                    }
                    
                    linesRenderer.render(vbo, renderContext);
                } else if (hit->type() == Model::HitType::FaceHandleHit) {
                    Renderer::LinesRenderer linesRenderer;
                    linesRenderer.setColor(prefs.getColor(Preferences::FaceHandleColor), prefs.getColor(Preferences::OccludedFaceHandleColor));

                    const Model::FaceList& faces = m_handleManager.faces(hit->vertex());
                    Model::FaceList::const_iterator faceIt, faceEnd;
                    for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                        const Model::Face& face = **faceIt;
                        const Model::EdgeList& edges = face.edges();
                        Model::EdgeList::const_iterator edgeIt, edgeEnd;
                        for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                            const Model::Edge& edge = **edgeIt;
                            linesRenderer.add(edge.start->position, edge.end->position);
                        }
                    }

                    linesRenderer.render(vbo, renderContext);
                }
                
                const Color& textColor = prefs.getColor(Preferences::InfoOverlayTextColor);
                const Color& backgroundColor = prefs.getColor(Preferences::InfoOverlayBackgroundColor);
                Renderer::ShaderProgram& textShader = renderContext.shaderManager().shaderProgram(Renderer::Shaders::TextShader);
                Renderer::ShaderProgram& backgroundShader = renderContext.shaderManager().shaderProgram(Renderer::Shaders::TextBackgroundShader);
                
                glDisable(GL_DEPTH_TEST);
                m_textRenderer->render(renderContext, m_textFilter, textShader, textColor, backgroundShader, backgroundColor);
                glEnable(GL_DEPTH_TEST);
            }
        }

        void MoveVerticesTool::handleFreeRenderResources() {
            MoveTool::handleFreeRenderResources();
            m_handleManager.freeRenderResources();
            delete m_textRenderer;
            m_textRenderer = NULL;
        }

        bool MoveVerticesTool::handleMouseDown(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone &&
                 inputState.modifierKeys() != ModifierKeys::MKCtrlCmd &&
                 inputState.modifierKeys() != ModifierKeys::MKAlt &&
                 inputState.modifierKeys() != ModifierKeys::MKShift &&
                 inputState.modifierKeys() != (ModifierKeys::MKAlt | ModifierKeys::MKShift)))
                return false;

            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;

            if (hit->type() == Model::HitType::VertexHandleHit) {
                m_handleManager.deselectEdgeHandles();
                m_handleManager.deselectFaceHandles();
                bool selected = m_handleManager.vertexHandleSelected(hit->vertex());
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    if (selected)
                        m_handleManager.deselectVertexHandle(hit->vertex());
                    else
                        m_handleManager.selectVertexHandle(hit->vertex());
                } else if (!selected) {
                    m_handleManager.deselectAll();
                    m_handleManager.selectVertexHandle(hit->vertex());
                }
            } else if (hit->type() == Model::HitType::EdgeHandleHit) {
                m_handleManager.deselectVertexHandles();
                m_handleManager.deselectFaceHandles();
                bool selected = m_handleManager.edgeHandleSelected(hit->vertex());
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    if (selected)
                        m_handleManager.deselectEdgeHandle(hit->vertex());
                    else
                        m_handleManager.selectEdgeHandle(hit->vertex());
                } else if (!selected) {
                    m_handleManager.deselectAll();
                    m_handleManager.selectEdgeHandle(hit->vertex());
                }
            } else if (hit->type() == Model::HitType::FaceHandleHit) {
                m_handleManager.deselectVertexHandles();
                m_handleManager.deselectEdgeHandles();
                bool selected = m_handleManager.faceHandleSelected(hit->vertex());
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    if (selected)
                        m_handleManager.deselectFaceHandle(hit->vertex());
                    else
                        m_handleManager.selectFaceHandle(hit->vertex());
                } else if (!selected) {
                    m_handleManager.deselectAll();
                    m_handleManager.selectFaceHandle(hit->vertex());
                }
            }
            return true;
        }

        bool MoveVerticesTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone &&
                 inputState.modifierKeys() != ModifierKeys::MKCtrlCmd &&
                 inputState.modifierKeys() != ModifierKeys::MKAlt &&
                 inputState.modifierKeys() != ModifierKeys::MKShift &&
                 inputState.modifierKeys() != (ModifierKeys::MKAlt | ModifierKeys::MKShift)))
                return false;
            
            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (hit != NULL)
                return true;
            
            if (m_handleManager.selectedVertexHandles().empty() &&
                m_handleManager.selectedEdgeHandles().empty() &&
                m_handleManager.selectedFaceHandles().empty())
                return false;
            
            m_handleManager.deselectAll();
            m_mode = VMMove;
            return true;
        }

        bool MoveVerticesTool::handleMouseDClick(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone &&
                 inputState.modifierKeys() != ModifierKeys::MKAlt &&
                 inputState.modifierKeys() != ModifierKeys::MKShift &&
                 inputState.modifierKeys() != (ModifierKeys::MKAlt | ModifierKeys::MKShift)))
                return false;

            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;
            
            if (hit->type() == Model::HitType::VertexHandleHit) {
                m_handleManager.deselectAll();
                m_handleManager.selectVertexHandle(hit->vertex());
                m_mode = VMSnap;
            } else if (hit->type() == Model::HitType::EdgeHandleHit) {
                m_handleManager.deselectAll();
                m_handleManager.selectEdgeHandle(hit->vertex());
                m_mode = VMSplit;
            } else if (hit->type() == Model::HitType::FaceHandleHit) {
                m_handleManager.deselectAll();
                m_handleManager.selectFaceHandle(hit->vertex());
                m_mode = VMSplit;
            }
            return true;
        }

        void MoveVerticesTool::handleMouseMove(InputState& inputState) {
        }

        void MoveVerticesTool::handleObjectsChange(InputState& inputState) {
            if (active() && !m_ignoreObjectChanges) {
                m_handleManager.clear();
                m_handleManager.add(document().editStateManager().selectedBrushes());
            }
        }
        
        void MoveVerticesTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            if (active()) {
                if (document().editStateManager().selectedBrushes().empty()) {
                    m_handleManager.clear();
                } else {
                    m_handleManager.remove(changeSet.brushesFrom(Model::EditState::Selected));
                    m_handleManager.add(changeSet.brushesTo(Model::EditState::Selected));
                }
            }
        }

        MoveVerticesTool::MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, float axisLength, float planeRadius, float vertexSize) :
        MoveTool(documentViewHolder, inputController, true),
        m_mode(VMMove),
        m_ignoreObjectChanges(false),
        m_textRenderer(NULL) {}

        bool MoveVerticesTool::hasSelection() {
            return !m_handleManager.selectedVertexHandles().empty() || !m_handleManager.selectedEdgeHandles().empty() || !m_handleManager.selectedFaceHandles().empty();
            
        }
        
        MoveVerticesTool::MoveResult MoveVerticesTool::moveVertices(const Vec3f& delta) {
            m_ignoreObjectChanges = true;
            if (m_mode == VMMove || m_mode == VMSnap) {
                assert((m_handleManager.selectedVertexHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedEdgeHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedFaceHandles().empty() ? 0 : 1) == 1);
                
                if (!m_handleManager.selectedVertexHandles().empty()) {
                    MoveVerticesCommand* command = MoveVerticesCommand::moveVertices(document(), m_handleManager.selectedVertexHandles(), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        
                        const Vec3f::Set& vertices = command->vertices();
                        if (vertices.empty() || m_mode == VMSnap) {
                            m_ignoreObjectChanges = false;
                            return Conclude;
                        }
                        
                        m_handleManager.selectVertexHandles(command->vertices());
                        m_ignoreObjectChanges = false;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectVertexHandles(command->vertices());
                        m_ignoreObjectChanges = false;
                        return Deny;
                    }
                } else if (!m_handleManager.selectedEdgeHandles().empty()) {
                    MoveEdgesCommand* command = MoveEdgesCommand::moveEdges(document(), m_handleManager.selectedEdgeHandles(), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectEdgeHandles(command->edges());
                        m_ignoreObjectChanges = false;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectEdgeHandles(command->edges());
                        m_ignoreObjectChanges = false;
                        return Deny;
                    }
                } else if (!m_handleManager.selectedFaceHandles().empty()) {
                    MoveFacesCommand* command = MoveFacesCommand::moveFaces(document(), m_handleManager.selectedFaceHandles(), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectFaceHandles(command->faces());
                        m_ignoreObjectChanges = false;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectFaceHandles(command->faces());
                        m_ignoreObjectChanges = false;
                        return Deny;
                    }
                }
            } else {
                assert(m_handleManager.selectedVertexHandles().size() == 0 &&
                       ((m_handleManager.selectedEdgeHandles().size() == 1) ^
                        (m_handleManager.selectedFaceHandles().size() == 1))
                       );
                
                if (!m_handleManager.selectedEdgeHandles().empty()) {
                    const Vec3f position = m_handleManager.selectedEdgeHandles().begin()->first;
                    
                    SplitEdgesCommand* command = SplitEdgesCommand::splitEdges(document(), m_handleManager.edges(position), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        const Vec3f::Set& vertices = command->vertices();
                        assert(!vertices.empty());
                        m_handleManager.selectVertexHandles(command->vertices());
                        m_mode = VMMove;
                        m_ignoreObjectChanges = false;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectEdgeHandle(position);
                        m_ignoreObjectChanges = false;
                        return Deny;
                    }
                } else if (!m_handleManager.selectedFaceHandles().empty()) {
                    const Vec3f position = m_handleManager.selectedFaceHandles().begin()->first;
                    
                    SplitFacesCommand* command = SplitFacesCommand::splitFaces(document(), m_handleManager.faces(position), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        const Vec3f::Set& vertices = command->vertices();
                        assert(!vertices.empty());
                        m_handleManager.selectVertexHandles(command->vertices());
                        m_mode = VMMove;
                        m_ignoreObjectChanges = false;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectFaceHandle(position);
                        m_ignoreObjectChanges = false;
                        m_ignoreObjectChanges = false;
                        return Deny;
                    }
                }
            }
            m_ignoreObjectChanges = false;
            return Continue;
        }
    }
}
