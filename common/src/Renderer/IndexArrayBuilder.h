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

#ifndef IndexArrayBuilder_h
#define IndexArrayBuilder_h

#include "Renderer/IndexArray.h"
#include "Renderer/VertexListBuilder.h"

namespace TrenchBroom {
    namespace Renderer {
        template <typename VertexSpec>
        class IndexArrayBuilder {
        public:
            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::List VertexList;
            typedef typename VertexListBuilder<VertexSpec>::IndexData IndexData;
        private:
            VertexListBuilder<VertexSpec> m_vertexListBuilder;
            IndexArray m_indexArray;
        public:
            IndexArrayBuilder(const size_t vertexCount, const IndexArray::Size& indexArraySize) :
            m_vertexListBuilder(vertexCount),
            m_indexArray(indexArraySize) {}
            
            const VertexList& vertices() const {
                return m_vertexListBuilder.vertices();
            }
            
            VertexList& vertices() {
                return m_vertexListBuilder.vertices();
            }
            
            const IndexArray& indexArray() const {
                return m_indexArray;
            }
            
            IndexArray& indexArray() {
                return m_indexArray;
            }
            
            void addPoint(const Vertex& v) {
                add(IndexArray::PT_Points, m_vertexListBuilder.addPoint(v));
            }
            
            void addPoints(const VertexList& vertices) {
                add(IndexArray::PT_Points, m_vertexListBuilder.addPoints(vertices));
            }
            
            void addLine(const Vertex& v1, const Vertex& v2) {
                add(IndexArray::PT_Lines, m_vertexListBuilder.addLine(v1, v2));
            }
            
            void addLines(const VertexList& vertices) {
                add(IndexArray::PT_Lines, m_vertexListBuilder.addLines(vertices));
            }
            
            void addLineStrip(const VertexList& vertices) {
                add(IndexArray::PT_LineStrips, m_vertexListBuilder.addLineStrip(vertices));
            }
            
            void addLineLoop(const VertexList& vertices) {
                add(IndexArray::PT_LineLoops, m_vertexListBuilder.addLineLoop(vertices));
            }
            
            void addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                add(IndexArray::PT_Triangles, m_vertexListBuilder.addTriangle(v1, v2, v3));
            }
            
            void addTriangles(const VertexList& vertices) {
                add(IndexArray::PT_Triangles, m_vertexListBuilder.addTriangles(vertices));
            }
            
            void addTriangleFan(const VertexList& vertices) {
                add(IndexArray::PT_TriangleFans, m_vertexListBuilder.addTriangleFan(vertices));
            }
            
            void addTriangleStrip(const VertexList& vertices) {
                add(IndexArray::PT_TriangleStrips, m_vertexListBuilder.addTriangleStrip(vertices));
            }
            
            void addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
                add(IndexArray::PT_Quads, m_vertexListBuilder.addQuad(v1, v2, v3, v4));
            }
            
            void addQuads(const VertexList& vertices) {
                add(IndexArray::PT_Quads, m_vertexListBuilder.addQuads(vertices));
            }
            
            void addQuadStrip(const VertexList& vertices) {
                add(IndexArray::PT_QuadStrips, m_vertexListBuilder.addQuadStrip(vertices));
            }
            
            void addPolygon(const VertexList& vertices) {
                add(IndexArray::PT_Polygons, m_vertexListBuilder.addPolygon(vertices));
            }
        private:
            void add(const IndexArray::PrimType primType, const IndexData& data) {
                m_indexArray.add(primType, data.index, data.count);
            }
        };
    }
}

#endif /* IndexArrayBuilder_h */
