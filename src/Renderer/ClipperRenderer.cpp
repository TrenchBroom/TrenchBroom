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
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Clipper.h"

namespace TrenchBroom {
    namespace Renderer {
        struct BrushFilter : public Renderer::BrushRenderer::Filter {
            bool operator()(const Model::Brush* brush) const {
                return true;
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return true;
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                return true;
            }
        };

        
        ClipperRenderer::ClipperRenderer(const View::Clipper& clipper) :
        m_clipper(clipper),
        m_frontRenderer(BrushFilter()),
        m_backRenderer(BrushFilter()),
        m_vbo(0xFFF) {
            PreferenceManager& prefs = PreferenceManager::instance();

            m_frontRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_frontRenderer.setEdgeColor(prefs.get(Preferences::ClipEdgeColor));
            m_frontRenderer.setTintColor(prefs.get(Preferences::ClipFaceColor));
            m_frontRenderer.setOccludedEdgeColor(prefs.get(Preferences::ClipOccludedEdgeColor));
            
            m_backRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_backRenderer.setEdgeColor(prefs.get(Preferences::ClipEdgeColor));
            m_backRenderer.setTintColor(prefs.get(Preferences::ClipFaceColor));
            m_backRenderer.setOccludedEdgeColor(prefs.get(Preferences::ClipOccludedEdgeColor));
        }
        
        void ClipperRenderer::renderClipPoints(RenderContext& renderContext) {
            const Vec3::List positions = m_clipper.clipPointPositions();
            if (positions.empty())
                return;
            
            Sphere pointHandle = makePointHandle();
            VertexArray lineArray = makeLineArray(positions);
            VertexArray triangleArray = makeTriangleArray(positions);
            
            SetVboState setVboState(m_vbo);
            setVboState.mapped();
            pointHandle.prepare();
            lineArray.prepare();
            triangleArray.prepare();
            setVboState.active();
            
            renderPointHandles(renderContext, positions, pointHandle);
            renderPlaneIndicators(renderContext, lineArray, triangleArray);
        }
        
        void ClipperRenderer::renderHighlight(RenderContext& renderContext, const size_t index) {
            assert(index < m_clipper.numPoints());

            PreferenceManager& prefs = PreferenceManager::instance();
            const float scaling = prefs.get(Preferences::HandleScalingFactor);
            const Vec3 position = m_clipper.clipPointPositions()[index];

            const Camera& camera = renderContext.camera();
            const Mat4x4f billboardMatrix = camera.orthogonalBillboardMatrix();
            const float factor = camera.distanceTo(position) * scaling;
            const Mat4x4f matrix = translationMatrix(position) * billboardMatrix * scalingMatrix(Vec3f(factor, factor, 0.0f));
            MultiplyModelMatrix billboard(renderContext.transformation(), matrix);
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);
            shader.set("Color", prefs.get(Preferences::SelectedHandleColor));

            Circle highlight = makePointHandleHighlight();
            SetVboState setVboState(m_vbo);
            setVboState.mapped();
            highlight.prepare();
            setVboState.active();

            glDisable(GL_DEPTH_TEST);
            highlight.render();
            glEnable(GL_DEPTH_TEST);
        }

        void ClipperRenderer::renderBrushes(RenderContext& renderContext) {
            setupBrushRenderer(m_frontRenderer, m_clipper.keepFrontBrushes());
            setupBrushRenderer(m_backRenderer, m_clipper.keepBackBrushes());
            
            m_frontRenderer.render(renderContext);
            m_backRenderer.render(renderContext);
        }
        
        void ClipperRenderer::renderCurrentPoint(RenderContext& renderContext, const Vec3& position) {
            Sphere pointHandle = makePointHandle();
            
            SetVboState setVboState(m_vbo);
            setVboState.mapped();
            pointHandle.prepare();
            setVboState.active();

            PreferenceManager& prefs = PreferenceManager::instance();
            ActiveShader sphereShader(renderContext.shaderManager(), Shaders::PointHandleShader);
            sphereShader.set("CameraPosition", renderContext.camera().position());
            sphereShader.set("ScalingFactor", prefs.get(Preferences::HandleScalingFactor));
            sphereShader.set("MaximumDistance", prefs.get(Preferences::MaximumHandleDistance));

            renderPointHandle(position, sphereShader, pointHandle,
                              prefs.get(Preferences::HandleColor),
                              prefs.get(Preferences::OccludedHandleColor));
        }

