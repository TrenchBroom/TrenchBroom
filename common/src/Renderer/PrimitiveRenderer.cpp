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

#include "PrimitiveRenderer.h"

#include "TrenchBroom.h"
#include "Color.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/scalar.h>

namespace TrenchBroom {
    namespace Renderer {
        PrimitiveRenderer::LineRenderAttributes::LineRenderAttributes(const Color& color, const float lineWidth, const OcclusionPolicy occlusionPolicy) :
        m_color(color),
        m_lineWidth(lineWidth),
        m_occlusionPolicy(occlusionPolicy),
        m_cullingPolicy(CP_ShowBackfaces) {}
        
        

        bool PrimitiveRenderer::LineRenderAttributes::operator<(const LineRenderAttributes& other) const {
            // As a special exception, sort by descending alpha so opaque batches render first.
            if (m_color.a() < other.m_color.a())
                return false;
            if (m_color.a() > other.m_color.a())
                return true;
            // alpha is equal; continue with the regular comparison.
            
            if (m_lineWidth < other.m_lineWidth)
                return true;
            if (m_lineWidth > other.m_lineWidth)
                return false;
            if (m_color < other.m_color)
                return true;
            if (m_color > other.m_color)
                return false;
            if (m_occlusionPolicy < other.m_occlusionPolicy)
                return true;
            if (m_occlusionPolicy > other.m_occlusionPolicy)
                return false;
            return false;
        }
        
        void PrimitiveRenderer::LineRenderAttributes::render(IndexRangeRenderer& renderer, ActiveShader& shader) const {
            glAssert(glLineWidth(m_lineWidth));
            switch (m_occlusionPolicy) {
                case OP_Hide:
                    shader.set("Color", m_color);
                    renderer.render();
                    break;
                case OP_Show:
                    glAssert(glDisable(GL_DEPTH_TEST));
                    shader.set("Color", m_color);
                    renderer.render();
                    glAssert(glEnable(GL_DEPTH_TEST));
                    break;
                case OP_Transparent:
                    glAssert(glDisable(GL_DEPTH_TEST));
                    shader.set("Color", Color(m_color, m_color.a() / 3.0f));
                    renderer.render();
                    glAssert(glEnable(GL_DEPTH_TEST));
                    shader.set("Color", m_color);
                    renderer.render();
                    break;
            }
        }

        PrimitiveRenderer::TriangleRenderAttributes::TriangleRenderAttributes(const Color& color, const OcclusionPolicy occlusionPolicy, CullingPolicy cullingPolicy) :
        m_color(color),
        m_occlusionPolicy(occlusionPolicy),
        m_cullingPolicy(cullingPolicy) {}
        
        bool PrimitiveRenderer::TriangleRenderAttributes::operator<(const TriangleRenderAttributes& other) const {
            // As a special exception, sort by descending alpha so opaque batches render first.
            if (m_color.a() < other.m_color.a())
                return false;
            if (m_color.a() > other.m_color.a())
                return true;
            // alpha is equal; continue with the regular comparison.

            if (m_color < other.m_color)
                return true;
            if (m_color > other.m_color)
                return false;
            if (m_occlusionPolicy < other.m_occlusionPolicy)
                return true;
            if (m_occlusionPolicy > other.m_occlusionPolicy)
                return false;
            if (m_cullingPolicy < other.m_cullingPolicy)
                return true;
            if (m_cullingPolicy > other.m_cullingPolicy)
                return false;
            return false;
        }
        
        void PrimitiveRenderer::TriangleRenderAttributes::render(IndexRangeRenderer& renderer, ActiveShader& shader) const {
            if (m_cullingPolicy == CP_ShowBackfaces) {
                glAssert(glPushAttrib(GL_POLYGON_BIT));
                glAssert(glDisable(GL_CULL_FACE));
                glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }
            
            // Disable depth writes if drawing something transparent
            if (m_color.a() < 1.0) {
                glAssert(glDepthMask(GL_FALSE));
            }
            
            switch (m_occlusionPolicy) {
                case OP_Hide:
                    shader.set("Color", m_color);
                    renderer.render();
                    break;
                case OP_Show:
                    glAssert(glDisable(GL_DEPTH_TEST));
                    shader.set("Color", m_color);
                    renderer.render();
                    glAssert(glEnable(GL_DEPTH_TEST));
                    break;
                case OP_Transparent:
                    glAssert(glDisable(GL_DEPTH_TEST));
                    shader.set("Color", Color(m_color, m_color.a() / 2.0f));
                    renderer.render();
                    glAssert(glEnable(GL_DEPTH_TEST));
                    shader.set("Color", m_color);
                    renderer.render();
                    break;
            }
            
            if (m_color.a() < 1.0) {
                glAssert(glDepthMask(GL_TRUE));
            }
            
            if (m_cullingPolicy == CP_ShowBackfaces) {
                glAssert(glPopAttrib());
            }
        }

        void PrimitiveRenderer::renderLine(const Color& color, const float lineWidth, const OcclusionPolicy occlusionPolicy, const vm::vec3f& start, const vm::vec3f& end) {
            m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLine(Vertex(start), Vertex(end));
        }
        
        void PrimitiveRenderer::renderLines(const Color& color, const float lineWidth, const OcclusionPolicy occlusionPolicy, const std::vector<vm::vec3f>& positions) {
            m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLines(Vertex::toList(std::begin(positions), positions.size()));
        }

        void PrimitiveRenderer::renderLineStrip(const Color& color, const float lineWidth, const OcclusionPolicy occlusionPolicy, const std::vector<vm::vec3f>& positions) {
            m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLineStrip(Vertex::toList(std::begin(positions), positions.size()));
        }

