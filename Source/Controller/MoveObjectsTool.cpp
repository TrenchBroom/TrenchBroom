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

#include "MoveObjectsTool.h"

#include "Controller/Command.h"
#include "Controller/MoveObjectsCommand.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Model/MapObject.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/Grid.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        MoveObjectsHandleHit::MoveObjectsHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea) :
        Hit(HitType::MoveObjectsHandleHit, hitPoint, distance),
        m_hitArea(hitArea) {}
        
        bool MoveObjectsHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        Model::MoveObjectsHandleHit* MoveObjectsTool::pickAxis(const Ray& ray, Vec3f& axis, Model::MoveObjectsHandleHit::HitArea hitArea) {
            float distance;
            float missDistance = ray.squaredDistanceToSegment(position() - m_axisLength * axis, position() + m_axisLength * axis, distance);
            if (isnan(missDistance) || missDistance > 5.0f)
                return NULL;
            
            return new Model::MoveObjectsHandleHit(ray.pointAtDistance(distance), distance, hitArea);
        }
        
        Model::MoveObjectsHandleHit* MoveObjectsTool::pickPlane(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::MoveObjectsHandleHit::HitArea hitArea) {
            Plane plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.lengthSquared();
                if (missDistance <= m_planeRadius * m_planeRadius &&
                    hitVector.dot(axis1) >= 0.0f && hitVector.dot(axis2) >= 0.0f)
                    return new Model::MoveObjectsHandleHit(hitPoint, distance, hitArea);
            }
            
            return NULL;
        }
        
        void MoveObjectsTool::renderAxes(Model::MoveObjectsHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Renderer::ActivateShader activateShader(renderContext.shaderManager(), Renderer::Shaders::ColoredHandleShader);
            Renderer::AxisFigure axisFigure(m_axisLength);
            
            if (hit != NULL && (hit->hitArea() == Model::MoveObjectsHandleHit::HAXAxis ||
                                hit->hitArea() == Model::MoveObjectsHandleHit::HAXYPlane ||
                                hit->hitArea() == Model::MoveObjectsHandleHit::HAXZPlane))
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            if (hit != NULL && (hit->hitArea() == Model::MoveObjectsHandleHit::HAYAxis ||
                                hit->hitArea() == Model::MoveObjectsHandleHit::HAXYPlane ||
                                hit->hitArea() == Model::MoveObjectsHandleHit::HAYZPlane))
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            if (hit != NULL && (hit->hitArea() == Model::MoveObjectsHandleHit::HAZAxis ||
                                hit->hitArea() == Model::MoveObjectsHandleHit::HAXZPlane ||
                                hit->hitArea() == Model::MoveObjectsHandleHit::HAYZPlane))
                axisFigure.setZColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            axisFigure.render(vbo, renderContext);
        }
        
        void MoveObjectsTool::renderPlanes(Model::MoveObjectsHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Renderer::ActivateShader activateShader(renderContext.shaderManager(), Renderer::Shaders::EdgeShader);
            
            Vec3f xAxis, yAxis, zAxis;
            axes(renderContext.camera().position(), xAxis, yAxis, zAxis);
            
            if (hit != NULL) {
                activateShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                if (hit->hitArea()== Model::MoveObjectsHandleHit::HAXYPlane)
                    Renderer::CircleFigure(Axis::AZ, xAxis, yAxis, m_planeRadius, 8, true).render(vbo, renderContext);
                if (hit->hitArea()== Model::MoveObjectsHandleHit::HAXZPlane)
                    Renderer::CircleFigure(Axis::AY, xAxis, zAxis, m_planeRadius, 8, true).render(vbo, renderContext);
                if (hit->hitArea()== Model::MoveObjectsHandleHit::HAYZPlane)
                    Renderer::CircleFigure(Axis::AX, yAxis, zAxis, m_planeRadius, 8,  true).render(vbo, renderContext);
            }
            
            activateShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.6f));
            Renderer::CircleFigure(Axis::AZ, xAxis, yAxis, m_planeRadius, 8, false).render(vbo, renderContext);
            Renderer::CircleFigure(Axis::AY, xAxis, zAxis, m_planeRadius, 8, false).render(vbo, renderContext);
            Renderer::CircleFigure(Axis::AX, yAxis, zAxis, m_planeRadius, 8, false).render(vbo, renderContext);
        }

        bool MoveObjectsTool::handleIsModal(InputState& inputState) {
            if (dragType() == DTDrag)
                return true;
            
            Model::MoveObjectsHandleHit* hit = hit = static_cast<Model::MoveObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::MoveObjectsHandleHit, true, view().filter()));
            return hit != NULL;
        }

        void MoveObjectsTool::handlePick(InputState& inputState) {
            if (locked())
                return;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;

            Vec3f xAxis, yAxis, zAxis;
            axes(inputState.pickRay().origin, xAxis, yAxis, zAxis);
            Model::MoveObjectsHandleHit* closestHit = NULL;
            
            closestHit = selectHit(closestHit, pickAxis(inputState.pickRay(), xAxis, Model::MoveObjectsHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickAxis(inputState.pickRay(), yAxis, Model::MoveObjectsHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickAxis(inputState.pickRay(), zAxis, Model::MoveObjectsHandleHit::HAZAxis));
            closestHit = selectHit(closestHit, pickPlane(inputState.pickRay(), Vec3f::PosX, yAxis, zAxis, Model::MoveObjectsHandleHit::HAYZPlane));
            closestHit = selectHit(closestHit, pickPlane(inputState.pickRay(), Vec3f::PosY, xAxis, zAxis, Model::MoveObjectsHandleHit::HAXZPlane));
            closestHit = selectHit(closestHit, pickPlane(inputState.pickRay(), Vec3f::PosZ, xAxis, yAxis, Model::MoveObjectsHandleHit::HAXYPlane));
            
            if (closestHit != NULL)
                inputState.pickResult().add(*closestHit);
        }

        bool MoveObjectsTool::handleUpdateState(InputState& inputState) {
            bool needsUpdate = false;
            if (!locked()) {
                Model::MoveObjectsHandleHit* hit = hit = static_cast<Model::MoveObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::MoveObjectsHandleHit, true, view().filter()));
                if (hit != NULL) {
                    if (m_lastHit == NULL || m_lastHit->hitArea() != hit->hitArea()) {
                        if (m_lastHit != NULL)
                            delete m_lastHit;
                        m_lastHit = new Model::MoveObjectsHandleHit(*hit);
                        needsUpdate = true;
                    }
                } else if (m_lastHit != NULL) {
                    delete m_lastHit;
                    m_lastHit = NULL;
                    needsUpdate = true;
                }
            }
            needsUpdate |= positionValid();
            
            return needsUpdate;
        }

        void MoveObjectsTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;
            
            Model::MoveObjectsHandleHit* hit = NULL;
            if (locked())
                hit = m_lastHit;
            else
                hit = static_cast<Model::MoveObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::MoveObjectsHandleHit, true, view().filter()));
            
            Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);
            
            Mat4f translation;
            translation.translate(position());
            Renderer::ApplyMatrix applyTranslation(renderContext.transformation(), translation);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            renderAxes(hit, vbo, renderContext);
            renderPlanes(hit, vbo, renderContext);
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
        
        bool MoveObjectsTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return false;
            
            Model::MoveObjectsHandleHit* hit = static_cast<Model::MoveObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::MoveObjectsHandleHit, true, view().filter()));
            
            if (hit == NULL)
                return false;
            
            switch (hit->hitArea()) {
                case Model::MoveObjectsHandleHit::HAXAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosX, inputState.pickRay().origin);
                    m_restrictToAxis = RXAxis;
                    break;
                case Model::MoveObjectsHandleHit::HAYAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosY, inputState.pickRay().origin);
                    m_restrictToAxis = RYAxis;
                    break;
                case Model::MoveObjectsHandleHit::HAZAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosZ, inputState.pickRay().origin);
                    m_restrictToAxis = RZAxis;
                    break;
                case Model::MoveObjectsHandleHit::HAXYPlane:
                    plane = Plane::horizontalDragPlane(hit->hitPoint());
                    m_restrictToAxis = RNone;
                    break;
                case Model::MoveObjectsHandleHit::HAXZPlane:
                    plane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosY);
                    m_restrictToAxis = RNone;
                    break;
                case Model::MoveObjectsHandleHit::HAYZPlane:
                    plane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosX);
                    m_restrictToAxis = RNone;
                    break;
            }
            
            initialPoint = hit->hitPoint();
            m_totalDelta = Vec3f::Null;
            lock();
            
            beginCommandGroup(Controller::Command::makeObjectActionName(wxT("Move"), entities, brushes));
            
            return true;
        }
        
        void MoveObjectsTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            Vec3f delta = curPoint - refPoint;
            switch (m_restrictToAxis) {
                case RXAxis:
                    delta.y = delta.z = 0.0f;
                    break;
                case RYAxis:
                    delta.x = delta.z = 0.0f;
                    break;
                case RZAxis:
                    delta.x = delta.y = 0.0f;
                    break;
                default:
                    break;
            }
            
            Utility::Grid& grid = document().grid();
            delta = grid.snap(delta);
            if (delta.null())
                return;
            
            setPosition(position() + delta);
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            Controller::MoveObjectsCommand* command = Controller::MoveObjectsCommand::moveObjects(document(), entities, brushes, delta, document().textureLock());
            submitCommand(command);
            
            refPoint += delta;
            m_totalDelta += delta;
        }
        
        void MoveObjectsTool::handleEndPlaneDrag(InputState& inputState) {
            if (m_totalDelta.null())
                discardCommandGroup();
            else
                endCommandGroup();
            unlock();
        }
        
        void MoveObjectsTool::handleObjectsChange(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;
            
            setPosition(Model::MapObject::center(entities, brushes));
        }
        
        void MoveObjectsTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;

           setPosition(Model::MapObject::center(entities, brushes));
        }

        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius) :
        PlaneDragTool(documentViewHolder, true),
        m_axisLength(axisLength),
        m_planeRadius(planeRadius),
        m_lastHit(NULL) {
            assert(m_planeRadius < m_axisLength);
        }

        MoveObjectsTool::~MoveObjectsTool() {
            if (m_lastHit != NULL) {
                delete m_lastHit;
                m_lastHit = NULL;
            }
        }
    }
}