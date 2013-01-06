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
#include "Renderer/PointHandleHighlightFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        void MoveVerticesTool::updateMoveHandle(InputState& inputState) {
            Vec3f closestVertex;
            float closestSquaredDistance = std::numeric_limits<float>::max();
            
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            const Model::VertexToBrushesMap& selectedVertexHandles = m_handleManager.selectedVertexHandles();
            for (vIt = selectedVertexHandles.begin(), vEnd = selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                float distanceToClosestPoint;
                float squaredDistance = inputState.pickRay().squaredDistanceToPoint(position, distanceToClosestPoint);
                if (squaredDistance < closestSquaredDistance) {
                    closestSquaredDistance = squaredDistance;
                    closestVertex = position;
                }
            }
            
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            const Model::VertexToEdgesMap& selectedEdgeHandles = m_handleManager.selectedEdgeHandles();
            for (eIt = selectedEdgeHandles.begin(), eEnd = selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3f& position = eIt->first;
                float distanceToClosestPoint;
                float squaredDistance = inputState.pickRay().squaredDistanceToPoint(position, distanceToClosestPoint);
                if (squaredDistance < closestSquaredDistance) {
                    closestSquaredDistance = squaredDistance;
                    closestVertex = position;
                }
            }
            
            Model::VertexToFacesMap::const_iterator fIt, fEnd;
            const Model::VertexToFacesMap& selectedFaceHandles = m_handleManager.selectedFaceHandles();
            for (fIt = selectedFaceHandles.begin(), fEnd = selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3f& position = fIt->first;
                float distanceToClosestPoint;
                float squaredDistance = inputState.pickRay().squaredDistanceToPoint(position, distanceToClosestPoint);
                if (squaredDistance < closestSquaredDistance) {
                    closestSquaredDistance = squaredDistance;
                    closestVertex = position;
                }
            }
            
            m_moveHandle.setEnabled(closestSquaredDistance <= m_moveHandle.axisLength() * m_moveHandle.axisLength());
            m_moveHandle.setPosition(closestVertex);
        }

        bool MoveVerticesTool::handleActivate(InputState& inputState) {
            m_mode = VMMove;
            m_handleManager.clear();
            m_handleManager.add(document().editStateManager().selectedBrushes());
            updateMoveHandle(inputState);
            
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
            Model::MoveHandleHit* moveHandleHit = m_moveHandle.pick(inputState.pickRay());
            if (moveHandleHit != NULL)
                inputState.pickResult().add(moveHandleHit);
            if (!m_moveHandle.locked())
                m_moveHandle.setLastHit(moveHandleHit);

            m_handleManager.pick(inputState.pickRay(), inputState.pickResult(), m_mode == VMSplit);
        }
        
        void MoveVerticesTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::MoveHandleHit* moveHandleHit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            m_moveHandle.render(moveHandleHit, vbo, renderContext);
            m_handleManager.render(vbo, renderContext, m_mode == VMSplit);
            
            Model::VertexHandleHit* vertexHandleHit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (vertexHandleHit != NULL) {
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const Color& color = prefs.getColor(Preferences::VertexHandleColor);
                const float radius = prefs.getFloat(Preferences::HandleRadius);
                const float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
                
                glDisable(GL_DEPTH_TEST);
                Renderer::PointHandleHighlightFigure highlightFigure(vertexHandleHit->vertex(), color, radius, scalingFactor);
                highlightFigure.render(vbo, renderContext);
                glEnable(GL_DEPTH_TEST);
            }
        }

        void MoveVerticesTool::handleFreeRenderResources() {
            m_handleManager.freeRenderResources();
        }

        bool MoveVerticesTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKCtrlCmd))
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
                } else {
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
                } else {
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
                } else {
                    m_handleManager.deselectAll();
                    m_handleManager.selectFaceHandle(hit->vertex());
                }
            }
            m_mode = VMMove;
            
            updateMoveHandle(inputState);
            return true;
        }

        bool MoveVerticesTool::handleMouseDClick(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;

            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::EdgeHandleHit | Model::HitType::FaceHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;
            
            m_handleManager.deselectAll();
            if (hit->type() == Model::HitType::EdgeHandleHit)
                m_handleManager.selectEdgeHandle(hit->vertex());
            else
                m_handleManager.selectFaceHandle(hit->vertex());
            m_mode = VMSplit;
            
            updateMoveHandle(inputState);
            return true;
        }

        void MoveVerticesTool::handleMouseMove(InputState& inputState) {
            updateMoveHandle(inputState);
        }

        bool MoveVerticesTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKAlt))
                return false;
            
            Model::MoveHandleHit* hit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;
            
            switch (hit->hitArea()) {
                case Model::MoveHandleHit::HAXAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosX, inputState.pickRay().origin);
                    m_restrictToAxis = MoveHandle::RXAxis;
                    break;
                case Model::MoveHandleHit::HAYAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosY, inputState.pickRay().origin);
                    m_restrictToAxis = MoveHandle::RYAxis;
                    break;
                case Model::MoveHandleHit::HAZAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosZ, inputState.pickRay().origin);
                    m_restrictToAxis = MoveHandle::RZAxis;
                    break;
                case Model::MoveHandleHit::HAXYPlane:
                    plane = Plane::horizontalDragPlane(hit->hitPoint());
                    m_restrictToAxis = MoveHandle::RNone;
                    break;
                case Model::MoveHandleHit::HAXZPlane:
                    plane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosY);
                    m_restrictToAxis = MoveHandle::RNone;
                    break;
                case Model::MoveHandleHit::HAYZPlane:
                    plane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosX);
                    m_restrictToAxis = MoveHandle::RNone;
                    break;
            }
            
            wxString name;
            if (m_mode == VMMove) {
                assert((m_handleManager.selectedVertexHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedEdgeHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedFaceHandles().empty() ? 0 : 1) == 1);
                
                if (!m_handleManager.selectedVertexHandles().empty())
                    name = m_handleManager.selectedVertexHandles().size() == 1 ? wxT("Move Vertex") : wxT("Move Vertices");
                else if (!m_handleManager.selectedEdgeHandles().empty())
                    name = m_handleManager.selectedEdgeHandles().size() == 1 ? wxT("Move Edge") : wxT("Move Edges");
                else if (!m_handleManager.selectedFaceHandles().empty())
                    name = m_handleManager.selectedFaceHandles().size() == 1 ? wxT("Move Face") : wxT("Move Faces");
            } else {
                assert(m_handleManager.selectedVertexHandles().size() == 0 &&
                       ((m_handleManager.selectedEdgeHandles().size() == 1) ^
                        (m_handleManager.selectedFaceHandles().size() == 1))
                       );
                
                if (!m_handleManager.selectedEdgeHandles().empty())
                    name = wxT("Split Edge");
                else
                    name = wxT("Split Face");
            }
            
            initialPoint = hit->hitPoint();
            m_moveHandle.lock();
            beginCommandGroup(name);

            return true;
        }
        
        bool MoveVerticesTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            Vec3f delta = curPoint - refPoint;
            switch (m_restrictToAxis) {
                case MoveHandle::RXAxis:
                    delta.y = delta.z = 0.0f;
                    break;
                case MoveHandle::RYAxis:
                    delta.x = delta.z = 0.0f;
                    break;
                case MoveHandle::RZAxis:
                    delta.x = delta.y = 0.0f;
                    break;
                default:
                    break;
            }
            
            Utility::Grid& grid = document().grid();
            delta = grid.snap(delta);
            
            if (delta.null())
                return true;

            VertexToolResult result = moveVertices(delta);
            if (result == Conclude) {
                return false;
            }
            
            if (result == Continue) {
                m_moveHandle.setPosition(m_moveHandle.position() + delta);
                refPoint += delta;
            }

            return true;
        }
        
        void MoveVerticesTool::handleEndPlaneDrag(InputState& inputState) {
            endCommandGroup();
            m_moveHandle.unlock();
            updateMoveHandle(inputState);
        }

        void MoveVerticesTool::handleObjectsChange(InputState& inputState) {
            if (active() && !m_ignoreObjectChanges) {
                m_handleManager.clear();
                m_handleManager.add(document().editStateManager().selectedBrushes());
                updateMoveHandle(inputState);
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

        void MoveVerticesTool::handleCameraChange(InputState& inputState) {
            if (active())
                updateMoveHandle(inputState);
        }

        MoveVerticesTool::MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize) :
        PlaneDragTool(documentViewHolder, true),
        m_moveHandle(axisLength, planeRadius),
        m_mode(VMMove),
        m_ignoreObjectChanges(false) {}

        bool MoveVerticesTool::hasSelection() {
            return !m_handleManager.selectedVertexHandles().empty() || !m_handleManager.selectedEdgeHandles().empty() || !m_handleManager.selectedFaceHandles().empty();
            
        }
        
        MoveVerticesTool::VertexToolResult MoveVerticesTool::moveVertices(const Vec3f& delta) {
            m_ignoreObjectChanges = true;
            if (m_mode == VMMove) {
                assert((m_handleManager.selectedVertexHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedEdgeHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedFaceHandles().empty() ? 0 : 1) == 1);
                
                if (!m_handleManager.selectedVertexHandles().empty()) {
                    MoveVerticesCommand* command = MoveVerticesCommand::moveVertices(document(), m_handleManager.selectedVertexHandles(), delta);
                    m_handleManager.remove(command->brushes());
                    
                    submitCommand(command);
                    m_handleManager.add(command->brushes());
                    
                    const Vec3f::Set& vertices = command->vertices();
                    if (vertices.empty()) {
                        m_ignoreObjectChanges = false;
                        return Conclude;
                    }
                    
                    m_handleManager.selectVertexHandles(command->vertices());
                    m_ignoreObjectChanges = false;
                    return Continue;
                } else if (!m_handleManager.selectedEdgeHandles().empty()) {
                    MoveEdgesCommand* command = MoveEdgesCommand::moveEdges(document(), m_handleManager.selectedEdgeHandles(), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectEdgeHandles(command->edges());
                        m_ignoreObjectChanges = false;
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
                    const Vec3f& position = m_handleManager.selectedEdgeHandles().begin()->first;
                    
                    SplitEdgesCommand* command = SplitEdgesCommand::splitEdges(document(), m_handleManager.edges(position), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        const Vec3f::Set& vertices = command->vertices();
                        assert(!vertices.empty());
                        m_handleManager.selectVertexHandles(command->vertices());
                        m_mode = VMMove;
                        m_ignoreObjectChanges = false;
                        return Continue;
                    } else {
                        m_handleManager.add(command->brushes());
                        m_handleManager.selectEdgeHandle(position);
                        m_ignoreObjectChanges = false;
                        return Deny;
                    }
                } else if (!m_handleManager.selectedFaceHandles().empty()) {
                    const Vec3f& position = m_handleManager.selectedFaceHandles().begin()->first;
                    
                    SplitFacesCommand* command = SplitFacesCommand::splitFaces(document(), m_handleManager.faces(position), delta);
                    m_handleManager.remove(command->brushes());
                    
                    if (submitCommand(command)) {
                        m_handleManager.add(command->brushes());
                        const Vec3f::Set& vertices = command->vertices();
                        assert(!vertices.empty());
                        m_handleManager.selectVertexHandles(command->vertices());
                        m_mode = VMMove;
                        m_ignoreObjectChanges = false;
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
