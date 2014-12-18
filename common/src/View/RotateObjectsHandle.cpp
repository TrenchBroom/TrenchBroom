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
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "View/InputState.h"
#include "View/PointHandle.h"

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
        
        const Vec3& RotateObjectsHandle::position() const {
            return m_position;
        }

        void RotateObjectsHandle::setPosition(const Vec3& position) {
            m_position = position;
        }
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pick(const InputState& inputState) const {
            if (inputState.camera().perspectiveProjection())
                return pick3D(inputState.pickRay(), inputState.camera());
            else
                return pick2D(inputState.pickRay(), inputState.camera());
        }
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pick2D(const Ray3& pickRay, const Renderer::Camera& camera) const {
            Vec3 xAxis, yAxis, zAxis;
            computeAxes(pickRay.origin, xAxis, yAxis, zAxis);
            
            Hit hit = pickPointHandle(pickRay, camera, m_position, HitArea_Center);
            switch (camera.direction().firstComponent()) {
                case Math::Axis::AX:
                    hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(yAxis), HitArea_YAxis));
                    break;
                case Math::Axis::AY:
                    hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(xAxis), HitArea_XAxis));
                    break;
                case Math::Axis::AZ:
                default:
                    hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(xAxis), HitArea_XAxis));
                    break;
            }
            return hit;
        }
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pick3D(const Ray3& pickRay, const Renderer::Camera& camera) const {
            Vec3 xAxis, yAxis, zAxis;
            computeAxes(pickRay.origin, xAxis, yAxis, zAxis);
            
            Hit hit = pickPointHandle(pickRay, camera, m_position, HitArea_Center);
            hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(xAxis), HitArea_XAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(yAxis), HitArea_YAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(zAxis), HitArea_ZAxis));
            return hit;
        }

        Vec3 RotateObjectsHandle::getPointHandlePosition(const HitArea area, const Vec3& cameraPos) const {
            Vec3 xAxis, yAxis, zAxis;
            computeAxes(cameraPos, xAxis, yAxis, zAxis);
            switch (area) {
                case HitArea_XAxis:
                    return getPointHandlePosition(xAxis);
                case HitArea_YAxis:
                    return getPointHandlePosition(yAxis);
                case HitArea_ZAxis:
                    return getPointHandlePosition(zAxis);
                case HitArea_None:
                case HitArea_Center:
                    return m_position;
                DEFAULT_SWITCH()
            }
        }
        
        Vec3 RotateObjectsHandle::getPointHandleAxis(const HitArea area, const Vec3& cameraPos) const {
            Vec3 xAxis, yAxis, zAxis;
            computeAxes(cameraPos, xAxis, yAxis, zAxis);
            switch (area) {
                case HitArea_XAxis:
                    return xAxis;
                case HitArea_YAxis:
                    return yAxis;
                case HitArea_ZAxis:
                    return zAxis;
                case HitArea_None:
                case HitArea_Center:
                    return Vec3::PosZ;
                DEFAULT_SWITCH()
            }
        }
        
        Vec3 RotateObjectsHandle::getRotationAxis(const HitArea area, const Vec3& cameraPos) const {
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
        
        void RotateObjectsHandle::renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea highlight) {
            if (renderContext.render2D())
                render2DHandle(renderContext, renderBatch, highlight);
            if (renderContext.render3D())
                render3DHandle(renderContext, renderBatch, highlight);
        }
        
        void RotateObjectsHandle::render2DHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea highlight) {
            
            const Renderer::Camera& camera = renderContext.camera();
            const float radius = static_cast<float>(pref(Preferences::RotateHandleRadius));
            Renderer::RenderService renderService(renderContext, renderBatch);
            
            const Color& color = pref(Preferences::axisColor(camera.direction().firstComponent()));
            renderService.renderCircle(color, m_position, camera.direction().firstComponent(), 64, radius);
            
            renderService.renderPointHandle(m_position);
            renderService.renderPointHandle(m_position + radius * camera.right());

            switch (highlight) {
                case RotateObjectsHandle::HitArea_Center:
                    renderService.renderPointHandleHighlight(m_position);
                    break;
                case RotateObjectsHandle::HitArea_XAxis:
                case RotateObjectsHandle::HitArea_YAxis:
                case RotateObjectsHandle::HitArea_ZAxis:
                    renderService.renderPointHandleHighlight(m_position + radius * camera.right());
                    break;
                case RotateObjectsHandle::HitArea_None:
                    break;
                    DEFAULT_SWITCH()
            };
        }
        
        void RotateObjectsHandle::render3DHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea highlight) {
            const float radius = static_cast<float>(pref(Preferences::RotateHandleRadius));

            Vec3f xAxis, yAxis, zAxis;
            computeAxes(renderContext.camera().position(), xAxis, yAxis, zAxis);

            Renderer::RenderService renderService(renderContext, renderBatch);
            
            renderService.renderCoordinateSystem(BBox3f(radius).translated(m_position));
            renderService.renderCircle(pref(Preferences::XAxisColor), m_position, Math::Axis::AX, 64, radius, zAxis, yAxis);
            renderService.renderCircle(pref(Preferences::YAxisColor), m_position, Math::Axis::AY, 64, radius, xAxis, zAxis);
            renderService.renderCircle(pref(Preferences::ZAxisColor), m_position, Math::Axis::AZ, 64, radius, xAxis, yAxis);

            renderService.renderCircle(pref(Preferences::HandleColor), m_position, Math::Axis::AX, 8, radius,
                                       Quatf(Vec3f::PosX, Math::radians(+15.0f)) * yAxis,
                                       Quatf(Vec3f::PosX, Math::radians(-15.0f)) * yAxis);
            renderService.renderCircle(pref(Preferences::HandleColor), m_position, Math::Axis::AY, 8, radius,
                                       Quatf(Vec3f::PosY, Math::radians(+15.0f)) * zAxis,
                                       Quatf(Vec3f::PosY, Math::radians(-15.0f)) * zAxis);
            renderService.renderCircle(pref(Preferences::HandleColor), m_position, Math::Axis::AZ, 8, radius,
                                       Quatf(Vec3f::PosZ, Math::radians(+15.0f)) * xAxis,
                                       Quatf(Vec3f::PosZ, Math::radians(-15.0f)) * xAxis);

            renderService.renderPointHandle(m_position);
            renderService.renderPointHandle(m_position + radius * xAxis);
            renderService.renderPointHandle(m_position + radius * yAxis);
            renderService.renderPointHandle(m_position + radius * zAxis);

            switch (highlight) {
                case RotateObjectsHandle::HitArea_Center:
                    renderService.renderPointHandleHighlight(m_position);
                    break;
                case RotateObjectsHandle::HitArea_XAxis:
                    renderService.renderPointHandleHighlight(m_position + radius * xAxis);
                    break;
                case RotateObjectsHandle::HitArea_YAxis:
                    renderService.renderPointHandleHighlight(m_position + radius * yAxis);
                    break;
                case RotateObjectsHandle::HitArea_ZAxis:
                    renderService.renderPointHandleHighlight(m_position + radius * zAxis);
                    break;
                case RotateObjectsHandle::HitArea_None:
                    break;
                    DEFAULT_SWITCH()
            };
        }

        /*
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
        */
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pickPointHandle(const Ray3& pickRay, const Renderer::Camera& camera, const Vec3& position, const HitArea area) const {
            const PointHandle handle(position, Color());
            const FloatType distance = handle.pick(pickRay, camera);
            
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
