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

#include "Controller/MoveVerticesCommand.h"
#include "Controller/SplitEdgesCommand.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Renderer/ManySpheresInstancedFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Model {
        VertexHandleHit::VertexHandleHit(HitType::Type type, const Vec3f& hitPoint, float distance, const Vec3f& vertex) :
        Hit(type, hitPoint, distance),
        m_vertex(vertex) {
            assert(type == HitType::VertexHandleHit ||
                   type == HitType::EdgeHandleHit);
        }
        
        bool VertexHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        void MoveVerticesTool::updateMoveHandle(InputState& inputState) {
            Vec3f closestVertex;
            float closestSquaredDistance = std::numeric_limits<float>::max();
            
            if (m_mode == VMMove) {
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
            } else {
                Model::VertexToEdgesMap::const_iterator eIt, eEnd;
                const Model::VertexToEdgesMap& edgeHandles = m_handleManager.edgeHandles();
                for (eIt = edgeHandles.begin(), eEnd = edgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3f& position = eIt->first;
                    float distanceToClosestPoint;
                    float squaredDistance = inputState.pickRay().squaredDistanceToPoint(position, distanceToClosestPoint);
                    if (squaredDistance < closestSquaredDistance) {
                        closestSquaredDistance = squaredDistance;
                        closestVertex = position;
                    }
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
            m_handleManager.clear();
            m_handleManager.add(document().editStateManager().selectedBrushes());
            updateMoveHandle(inputState);
            
            m_unselectedHandleFigure = new Renderer::ManySpheresInstancedFigure(m_vertexHandleSize, 3);
            m_selectedHandleFigure = new Renderer::ManySpheresInstancedFigure(m_vertexHandleSize, 3);
            m_figuresValid = false;
            return true;
        }
        
        bool MoveVerticesTool::handleDeactivate(InputState& inputState) {
            m_handleManager.clear();
            delete m_unselectedHandleFigure;
            m_unselectedHandleFigure = NULL;
            delete m_selectedHandleFigure;
            m_selectedHandleFigure = NULL;
            return true;
        }
        
        bool MoveVerticesTool::handleIsModal(InputState& inputState) {
            return true;
        }

        void MoveVerticesTool::handlePick(InputState& inputState) {
            Model::MoveHandleHit* moveHandleHit = m_moveHandle.pick(inputState.pickRay());
            if (moveHandleHit != NULL)
                inputState.pickResult().add(moveHandleHit);
            
            if (m_mode == VMMove) {
                Model::VertexToBrushesMap::const_iterator vIt, vEnd;
                const Model::VertexToBrushesMap& unselectedVertexHandles = m_handleManager.unselectedVertexHandles();
                for (vIt = unselectedVertexHandles.begin(), vEnd = unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    float distance = inputState.pickRay().intersectWithSphere(position, m_vertexHandleSize);
                    if (!Math::isnan(distance)) {
                        Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                        inputState.pickResult().add(new Model::VertexHandleHit(Model::HitType::VertexHandleHit, hitPoint, distance, position));
                    }
                }
                
                const Model::VertexToBrushesMap& selectedVertexHandles = m_handleManager.selectedVertexHandles();
                for (vIt = selectedVertexHandles.begin(), vEnd = selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    float distance = inputState.pickRay().intersectWithSphere(position, m_vertexHandleSize);
                    if (!Math::isnan(distance)) {
                        Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                        inputState.pickResult().add(new Model::VertexHandleHit(Model::HitType::VertexHandleHit, hitPoint, distance, position));
                    }
                }
            } else {
                Model::VertexToEdgesMap::const_iterator eIt, eEnd;
                const Model::VertexToEdgesMap& edgeHandles = m_handleManager.edgeHandles();
                for (eIt = edgeHandles.begin(), eEnd = edgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3f& position = eIt->first;
                    float distance = inputState.pickRay().intersectWithSphere(position, m_vertexHandleSize);
                    if (!Math::isnan(distance)) {
                        Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                        inputState.pickResult().add(new Model::VertexHandleHit(Model::HitType::EdgeHandleHit, hitPoint, distance, position));
                    }
                }
            }
        }
        
        bool MoveVerticesTool::handleUpdateState(InputState& inputState) {
            if (!m_moveHandle.locked()) {
                Model::MoveHandleHit* moveHandleHit = m_moveHandle.pick(inputState.pickRay());
                if ((m_moveHandle.lastHit() == NULL) != (moveHandleHit == NULL))
                    setNeedsUpdate();
                else if (moveHandleHit != NULL && m_moveHandle.lastHit()->hitArea() != moveHandleHit->hitArea())
                    setNeedsUpdate();
                
                m_moveHandle.setLastHit(moveHandleHit);
            }
            return false;
        }
        
        void MoveVerticesTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            assert(m_unselectedHandleFigure != NULL);
            assert(m_selectedHandleFigure != NULL);
            
            Model::MoveHandleHit* moveHandleHit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            m_moveHandle.render(moveHandleHit, vbo, renderContext);

            if (!m_figuresValid) {
                m_unselectedHandleFigure->clear();
                m_selectedHandleFigure->clear();
                
                if (m_mode == VMMove) {
                    Model::VertexToBrushesMap::const_iterator vIt, vEnd;
                    const Model::VertexToBrushesMap& unselectedVertexHandles = m_handleManager.unselectedVertexHandles();
                    for (vIt = unselectedVertexHandles.begin(), vEnd = unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                        const Vec3f& position = vIt->first;
                        m_unselectedHandleFigure->add(position);
                    }
                    
                    const Model::VertexToBrushesMap& selectedVertexHandles = m_handleManager.selectedVertexHandles();
                    for (vIt = selectedVertexHandles.begin(), vEnd = selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                        const Vec3f& position = vIt->first;
                        m_selectedHandleFigure->add(position);
                    }
                } else {
                    Model::VertexToEdgesMap::const_iterator eIt, eEnd;
                    const Model::VertexToEdgesMap& edgeHandles = m_handleManager.edgeHandles();
                    for (eIt = edgeHandles.begin(), eEnd = edgeHandles.end(); eIt != eEnd; ++eIt) {
                        const Vec3f& position = eIt->first;
                        if (position.equals(m_moveHandle.position()))
                            m_selectedHandleFigure->add(position);
                        else
                            m_unselectedHandleFigure->add(position);
                    }
                }
                

                m_figuresValid = true;
            }
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            if (m_mode == VMMove) {
                m_unselectedHandleFigure->setColor(prefs.getColor(Preferences::VertexHandleColor));
                m_selectedHandleFigure->setColor(prefs.getColor(Preferences::SelectedVertexHandleColor));
            } else {
                m_unselectedHandleFigure->setColor(prefs.getColor(Preferences::SplitHandleColor));
                m_selectedHandleFigure->setColor(prefs.getColor(Preferences::SelectedSplitHandleColor));
            }

            m_unselectedHandleFigure->render(vbo, renderContext);
            m_selectedHandleFigure->render(vbo, renderContext);

            if (m_mode == VMMove) {
                m_unselectedHandleFigure->setColor(prefs.getColor(Preferences::OccludedVertexHandleColor));
                m_selectedHandleFigure->setColor(prefs.getColor(Preferences::OccludedSelectedVertexHandleColor));
            } else {
                m_unselectedHandleFigure->setColor(prefs.getColor(Preferences::OccludedSplitHandleColor));
                m_selectedHandleFigure->setColor(prefs.getColor(Preferences::OccludedSelectedSplitHandleColor));
            }

            glDisable(GL_DEPTH_TEST);
            m_selectedHandleFigure->render(vbo, renderContext);
            m_unselectedHandleFigure->render(vbo, renderContext);
            glEnable(GL_DEPTH_TEST);
        }

        void MoveVerticesTool::handleModifierKeyChange(InputState& inputState) {
            if (dragType() == DTNone) {
                VertexMode newMode = inputState.modifierKeys() == ModifierKeys::MKAlt ? VMSplit : VMMove;
                if (m_mode != newMode) {
                    m_mode = newMode;
                    m_figuresValid = false;
                    setNeedsUpdate();
                    updateMoveHandle(inputState);
                }
            }
        }

        bool MoveVerticesTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKCtrlCmd))
                return false;
            
            Model::VertexHandleHit* hit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit | Model::HitType::EdgeHandleHit, true, view().filter()));
            if (hit == NULL)
                return false;

            if (hit->type() == Model::HitType::VertexHandleHit) {
                bool selected = m_handleManager.vertexHandleSelected(hit->vertex());
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    if (selected)
                        m_handleManager.deselectVertexHandle(hit->vertex());
                    else
                        m_handleManager.selectVertexHandle(hit->vertex());
                } else {
                    m_handleManager.deselectVertexHandles();
                    m_handleManager.selectVertexHandle(hit->vertex());
                }

                m_figuresValid = false;
                setNeedsUpdate();
                updateMoveHandle(inputState);
            }
            
            return true;
        }

        void MoveVerticesTool::handleMouseMove(InputState& inputState) {
            Vec3f position = m_moveHandle.position();
            updateMoveHandle(inputState);
            if (!position.equals(m_moveHandle.position())) {
                m_figuresValid = false;
                setNeedsUpdate();
            }
        }

        bool MoveVerticesTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKAlt))
                return false;
            
            Model::MoveHandleHit* hit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit | Model::HitType::EdgeHandleHit, true, view().filter()));
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
            m_moveHandle.lock();
            
            wxString name;
            if (m_mode == VMMove) {
                name = m_handleManager.selectedVertexHandles().size() == 1 ? wxT("Move Vertex") : wxT("Move Vertices");
            } else {
                // wrong, it's always a MoveHandleHit!
                if (hit->type() == Model::HitType::EdgeHandleHit) {
                    m_mode = VMSplitEdge;
                    name = wxT("Split Edge");
                } else {
                    m_mode = VMSplitFace;
                    name = wxT("Split Face");
                }
            }
            
            beginCommandGroup(name);

            return true;
        }
        
        void MoveVerticesTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            if (m_mode == VMSplit)
                return;
            
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
            
            if (m_mode == VMSplitEdge) {
                const Model::EdgeList& edges = m_handleManager.edges(m_moveHandle.position());
                if (edges.empty())
                    return;
                
                SplitEdgesCommand* command = SplitEdgesCommand::splitEdges(document(), edges, delta);
                m_handleManager.remove(command->brushes());
                
                if (submitCommand(command)) {
                    m_handleManager.add(command->brushes());
                    m_handleManager.selectVertexHandles(command->vertices());
                    m_mode = VMMove;
                    m_moveHandle.setPosition(m_moveHandle.position() + delta);
                    refPoint += delta;
                } else {
                    m_handleManager.add(command->brushes());
                }
            } else if (m_mode == VMSplitFace) {
            } else {
                MoveVerticesCommand* command = MoveVerticesCommand::moveVertices(document(), m_handleManager.selectedVertexHandles(), delta);
                m_handleManager.remove(command->brushes());
                
                submitCommand(command);
                m_handleManager.add(command->brushes());
                m_handleManager.selectVertexHandles(command->vertices());
                m_moveHandle.setPosition(m_moveHandle.position() + delta);
                refPoint += delta;
            }
            
            m_figuresValid = false;
            setNeedsUpdate();

        }
        
        void MoveVerticesTool::handleEndPlaneDrag(InputState& inputState) {
            endCommandGroup();
            m_mode = inputState.modifierKeys() == ModifierKeys::MKAlt ? VMSplit : VMMove;
            m_moveHandle.unlock();
            updateMoveHandle(inputState);
        }

        void MoveVerticesTool::handleObjectsChange(InputState& inputState) {
            if (dragType() == DTNone) {
                m_handleManager.clear();
                m_handleManager.add(document().editStateManager().selectedBrushes());
                m_figuresValid = false;
                
                setNeedsUpdate();
                updateMoveHandle(inputState);
            }
        }

        void MoveVerticesTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            if (document().editStateManager().selectedBrushes().empty()) {
                m_handleManager.clear();
            } else {
                m_handleManager.remove(changeSet.brushesFrom(Model::EditState::Selected));
                m_handleManager.add(changeSet.brushesTo(Model::EditState::Selected));
            }

            m_figuresValid = false;
            setNeedsUpdate();
        }

        void MoveVerticesTool::handleCameraChange(InputState& inputState) {
            updateMoveHandle(inputState);
        }

        MoveVerticesTool::MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize) :
        PlaneDragTool(documentViewHolder, true),
        m_mode(VMMove),
        m_moveHandle(axisLength, planeRadius),
        m_vertexHandleSize(vertexSize),
        m_unselectedHandleFigure(NULL),
        m_selectedHandleFigure(NULL),
        m_figuresValid(false) {}
    }
}
