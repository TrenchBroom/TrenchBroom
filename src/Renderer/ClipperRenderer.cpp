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

#include "ClipperRenderer.h"

#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Clipper.h"

namespace TrenchBroom {
    namespace Renderer {
        ClipperRenderer::ClipperRenderer(const View::Clipper& clipper) :
        m_vbo(0xFFF),
        m_clipper(clipper),
        m_hasCurrentPoint(false) {}
        
        void ClipperRenderer::setCurrentPoint(const bool hasPoint, const Vec3& point) {
            m_hasCurrentPoint = hasPoint;
            m_currentPoint = point;
        }

        void ClipperRenderer::render(RenderContext& renderContext) {
            renderHandles(renderContext);
        }

        void ClipperRenderer::renderHandles(RenderContext& renderContext) {
            const Vec3::List positions = m_clipper.clipPoints();
            if (positions.empty() && !m_hasCurrentPoint)
                return;
            
            VertexArray handleArray = makeHandleArray();
            VertexArray lineArray = makeLineArray(positions);
            VertexArray triangleArray = makeTriangleArray(positions);
            
            SetVboState setVboState(m_vbo);
            setVboState.mapped();
            handleArray.prepare();
            lineArray.prepare();
            triangleArray.prepare();
            setVboState.active();
            
            renderPointHandles(renderContext, positions, handleArray);
            renderPlaneIndicators(renderContext, lineArray, triangleArray);
        }

        void ClipperRenderer::renderPointHandles(RenderContext& renderContext, const Vec3::List& positions, VertexArray& handleArray) {
            PreferenceManager& prefs = PreferenceManager::instance();
            ActiveShader sphereShader(renderContext.shaderManager(), Shaders::PointHandleShader);
            
            sphereShader.set("CameraPosition", renderContext.camera().position());
            sphereShader.set("ScalingFactor", prefs.getFloat(Preferences::HandleScalingFactor));
            sphereShader.set("MaximumDistance", prefs.getFloat(Preferences::MaximumHandleDistance));

            bool currentPointIsHit = false;
            
            for (size_t i = 0; i < positions.size(); ++i) {
                renderPointHandle(positions[i], sphereShader, handleArray);
                currentPointIsHit |= (positions[i] == m_currentPoint);
            }
            
            if (m_hasCurrentPoint) {
                if (currentPointIsHit) {
                } else {
                    renderPointHandle(m_currentPoint, sphereShader, handleArray);
                }
            }
        }
        
        void ClipperRenderer::renderPointHandle(const Vec3& position, ActiveShader& shader, VertexArray& array) {
            PreferenceManager& prefs = PreferenceManager::instance();

            shader.set("Position", Vec4f(Vec3f(position), 1.0f));
            glDisable(GL_DEPTH_TEST);
            shader.set("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
            array.render();
            glEnable(GL_DEPTH_TEST);
            shader.set("Color", prefs.getColor(Preferences::ClipHandleColor));
            array.render();
        }
        
        void ClipperRenderer::renderPlaneIndicators(RenderContext& renderContext, VertexArray& lineArray, VertexArray& triangleArray) {
            PreferenceManager& prefs = PreferenceManager::instance();
            ActiveShader planeShader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            planeShader.set("Color", prefs.getColor(Preferences::ClipPlaneColor));
            triangleArray.render();
            glEnable(GL_CULL_FACE);
            
            planeShader.set("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
            lineArray.render();
            glEnable(GL_DEPTH_TEST);
            planeShader.set("Color", prefs.getColor(Preferences::ClipHandleColor));
            lineArray.render();
        }

        VertexArray ClipperRenderer::makeHandleArray() {
            PreferenceManager& prefs = PreferenceManager::instance();
            typedef VertexSpecs::P3::Vertex Vertex;

            const Vec3f::List positions = sphere(prefs.getFloat(Preferences::HandleRadius), 1);
            const Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            return VertexArray(m_vbo, GL_TRIANGLES, vertices);
        }
        
        VertexArray ClipperRenderer::makeLineArray(const Vec3::List& positions) {
            if (positions.size() <= 1)
                return VertexArray();
            
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices.push_back(Vertex(Vec3f(positions[i])));
            return VertexArray(m_vbo, GL_LINE_LOOP, vertices);
        }
        
        VertexArray ClipperRenderer::makeTriangleArray(const Vec3::List& positions) {
            if (positions.size() <= 2)
                return VertexArray();
            
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices.push_back(Vertex(Vec3f(positions[i])));
            return VertexArray(m_vbo, GL_TRIANGLE_FAN, vertices);
        }
    }
}