        void ClipperRenderer::setBrushes(const Model::BrushList& frontBrushes, const Model::BrushList& backBrushes) {
            m_frontRenderer.setBrushes(frontBrushes);
            m_backRenderer.setBrushes(backBrushes);
        }

        void ClipperRenderer::renderPointHandles(RenderContext& renderContext, const Vec3::List& positions, Sphere& pointHandle) {
            PreferenceManager& prefs = PreferenceManager::instance();
            ActiveShader sphereShader(renderContext.shaderManager(), Shaders::PointHandleShader);
            
            sphereShader.set("CameraPosition", renderContext.camera().position());
            sphereShader.set("ScalingFactor", prefs.get(Preferences::HandleScalingFactor));
            sphereShader.set("MaximumDistance", prefs.get(Preferences::MaximumHandleDistance));
            
            for (size_t i = 0; i < positions.size(); ++i) {
                renderPointHandle(positions[i], sphereShader, pointHandle,
                                  prefs.get(Preferences::HandleColor),
                                  prefs.get(Preferences::OccludedHandleColor));
            }
        }
        
        void ClipperRenderer::renderPointHandle(const Vec3& position, ActiveShader& shader, Sphere& pointHandle, const Color& color, const Color& occludedColor) {
            shader.set("Position", Vec4f(Vec3f(position), 1.0f));
            glDisable(GL_DEPTH_TEST);
            shader.set("Color", color);
            pointHandle.render();
            glEnable(GL_DEPTH_TEST);
            shader.set("Color", occludedColor);
            pointHandle.render();
        }
        
        void ClipperRenderer::renderPlaneIndicators(RenderContext& renderContext, VertexArray& lineArray, VertexArray& triangleArray) {
            PreferenceManager& prefs = PreferenceManager::instance();
            ActiveShader planeShader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            planeShader.set("Color", prefs.get(Preferences::ClipPlaneColor));
            triangleArray.render();
            glEnable(GL_CULL_FACE);
            
            planeShader.set("Color", prefs.get(Preferences::OccludedHandleColor));
            lineArray.render();
            glEnable(GL_DEPTH_TEST);
            planeShader.set("Color", prefs.get(Preferences::HandleColor));
            lineArray.render();
        }

        Sphere ClipperRenderer::makePointHandle() {
            PreferenceManager& prefs = PreferenceManager::instance();
            return Sphere(m_vbo, prefs.get(Preferences::HandleRadius), 1);
        }
        
        Circle ClipperRenderer::makePointHandleHighlight() {
            PreferenceManager& prefs = PreferenceManager::instance();
            return Circle(m_vbo, 2.0f * prefs.get(Preferences::HandleRadius), 16, false);
        }
        
        VertexArray ClipperRenderer::makeLineArray(const Vec3::List& positions) {
            if (positions.size() <= 1)
                return VertexArray();
            
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices.push_back(Vertex(Vec3f(positions[i])));
            return VertexArray::swap(m_vbo, GL_LINE_LOOP, vertices);
        }
        
        VertexArray ClipperRenderer::makeTriangleArray(const Vec3::List& positions) {
            if (positions.size() <= 2)
                return VertexArray();
            
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices.push_back(Vertex(Vec3f(positions[i])));
            return VertexArray::swap(m_vbo, GL_TRIANGLE_FAN, vertices);
        }

        void ClipperRenderer::setupBrushRenderer(BrushRenderer& renderer, const bool keep) {
            if (keep) {
                renderer.setTintFaces(true);
                renderer.setRenderOccludedEdges(true);
                renderer.setGrayscale(false);
            } else {
                renderer.setTintFaces(false);
                renderer.setRenderOccludedEdges(false);
                renderer.setGrayscale(true);
            }
        }
    }
}
