/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "TrenchBroom.h"
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

#include <vecmath/vec.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        template <typename T>
        void computeAxes(const vm::vec3& handlePos, const vm::vec<T,3>& cameraPos, vm::vec<T,3>& xAxis, vm::vec<T,3>& yAxis, vm::vec<T,3>& zAxis) {
            const auto viewDir = vm::vec<T,3>(vm::normalize(vm::vec<T,3>(handlePos) - cameraPos));
            if (vm::isEqual(std::abs(viewDir.z()), static_cast<T>(1.0))) {
                xAxis = vm::vec<T,3>::pos_x;
                yAxis = vm::vec<T,3>::pos_y;
            } else {
                xAxis = vm::isPositive(viewDir.x()) ? vm::vec<T,3>::neg_x : vm::vec<T,3>::pos_x;
                yAxis = vm::isPositive(viewDir.y()) ? vm::vec<T,3>::neg_y : vm::vec<T,3>::pos_y;
            }
            zAxis = vm::isPositive(viewDir.z()) ? vm::vec<T,3>::neg_z : vm::vec<T,3>::pos_z;
        }

        const Model::Hit::HitType RotateObjectsHandle::HandleHit = Model::Hit::freeHitType();

        const vm::vec3& RotateObjectsHandle::position() const {
            return m_position;
        }

        void RotateObjectsHandle::setPosition(const vm::vec3& position) {
            m_position = position;
        }
        
        Model::Hit RotateObjectsHandle::pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera) const {
            vm::vec3 xAxis, yAxis, zAxis;
            computeAxes(m_position, pickRay.origin, xAxis, yAxis, zAxis);
            
            auto hit = pickPointHandle(pickRay, camera, m_position, HitArea_Center);
            switch (firstComponent(camera.direction())) {
                case vm::axis::x:
                    hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(yAxis), HitArea_YAxis));
                    break;
                case vm::axis::y:
                    hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(zAxis), HitArea_ZAxis));
                    break;
                case vm::axis::z:
                default:
                    hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(xAxis), HitArea_XAxis));
                    break;
            }
            return hit;
        }
        
        Model::Hit RotateObjectsHandle::pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera) const {
            vm::vec3 xAxis, yAxis, zAxis;
            computeAxes(m_position, pickRay.origin, xAxis, yAxis, zAxis);
            
            Model::Hit hit = pickPointHandle(pickRay, camera, m_position, HitArea_Center);
            hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(xAxis), HitArea_XAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(yAxis), HitArea_YAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, camera, getPointHandlePosition(zAxis), HitArea_ZAxis));
            return hit;
        }

        vm::vec3 RotateObjectsHandle::pointHandlePosition(const HitArea area, const vm::vec3& cameraPos) const {
            vm::vec3 xAxis, yAxis, zAxis;
            computeAxes(m_position, cameraPos, xAxis, yAxis, zAxis);
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
                switchDefault()
            }
        }
        
        FloatType RotateObjectsHandle::handleRadius() const {
            return pref(Preferences::RotateHandleRadius);
        }

        vm::vec3 RotateObjectsHandle::pointHandleAxis(const HitArea area, const vm::vec3& cameraPos) const {
            vm::vec3 xAxis, yAxis, zAxis;
            computeAxes(m_position, cameraPos, xAxis, yAxis, zAxis);
            switch (area) {
                case HitArea_XAxis:
                    return xAxis;
                case HitArea_YAxis:
                    return yAxis;
                case HitArea_ZAxis:
                    return zAxis;
                case HitArea_None:
                case HitArea_Center:
                    return vm::vec3::pos_z;
                switchDefault()
            }
        }
        
        vm::vec3 RotateObjectsHandle::rotationAxis(const HitArea area) const {
            switch (area) {
                case HitArea_XAxis:
                    return vm::vec3::pos_z;
                case HitArea_YAxis:
                    return vm::vec3::pos_x;
                case HitArea_ZAxis:
                    return vm::vec3::pos_y;
                case HitArea_None:
                case HitArea_Center:
                    return vm::vec3::pos_z;
                switchDefault()
            }
        }
        
        void RotateObjectsHandle::renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const auto& camera = renderContext.camera();
            const auto radius = static_cast<float>(pref(Preferences::RotateHandleRadius));
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setShowOccludedObjects();
            
            renderService.setForegroundColor(pref(Preferences::axisColor(firstComponent(camera.direction()))));
            renderService.renderCircle(vm::vec3f(m_position), firstComponent(camera.direction()), 64, radius);
            
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            renderService.renderHandle(vm::vec3f(m_position));
            
            const auto viewDirection = camera.direction();
            switch (firstComponent(viewDirection)) {
                case vm::axis::x:
                case vm::axis::z:
                    renderService.renderHandle(vm::vec3f(m_position) + radius * camera.right());
                    break;
                case vm::axis::y:
                    renderService.renderHandle(vm::vec3f(m_position) + radius * camera.up());
                    break;
               switchDefault()
            };
        }
        
        void RotateObjectsHandle::renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const auto radius = static_cast<float>(pref(Preferences::RotateHandleRadius));

            vm::vec3f xAxis, yAxis, zAxis;
            computeAxes(m_position, renderContext.camera().position(), xAxis, yAxis, zAxis);

            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setShowOccludedObjects();
            
            renderService.renderCoordinateSystem(vm::bbox3f(radius).translate(vm::vec3f(m_position)));
            
            renderService.setForegroundColor(pref(Preferences::XAxisColor));
            renderService.renderCircle(vm::vec3f(m_position), vm::axis::x, 64, radius, zAxis, yAxis);
            renderService.setForegroundColor(pref(Preferences::YAxisColor));
            renderService.renderCircle(vm::vec3f(m_position), vm::axis::y, 64, radius, xAxis, zAxis);
            renderService.setForegroundColor(pref(Preferences::ZAxisColor));
            renderService.renderCircle(vm::vec3f(m_position), vm::axis::z, 64, radius, xAxis, yAxis);

            /*
            renderService.renderCircle(m_position, vm::Axis::AX, 8, radius,
                                       vm::quatf(vm::vec3f::pos_x, vm::radians(+15.0f)) * yAxis,
                                       vm::quatf(vm::vec3f::pos_x, vm::radians(-15.0f)) * yAxis);
            renderService.renderCircle(m_position, vm::Axis::AY, 8, radius,
                                       vm::quatf(vm::vec3f::pos_y, vm::radians(+15.0f)) * zAxis,
                                       vm::quatf(vm::vec3f::pos_y, vm::radians(-15.0f)) * zAxis);
            renderService.renderCircle(m_position, vm::Axis::AZ, 8, radius,
                                       vm::quatf(vm::vec3f::pos_z, vm::radians(+15.0f)) * xAxis,
                                       vm::quatf(vm::vec3f::pos_z, vm::radians(-15.0f)) * xAxis);

             */
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            renderService.renderHandle(vm::vec3f(m_position));

            renderService.setForegroundColor(pref(Preferences::ZAxisColor));
            renderService.renderHandle(vm::vec3f(m_position) + radius * xAxis);

            renderService.setForegroundColor(pref(Preferences::XAxisColor));
            renderService.renderHandle(vm::vec3f(m_position) + radius * yAxis);

            renderService.setForegroundColor(pref(Preferences::YAxisColor));
            renderService.renderHandle(vm::vec3f(m_position) + radius * zAxis);

        }

        void RotateObjectsHandle::renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea area) {
            const auto radius = static_cast<float>(pref(Preferences::RotateHandleRadius));
            const auto& camera = renderContext.camera();

            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.setShowOccludedObjects();

            switch (area) {
                case RotateObjectsHandle::HitArea_Center:
                    renderService.renderHandleHighlight(vm::vec3f(m_position));
                    break;
                case RotateObjectsHandle::HitArea_XAxis:
                case RotateObjectsHandle::HitArea_YAxis:
                    renderService.renderHandleHighlight(vm::vec3f(m_position) + radius * camera.right());
                    break;
                case RotateObjectsHandle::HitArea_ZAxis:
                    renderService.renderHandleHighlight(vm::vec3f(m_position) + radius * camera.up());
                    break;
                case RotateObjectsHandle::HitArea_None:
                    break;
                    switchDefault()
            };
        }

        void RotateObjectsHandle::renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea area) {
            const auto radius = static_cast<float>(pref(Preferences::RotateHandleRadius));
            vm::vec3f xAxis, yAxis, zAxis;
            computeAxes(m_position, renderContext.camera().position(), xAxis, yAxis, zAxis);

            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.setShowOccludedObjects();

            switch (area) {
                case RotateObjectsHandle::HitArea_Center:
                    renderService.renderHandleHighlight(vm::vec3f(m_position));
                    renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
                    renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
                    renderService.renderString(StringUtils::toString(m_position), vm::vec3f(m_position));
                    break;
                case RotateObjectsHandle::HitArea_XAxis:
                    renderService.renderHandleHighlight(vm::vec3f(m_position) + radius * xAxis);
                    break;
                case RotateObjectsHandle::HitArea_YAxis:
                    renderService.renderHandleHighlight(vm::vec3f(m_position) + radius * yAxis);
                    break;
                case RotateObjectsHandle::HitArea_ZAxis:
                    renderService.renderHandleHighlight(vm::vec3f(m_position) + radius * zAxis);
                    break;
                case RotateObjectsHandle::HitArea_None:
                    break;
                switchDefault()
            };
        }

        /*
        void RotateObjectsHandle::renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle) {
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Color& pointHandleColor = prefs.get(Preferences::RotateHandleColor);
            
            const vm::vec3f rotationAxis(getRotationAxis(handle));
            const vm::vec3f startAxis(getPointHandleAxis(handle));
            const vm::vec3f endAxis(vm::quat3(rotationAxis, angle) * startAxis);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            
            glAssert(glDisable(GL_DEPTH_TEST));
            {
                glAssert(glDisable(GL_CULL_FACE));
                glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
                Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", getAngleIndicatorColor(handle));
                
                Renderer::Circle circle(handleRadius, 24, true, rotationAxis.firstComponent(), startAxis, endAxis);
                
                setVboState.mapped();
                circle.prepare(m_vbo);
                
                setVboState.active();
                circle.render();
                
                glAssert(glPolygonMode(GL_FRONT, GL_FILL));
                glAssert(glEnable(GL_CULL_FACE));
            }

            m_pointHandleRenderer.setColor(pointHandleColor);
            m_pointHandleRenderer.renderSingleHandle(renderContext, m_position);
            m_pointHandleRenderer.renderSingleHandle(renderContext, getPointHandlePosition(handle));

            glAssert(glEnable(GL_DEPTH_TEST));
        }
        */
        
        Model::Hit RotateObjectsHandle::pickPointHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, const vm::vec3& position, const HitArea area) const {
            const FloatType distance = camera.pickPointHandle(pickRay, position, pref(Preferences::HandleRadius));
            if (vm::isNan(distance))
                return Model::Hit::NoHit;
            return Model::Hit(HandleHit, distance, pickRay.pointAtDistance(distance), area);
        }
        
        Model::Hit RotateObjectsHandle::selectHit(const Model::Hit& closest, const Model::Hit& hit) const {
            if (!closest.isMatch())
                return hit;
            if (hit.isMatch()) {
                if (hit.distance() < closest.distance())
                    return hit;
            }
            return closest;
        }
        
        vm::vec3 RotateObjectsHandle::getPointHandlePosition(const vm::vec3& axis) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return m_position + axis * FloatType(prefs.get(Preferences::RotateHandleRadius));
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
                switchDefault()
            };
        }
    }
}
