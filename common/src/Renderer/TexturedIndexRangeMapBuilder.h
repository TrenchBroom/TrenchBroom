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

#ifndef TexturedIndexRangeBuilder_h
#define TexturedIndexRangeBuilder_h

#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/VertexListBuilder.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        template <typename VertexSpec>
        class TexturedIndexRangeMapBuilder {
        public:
            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::List VertexList;
            typedef Assets::Texture Texture;
        private:
            typedef typename VertexListBuilder<VertexSpec>::IndexData IndexData;
        private:
            VertexListBuilder<VertexSpec> m_vertexListBuilder;
            TexturedIndexRangeMap m_indexRange;
        public:
            TexturedIndexRangeMapBuilder(const size_t vertexCount, const TexturedIndexRangeMap::Size& indexRangeSize) :
            m_vertexListBuilder(vertexCount),
            m_indexRange(indexRangeSize) {}
            
            const VertexList& vertices() const {
                return m_vertexListBuilder.vertices();
            }
            
            VertexList& vertices() {
                return m_vertexListBuilder.vertices();
            }
            
            const TexturedIndexRangeMap& indices() const {
                return m_indexRange;
                
            }
            
            TexturedIndexRangeMap& indices() {
                return m_indexRange;
            }
            
            void addPoint(const Texture* texture, const Vertex& v) {
                add(texture, GL_POINTS, m_vertexListBuilder.addPoint(v));
            }
            
            void addPoints(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_POINTS, m_vertexListBuilder.addPoints(vertices));
            }
            
            void addLine(const Texture* texture, const Vertex& v1, const Vertex& v2) {
                add(texture, GL_LINES, m_vertexListBuilder.addLine(v1, v2));
            }
            
            void addLines(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_LINES, m_vertexListBuilder.addLines(vertices));
            }
            
            void addLineStrip(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_LINE_STRIP, m_vertexListBuilder.addLineStrip(vertices));
            }
            
            void addLineLoop(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_LINE_LOOP, m_vertexListBuilder.addLineLoop(vertices));
            }
            
            void addTriangle(const Texture* texture, const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                add(texture, GL_TRIANGLES, m_vertexListBuilder.addTriangle(v1, v2, v3));
            }
            
            void addTriangles(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_TRIANGLES, m_vertexListBuilder.addTriangles(vertices));
            }
            
            void addTriangleFan(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_TRIANGLE_FAN, m_vertexListBuilder.addTriangleFan(vertices));
            }
            
            void addTriangleStrip(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_TRIANGLE_STRIP, m_vertexListBuilder.addTriangleStrip(vertices));
            }
            
            void addQuad(const Texture* texture, const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
                add(texture, GL_QUADS, m_vertexListBuilder.addQuad(v1, v2, v3, v4));
            }
            
            void addQuads(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_QUADS, m_vertexListBuilder.addQuads(vertices));
            }
            
            void addQuadStrip(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_QUAD_STRIP, m_vertexListBuilder.addQuadStrip(vertices));
            }
            
            void addPolygon(const Texture* texture, const VertexList& vertices) {
                add(texture, GL_POLYGON, m_vertexListBuilder.addPolygon(vertices));
            }
        private:
            void add(const Texture* texture, const PrimType primType, const IndexData& data) {
                m_indexRange.add(texture, primType, data.index, data.count);
            }
        };
    }
}

#endif /* TexturedIndexRangeBuilder_h */
