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
#include "Model/ModelUtils.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RingFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"
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
        void RotateObjectsTool::updateHandlePosition(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;
            
            Vec3f position = referencePoint(entities, brushes, document().grid());
            setPosition(position);
        }

        Model::RotateObjectsHandleHit* RotateObjectsTool::pickRing(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::RotateObjectsHandleHit::HitArea hitArea) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            float factor = (position() - ray.origin).length() * scalingFactor;

            Plane plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.length() / factor;
                if (missDistance >= m_ringRadius &&
                    missDistance <= (m_ringRadius + m_ringThickness) &&
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
                axisFigure.setAxes(true, false, false);
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else if (hit->hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                axisFigure.setAxes(false, true, false);
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                axisFigure.setAxes(false, false, true);
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
                rotation.rotateCCW(angle, Vec3f::PosX);
                Renderer::ApplyMatrix applyRotation(context.transformation(), rotation);

                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AX, yAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AX, 0.0f, 2.0f * Math::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            } else if (hit->hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                rotation.rotateCCW(angle, Vec3f::PosY);
                Renderer::ApplyMatrix applyRotation(context.transformation(), rotation);

                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AY, xAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AY, 0.0f, 2.0f * Math::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            } else {
                rotation.rotateCCW(angle, Vec3f::PosZ);
                Renderer::ApplyMatrix applyRotation(context.transformation(), rotation);

                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AZ, xAxis, yAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AZ, 0.0f, 2.0f * Math::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            }
        }

        bool RotateObjectsTool::handleIsModal(InputState& inputState) {
            if (dragType() == DTDrag)
                return true;

            return inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()) != NULL;
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
            if (!locked()) {
                delete m_lastHit;
                if (closestHit != NULL)
                    m_lastHit = new Model::RotateObjectsHandleHit(*closestHit);
                else
                    m_lastHit = NULL;
            }
        }

        void RotateObjectsTool::handleRenderFirst(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;

            Model::RotateObjectsHandleHit* hit = NULL;
            if (locked())
                hit = m_lastHit;
            else
                hit = static_cast<Model::RotateObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()));

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float distance = renderContext.camera().distanceTo(position());
            float factor = prefs.getFloat(Preferences::HandleScalingFactor) * distance;

            Mat4f translation;
            translation.translate(position());
            translation.scale(factor);
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

        void RotateObjectsTool::handleObjectsChange(InputState& inputState) {
            if (!m_ignoreObjectsChange)
                updateHandlePosition(inputState);
        }

        void RotateObjectsTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            updateHandlePosition(inputState);
        }

        void RotateObjectsTool::handleGridChange(InputState& inputState) {
            updateHandlePosition(inputState);
        }

        bool RotateObjectsTool::handleStartDrag(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return false;
            
            Model::RotateObjectsHandleHit* hit = static_cast<Model::RotateObjectsHandleHit*>(inputState.pickResult().first(Model::HitType::RotateObjectsHandleHit, true, view().filter()));
            
            if (hit == NULL)
                return false;
            
            Vec3f test = hit->hitPoint() - position();
            switch (hit->hitArea()) {
                case Model::RotateObjectsHandleHit::HAXAxis:
                    m_axis = Vec3f::PosX;
                    m_invert = (test.dot(Vec3f::PosX) > 0.0f == test.dot(Vec3f::PosY) > 0.0f);
                    break;
                case Model::RotateObjectsHandleHit::HAYAxis:
                    m_axis = Vec3f::PosY;
                    m_invert = (test.dot(Vec3f::PosX) > 0.0f != test.dot(Vec3f::PosY) > 0.0f);
                    break;
                case Model::RotateObjectsHandleHit::HAZAxis:
                    m_axis = Vec3f::PosZ;
                    m_invert = false;
                    break;
            }

            m_startX = inputState.x();
            m_startY = inputState.y();
            m_angle = 0.0f;
            m_center = position().rounded();
            lock();
            beginCommandGroup(Controller::Command::makeObjectActionName(wxT("Rotate"), entities, brushes));
            
            return true;
        }
        
        bool RotateObjectsTool::handleDrag(InputState& inputState) {
            int delta = 0;
            if (m_axis == Vec3f::PosZ) {
                delta = -(inputState.x() - m_startX);
            } else {
                delta = inputState.y() - m_startY;
                if (m_invert)
                    delta *= -1;
            }
            
            m_angle = delta / 150.0f * Math::Pi;
            
            Utility::Grid& grid = document().grid();
            m_angle = grid.snapAngle(m_angle);
            
            m_ignoreObjectsChange = true;
            rollbackCommandGroup();

            if (m_angle != 0.0f) {
                Model::EditStateManager& editStateManager = document().editStateManager();
                const Model::EntityList& entities = editStateManager.selectedEntities();
                const Model::BrushList& brushes = editStateManager.selectedBrushes();
                RotateObjectsCommand* command = RotateObjectsCommand::rotate(document(), entities, brushes, m_axis, m_angle, false, m_center, document().textureLock());
                submitCommand(command);
            }
            
            m_ignoreObjectsChange = false;
            return true;
        }
        
        void RotateObjectsTool::handleEndDrag(InputState& inputState) {
            endCommandGroup();
            unlock();
            m_angle = 0.0f;
        }

        RotateObjectsTool::RotateObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, float axisLength, float ringRadius, float ringThickness) :
        Tool(documentViewHolder, inputController, true),
        m_axisLength(axisLength),
        m_ringRadius(ringRadius),
        m_ringThickness(ringThickness),
        m_lastHit(NULL),
        m_ignoreObjectsChange(false) {}

        RotateObjectsTool::~RotateObjectsTool() {
            if (m_lastHit != NULL) {
                delete m_lastHit;
                m_lastHit = NULL;
            }
        }
    }
}
