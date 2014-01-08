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
        m_vbo(0xFFF),
        m_frontRenderer(BrushFilter()),
        m_backRenderer(BrushFilter()),
        m_handleRenderer(m_vbo) {
            PreferenceManager& prefs = PreferenceManager::instance();

            m_frontRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_frontRenderer.setEdgeColor(prefs.get(Preferences::ClipEdgeColor));
            m_frontRenderer.setTintColor(prefs.get(Preferences::ClipFaceColor));
            m_frontRenderer.setOccludedEdgeColor(prefs.get(Preferences::ClipOccludedEdgeColor));
            
            m_backRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_backRenderer.setEdgeColor(prefs.get(Preferences::ClipEdgeColor));
            m_backRenderer.setTintColor(prefs.get(Preferences::ClipFaceColor));
            m_backRenderer.setOccludedEdgeColor(prefs.get(Preferences::ClipOccludedEdgeColor));
            
            m_handleRenderer.setRadius(prefs.get(Preferences::HandleRadius), 1);
        }
        
        void ClipperRenderer::renderClipPoints(RenderContext& renderContext) {
            const Vec3f::List positions = VectorUtils::cast<Vec3f>(m_clipper.clipPointPositions());
            if (positions.empty())
                return;
            
            VertexArray lineArray = makeLineArray(positions);
            VertexArray triangleArray = makeTriangleArray(positions);
            
            SetVboState setVboState(m_vbo);
            setVboState.mapped();
            lineArray.prepare(m_vbo);
            triangleArray.prepare(m_vbo);
            setVboState.active();
            
            m_handleRenderer.renderMultipleHandles(renderContext, positions);
            renderPlaneIndicators(renderContext, lineArray, triangleArray);
        }
        
        void ClipperRenderer::renderHighlight(RenderContext& renderContext, const size_t index) {
            assert(index < m_clipper.numPoints());

            const Vec3f position(m_clipper.clipPointPositions()[index]);
            m_handleRenderer.renderHandleHighlight(renderContext, position);
        }

        void ClipperRenderer::renderBrushes(RenderContext& renderContext) {
            setupBrushRenderer(m_frontRenderer, m_clipper.keepFrontBrushes());
            setupBrushRenderer(m_backRenderer, m_clipper.keepBackBrushes());
            
            m_frontRenderer.render(renderContext);
            m_backRenderer.render(renderContext);
        }
        
        void ClipperRenderer::renderCurrentPoint(RenderContext& renderContext, const Vec3& position) {
            m_handleRenderer.renderSingleHandle(renderContext, Vec3f(position));
        }

        void ClipperRenderer::setBrushes(const Model::BrushList& frontBrushes, const Model::BrushList& backBrushes) {
            m_frontRenderer.setBrushes(frontBrushes);
            m_backRenderer.setBrushes(backBrushes);
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

        VertexArray ClipperRenderer::makeLineArray(const Vec3f::List& positions) {
            if (positions.size() <= 1)
                return VertexArray();
            
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices.push_back(Vertex(positions[i]));
            return VertexArray::swap(GL_LINE_LOOP, vertices);
        }
        
        VertexArray ClipperRenderer::makeTriangleArray(const Vec3f::List& positions) {
            if (positions.size() <= 2)
                return VertexArray();
            
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices.push_back(Vertex(positions[i]));
            return VertexArray::swap(GL_TRIANGLE_FAN, vertices);
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
