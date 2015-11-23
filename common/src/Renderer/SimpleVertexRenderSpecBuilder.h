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

#ifndef SimpleVertexRenderSpecBuilder_h
#define SimpleVertexRenderSpecBuilder_h

#include "Renderer/VertexArrayBuilder.h"
#include "Renderer/VertexRenderSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        template <typename VertexSpec>
        class SimpleVertexRenderSpecBuilder {
        private:
            VertexArrayBuilder<VertexSpec> m_vertexArrayBuilder;
            SimpleVertexRenderSpec m_renderSpec;

            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::List VertexList;
            typedef typename VertexArrayBuilder<VertexSpec>::IndexData IndexData;
        public:
            SimpleVertexRenderSpecBuilder(const size_t vertexCount, const SimpleVertexRenderSpec::Size& renderSpecSize) :
            m_vertexArrayBuilder(vertexCount),
            m_renderSpec(renderSpecSize) {}
            
            VertexArray vertexArray() {
                return m_vertexArrayBuilder.vertexArray();
            }
            
            SimpleVertexRenderSpec& renderSpec() {
                return m_renderSpec;
            }
            
            void addPoint(const Vertex& v) {
                addSpec(VertexRenderSpec::PT_Points, m_vertexArrayBuilder.addPoint(v));
            }
            
            void addPoints(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_Points, m_vertexArrayBuilder.addPoints(vertices));
            }
            
            void addLine(const Vertex& v1, const Vertex& v2) {
                addSpec(VertexRenderSpec::PT_Lines, m_vertexArrayBuilder.addLine(v1, v2));
            }
            
            void addLines(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_Lines, m_vertexArrayBuilder.addLines(vertices));
            }
            
            void addLineStrip(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_LineStrips, m_vertexArrayBuilder.addLineStrip(vertices));
            }
            
            void addLineLoop(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_LineLoops, m_vertexArrayBuilder.addLineLoop(vertices));
            }
            
            void addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                addSpec(VertexRenderSpec::PT_Triangles, m_vertexArrayBuilder.addTriangle(v1, v2, v3));
            }
            
            void addTriangles(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_Triangles, m_vertexArrayBuilder.addTriangles(vertices));
            }
            
            void addTriangleFan(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_TriangleFans, m_vertexArrayBuilder.addTriangleFan(vertices));
            }

            void addTriangleStrip(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_TriangleStrips, m_vertexArrayBuilder.addTriangleStrip(vertices));
            }

            void addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
                addSpec(VertexRenderSpec::PT_Quads, m_vertexArrayBuilder.addQuad(v1, v2, v3, v4));
            }
            
            void addQuads(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_Quads, m_vertexArrayBuilder.addQuads(vertices));
            }
            
            void addQuadStrip(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_QuadStrips, m_vertexArrayBuilder.addQuadStrip(vertices));
            }
            
            void addPolygon(const VertexList& vertices) {
                addSpec(VertexRenderSpec::PT_Polygons, m_vertexArrayBuilder.addPolygon(vertices));
            }
        private:
            void addSpec(const VertexRenderSpec::PrimType primType, const IndexData& data) {
                m_renderSpec.add(primType, data.index, data.count);
            }
        };
    }
}

#endif /* SimpleVertexRenderSpecBuilder_h */
