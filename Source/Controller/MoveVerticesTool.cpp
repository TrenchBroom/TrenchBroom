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

#include "Controller/ChangeEditStateCommand.h"
#include "Controller/Command.h"
#include "Controller/MoveEdgesCommand.h"
#include "Controller/MoveFacesCommand.h"
#include "Controller/MoveVerticesCommand.h"
#include "Controller/PreferenceChangeEvent.h"
#include "Controller/RebuildBrushGeometryCommand.h"
#include "Controller/SplitEdgesCommand.h"
#include "Controller/SplitFacesCommand.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Model/Map.h"
#include "Renderer/LinesRenderer.h"
#include "Renderer/PointHandleHighlightFigure.h"
#include "Renderer/PointGuideRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Text/FontManager.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const float MoveVerticesTool::MaxVertexDistance = 0.25f;
        
        MoveVerticesTool::HandleHitList MoveVerticesTool::firstHits(Model::PickResult& pickResult) const {
            HandleHitList list;
            Model::BrushSet brushes;

            Model::VertexHandleHit* firstHit = static_cast<Model::VertexHandleHit*>(pickResult.first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (firstHit != NULL) {
                list.push_back(firstHit);
                
                const Model::BrushList& firstHandleBrushes = m_handleManager.brushes(firstHit->vertex());
                brushes.insert(firstHandleBrushes.begin(), firstHandleBrushes.end());
                
                const Model::HitList allHits = pickResult.hits(firstHit->type(), view().filter());
                Model::HitList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = allHits.begin(), vertexEnd = allHits.end(); vertexIt != vertexEnd; ++vertexIt) {
                    Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(*vertexIt);
                    if (hit != firstHit && firstHit->vertex().squaredDistanceTo(hit->vertex()) < MaxVertexDistance * MaxVertexDistance) {
                        bool newBrush = true;
                        const Model::BrushList& handleBrushes = m_handleManager.brushes(hit->vertex());
                        Model::BrushList::const_iterator brushIt, brushEnd;
                        for (brushIt = handleBrushes.begin(), brushEnd = handleBrushes.end(); brushIt != brushEnd && newBrush; ++brushIt) {
                            Model::Brush* brush = *brushIt;
                            newBrush = brushes.count(brush) == 0;
                        }
                        
                        if (newBrush) {
                            list.push_back(hit);
                            brushes.insert(handleBrushes.begin(), handleBrushes.end());
                        }
                    }
                    
                }
            }
            
            return list;
        }

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
        
        wxString MoveVerticesTool::actionName(InputState& inputState) {
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
        
        void MoveVerticesTool::rebuildBrushGeometry() {
            Model::BrushList brushesToRebuild;

            // previouly, this was rebuilding document().editStateManager().selectedBrushes().
            // this list was empty when exiting the vertex tool by clicking to deselect all
            // (which is processed by the selection tool), which would lead to brushes
            // not being rebuilt.
            //
            // now we flag brushes in Brush::moveVertices, Brush::moveEdges, Brush::moveFaces,
            // Brush::splitEdge, Brush::splitFace, and rebuild all of them here.

            Model::EntityList::const_iterator entityIt, entityEnd;
            const Model::EntityList &mapEnts = document().map().entities();
            for (entityIt = mapEnts.begin(), entityEnd = mapEnts.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& brushes = entity.brushes();
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush *brush = *brushIt;
                    if (brush->needsRebuild()) {
                        brushesToRebuild.push_back(brush);
                        brush->setNeedsRebuild(false);
                    }
                }
            }

            document().console().info("Rebuilding geometry for %d brushes\n", static_cast<int>(brushesToRebuild.size()));
            RebuildBrushGeometryCommand* command = RebuildBrushGeometryCommand::rebuildGeometry(document(), brushesToRebuild, m_changeCount);
            submitCommand(command);
        }
        
        bool MoveVerticesTool::handleActivate(InputState& inputState) {
            m_mode = VMMove;
            m_handleManager.clear();
            m_handleManager.add(document().editStateManager().selectedBrushes());
            m_changeCount = 0;
            return true;
        }

        bool MoveVerticesTool::handleDeactivate(InputState& inputState) {
            m_handleManager.clear();
            
            if (m_changeCount > 0) {
                rebuildBrushGeometry();
            }
        
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
            
            if (m_textRenderer == NULL)
                initTextRenderer();
            m_textRenderer->clear();
            
            if (dragType() == DTDrag) {
                renderGuide(vbo, renderContext, m_dragHandlePosition);
                renderHighlight(vbo, renderContext, m_dragHandlePosition);
                addVertexPositionText(m_dragHandlePosition);
            }
            
            m_handleManager.render(vbo, renderContext, m_mode == VMSplit);
            
            HandleHitList hits = firstHits(inputState.pickResult());
            if (!hits.empty()) {
                const Model::VertexHandleHit* firstHit = hits.front();
                const Model::HitType::Type hitType = firstHit->type();
                renderHighlight(vbo, renderContext, firstHit->vertex());

                if (hitType == Model::HitType::VertexHandleHit) {
                    if (dragType() == DTNone)
                        addVertexPositionText(firstHit->vertex());
                } else {
                    renderHighlightEdges(vbo, renderContext, hitType, hits);
                }
 
            }
            renderText(renderContext);
        }

        void MoveVerticesTool::initTextRenderer() {
            assert(m_textRenderer == NULL);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const String fontName = prefs.getString(Preferences::RendererFontName);
            const int fontSize = prefs.getInt(Preferences::RendererFontSize);
            assert(fontSize >= 0);
            const Renderer::Text::FontDescriptor fontDescriptor(fontName, static_cast<unsigned int>(fontSize));
            
            Renderer::Text::TexturedFont* font = document().sharedResources().fontManager().font(fontDescriptor);
            m_textRenderer = new Renderer::Text::TextRenderer<Vec3f, Vec3f::LexicographicOrder>(*font);
            m_textRenderer->setFadeDistance(10000.0f);
        }

        void MoveVerticesTool::renderGuide(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, const Vec3f& position) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& color = prefs.getColor(Preferences::GuideColor);

            Renderer::PointGuideRenderer guideRenderer(m_dragHandlePosition, document().picker(), view().filter());
            guideRenderer.setColor(color);
            guideRenderer.render(vbo, renderContext);
        }

        void MoveVerticesTool::renderHighlight(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, const Vec3f& position) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& color = prefs.getColor(Preferences::HandleHighlightColor);
            const float radius = prefs.getFloat(Preferences::HandleRadius);
            const float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            
            glDisable(GL_DEPTH_TEST);
            Renderer::PointHandleHighlightFigure highlightFigure(position, color, radius, scalingFactor);
            highlightFigure.render(vbo, renderContext);
            glEnable(GL_DEPTH_TEST);
        }
        
        void MoveVerticesTool::addVertexPositionText(const Vec3f& position) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const float radius = prefs.getFloat(Preferences::HandleRadius);
            
            Renderer::Text::TextAnchor::Ptr anchor(new Renderer::Text::SimpleTextAnchor(position + Vec3f(0.0f, 0.0f, radius + 2.0f), Renderer::Text::Alignment::Bottom));
            m_textRenderer->addString(position, position.asString(), anchor);
        }
        
        void MoveVerticesTool::renderHighlightEdges(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, const Model::HitType::Type firstHitType, HandleHitList& hits) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::LinesRenderer linesRenderer;
            linesRenderer.setColor(prefs.getColor(Preferences::EdgeHandleColor), prefs.getColor(Preferences::OccludedEdgeHandleColor));
            
            if (firstHitType == Model::HitType::EdgeHandleHit)
                gatherEdgeVertices(linesRenderer, hits);
            else
                gatherFaceEdgeVertices(linesRenderer, hits);
            
            linesRenderer.render(vbo, renderContext);
        }
        
        void MoveVerticesTool::gatherEdgeVertices(Renderer::LinesRenderer& linesRenderer, HandleHitList& hits) {
            HandleHitList::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Model::VertexHandleHit* hit = *it;
                const Model::EdgeList& edges = m_handleManager.edges(hit->vertex());
                
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge& edge = **edgeIt;
                    linesRenderer.add(edge.start->position, edge.end->position);
                }
            }
        }
        
        void MoveVerticesTool::gatherFaceEdgeVertices(Renderer::LinesRenderer& linesRenderer, HandleHitList& hits) {
            HandleHitList::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Model::VertexHandleHit* hit = *it;
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
            }
        }
        
        void MoveVerticesTool::renderText(Renderer::RenderContext& renderContext) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& textColor = prefs.getColor(Preferences::InfoOverlayTextColor);
            const Color& backgroundColor = prefs.getColor(Preferences::InfoOverlayBackgroundColor);
            Renderer::ShaderProgram& textShader = renderContext.shaderManager().shaderProgram(Renderer::Shaders::TextShader);
            Renderer::ShaderProgram& backgroundShader = renderContext.shaderManager().shaderProgram(Renderer::Shaders::TextBackgroundShader);
            
            glDisable(GL_DEPTH_TEST);
            m_textRenderer->render(renderContext, m_textFilter, textShader, textColor, backgroundShader, backgroundColor);
            glEnable(GL_DEPTH_TEST);
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

            HandleHitList hits = firstHits(inputState.pickResult());
            if (hits.empty())
                return false;
            
            const Model::VertexHandleHit* firstHit = hits.front();
            const Model::HitType::Type hitType = firstHit->type();
            
            if (hitType == Model::HitType::VertexHandleHit) {
                m_handleManager.deselectEdgeHandles();
                m_handleManager.deselectFaceHandles();
                
                size_t selected = 0;
                HandleHitList::const_iterator it, end;
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::VertexHandleHit* hit = *it;
                    if (m_handleManager.vertexHandleSelected(hit->vertex()))
                        selected++;
                }
                
                if (selected < hits.size()) {
                    if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                        m_handleManager.deselectAll();
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::VertexHandleHit* hit = *it;
                        m_handleManager.selectVertexHandle(hit->vertex());
                    }
                } else {
                    if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                        for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                            const Model::VertexHandleHit* hit = *it;
                            m_handleManager.deselectVertexHandle(hit->vertex());
                        }
                    }
                }
            } else if (hitType == Model::HitType::EdgeHandleHit) {
                m_handleManager.deselectVertexHandles();
                m_handleManager.deselectFaceHandles();
                
                size_t selected = 0;
                HandleHitList::const_iterator it, end;
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::VertexHandleHit* hit = *it;
                    if (m_handleManager.edgeHandleSelected(hit->vertex()))
                        selected++;
                }
                
                if (selected < hits.size()) {
                    if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                        m_handleManager.deselectAll();
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::VertexHandleHit* hit = *it;
                        m_handleManager.selectEdgeHandle(hit->vertex());
                    }
                } else {
                    if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                        for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                            const Model::VertexHandleHit* hit = *it;
                            m_handleManager.deselectEdgeHandle(hit->vertex());
                        }
                    }
                }
            } else if (hitType == Model::HitType::FaceHandleHit) {
                m_handleManager.deselectVertexHandles();
                m_handleManager.deselectEdgeHandles();

                size_t selected = 0;
                HandleHitList::const_iterator it, end;
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::VertexHandleHit* hit = *it;
                    if (m_handleManager.faceHandleSelected(hit->vertex()))
                        selected++;
                }
                
                if (selected < hits.size()) {
                    if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                        m_handleManager.deselectAll();
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::VertexHandleHit* hit = *it;
                        m_handleManager.selectFaceHandle(hit->vertex());
                    }
                } else {
                    if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                        for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                            const Model::VertexHandleHit* hit = *it;
                            m_handleManager.deselectFaceHandle(hit->vertex());
                        }
                    }
                }
            }
            
            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::MoveVerticesToolChange, document());
            submitCommand(command, false);
            
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
            
            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::MoveVerticesToolChange, document());
            submitCommand(command, false);
            
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
            
            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::MoveVerticesToolChange, document());
            submitCommand(command, false);
            
            return true;
        }

        bool MoveVerticesTool::handleNavigateUp(InputState& inputState) {
            assert(active());
            if (hasSelection()) {
                m_mode = VMMove;
                m_handleManager.deselectAll();
                return true;
            }
            return false;
        }

        void MoveVerticesTool::handleUpdate(const Command& command, InputState& inputState) {
            if (active()) {
                switch (command.type()) {
                    case Controller::Command::LoadMap:
                    case Controller::Command::ClearMap:
                    case Controller::Command::TransformObjects:
                    case Controller::Command::ResizeBrushes:
                    case Controller::Command::SnapVertices:
                        m_handleManager.clear();
                        m_handleManager.add(document().editStateManager().selectedBrushes());
                        break;
                    case Controller::Command::ChangeEditState:
                        if (document().editStateManager().selectedBrushes().empty()) {
                            m_handleManager.clear();
                        } else {
                            const ChangeEditStateCommand& changeEditStateCommand = static_cast<const ChangeEditStateCommand&>(command);
                            const Model::EditStateChangeSet& changeSet = changeEditStateCommand.changeSet();
                            m_handleManager.remove(changeSet.brushesFrom(Model::EditState::Selected));
                            m_handleManager.add(changeSet.brushesTo(Model::EditState::Selected));
                        }
                        break;
                    case Controller::Command::PreferenceChange: {
                        const Controller::PreferenceChangeEvent& preferenceChangeEvent = static_cast<const Controller::PreferenceChangeEvent&>(command);
                        if (preferenceChangeEvent.isPreferenceChanged(Preferences::RendererInstancingMode))
                            m_handleManager.recreateRenderers();
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        MoveVerticesTool::MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, float axisLength, float planeRadius, float vertexSize) :
        MoveTool(documentViewHolder, inputController, true),
        m_mode(VMMove),
        m_changeCount(0),
        m_textRenderer(NULL) {}

        MoveVerticesTool::MoveResult MoveVerticesTool::moveVertices(const Vec3f& delta) {
            if (m_mode == VMMove || m_mode == VMSnap) {
                assert((m_handleManager.selectedVertexHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedEdgeHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedFaceHandles().empty() ? 0 : 1) == 1);
                
                if (!m_handleManager.selectedVertexHandles().empty()) {
                    MoveVerticesCommand* command = MoveVerticesCommand::moveVertices(document(), m_handleManager, delta);
                    if (submitCommand(command)) {
                        if (!command->hasRemainingVertices() || m_mode == VMSnap)
                            return Conclude;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        return Deny;
                    }
                } else if (!m_handleManager.selectedEdgeHandles().empty()) {
                    MoveEdgesCommand* command = MoveEdgesCommand::moveEdges(document(), m_handleManager, delta);
                    if (submitCommand(command)) {
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        return Deny;
                    }
                } else if (!m_handleManager.selectedFaceHandles().empty()) {
                    MoveFacesCommand* command = MoveFacesCommand::moveFaces(document(), m_handleManager, delta);
                    if (submitCommand(command)) {
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        return Deny;
                    }
                }
            } else {
                assert(m_handleManager.selectedVertexHandles().size() == 0 &&
                       ((m_handleManager.selectedEdgeHandles().size() == 1) ^
                        (m_handleManager.selectedFaceHandles().size() == 1))
                       );
                
                if (!m_handleManager.selectedEdgeHandles().empty()) {
                    SplitEdgesCommand* command = SplitEdgesCommand::splitEdges(document(), m_handleManager, delta);
                    if (submitCommand(command)) {
                        m_mode = VMMove;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        return Deny;
                    }
                } else if (!m_handleManager.selectedFaceHandles().empty()) {
                    SplitFacesCommand* command = SplitFacesCommand::splitFaces(document(), m_handleManager, delta);
                    if (submitCommand(command)) {
                        m_mode = VMMove;
                        m_dragHandlePosition += delta;
                        return Continue;
                    } else {
                        return Deny;
                    }
                }
            }
            return Continue;
        }
    }
}