        void PrimitiveRenderer::renderCoordinateSystemXY(const Color& x, const Color& y, float lineWidth, const OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds) {
            vm::vec3f start, end;

            coordinateSystemVerticesX(bounds, start, end);
            renderLine(x, lineWidth, occlusionPolicy, start, end);
            
            coordinateSystemVerticesY(bounds, start, end);
            renderLine(y, lineWidth, occlusionPolicy, start, end);
        }
        
        void PrimitiveRenderer::renderCoordinateSystemXZ(const Color& x, const Color& z, float lineWidth, const OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds) {
            vm::vec3f start, end;
            
            coordinateSystemVerticesX(bounds, start, end);
            renderLine(x, lineWidth, occlusionPolicy, start, end);
            
            coordinateSystemVerticesZ(bounds, start, end);
            renderLine(z, lineWidth, occlusionPolicy, start, end);
        }
        
        void PrimitiveRenderer::renderCoordinateSystemYZ(const Color& y, const Color& z, float lineWidth, const OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds) {
            vm::vec3f start, end;
            
            coordinateSystemVerticesY(bounds, start, end);
            renderLine(y, lineWidth, occlusionPolicy, start, end);

            coordinateSystemVerticesZ(bounds, start, end);
            renderLine(z, lineWidth, occlusionPolicy, start, end);
        }

        void PrimitiveRenderer::renderCoordinateSystem3D(const Color& x, const Color& y, const Color& z, const float lineWidth, const OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds) {
            vm::vec3f start, end;
            
            coordinateSystemVerticesX(bounds, start, end);
            renderLine(x, lineWidth, occlusionPolicy, start, end);
            
            coordinateSystemVerticesY(bounds, start, end);
            renderLine(y, lineWidth, occlusionPolicy, start, end);
            
            coordinateSystemVerticesZ(bounds, start, end);
            renderLine(z, lineWidth, occlusionPolicy, start, end);
        }

        void PrimitiveRenderer::renderPolygon(const Color& color, float lineWidth, const OcclusionPolicy occlusionPolicy, const std::vector<vm::vec3f>& positions) {
            m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLineLoop(Vertex::toList(std::begin(positions), positions.size()));
        }

        void PrimitiveRenderer::renderFilledPolygon(const Color& color, const OcclusionPolicy occlusionPolicy, CullingPolicy cullingPolicy, const std::vector<vm::vec3f>& positions) {
            m_triangleMeshes[TriangleRenderAttributes(color, occlusionPolicy, cullingPolicy)].addTriangleFan(Vertex::toList(std::begin(positions), positions.size()));
        }

        void PrimitiveRenderer::renderCylinder(const Color& color, const float radius, const size_t segments, const OcclusionPolicy occlusionPolicy, CullingPolicy cullingPolicy, const vm::vec3f& start, const vm::vec3f& end) {
            assert(radius > 0.0);
            assert(segments > 2);
            
            const vm::vec3f vec = end - start;
            const float len = vm::length(vec);
            const vm::vec3f dir = vec / len;
            
            const vm::mat4x4f translation = vm::translationMatrix(start);
            const vm::mat4x4f rotation    = vm::rotationMatrix(vm::vec3f::pos_z, dir);
            const vm::mat4x4f transform   = translation * rotation;
            
            const VertsAndNormals cylinder = cylinder3D(radius, len, segments);
            const std::vector<vm::vec3f> vertices = transform * cylinder.vertices;
            
            m_triangleMeshes[TriangleRenderAttributes(color, occlusionPolicy, cullingPolicy)].addTriangleStrip(Vertex::toList(std::begin(vertices), vertices.size()));
        }

        void PrimitiveRenderer::doPrepareVertices(Vbo& vertexVbo) {
            prepareLines(vertexVbo);
            prepareTriangles(vertexVbo);
        }
        
        void PrimitiveRenderer::prepareLines(Vbo& vertexVbo) {
            for (auto& entry : m_lineMeshes) {
                const LineRenderAttributes& attributes = entry.first;
                IndexRangeMapBuilder<Vertex::Spec>& mesh = entry.second;
                IndexRangeRenderer& renderer = m_lineMeshRenderers.insert(std::make_pair(attributes, IndexRangeRenderer(mesh))).first->second;
                renderer.prepare(vertexVbo);
            }
        }
        
        void PrimitiveRenderer::prepareTriangles(Vbo& vertexVbo) {
            for (auto& entry : m_triangleMeshes) {
                const TriangleRenderAttributes& attributes = entry.first;
                IndexRangeMapBuilder<Vertex::Spec>& mesh = entry.second;
                IndexRangeRenderer& renderer = m_triangleMeshRenderers.insert(std::make_pair(attributes, IndexRangeRenderer(mesh))).first->second;
                renderer.prepare(vertexVbo);
            }
        }

        void PrimitiveRenderer::doRender(RenderContext& renderContext) {
            renderLines(renderContext);
            renderTriangles(renderContext);
        }

        void PrimitiveRenderer::renderLines(RenderContext& renderContext) {
            ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPUniformCShader);
            
            for (auto& entry : m_lineMeshRenderers) {
                const LineRenderAttributes& attributes = entry.first;
                IndexRangeRenderer& renderer = entry.second;
                attributes.render(renderer, shader);
            }
            glAssert(glLineWidth(1.0f));
        }
        
        void PrimitiveRenderer::renderTriangles(RenderContext& renderContext) {
            ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPUniformCShader);
            
            for (auto& entry : m_triangleMeshRenderers) {
                const TriangleRenderAttributes& attributes = entry.first;
                IndexRangeRenderer& renderer = entry.second;
                attributes.render(renderer, shader);
            }
        }
    }
}
