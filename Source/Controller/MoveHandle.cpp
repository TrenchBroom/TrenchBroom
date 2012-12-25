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

#include "MoveHandle.h"

#include "Model/EditStateManager.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Vbo.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        MoveHandleHit::MoveHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea) :
        Hit(HitType::MoveHandleHit, hitPoint, distance),
        m_hitArea(hitArea) {}
        
        bool MoveHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }

    namespace Controller {
        Model::MoveHandleHit* MoveHandle::pickAxis(const Ray& ray, Vec3f& axis, Model::MoveHandleHit::HitArea hitArea) {
            float distance;
            Vec3f pointOnAxis;
            float missDistance = ray.squaredDistanceToSegment(position() - m_axisLength * axis, position() + m_axisLength * axis, pointOnAxis, distance);
            if (isnan(missDistance) || missDistance > 5.0f)
                return NULL;
            
            return new Model::MoveHandleHit(ray.pointAtDistance(distance), distance, hitArea);
        }
        
        Model::MoveHandleHit* MoveHandle::pickPlane(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::MoveHandleHit::HitArea hitArea) {
            Plane plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.lengthSquared();
                if (missDistance <= m_planeRadius * m_planeRadius &&
                    hitVector.dot(axis1) >= 0.0f && hitVector.dot(axis2) >= 0.0f)
                    return new Model::MoveHandleHit(hitPoint, distance, hitArea);
            }
            
            return NULL;
        }
        
        void MoveHandle::renderAxes(Model::MoveHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Renderer::ActivateShader activateShader(renderContext.shaderManager(), Renderer::Shaders::ColoredHandleShader);
            Renderer::AxisFigure axisFigure(m_axisLength);
            
            if (hit != NULL && (hit->hitArea() == Model::MoveHandleHit::HAXAxis ||
                                hit->hitArea() == Model::MoveHandleHit::HAXYPlane ||
                                hit->hitArea() == Model::MoveHandleHit::HAXZPlane))
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            if (hit != NULL && (hit->hitArea() == Model::MoveHandleHit::HAYAxis ||
                                hit->hitArea() == Model::MoveHandleHit::HAXYPlane ||
                                hit->hitArea() == Model::MoveHandleHit::HAYZPlane))
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            if (hit != NULL && (hit->hitArea() == Model::MoveHandleHit::HAZAxis ||
                                hit->hitArea() == Model::MoveHandleHit::HAXZPlane ||
                                hit->hitArea() == Model::MoveHandleHit::HAYZPlane))
                axisFigure.setZColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            axisFigure.render(vbo, renderContext);
        }
        
        void MoveHandle::renderPlanes(Model::MoveHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Renderer::ActivateShader activateShader(renderContext.shaderManager(), Renderer::Shaders::EdgeShader);
            
            Vec3f xAxis, yAxis, zAxis;
            axes(renderContext.camera().position(), xAxis, yAxis, zAxis);
            
            if (hit != NULL) {
                activateShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                if (hit->hitArea()== Model::MoveHandleHit::HAXYPlane)
                    Renderer::CircleFigure(Axis::AZ, xAxis, yAxis, m_planeRadius, 8, true).render(vbo, renderContext);
                if (hit->hitArea()== Model::MoveHandleHit::HAXZPlane)
                    Renderer::CircleFigure(Axis::AY, xAxis, zAxis, m_planeRadius, 8, true).render(vbo, renderContext);
                if (hit->hitArea()== Model::MoveHandleHit::HAYZPlane)
                    Renderer::CircleFigure(Axis::AX, yAxis, zAxis, m_planeRadius, 8,  true).render(vbo, renderContext);
            }
            
            activateShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.6f));
            Renderer::CircleFigure(Axis::AZ, xAxis, yAxis, m_planeRadius, 8, false).render(vbo, renderContext);
            Renderer::CircleFigure(Axis::AY, xAxis, zAxis, m_planeRadius, 8, false).render(vbo, renderContext);
            Renderer::CircleFigure(Axis::AX, yAxis, zAxis, m_planeRadius, 8, false).render(vbo, renderContext);
        }

        MoveHandle::MoveHandle(float axisLength, float planeRadius) :
        m_axisLength(axisLength),
        m_planeRadius(planeRadius),
        m_enabled(true) {
            assert(m_axisLength > 0.0f);
            assert(m_planeRadius > 0.0f);
        }

        Model::MoveHandleHit* MoveHandle::pick(const Ray& ray) {
            if (!enabled() || locked())
                return NULL;
            
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Model::MoveHandleHit* closestHit = NULL;
            
            closestHit = selectHit(closestHit, pickAxis(ray, xAxis, Model::MoveHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, yAxis, Model::MoveHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, zAxis, Model::MoveHandleHit::HAZAxis));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosX, yAxis, zAxis, Model::MoveHandleHit::HAYZPlane));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosY, xAxis, zAxis, Model::MoveHandleHit::HAXZPlane));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosZ, xAxis, yAxis, Model::MoveHandleHit::HAXYPlane));
            
            return closestHit;
        }
        
        void MoveHandle::render(Model::MoveHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (!enabled())
                return;
            
            Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);
            
            Mat4f translation;
            translation.translate(position());
            Renderer::ApplyMatrix applyTranslation(renderContext.transformation(), translation);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            Model::MoveHandleHit* actualHit = locked() ? lastHit() : hit;
            renderAxes(actualHit, vbo, renderContext);
            renderPlanes(actualHit, vbo, renderContext);
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}
