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
            Hit hit = pickPointHandle(pickRay, m_position, HACenter);
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(m_xAxis), HAXAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(m_yAxis), HAYAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(m_zAxis), HAZAxis));
            return hit;
        }
        
        Vec3 RotateObjectsHandle::getPointHandlePosition(const HitArea area) const {
            switch (area) {
                case HAXAxis:
                    return getPointHandlePosition(m_xAxis);
                case HAYAxis:
                    return getPointHandlePosition(m_yAxis);
                case HAZAxis:
                    return getPointHandlePosition(m_zAxis);
                default:
                    return m_position;
            }
        }
        
        Vec3 RotateObjectsHandle::getPointHandleAxis(const HitArea area) const {
            switch (area) {
                case HAXAxis:
                    return m_xAxis;
                case HAYAxis:
                    return m_yAxis;
                case HAZAxis:
                    return m_zAxis;
                default:
                    return Vec3::PosZ;
            }
        }
        
        Vec3 RotateObjectsHandle::getRotationAxis(const HitArea area) const {
            switch (area) {
                case HAXAxis:
                    return Vec3::PosZ;
                case HAYAxis:
                    return Vec3::PosX;
                case HAZAxis:
                    return Vec3::PosY;
                default:
                    return Vec3::PosZ;
            }
        }
        
        void RotateObjectsHandle::renderHandle(Renderer::RenderContext& renderContext, const HitArea highlight) const {
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
        
        void RotateObjectsHandle::renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle) const {
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType handleRadius = prefs.get(Preferences::RotateHandleRadius);
            const Color& pointHandleColor = prefs.get(Preferences::RotateHandleColor);
            
            const Vec3 rotationAxis = getRotationAxis(handle);
            const Vec3 startAxis = getPointHandleAxis(handle);
            const Vec3 endAxis = Quat3(rotationAxis, angle) * startAxis;
            
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
            renderPointHandle(renderContext, m_position, pointHandleColor);
            renderPointHandle(renderContext, getPointHandlePosition(handle), pointHandleColor);
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
        
        void RotateObjectsHandle::renderAxes(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType handleRadius = prefs.get(Preferences::RotateHandleRadius);
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
        
        void RotateObjectsHandle::renderRings(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType handleRadius = prefs.get(Preferences::RotateHandleRadius);
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
        
        void RotateObjectsHandle::renderRingIndicators(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType handleRadius = prefs.get(Preferences::RotateHandleRadius);
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
        
        void RotateObjectsHandle::renderPointHandles(Renderer::RenderContext& renderContext) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& color = prefs.get(Preferences::RotateHandleColor);
            
            renderPointHandle(renderContext, m_position, color);
            renderPointHandle(renderContext, getPointHandlePosition(m_xAxis), color);
            renderPointHandle(renderContext, getPointHandlePosition(m_yAxis), color);
            renderPointHandle(renderContext, getPointHandlePosition(m_zAxis), color);
        }
        
        void RotateObjectsHandle::renderPointHandle(Renderer::RenderContext& renderContext, const Vec3& position, const Color& color) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::PointHandleShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("ScalingFactor", prefs.get(Preferences::HandleScalingFactor));
            shader.set("MaximumDistance", prefs.get(Preferences::MaximumHandleDistance));
            shader.set("Position", Vec4f(Vec3f(position), 1.0f));
            shader.set("Color", color);
            
            Renderer::Sphere sphere(prefs.get(Preferences::HandleRadius), 1);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            sphere.prepare(m_vbo);
            setVboState.active();
            sphere.render();
        }
        
        void RotateObjectsHandle::renderPointHandleHighlight(Renderer::RenderContext& renderContext, const HitArea highlight) const {
            switch (highlight) {
                case HACenter:
                    renderPointHandleHighlight(renderContext, m_position);
                    break;
                case HAXAxis:
                    renderPointHandleHighlight(renderContext, getPointHandlePosition(m_xAxis));
                    break;
                case HAYAxis:
                    renderPointHandleHighlight(renderContext, getPointHandlePosition(m_yAxis));
                    break;
                case HAZAxis:
                    renderPointHandleHighlight(renderContext, getPointHandlePosition(m_zAxis));
                    break;
                default:
                    break;
            };
        }
        
        void RotateObjectsHandle::renderPointHandleHighlight(Renderer::RenderContext& renderContext, const Vec3& position) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            const float scaling = prefs.get(Preferences::HandleScalingFactor);
            
            const Renderer::Camera& camera = renderContext.camera();
            const Mat4x4f billboardMatrix = camera.orthogonalBillboardMatrix();
            const float factor = camera.distanceTo(position) * scaling;
            const Mat4x4f matrix = translationMatrix(position) * billboardMatrix * scalingMatrix(Vec3f(factor, factor, 0.0f));
            Renderer::MultiplyModelMatrix billboard(renderContext.transformation(), matrix);
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
            shader.set("Color", prefs.get(Preferences::SelectedHandleColor));
            
            Renderer::Circle circle(2.0f * prefs.get(Preferences::HandleRadius), 16, false);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            circle.prepare(m_vbo);
            setVboState.active();
            circle.render();
        }
        
        Vec3 RotateObjectsHandle::getPointHandlePosition(const Vec3& axis) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return m_position + axis * prefs.get(Preferences::RotateHandleRadius);
        }
        
        Color RotateObjectsHandle::getAngleIndicatorColor(const HitArea area) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (area) {
                case HAXAxis:
                    return Color(prefs.get(Preferences::ZAxisColor), 0.5f);
                case HAYAxis:
                    return Color(prefs.get(Preferences::XAxisColor), 0.5f);
                case HAZAxis:
                    return Color(prefs.get(Preferences::YAxisColor), 0.5f);
                default:
                    return Color(1.0f, 1.0f, 1.0f, 1.0f);
            };
        }
    }
}
