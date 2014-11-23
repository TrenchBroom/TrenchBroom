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
        void PrimitiveRenderer::renderLine(const Color& color, const Vec3f& start, const Vec3f& end) {
            m_lineMesh.beginLines();
            m_lineMesh.addLine(Vertex(start, color), Vertex(end, color));
            m_lineMesh.endLines();
        }
        
        void PrimitiveRenderer::renderLines(const Color& color, const Vec3f::List& positions) {
            m_lineMesh.beginLines();
            m_lineMesh.addLines(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 1, 0));
            m_lineMesh.endLines();
        }

        void PrimitiveRenderer::renderCoordinateSystem(const BBox3f& bounds, const Color& x, const Color& y, const Color& z) {
            const Vertex::List vertices = coordinateSystem(bounds, x, y, z);
            m_lineMesh.beginLines();
            m_lineMesh.addLines(vertices);
            m_lineMesh.endLines();
        }

        void PrimitiveRenderer::renderCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            const std::pair<float, float> angles = startAngleAndLength(normal, startAxis, endAxis);
            renderCircle(color, position, normal, segments, radius, angles.first, angles.second);
        }
        
        void PrimitiveRenderer::renderCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            const Vec3f::List positions = circle2D(radius, normal, startAngle, angleLength, segments) + position;
            
            m_lineMesh.beginLineStrip();
            m_lineMesh.addLineStrip(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 1, 0));
            m_lineMesh.endLineStrip();
        }
        
        void PrimitiveRenderer::renderFilledCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            const std::pair<float, float> angles = startAngleAndLength(normal, startAxis, endAxis);
            renderFilledCircle(color, position, normal, segments, radius, angles.first, angles.second);
        }
        
        void PrimitiveRenderer::renderFilledCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            const Vec3f::List positions = circle2D(radius, normal, startAngle, angleLength, segments) + position;

            m_triangleMesh.beginTriangleFan();
            m_triangleMesh.addTriangleFan(Vertex::fromLists(positions, Color::List(1, color), positions.size(), 1, 0));
            m_triangleMesh.endTriangleFan();
        }

        void PrimitiveRenderer::doPrepare(Vbo& vbo) {
            m_lineRenderer = LineMeshRenderer(m_lineMesh);
            m_lineRenderer.prepare(vbo);
            
            m_triangleRenderer = SimpleTriangleMeshRenderer(m_triangleMesh);
            m_triangleRenderer.prepare(vbo);

            m_lineMesh.clear();
            m_triangleMesh.clear();
        }
        
        void PrimitiveRenderer::doRender(RenderContext& renderContext) {
            m_lineRenderer.render();
            m_triangleRenderer.render();
        }
    }
}
