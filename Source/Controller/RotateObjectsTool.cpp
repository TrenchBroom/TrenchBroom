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

#include "RotateObjectsTool.h"

#include "Controller/Command.h"
#include "Controller/RotateObjectsCommand.h"
#include "Model/EditStateManager.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RingFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/Grid.h"
#include "Utility/VecMath.h"

#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        RotateObjectsHandleHit::RotateObjectsHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea) :
        Hit(HitType::RotateObjectsHandleHit, hitPoint, distance),
        m_hitArea(hitArea) {}

        bool RotateObjectsHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        Model::RotateObjectsHandleHit* RotateObjectsTool::pickRing(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::RotateObjectsHandleHit::HitArea hitArea) {
            Plane plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.lengthSquared();
                if (missDistance >= m_ringRadius * m_ringRadius &&
                    missDistance <= (m_ringRadius + m_ringThickness) * (m_ringRadius + m_ringThickness) &&
                    hitVector.dot(axis1) >= 0.0f && hitVector.dot(axis2) >= 0.0f)
                    return new Model::RotateObjectsHandleHit(hitPoint, distance, hitArea);
            }
            
            return NULL;
        }
        
        void RotateObjectsTool::renderAxis(Model::RotateObjectsHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            assert(hit != NULL);
            
            Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::ColoredHandleShader);
            Renderer::AxisFigure axisFigure(m_axisLength);
            if (hit->hitArea() == Model::RotateObjectsHandleHit::HAXAxis) {
                axisFigure.setAxes(Axis::AX);
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else if (hit->hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                axisFigure.setAxes(Axis::AY);
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                axisFigure.setAxes(Axis::AZ);
                axisFigure.setZColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            }
            axisFigure.render(vbo, context);
        }
        
        void RotateObjectsTool::renderRing(Model::RotateObjectsHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& context, float angle) {
            assert(hit != NULL);
            
            Vec3f xAxis, yAxis, zAxis;
            axes(context.camera().position(), xAxis, yAxis, zAxis);
            
            Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::HandleShader);
            
            Mat4f rotation;
            if (hit->hitArea() == Model::RotateObjectsHandleHit::HAXAxis) {
                rotation.rotateCCW(angle, angle > 0.0f ? Vec3f::PosX : Vec3f::NegX);
                Renderer::ApplyMatrix applyRotation(context.transformation(), rotation);
                
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AX, yAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AX, 0.0f, 2 * Math::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            } else if (hit->hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                rotation.rotateCCW(angle, angle > 0.0f ? Vec3f::PosY : Vec3f::NegY);
                Renderer::ApplyMatrix applyRotation(context.transformation(), rotation);
                
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AY, xAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AY, 0.0f, 2 * Math::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            } else {
                rotation.rotateCCW(angle, angle > 0.0f ? Vec3f::PosZ : Vec3f::NegZ);
                Renderer::ApplyMatrix applyRotation(context.transformation(), rotation);
                
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AZ, xAxis, yAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AZ, 0.0f, 2 * Math::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            }
        }
        
        bool RotateObjectsTool::handleIsModal(InputState& inputState) {
            if (dragType() == DTDrag)
                return true;
            
            Model::RotateObjectsHandleHit* hit = hit = static_cast<Model::RotateObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()));
            return hit != NULL;
        }

        void RotateObjectsTool::handlePick(InputState& inputState) {
            Vec3f xAxis, yAxis, zAxis;
            axes(inputState.pickRay().origin, xAxis, yAxis, zAxis);
            Model::RotateObjectsHandleHit* closestHit = NULL;
            
            closestHit = selectHit(closestHit, pickRing(inputState.pickRay(), xAxis, yAxis, zAxis, Model::RotateObjectsHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickRing(inputState.pickRay(), yAxis, xAxis, zAxis, Model::RotateObjectsHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickRing(inputState.pickRay(), zAxis, xAxis, yAxis, Model::RotateObjectsHandleHit::HAZAxis));

            if (closestHit != NULL)
                inputState.pickResult().add(closestHit);
        }
        
        bool RotateObjectsTool::handleUpdateState(InputState& inputState) {
            bool needsUpdate = false;
            if (!locked()) {
                Model::RotateObjectsHandleHit* hit = hit = static_cast<Model::RotateObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()));
                if (hit != NULL) {
                    if (m_lastHit == NULL || m_lastHit->hitArea() != hit->hitArea()) {
                        if (m_lastHit != NULL)
                            delete m_lastHit;
                        m_lastHit = new Model::RotateObjectsHandleHit(*hit);
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
        
        void RotateObjectsTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;
            
            Model::RotateObjectsHandleHit* hit = NULL;
            if (locked())
                hit = m_lastHit;
            else
                hit = static_cast<Model::RotateObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()));

            Mat4f translation;
            translation.translate(position());
            Renderer::ApplyMatrix applyTranslation(renderContext.transformation(), translation);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);
            if (hit != NULL) {
                renderAxis(hit, vbo, renderContext);
                renderRing(hit, vbo, renderContext, m_angle);
            } else {
                Vec3f xAxis, yAxis, zAxis;
                axes(renderContext.camera().position(), xAxis, yAxis, zAxis);
                
                Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                
                Renderer::RingFigure(Axis::AX, yAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, renderContext);
                Renderer::RingFigure(Axis::AY, xAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, renderContext);
                Renderer::RingFigure(Axis::AZ, xAxis, yAxis, m_ringRadius, m_ringThickness, 8).render(vbo, renderContext);
            }
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
        
        bool RotateObjectsTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return false;
            
            Model::RotateObjectsHandleHit* hit = hit = static_cast<Model::RotateObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()));
            
            if (hit == NULL)
                return false;

            switch (hit->hitArea()) {
                case Model::RotateObjectsHandleHit::HAXAxis:
                    plane = Plane::orthogonalDragPlane(hit->hitPoint(), Vec3f::PosX);
                    m_axis = Vec3f::PosX;
                    break;
                case Model::RotateObjectsHandleHit::HAYAxis:
                    plane = Plane::orthogonalDragPlane(hit->hitPoint(), Vec3f::PosY);
                    m_axis = Vec3f::PosY;
                    break;
                case Model::RotateObjectsHandleHit::HAZAxis:
                    plane = Plane::orthogonalDragPlane(hit->hitPoint(), Vec3f::PosZ);
                    m_axis = Vec3f::PosZ;
                    break;
            }
            
            initialPoint = hit->hitPoint();
            lock();
            beginCommandGroup(Controller::Command::makeObjectActionName(wxT("Rotate"), entities, brushes));
            
            return true;
        }
        
        void RotateObjectsTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            Vec3f startVector = refPoint - position();
            startVector.normalize();
            Vec3f currentVector = curPoint - position();
            currentVector.normalize();
            
            Utility::Grid& grid = document().grid();
            m_angle = grid.snapAngle(currentVector.angleFrom(startVector, m_axis));
            if (m_angle == 0.0f)
                return;
            
            rollbackCommandGroup();
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            RotateObjectsCommand* command = RotateObjectsCommand::rotate(document(), entities, brushes, m_axis, m_angle, false, position(), document().textureLock());
            submitCommand(command);
        }
        
        void RotateObjectsTool::handleEndPlaneDrag(InputState& inputState) {
            endCommandGroup();
            unlock();
            m_angle = 0.0f;
        }
        
        void RotateObjectsTool::handleObjectsChange(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;
            
            setPosition(Model::MapObject::center(entities, brushes));
        }

        void RotateObjectsTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;
            
            setPosition(Model::MapObject::center(entities, brushes));
        }

        RotateObjectsTool::RotateObjectsTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float ringRadius, float ringThickness) :
        PlaneDragTool(documentViewHolder, true),
        m_axisLength(axisLength),
        m_ringRadius(ringRadius),
        m_ringThickness(ringThickness),
        m_lastHit(NULL),
        m_angle(0.0f) {}
        
        RotateObjectsTool::~RotateObjectsTool() {
            if (m_lastHit != NULL) {
                delete m_lastHit;
                m_lastHit = NULL;
            }
        }
    }
}