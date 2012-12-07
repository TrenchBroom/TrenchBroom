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

#include "Controller/Command.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Renderer/ManyCubesInstancedFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Model {
        VertexHandleHit::VertexHandleHit(const Vec3f& hitPoint, float distance, const Vec3f& vertex) :
        Hit(HitType::VertexHandleHit, hitPoint, distance),
        m_vertex(vertex) {}
        
        bool VertexHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        void MoveVerticesTool::updateMoveHandle(InputState& inputState) {
            Vec3f closestVertex;
            float closestSquaredDistance = std::numeric_limits<float>::max();
            
            VertexSelection::VertexBrushMap::const_iterator it, end;
            const VertexSelection::VertexBrushMap& selectedVertices = m_selection.selectedVertices();
            for (it = selectedVertices.begin(), end = selectedVertices.end(); it != end; ++it) {
                const Vec3f& vertex = it->first;
                float distanceToClosestPoint;
                float squaredDistance = inputState.pickRay().squaredDistanceToPoint(vertex, distanceToClosestPoint);
                if (squaredDistance < closestSquaredDistance) {
                    closestSquaredDistance = squaredDistance;
                    closestVertex = vertex;
                }
            }
            
            bool enableMoveHandle = closestSquaredDistance <= m_moveHandle.axisLength() * m_moveHandle.axisLength();
            if (m_moveHandle.enabled() != enableMoveHandle) {
                m_moveHandle.setEnabled(enableMoveHandle);
                setNeedsUpdate();
            }
            if (!m_moveHandle.position().equals(closestVertex)) {
                m_moveHandle.setPosition(closestVertex);
                setNeedsUpdate();
            }
        }

        bool MoveVerticesTool::handleActivate(InputState& inputState) {
            m_selection.clear();
            m_selection.addVertices(document().editStateManager().selectedBrushes());
            updateMoveHandle(inputState);
            
            m_vertexFigure = new Renderer::ManyCubesInstancedFigure(m_vertexHandleSize);
            m_selectedVertexFigure = new Renderer::ManyCubesInstancedFigure(m_vertexHandleSize);
            m_vertexFigureValid = false;
            return true;
        }
        
        bool MoveVerticesTool::handleDeactivate(InputState& inputState) {
            m_selection.clear();
            delete m_vertexFigure;
            m_vertexFigure = NULL;
            delete m_selectedVertexFigure;
            m_selectedVertexFigure = NULL;
            return true;
        }
        
        bool MoveVerticesTool::handleIsModal(InputState& inputState) {
            return true;
        }

        void MoveVerticesTool::handlePick(InputState& inputState) {
            Model::MoveHandleHit* moveHandleHit = m_moveHandle.pick(inputState.pickRay());
            if (moveHandleHit != NULL)
                inputState.pickResult().add(moveHandleHit);
            
            VertexSelection::VertexBrushMap::const_iterator it, end;
            const VertexSelection::VertexBrushMap& unselectedVertices = m_selection.vertices();
            for (it = unselectedVertices.begin(), end = unselectedVertices.end(); it != end; ++it) {
                const Vec3f& vertex = it->first;
                float distance = inputState.pickRay().intersectWithCube(vertex, m_vertexHandleSize);
                if (!Math::isnan(distance)) {
                    Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                    inputState.pickResult().add(new Model::VertexHandleHit(hitPoint, distance, vertex));
                }
            }
            
            const VertexSelection::VertexBrushMap& selectedVertices = m_selection.selectedVertices();
            for (it = selectedVertices.begin(), end = selectedVertices.end(); it != end; ++it) {
                const Vec3f& vertex = it->first;
                float distance = inputState.pickRay().intersectWithCube(vertex, m_vertexHandleSize);
                if (!Math::isnan(distance)) {
                    Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                    inputState.pickResult().add(new Model::VertexHandleHit(hitPoint, distance, vertex));
                }
            }
        }
        
        bool MoveVerticesTool::handleUpdateState(InputState& inputState) {
            Model::MoveHandleHit* moveHandleHit = m_moveHandle.pick(inputState.pickRay());
            if ((m_moveHandle.lastHit() == NULL) != (moveHandleHit == NULL))
                setNeedsUpdate();
            else if (moveHandleHit != NULL && m_moveHandle.lastHit()->hitArea() != moveHandleHit->hitArea())
                setNeedsUpdate();
            
            m_moveHandle.setLastHit(moveHandleHit);
            return false;
        }
        
        void MoveVerticesTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            assert(m_vertexFigure != NULL);
            assert(m_selectedVertexFigure != NULL);
            
            Model::MoveHandleHit* moveHandleHit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            m_moveHandle.render(moveHandleHit, vbo, renderContext);

            if (!m_vertexFigureValid) {
                m_vertexFigure->clear();
                m_selectedVertexFigure->clear();
                
                VertexSelection::VertexBrushMap::const_iterator it, end;
                const VertexSelection::VertexBrushMap& unselectedVertices = m_selection.vertices();
                for (it = unselectedVertices.begin(), end = unselectedVertices.end(); it != end; ++it) {
                    const Vec3f& vertex = it->first;
                    m_vertexFigure->addCube(vertex);
                }
                
                const VertexSelection::VertexBrushMap& selectedVertices = m_selection.selectedVertices();
                for (it = selectedVertices.begin(), end = selectedVertices.end(); it != end; ++it) {
                    const Vec3f& vertex = it->first;
                    m_selectedVertexFigure->addCube(vertex);
                }
                
                m_vertexFigureValid = true;
            }
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            m_vertexFigure->setColor(prefs.getColor(Preferences::VertexHandleColor));
            m_vertexFigure->render(vbo, renderContext);
            m_selectedVertexFigure->setColor(prefs.getColor(Preferences::SelectedVertexHandleColor));
            m_selectedVertexFigure->render(vbo, renderContext);
            
            glDisable(GL_DEPTH_TEST);
            m_vertexFigure->setColor(prefs.getColor(Preferences::OccludedVertexHandleColor));
            m_vertexFigure->render(vbo, renderContext);
            m_selectedVertexFigure->setColor(prefs.getColor(Preferences::OccludedSelectedVertexHandleColor));
            m_selectedVertexFigure->render(vbo, renderContext);
            glEnable(GL_DEPTH_TEST);
        }

        bool MoveVerticesTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKCtrlCmd))
                return false;
            
            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;

            bool selected = m_selection.selected(hit->vertex());
            if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                if (selected)
                    m_selection.deselectVertex(hit->vertex());
                else
                    m_selection.selectVertex(hit->vertex());
            } else {
                m_selection.deselectAll();
                m_selection.selectVertex(hit->vertex());
            }
            
            m_vertexFigureValid = false;
            setNeedsUpdate();
            updateMoveHandle(inputState);
            return true;
        }

        void MoveVerticesTool::handleMouseMove(InputState& inputState) {
            updateMoveHandle(inputState);
        }

        bool MoveVerticesTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKCtrlCmd))
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
            
            initialPoint = hit->hitPoint();
            m_totalDelta = Vec3f::Null;
            m_moveHandle.lock();
            
            beginCommandGroup(m_selection.selectedVertices().size() == 1 ? wxT("Move Vertex") : wxT("Move Vertices"));

            return true;
        }
        
        void MoveVerticesTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
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
                return;
            
            m_moveHandle.setPosition(m_moveHandle.position() + delta);
            setNeedsUpdate();
            
            refPoint += delta;
            m_totalDelta += delta;
        }
        
        void MoveVerticesTool::handleEndPlaneDrag(InputState& inputState) {
            if (m_totalDelta.null())
                discardCommandGroup();
            else
                endCommandGroup();
            m_moveHandle.unlock();
            updateMoveHandle(inputState);
        }

        void MoveVerticesTool::handleObjectsChange(InputState& inputState) {
            m_selection.clear();
            m_vertexFigureValid = false;
            setNeedsUpdate();
            updateMoveHandle(inputState);
        }

        void MoveVerticesTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            if (document().editStateManager().selectedBrushes().empty()) {
                m_selection.clear();
            } else {
                m_selection.removeVertices(changeSet.brushesFrom(Model::EditState::Selected));
                m_selection.addVertices(changeSet.brushesTo(Model::EditState::Selected));
            }

            m_vertexFigureValid = false;
            setNeedsUpdate();
        }

        void MoveVerticesTool::handleCameraChange(InputState& inputState) {
            updateMoveHandle(inputState);
        }

        MoveVerticesTool::MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize) :
        PlaneDragTool(documentViewHolder, true),
        m_moveHandle(axisLength, planeRadius),
        m_vertexHandleSize(vertexSize),
        m_vertexFigure(NULL),
        m_selectedVertexFigure(NULL),
        m_vertexFigureValid(false) {}
    }
}
