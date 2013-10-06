/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "RotateObjectsHandle.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/Circle.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Ring.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Sphere.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        RotateObjectsHandle::Hit::Hit() :
        m_area(HANone) {}
        
        RotateObjectsHandle::Hit::Hit(const HitArea area, const FloatType distance, const Vec3& point) :
        m_area(area),
        m_distance(distance),
        m_point(point) {
            assert(m_area != HANone);
        }
        
        bool RotateObjectsHandle::Hit::matches() const {
            return m_area != HANone;
        }
            
        RotateObjectsHandle::HitArea RotateObjectsHandle::Hit::area() const {
            assert(matches());
            return m_area;
        }
        
        FloatType RotateObjectsHandle::Hit::distance() const {
            assert(matches());
            return m_distance;
        }
        
        const Vec3& RotateObjectsHandle::Hit::point() const {
            assert(matches());
            return m_point;
        }
 
        RotateObjectsHandle::RotateObjectsHandle() :
        m_vbo(0xFFF),
        m_locked(false) {}
        
        void RotateObjectsHandle::setLocked(const bool locked) {
            m_locked = locked;
        }

        void RotateObjectsHandle::setPosition(const Vec3& position) {
            m_position = position;
        }

        void RotateObjectsHandle::updateAxes(const Vec3& viewPos) {
            if (m_locked)
                return;
            
            const Vec3 viewDir = (m_position - viewPos).normalized();
            if (Math::eq(std::abs(viewDir.z()), 1.0)) {
                m_xAxis = Vec3::PosX;
                m_yAxis = Vec3::PosY;
            } else {
                m_xAxis = viewDir.x() > 0.0 ? Vec3::NegX : Vec3::PosX;
                m_yAxis = viewDir.y() > 0.0 ? Vec3::NegY : Vec3::PosY;
            }
            m_zAxis = viewDir.z() > 0.0 ? Vec3::NegZ : Vec3::PosZ;
        }

        RotateObjectsHandle::Hit RotateObjectsHandle::pick(const Ray3& pickRay) const {
            Hit hit = pickCenterHandle(pickRay);
            hit = selectHit(hit, pickRingHandle(pickRay, m_xAxis, m_yAxis, m_zAxis, HAXAxis));
            hit = selectHit(hit, pickRingHandle(pickRay, m_yAxis, m_xAxis, m_zAxis, HAYAxis));
            hit = selectHit(hit, pickRingHandle(pickRay, m_zAxis, m_xAxis, m_yAxis, HAZAxis));
            return hit;
        }
        
        void RotateObjectsHandle::renderHandle(Renderer::RenderContext& renderContext) const {

            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();

            glDisable(GL_DEPTH_TEST);
            renderAxes(renderContext);
            renderRings(renderContext);
            renderRingIndicators(renderContext);
            renderPointHandles(renderContext);
            glEnable(GL_DEPTH_TEST);
        }

        RotateObjectsHandle::Hit RotateObjectsHandle::pickCenterHandle(const Ray3& pickRay) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType radius = 2.0 * prefs.getDouble(Preferences::HandleRadius);
            const FloatType scaling = prefs.getDouble(Preferences::HandleScalingFactor);
            const FloatType maxDist = prefs.getDouble(Preferences::MaximumHandleDistance);
            const FloatType distance = pickRay.intersectWithSphere(m_position, radius, scaling, maxDist);
            
            if (Math::isnan(distance))
                return Hit();
            return Hit(HACenter, distance, pickRay.pointAtDistance(distance));
        }

        RotateObjectsHandle::Hit RotateObjectsHandle::pickRingHandle(const Ray3& pickRay, const Vec3& normal, const Vec3& axis1, const Vec3& axis2, const HitArea area) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType radius = prefs.getDouble(Preferences::RotateHandleRadius);
            const FloatType width = prefs.getDouble(Preferences::RotateHandleWidth);
            const FloatType scaling = prefs.getDouble(Preferences::HandleScalingFactor);
            const FloatType factor = (m_position - pickRay.origin).length() * scaling;
            
            const Plane3 plane(m_position, normal);
            const FloatType distance = plane.intersectWithRay(pickRay);
            if (Math::isnan(distance))
                return Hit();
            
            const Vec3 point = pickRay.pointAtDistance(distance);
            const Vec3 vec = point - m_position;
            const FloatType missDist = vec.length() / factor;
            if (missDist < radius ||
                missDist > radius + width ||
                vec.dot(axis1) < 0.0 || vec.dot(axis2) < 0.0)
                return Hit();
            
            return Hit(area, distance, pickRay.pointAtDistance(distance));
        }

        RotateObjectsHandle::Hit RotateObjectsHandle::selectHit(const Hit& closest, const Hit& hit) const {
            if (!closest.matches())
                return hit;
            if (hit.matches()) {
                if (hit.distance() < closest.distance())
                    return hit;
            }
            return closest;
        }

        void RotateObjectsHandle::renderAxes(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xColor = prefs.getColor(Preferences::XAxisColor);
            const Color& yColor = prefs.getColor(Preferences::YAxisColor);
            const Color& zColor = prefs.getColor(Preferences::ZAxisColor);

            const BBox3f bounds(64.0f);
            Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::VertexArray array(m_vbo, GL_LINES,
                                        Renderer::coordinateSystem(bounds, xColor, yColor, zColor));
            array.render();
        }

        void RotateObjectsHandle::renderRings(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xColor = prefs.getColor(Preferences::XAxisColor);
            const Color& yColor = prefs.getColor(Preferences::YAxisColor);
            const Color& zColor = prefs.getColor(Preferences::ZAxisColor);

            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));

            shader.set("Color", xColor);
            Renderer::Circle(m_vbo, 64.0f, 24, false, Math::Axis::AX, m_zAxis, m_yAxis).render();
            
            shader.set("Color", yColor);
            Renderer::Circle(m_vbo, 64.0f, 24, false, Math::Axis::AY, m_xAxis, m_zAxis).render();
             
            shader.set("Color", zColor);
            Renderer::Circle(m_vbo, 64.0f, 24, false, Math::Axis::AZ, m_xAxis, m_yAxis).render();
        }

        void RotateObjectsHandle::renderRingIndicators(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& color = prefs.getColor(Preferences::RotateHandleColor);

            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            shader.set("Color", color);
            
            Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));

            Renderer::Circle(m_vbo, 64.0f, 8, false, Math::Axis::AX,
                             Quatf(Vec3f::PosX, Math::radians(+15.0f)) * m_yAxis,
                             Quatf(Vec3f::PosX, Math::radians(-15.0f)) * m_yAxis).render();
            Renderer::Circle(m_vbo, 64.0f, 8, false, Math::Axis::AY,
                             Quatf(Vec3f::PosY, Math::radians(+15.0f)) * m_zAxis,
                             Quatf(Vec3f::PosY, Math::radians(-15.0f)) * m_zAxis).render();
            Renderer::Circle(m_vbo, 64.0f, 8, false, Math::Axis::AZ,
                             Quatf(Vec3f::PosZ, Math::radians(+15.0f)) * m_xAxis,
                             Quatf(Vec3f::PosZ, Math::radians(-15.0f)) * m_xAxis).render();
        }

        void RotateObjectsHandle::renderPointHandles(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& color = prefs.getColor(Preferences::RotateHandleColor);

            renderPointHandle(renderContext, m_position, color);
            renderPointHandle(renderContext, m_position + m_xAxis * 64.0, color);
            renderPointHandle(renderContext, m_position + m_yAxis * 64.0, color);
            renderPointHandle(renderContext, m_position + m_zAxis * 64.0, color);
        }

        void RotateObjectsHandle::renderPointHandle(Renderer::RenderContext& renderContext, const Vec3& position, const Color& color) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::PointHandleShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("ScalingFactor", prefs.getFloat(Preferences::HandleScalingFactor));
            shader.set("MaximumDistance", prefs.getFloat(Preferences::MaximumHandleDistance));
            shader.set("Position", Vec4f(Vec3f(position), 1.0f));
            shader.set("Color", color);

            Renderer::Sphere sphere(m_vbo, prefs.getFloat(Preferences::HandleRadius), 1);
            sphere.render();
        }
    }
}
