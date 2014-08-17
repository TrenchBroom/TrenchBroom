/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/Circle.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Sphere.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        RotateObjectsHandle::Hit::Hit() :
        m_area(HitArea_None) {}
        
        RotateObjectsHandle::Hit::Hit(const HitArea area, const FloatType distance, const Vec3& point) :
        m_area(area),
        m_distance(distance),
        m_point(point) {
            assert(m_area != HitArea_None);
        }
        
        bool RotateObjectsHandle::Hit::matches() const {
            return m_area != HitArea_None;
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
        m_locked(false),
        m_pointHandleRenderer(m_vbo) {
            PreferenceManager& prefs = PreferenceManager::instance();
            m_pointHandleRenderer.setRadius(prefs.get(Preferences::HandleRadius), 1);
            m_pointHandleRenderer.setRenderOccluded(false);
        }
        
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
            Hit hit = pickPointHandle(pickRay, m_position, HitArea_Center);
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(m_xAxis), HitArea_XAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(m_yAxis), HitArea_YAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(m_zAxis), HitArea_ZAxis));
            return hit;
        }
        
        Vec3 RotateObjectsHandle::getPointHandlePosition(const HitArea area) const {
            switch (area) {
                case HitArea_XAxis:
                    return getPointHandlePosition(m_xAxis);
                case HitArea_YAxis:
                    return getPointHandlePosition(m_yAxis);
                case HitArea_ZAxis:
                    return getPointHandlePosition(m_zAxis);
                case HitArea_None:
                case HitArea_Center:
                    return m_position;
                DEFAULT_SWITCH()
            }
        }
        
        Vec3 RotateObjectsHandle::getPointHandleAxis(const HitArea area) const {
            switch (area) {
                case HitArea_XAxis:
                    return m_xAxis;
                case HitArea_YAxis:
                    return m_yAxis;
                case HitArea_ZAxis:
                    return m_zAxis;
                case HitArea_None:
                case HitArea_Center:
                    return Vec3::PosZ;
                DEFAULT_SWITCH()
            }
        }
        
        Vec3 RotateObjectsHandle::getRotationAxis(const HitArea area) const {
            switch (area) {
                case HitArea_XAxis:
                    return Vec3::PosZ;
                case HitArea_YAxis:
                    return Vec3::PosX;
                case HitArea_ZAxis:
                    return Vec3::PosY;
                case HitArea_None:
                case HitArea_Center:
                    return Vec3::PosZ;
                DEFAULT_SWITCH()
            }
        }
        
        void RotateObjectsHandle::renderHandle(Renderer::RenderContext& renderContext, const HitArea highlight) {
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            
            glDisable(GL_DEPTH_TEST);
            renderAxes(renderContext);
            renderRings(renderContext);
            renderRingIndicators(renderContext);
            renderPointHandles(renderContext);
            renderPointHandleHighlight(renderContext, highlight);
            glEnable(GL_DEPTH_TEST);
        }
        
        void RotateObjectsHandle::renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle) {
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Color& pointHandleColor = prefs.get(Preferences::RotateHandleColor);
            
            const Vec3f rotationAxis(getRotationAxis(handle));
            const Vec3f startAxis(getPointHandleAxis(handle));
            const Vec3f endAxis(Quat3(rotationAxis, angle) * startAxis);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            
            glDisable(GL_DEPTH_TEST);
            {
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", getAngleIndicatorColor(handle));
                
                Renderer::Circle circle(handleRadius, 24, true, rotationAxis.firstComponent(), startAxis, endAxis);
                
                setVboState.mapped();
                circle.prepare(m_vbo);
                
                setVboState.active();
                circle.render();
                
                glPolygonMode(GL_FRONT, GL_FILL);
                glEnable(GL_CULL_FACE);
            }

            m_pointHandleRenderer.setColor(pointHandleColor);
            m_pointHandleRenderer.renderSingleHandle(renderContext, m_position);
            m_pointHandleRenderer.renderSingleHandle(renderContext, getPointHandlePosition(handle));

            glEnable(GL_DEPTH_TEST);
        }
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pickPointHandle(const Ray3& pickRay, const Vec3& position, const HitArea area) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType radius = 2.0 * prefs.get(Preferences::HandleRadius);
            const FloatType scaling = prefs.get(Preferences::HandleScalingFactor);
            const FloatType maxDist = prefs.get(Preferences::MaximumHandleDistance);
            const FloatType distance = pickRay.intersectWithSphere(position, radius, scaling, maxDist);
            
            if (Math::isnan(distance))
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
        
        void RotateObjectsHandle::renderAxes(Renderer::RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Color& xColor = prefs.get(Preferences::XAxisColor);
            const Color& yColor = prefs.get(Preferences::YAxisColor);
            const Color& zColor = prefs.get(Preferences::ZAxisColor);
            
            const BBox3f bounds(handleRadius);
            Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
            
            Renderer::VertexSpecs::P3C4::Vertex::List vertices = Renderer::coordinateSystem(bounds, xColor, yColor, zColor);
            Renderer::VertexArray array = Renderer::VertexArray::swap(GL_LINES, vertices);
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPCShader);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            array.prepare(m_vbo);
            
            setVboState.active();
            array.render();
        }
        
        void RotateObjectsHandle::renderRings(Renderer::RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Color& xColor = prefs.get(Preferences::XAxisColor);
            const Color& yColor = prefs.get(Preferences::YAxisColor);
            const Color& zColor = prefs.get(Preferences::ZAxisColor);
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
            
            Renderer::Circle xRing(handleRadius, 24, false, Math::Axis::AX, m_zAxis, m_yAxis);
            Renderer::Circle yRing(handleRadius, 24, false, Math::Axis::AY, m_xAxis, m_zAxis);
            Renderer::Circle zRing(handleRadius, 24, false, Math::Axis::AZ, m_xAxis, m_yAxis);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            xRing.prepare(m_vbo);
            yRing.prepare(m_vbo);
            zRing.prepare(m_vbo);
            setVboState.active();
            
            shader.set("Color", xColor);
            xRing.render();
            
            shader.set("Color", yColor);
            yRing.render();
            
            shader.set("Color", zColor);
            zRing.render();
        }
        
        void RotateObjectsHandle::renderRingIndicators(Renderer::RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Color& color = prefs.get(Preferences::RotateHandleColor);
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            shader.set("Color", color);
            
            Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
            
            Renderer::Circle xRing(handleRadius, 8, false, Math::Axis::AX,
                                   Quatf(Vec3f::PosX, Math::radians(+15.0f)) * m_yAxis,
                                   Quatf(Vec3f::PosX, Math::radians(-15.0f)) * m_yAxis);
            Renderer::Circle yRing(handleRadius, 8, false, Math::Axis::AY,
                                   Quatf(Vec3f::PosY, Math::radians(+15.0f)) * m_zAxis,
                                   Quatf(Vec3f::PosY, Math::radians(-15.0f)) * m_zAxis);
            Renderer::Circle zRing(handleRadius, 8, false, Math::Axis::AZ,
                                   Quatf(Vec3f::PosZ, Math::radians(+15.0f)) * m_xAxis,
                                   Quatf(Vec3f::PosZ, Math::radians(-15.0f)) * m_xAxis);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            xRing.prepare(m_vbo);
            yRing.prepare(m_vbo);
            zRing.prepare(m_vbo);
            
            setVboState.active();
            xRing.render();
            yRing.render();
            zRing.render();
        }
        
        void RotateObjectsHandle::renderPointHandles(Renderer::RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();
            m_pointHandleRenderer.setColor(prefs.get(Preferences::RotateHandleColor));

            m_pointHandleRenderer.renderSingleHandle(renderContext, m_position);
            m_pointHandleRenderer.renderSingleHandle(renderContext, getPointHandlePosition(m_xAxis));
            m_pointHandleRenderer.renderSingleHandle(renderContext, getPointHandlePosition(m_yAxis));
            m_pointHandleRenderer.renderSingleHandle(renderContext, getPointHandlePosition(m_zAxis));
        }
        
        void RotateObjectsHandle::renderPointHandleHighlight(Renderer::RenderContext& renderContext, const HitArea highlight) {
            PreferenceManager& prefs = PreferenceManager::instance();
            m_pointHandleRenderer.setHighlightColor(prefs.get(Preferences::SelectedHandleColor));
            
            switch (highlight) {
                case HitArea_Center:
                    m_pointHandleRenderer.renderHandleHighlight(renderContext, m_position);
                    break;
                case HitArea_XAxis:
                    m_pointHandleRenderer.renderHandleHighlight(renderContext, getPointHandlePosition(m_xAxis));
                    break;
                case HitArea_YAxis:
                    m_pointHandleRenderer.renderHandleHighlight(renderContext, getPointHandlePosition(m_yAxis));
                    break;
                case HitArea_ZAxis:
                    m_pointHandleRenderer.renderHandleHighlight(renderContext, getPointHandlePosition(m_zAxis));
                    break;
                case HitArea_None:
                    break;
                DEFAULT_SWITCH()
            };
        }
        
        Vec3 RotateObjectsHandle::getPointHandlePosition(const Vec3& axis) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return m_position + axis * prefs.get(Preferences::RotateHandleRadius);
        }
        
        Color RotateObjectsHandle::getAngleIndicatorColor(const HitArea area) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (area) {
                case HitArea_XAxis:
                    return Color(prefs.get(Preferences::ZAxisColor), 0.5f);
                case HitArea_YAxis:
                    return Color(prefs.get(Preferences::XAxisColor), 0.5f);
                case HitArea_ZAxis:
                    return Color(prefs.get(Preferences::YAxisColor), 0.5f);
                case HitArea_Center:
                case HitArea_None:
                    return Color(1.0f, 1.0f, 1.0f, 1.0f);
                DEFAULT_SWITCH()
            };
        }
    }
}
