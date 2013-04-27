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

#include "RotateHandle.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RingFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Vbo.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        RotateHandleHit::RotateHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea) :
        Hit(HitType::RotateHandleHit, hitPoint, distance),
        m_hitArea(hitArea) {}

        bool RotateHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }

    namespace Controller {
        Model::RotateHandleHit* RotateHandle::pickRing(const Rayf& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::RotateHandleHit::HitArea hitArea) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            float factor = (position() - ray.origin).length() * scalingFactor;

            Planef plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!Math<float>::isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.length() / factor;
                if (missDistance >= m_ringRadius &&
                    missDistance <= (m_ringRadius + m_ringThickness) &&
                    hitVector.dot(axis1) >= 0.0f && hitVector.dot(axis2) >= 0.0f)
                    return new Model::RotateHandleHit(hitPoint, distance, hitArea);
            }

            return NULL;
        }

        void RotateHandle::renderAxis(Model::RotateHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            assert(hit != NULL);

            Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::ColoredHandleShader);
            Renderer::AxisFigure axisFigure(m_axisLength);
            if (hit->hitArea() == Model::RotateHandleHit::HAXAxis) {
                axisFigure.setAxes(true, false, false);
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else if (hit->hitArea() == Model::RotateHandleHit::HAYAxis) {
                axisFigure.setAxes(false, true, false);
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                axisFigure.setAxes(false, false, true);
                axisFigure.setZColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            }
            axisFigure.render(vbo, context);
        }

        void RotateHandle::renderRing(Model::RotateHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& context, float angle) {
            assert(hit != NULL);

            Vec3f xAxis, yAxis, zAxis;
            axes(context.camera().position(), xAxis, yAxis, zAxis);

            Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::HandleShader);

            Mat4f rotation;
            if (hit->hitArea() == Model::RotateHandleHit::HAXAxis) {
                rotateCCW(rotation, angle, Vec3f::PosX);
                Renderer::ApplyModelMatrix applyRotation(context.transformation(), rotation);

                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AX, yAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AX, 0.0f, 2.0f * Math<float>::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            } else if (hit->hitArea() == Model::RotateHandleHit::HAYAxis) {
                rotateCCW(rotation, angle, Vec3f::PosY);
                Renderer::ApplyModelMatrix applyRotation(context.transformation(), rotation);

                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AY, xAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AY, 0.0f, 2.0f * Math<float>::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            } else {
                rotateCCW(rotation, angle, Vec3f::PosZ);
                Renderer::ApplyModelMatrix applyRotation(context.transformation(), rotation);

                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                Renderer::RingFigure(Axis::AZ, xAxis, yAxis, m_ringRadius, m_ringThickness, 8).render(vbo, context);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::CircleFigure(Axis::AZ, 0.0f, 2.0f * Math<float>::Pi, m_ringRadius + m_ringThickness, 32, false).render(vbo, context);
            }
        }

        RotateHandle::RotateHandle(float axisLength, float ringRadius, float ringThickness) :
        m_axisLength(axisLength),
        m_ringRadius(ringRadius),
        m_ringThickness(ringThickness) {
            assert(m_axisLength > 0.0f);
            assert(m_ringRadius > 0.0f);
            assert(m_ringThickness > 0.0f);
        }

        RotateHandle::~RotateHandle() {
        }

        Model::RotateHandleHit* RotateHandle::pick(const Rayf& ray) {
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Model::RotateHandleHit* closestHit = NULL;

            closestHit = selectHit(closestHit, pickRing(ray, xAxis, yAxis, zAxis, Model::RotateHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickRing(ray, yAxis, xAxis, zAxis, Model::RotateHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickRing(ray, zAxis, xAxis, yAxis, Model::RotateHandleHit::HAZAxis));

            if (!locked())
                setLastHit(closestHit);
            return closestHit;
        }

        void RotateHandle::render(Model::RotateHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, float angle) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const float distance = renderContext.camera().distanceTo(position());
            const float factor = prefs.getFloat(Preferences::HandleScalingFactor) * distance;

            Mat4f translation = translated(Mat4f::Identity, position());
            scale(translation, factor);
            Renderer::ApplyModelMatrix applyTranslation(renderContext.transformation(), translation);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);
            if (hit != NULL) {
                renderAxis(hit, vbo, renderContext);
                renderRing(hit, vbo, renderContext, angle);
            } else {
                Vec3f xAxis, yAxis, zAxis;
                axes(renderContext.camera().position(), xAxis, yAxis, zAxis);

                Renderer::ActivateShader coloredShader(renderContext.shaderManager(), Renderer::Shaders::ColoredHandleShader);

                Renderer::AxisFigure axisFigure(m_axisLength);
                axisFigure.setAxes(true, true, true);
                axisFigure.setXColor(prefs.getColor(Preferences::XColor));
                axisFigure.setYColor(prefs.getColor(Preferences::YColor));
                axisFigure.setZColor(prefs.getColor(Preferences::ZColor));
                axisFigure.render(vbo, renderContext);

                Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));

                Renderer::RingFigure(Axis::AX, yAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, renderContext);
                Renderer::RingFigure(Axis::AY, xAxis, zAxis, m_ringRadius, m_ringThickness, 8).render(vbo, renderContext);
                Renderer::RingFigure(Axis::AZ, xAxis, yAxis, m_ringRadius, m_ringThickness, 8).render(vbo, renderContext);
            }

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}
