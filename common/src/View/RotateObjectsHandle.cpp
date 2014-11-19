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
        
        const Vec3& RotateObjectsHandle::position() const {
            return m_position;
        }

        void RotateObjectsHandle::setPosition(const Vec3& position) {
            m_position = position;
        }
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pick(const Ray3& pickRay, const Vec3& cameraPos) const {
            Vec3 xAxis, yAxis, zAxis;
            computeAxes(cameraPos, xAxis, yAxis, zAxis);
            Hit hit = pickPointHandle(pickRay, m_position, HitArea_Center);
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(xAxis), HitArea_XAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(yAxis), HitArea_YAxis));
            hit = selectHit(hit, pickPointHandle(pickRay, getPointHandlePosition(zAxis), HitArea_ZAxis));
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
        
        class RotateObjectsHandle::Render2DHandle : public Renderer::Renderable {
        private:
            float m_radius;
            Vec3f m_position;
            Vec3f m_cameraDirection;
            Vec3f m_cameraRight;
            HitArea m_highlight;
            Renderer::Circle m_ring;
            Renderer::PointHandleRenderer m_pointHandleRenderer;
        public:
            Render2DHandle(const float radius, const Vec3& position, const Vec3& cameraDirection, const Vec3& cameraRight, const HitArea highlight) :
            m_radius(radius),
            m_position(position),
            m_cameraDirection(cameraDirection),
            m_cameraRight(cameraRight),
            m_highlight(highlight),
            m_ring(m_radius, 24, false, m_cameraDirection.firstComponent(), 0.0f, Math::Cf::twoPi()) {
                PreferenceManager& prefs = PreferenceManager::instance();
                m_pointHandleRenderer.setRadius(prefs.get(Preferences::HandleRadius), 1);
                m_pointHandleRenderer.setRenderOccluded(false);
            }
        private:
            void doPrepare(Renderer::Vbo& vbo) {
                m_ring.prepare(vbo),
                m_pointHandleRenderer.prepare(vbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) {
                renderTranslated(renderContext);
                renderPointHandles(renderContext);
            }

            void renderTranslated(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
                
                const BBox3f bounds(handleRadius);
                Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));

                renderRing(renderContext);
            }

            void renderRing(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                switch (m_cameraDirection.firstComponent()) {
                    case Math::Axis::AX:
                        shader.set("Color", prefs.get(Preferences::XAxisColor));
                        break;
                    case Math::Axis::AY:
                        shader.set("Color", prefs.get(Preferences::YAxisColor));
                        break;
                    case Math::Axis::AZ:
                        shader.set("Color", prefs.get(Preferences::ZAxisColor));
                        break;
                };
                
                m_ring.render();
            }
            
            void renderPointHandles(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                m_pointHandleRenderer.setColor(prefs.get(Preferences::RotateHandleColor));
                m_pointHandleRenderer.setHighlightColor(prefs.get(Preferences::SelectedHandleColor));
                
                m_pointHandleRenderer.clear();
                
                m_pointHandleRenderer.addPoint(m_position);
                m_pointHandleRenderer.addPoint(m_position + m_radius * m_cameraRight);
                
                switch (m_highlight) {
                    case RotateObjectsHandle::HitArea_Center:
                        m_pointHandleRenderer.addHighlight(m_position);
                        break;
                    case RotateObjectsHandle::HitArea_XAxis:
                    case RotateObjectsHandle::HitArea_YAxis:
                    case RotateObjectsHandle::HitArea_ZAxis:
                        m_pointHandleRenderer.addHighlight(m_position + m_radius * m_cameraRight);
                        break;
                    case RotateObjectsHandle::HitArea_None:
                        break;
                    DEFAULT_SWITCH()
                };
                
                m_pointHandleRenderer.render(renderContext);
            }
        };
        
        class RotateObjectsHandle::Render3DHandle : public Renderer::Renderable {
        private:
            float m_radius;
            Vec3f m_position;
            Vec3f m_xAxis;
            Vec3f m_yAxis;
            Vec3f m_zAxis;
            HitArea m_highlight;
            Renderer::VertexArray m_axesArray;
            Renderer::Circle m_xRing;
            Renderer::Circle m_yRing;
            Renderer::Circle m_zRing;
            Renderer::Circle m_xRingIndicator;
            Renderer::Circle m_yRingIndicator;
            Renderer::Circle m_zRingIndicator;
            Renderer::PointHandleRenderer m_pointHandleRenderer;
        public:
            Render3DHandle(const float radius, const Vec3& position, const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis, const HitArea highlight) :
            m_radius(radius),
            m_position(position),
            m_xAxis(xAxis),
            m_yAxis(yAxis),
            m_zAxis(zAxis),
            m_highlight(highlight),
            m_axesArray(axesArray(m_radius)),
            m_xRing(m_radius, 24, false, Math::Axis::AX, m_zAxis, m_yAxis),
            m_yRing(m_radius, 24, false, Math::Axis::AY, m_xAxis, m_zAxis),
            m_zRing(m_radius, 24, false, Math::Axis::AZ, m_xAxis, m_yAxis),
            m_xRingIndicator(m_radius, 8, false, Math::Axis::AX,
                             Quatf(Vec3f::PosX, Math::radians(+15.0f)) * m_yAxis,
                             Quatf(Vec3f::PosX, Math::radians(-15.0f)) * m_yAxis),
            m_yRingIndicator(m_radius, 8, false, Math::Axis::AY,
                             Quatf(Vec3f::PosY, Math::radians(+15.0f)) * m_zAxis,
                             Quatf(Vec3f::PosY, Math::radians(-15.0f)) * m_zAxis),
            m_zRingIndicator(m_radius, 8, false, Math::Axis::AZ,
                             Quatf(Vec3f::PosZ, Math::radians(+15.0f)) * m_xAxis,
                             Quatf(Vec3f::PosZ, Math::radians(-15.0f)) * m_xAxis) {
                PreferenceManager& prefs = PreferenceManager::instance();
                m_pointHandleRenderer.setRadius(prefs.get(Preferences::HandleRadius), 1);
                m_pointHandleRenderer.setRenderOccluded(false);
            }
        private:
            static Renderer::VertexArray axesArray(const float radius) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const Color& xColor = prefs.get(Preferences::XAxisColor);
                const Color& yColor = prefs.get(Preferences::YAxisColor);
                const Color& zColor = prefs.get(Preferences::ZAxisColor);
                
                const BBox3f bounds(radius);
                Renderer::VertexSpecs::P3C4::Vertex::List vertices = Renderer::coordinateSystem(bounds, xColor, yColor, zColor);
                return Renderer::VertexArray::swap(GL_LINES, vertices);
            }
            
            void doPrepare(Renderer::Vbo& vbo) {
                m_axesArray.prepare(vbo);
                m_xRing.prepare(vbo);
                m_yRing.prepare(vbo);
                m_zRing.prepare(vbo);
                m_xRingIndicator.prepare(vbo);
                m_yRingIndicator.prepare(vbo);
                m_zRingIndicator.prepare(vbo);
                m_pointHandleRenderer.prepare(vbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) {
                glDisable(GL_DEPTH_TEST);
                renderTranslated(renderContext);
                renderPointHandles(renderContext);
                glEnable(GL_DEPTH_TEST);
            }

            void renderTranslated(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
                
                const BBox3f bounds(handleRadius);
                Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
                
                renderAxes(renderContext);
                renderRings(renderContext);
                renderRingIndicators(renderContext);
            }
            
            void renderAxes(Renderer::RenderContext& renderContext) {
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPCShader);
                m_axesArray.render();
            }
            
            void renderRings(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const Color& xColor = prefs.get(Preferences::XAxisColor);
                const Color& yColor = prefs.get(Preferences::YAxisColor);
                const Color& zColor = prefs.get(Preferences::ZAxisColor);

                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", xColor);
                m_xRing.render();
                shader.set("Color", yColor);
                m_yRing.render();
                shader.set("Color", zColor);
                m_zRing.render();
            }
            
            void renderRingIndicators(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const Color& color = prefs.get(Preferences::RotateHandleColor);

                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", color);

                m_xRingIndicator.render();
                m_yRingIndicator.render();
                m_zRingIndicator.render();
            }
            
            void renderPointHandles(Renderer::RenderContext& renderContext) {
                PreferenceManager& prefs = PreferenceManager::instance();
                m_pointHandleRenderer.setColor(prefs.get(Preferences::RotateHandleColor));
                m_pointHandleRenderer.setHighlightColor(prefs.get(Preferences::SelectedHandleColor));

                m_pointHandleRenderer.clear();
                
                m_pointHandleRenderer.addPoint(m_position);
                m_pointHandleRenderer.addPoint(m_position + m_radius * m_xAxis);
                m_pointHandleRenderer.addPoint(m_position + m_radius * m_yAxis);
                m_pointHandleRenderer.addPoint(m_position + m_radius * m_zAxis);
                
                switch (m_highlight) {
                    case RotateObjectsHandle::HitArea_Center:
                        m_pointHandleRenderer.addHighlight(m_position);
                        break;
                    case RotateObjectsHandle::HitArea_XAxis:
                        m_pointHandleRenderer.addHighlight(m_position + m_radius * m_xAxis);
                        break;
                    case RotateObjectsHandle::HitArea_YAxis:
                        m_pointHandleRenderer.addHighlight(m_position + m_radius * m_yAxis);
                        break;
                    case RotateObjectsHandle::HitArea_ZAxis:
                        m_pointHandleRenderer.addHighlight(m_position + m_radius * m_zAxis);
                        break;
                    case RotateObjectsHandle::HitArea_None:
                        break;
                        DEFAULT_SWITCH()
                };
                
                m_pointHandleRenderer.render(renderContext);
            }
        };
        
        void RotateObjectsHandle::renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea highlight) {
            if (renderContext.render2D())
                render2DHandle(renderContext, renderBatch, highlight);
            if (renderContext.render3D())
                render3DHandle(renderContext, renderBatch, highlight);
        }
        
        void RotateObjectsHandle::render2DHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea highlight) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float radius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            
            const Renderer::Camera& camera = renderContext.camera();
            
            renderBatch.addOneShot(new Render2DHandle(radius, m_position, camera.direction(), camera.right(), highlight));
        }
        
        void RotateObjectsHandle::render3DHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HitArea highlight) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float radius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));

            Vec3 xAxis, yAxis, zAxis;
            computeAxes(Vec3(renderContext.camera().position()), xAxis, yAxis, zAxis);

            renderBatch.addOneShot(new Render3DHandle(radius, m_position, xAxis, yAxis, zAxis, highlight));
        }

        void RotateObjectsHandle::computeAxes(const Vec3& cameraPos, Vec3& xAxis, Vec3& yAxis, Vec3& zAxis) const {
            const Vec3 viewDir = (m_position - cameraPos).normalized();
            if (Math::eq(std::abs(viewDir.z()), 1.0)) {
                xAxis = Vec3::PosX;
                yAxis = Vec3::PosY;
            } else {
                xAxis = viewDir.x() > 0.0 ? Vec3::NegX : Vec3::PosX;
                yAxis = viewDir.y() > 0.0 ? Vec3::NegY : Vec3::PosY;
            }
            zAxis = viewDir.z() > 0.0 ? Vec3::NegZ : Vec3::PosZ;
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
        
        RotateObjectsHandle::Hit RotateObjectsHandle::pickPointHandle(const Ray3& pickRay, const Vec3& position, const HitArea area) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType radius = 2.0 * prefs.get(Preferences::HandleRadius);
            const FloatType maxDist = prefs.get(Preferences::MaximumHandleDistance);
            const FloatType distance = pickRay.intersectWithSphere(position, radius, maxDist);
            
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
