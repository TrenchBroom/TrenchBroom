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

#include "PointHandleRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexSpec.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        PointHandleRenderer::PointHandleRenderer(Vbo& vbo, const float radius, const size_t iterations) :
        m_vbo(vbo),
        m_sphere(radius, iterations),
        m_circle(2.0f * radius, 16, false),
        m_color(1.0f, 1.0f, 1.0f, 1.0f),
        m_occludedColor(1.0f, 1.0f, 1.0f, 0.5f),
        m_highlightColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_renderOccluded(true) {}
        
        void PointHandleRenderer::setRadius(float radius, size_t iterations) {
            m_sphere = Sphere(radius, iterations);
            m_circle = Circle(2.0f * radius, 16, false);
        }

        void PointHandleRenderer::setPositions(Vec3f::List& positions) {
            using std::swap;
            swap(m_positions, positions);
        }
        
        void PointHandleRenderer::setColor(const Color& color) {
            m_color = color;
        }
        
        void PointHandleRenderer::setOccludedColor(const Color& occludedColor) {
            m_occludedColor = occludedColor;
        }
        
        void PointHandleRenderer::setHighlightColor(const Color& highlightColor) {
            m_highlightColor = highlightColor;
        }

        void PointHandleRenderer::setRenderOccluded(const bool renderOccluded) {
            m_renderOccluded = renderOccluded;
        }

        void PointHandleRenderer::renderSingleHandle(RenderContext& renderContext, const Vec3f& position) {
            SetVboState activateVbo(m_vbo);
            activateVbo.active();
            prepare();

            ActiveShader shader(renderContext.shaderManager(), Shaders::PointHandleShader);
            setupHandle(renderContext, shader);
            renderHandle(position, shader);
        }
        
        void PointHandleRenderer::renderMultipleHandles(RenderContext& renderContext, const Vec3f::List& positions) {
            SetVboState activateVbo(m_vbo);
            activateVbo.active();
            prepare();
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::PointHandleShader);
            setupHandle(renderContext, shader);
            
            Vec3f::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it) {
                const Vec3f& position = *it;
                renderHandle(position, shader);
            }
        }
        
        void PointHandleRenderer::renderStoredHandles(RenderContext& renderContext) {
            renderMultipleHandles(renderContext, m_positions);
        }

        void PointHandleRenderer::renderHandleHighlight(RenderContext& renderContext, const Vec3f& position) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float scaling = static_cast<float>(prefs.get(Preferences::HandleScalingFactor));
            
            const Camera& camera = renderContext.camera();
            const Mat4x4f billboardMatrix = camera.orthogonalBillboardMatrix();
            const float factor = camera.distanceTo(position) * scaling;
            const Mat4x4f matrix = translationMatrix(position) * billboardMatrix * scalingMatrix(Vec3f(factor, factor, 0.0f));
            MultiplyModelMatrix billboard(renderContext.transformation(), matrix);

            SetVboState activateVbo(m_vbo);
            activateVbo.active();
            prepare();

            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);
            shader.set("Color", m_highlightColor);
            
            glDisable(GL_DEPTH_TEST);
            m_circle.render();
            glEnable(GL_DEPTH_TEST);
        }

        void PointHandleRenderer::prepare() {
            if (!m_sphere.prepared() || !m_circle.prepared()) {
                SetVboState mapVbo(m_vbo);
                mapVbo.mapped();
                if (!m_sphere.prepared())
                    m_sphere.prepare(m_vbo);
                if (!m_circle.prepared())
                    m_circle.prepare(m_vbo);
            }
        }

        void PointHandleRenderer::setupHandle(RenderContext& renderContext, ActiveShader& shader) {
            PreferenceManager& prefs = PreferenceManager::instance();
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("ScalingFactor", prefs.get(Preferences::HandleScalingFactor));
            shader.set("MaximumDistance", prefs.get(Preferences::MaximumHandleDistance));
        }
        
        void PointHandleRenderer::renderHandle(const Vec3f& position, ActiveShader& shader) {
            shader.set("Position", Vec4f(Vec3f(position), 1.0f));
            if (m_renderOccluded) {
                glDisable(GL_DEPTH_TEST);
                shader.set("Color", m_occludedColor);
                m_sphere.render();
                glEnable(GL_DEPTH_TEST);
            }
            shader.set("Color", m_color);
            m_sphere.render();
        }
    }
}
