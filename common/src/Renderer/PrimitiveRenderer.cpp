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

#include "PrimitiveRenderer.h"

#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        void PrimitiveRenderer::renderLine(const Color& color, const float lineWidth, const Vec3f& start, const Vec3f& end) {
            m_lineMeshes[lineWidth].addLine(Vertex(start, color), Vertex(end, color));
        }
        
        void PrimitiveRenderer::renderLines(const Color& color, const float lineWidth, const Vec3f::List& positions) {
            m_lineMeshes[lineWidth].addLines(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 0, 1, 0, 0));
        }

        void PrimitiveRenderer::renderCoordinateSystemXY(const Color& x, const Color& y, float lineWidth, const BBox3f& bounds) {
            const Vertex::List vertices = BuildCoordinateSystem::xy(x, y).vertices(bounds);
            m_lineMeshes[lineWidth].addLines(vertices);
        }
        
        void PrimitiveRenderer::renderCoordinateSystemXZ(const Color& x, const Color& z, float lineWidth, const BBox3f& bounds) {
            m_lineMeshes[lineWidth].addLines(BuildCoordinateSystem::xz(x, z).vertices(bounds));
        }
        
        void PrimitiveRenderer::renderCoordinateSystemYZ(const Color& y, const Color& z, float lineWidth, const BBox3f& bounds) {
            m_lineMeshes[lineWidth].addLines(BuildCoordinateSystem::yz(y, z).vertices(bounds));
        }

        void PrimitiveRenderer::renderCoordinateSystem3D(const Color& x, const Color& y, const Color& z, const float lineWidth, const BBox3f& bounds) {
            m_lineMeshes[lineWidth].addLines(BuildCoordinateSystem::xyz(x, y, z).vertices(bounds));
        }

        void PrimitiveRenderer::renderPolygon(const Color& color, float lineWidth, const Vec3f::List& positions) {
            m_lineMeshes[lineWidth].addLineLoop(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 0, 1, 0, 0));
        }

        void PrimitiveRenderer::renderFilledPolygon(const Color& color, const Vec3f::List& positions) {
            m_triangleMesh.addTriangleFan(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 0, 1, 0, 0));
        }

        void PrimitiveRenderer::renderCircle(const Color& color, const float lineWidth, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            const std::pair<float, float> angles = startAngleAndLength(normal, startAxis, endAxis);
            renderCircle(color, lineWidth, position, normal, segments, radius, angles.first, angles.second);
        }
        
        void PrimitiveRenderer::renderCircle(const Color& color, const float lineWidth, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            const Vec3f::List positions = circle2D(radius, normal, startAngle, angleLength, segments) + position;
            
            m_lineMeshes[lineWidth].addLineStrip(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 0, 1, 0, 0));
        }
        
        void PrimitiveRenderer::renderFilledCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            const std::pair<float, float> angles = startAngleAndLength(normal, startAxis, endAxis);
            renderFilledCircle(color, position, normal, segments, radius, angles.first, angles.second);
        }
        
        void PrimitiveRenderer::renderFilledCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            const Vec3f::List positions = circle2D(radius, normal, startAngle, angleLength, segments) + position;

            m_triangleMesh.addTriangleFan(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 0, 1, 0, 0));
        }

        void PrimitiveRenderer::doPrepareVertices(Vbo& vertexVbo) {
            LineMeshMap::iterator it, end;
            for (it = m_lineMeshes.begin(), end = m_lineMeshes.end(); it != end; ++it) {
                const float lineWidth = it->first;
                IndexRangeMapBuilder<Vertex::Spec>& mesh = it->second;
                IndexRangeRenderer& renderer = m_lineMeshRenderers.insert(std::make_pair(lineWidth, IndexRangeRenderer(mesh))).first->second;
                renderer.prepare(vertexVbo);
            }
            
            m_triangleMeshRenderer = IndexRangeRenderer(m_triangleMesh);
            m_triangleMeshRenderer.prepare(vertexVbo);
        }
        
        void PrimitiveRenderer::doRender(RenderContext& renderContext) {
            glAssert(glDisable(GL_DEPTH_TEST));
            renderLines(renderContext);
            renderTriangles(renderContext);
            glAssert(glEnable(GL_DEPTH_TEST));
        }

        void PrimitiveRenderer::renderLines(RenderContext& renderContext) {
            LineMeshRendererMap::iterator it, end;
            for (it = m_lineMeshRenderers.begin(), end = m_lineMeshRenderers.end(); it != end; ++it) {
                const float lineWidth = it->first;
                IndexRangeRenderer& renderer = it->second;
                glAssert(glLineWidth(lineWidth));
                renderer.render();
            }
            glAssert(glLineWidth(1.0f));
        }
        
        void PrimitiveRenderer::renderTriangles(RenderContext& renderContext) {
            m_triangleMeshRenderer.render();
        }
    }
}
