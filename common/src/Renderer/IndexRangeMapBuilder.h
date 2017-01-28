/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef IndexRangeBuilder_h
#define IndexRangeBuilder_h

#include "Renderer/IndexRangeMap.h"
#include "Renderer/VertexListBuilder.h"

namespace TrenchBroom {
    namespace Renderer {
        template <typename VertexSpec>
        class IndexRangeMapBuilder {
        public:
            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::Array VertexArray;
            typedef typename VertexListBuilder<VertexSpec>::IndexData IndexData;
        private:
            VertexListBuilder<VertexSpec> m_vertexListBuilder;
            IndexRangeMap m_indexRange;
        public:
            IndexRangeMapBuilder() {} // default constructors allow dynamic growth
            
            IndexRangeMapBuilder(const size_t vertexCount, const IndexRangeMap::Size& indexRangeSize) :
            m_vertexListBuilder(vertexCount),
            m_indexRange(indexRangeSize) {}
            
            const VertexArray& vertices() const {
                return m_vertexListBuilder.vertices();
            }
            
            VertexArray& vertices() {
                return m_vertexListBuilder.vertices();
            }
            
            const IndexRangeMap& indexArray() const {
                return m_indexRange;
            }
            
            IndexRangeMap& indexArray() {
                return m_indexRange;
            }
            
            void addPoint(const Vertex& v) {
                add(GL_POINTS, m_vertexListBuilder.addPoint(v));
            }
            
            void addPoints(const VertexArray& vertices) {
                add(GL_POINTS, m_vertexListBuilder.addPoints(vertices));
            }
            
            void addLine(const Vertex& v1, const Vertex& v2) {
                add(GL_LINES, m_vertexListBuilder.addLine(v1, v2));
            }
            
            void addLines(const VertexArray& vertices) {
                add(GL_LINES, m_vertexListBuilder.addLines(vertices));
            }
            
            void addLineStrip(const VertexArray& vertices) {
                add(GL_LINE_STRIP, m_vertexListBuilder.addLineStrip(vertices));
            }
            
            void addLineLoop(const VertexArray& vertices) {
                add(GL_LINE_LOOP, m_vertexListBuilder.addLineLoop(vertices));
            }
            
            void addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                add(GL_TRIANGLES, m_vertexListBuilder.addTriangle(v1, v2, v3));
            }
            
            void addTriangles(const VertexArray& vertices) {
                add(GL_TRIANGLES, m_vertexListBuilder.addTriangles(vertices));
            }
            
            void addTriangleFan(const VertexArray& vertices) {
                add(GL_TRIANGLE_FAN, m_vertexListBuilder.addTriangleFan(vertices));
            }
            
            void addTriangleStrip(const VertexArray& vertices) {
                add(GL_TRIANGLE_STRIP, m_vertexListBuilder.addTriangleStrip(vertices));
            }
            
            void addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
                add(GL_QUADS, m_vertexListBuilder.addQuad(v1, v2, v3, v4));
            }
            
            void addQuads(const VertexArray& vertices) {
                add(GL_QUADS, m_vertexListBuilder.addQuads(vertices));
            }
            
            void addQuadStrip(const VertexArray& vertices) {
                add(GL_QUAD_STRIP, m_vertexListBuilder.addQuadStrip(vertices));
            }
            
            void addPolygon(const VertexArray& vertices) {
                add(GL_POLYGON, m_vertexListBuilder.addPolygon(vertices));
            }
        private:
            void add(const PrimType primType, const IndexData& data) {
                m_indexRange.add(primType, data.index, data.count);
            }
        };
    }
}

#endif /* IndexRangeBuilder_h */
