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

#ifndef VertexRenderSpecBuilder_h
#define VertexRenderSpecBuilder_h

#include "Renderer/VertexArrayBuilder.h"
#include "Renderer/VertexArrayRenderer.h"
#include "Renderer/VertexRenderSpec.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        template <typename VertexSpec, typename Key = int, typename Func = DefaultRenderFunc<Key> >
        class VertexRenderSpecBuilder {
        private:
            VertexArrayBuilder<VertexSpec> m_vertexArrayBuilder;
            KeyedVertexRenderSpec<Key, Func> m_renderSpec;

            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::List VertexList;
            typedef typename VertexArrayBuilder<VertexSpec>::IndexData IndexData;
        public:
            VertexRenderSpecBuilder(const size_t vertexCount, const typename KeyedVertexRenderSpec<Key, Func>::Size& renderSpecSize) :
            m_vertexArrayBuilder(vertexCount),
            m_renderSpec(renderSpecSize) {}
            
            virtual ~VertexRenderSpecBuilder() {}
            
            VertexArrayRenderer<Key, Func>* createRenderer() {
                return new VertexArrayRenderer<Key, Func>(vertexArray(), renderSpec());
            }
            
            VertexArrayRenderer<Key, Func> renderer() {
                return VertexArrayRenderer<Key, Func>(vertexArray(), renderSpec());
            }
            
            const VertexList& vertices() const {
                return m_vertexArrayBuilder.vertices();
            }
            
            VertexArray vertexArray() {
                return m_vertexArrayBuilder.vertexArray();
            }
            
            KeyedVertexRenderSpec<Key, Func> renderSpec() {
                return m_renderSpec;
            }

            void addPoint(const Vertex& v, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Points, m_vertexArrayBuilder.addPoint(v));
            }
            
            void addPoints(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Points, m_vertexArrayBuilder.addPoints(vertices));
            }
            
            void addLine(const Vertex& v1, const Vertex& v2, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Lines, m_vertexArrayBuilder.addLine(v1, v2));
            }
            
            void addLines(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Lines, m_vertexArrayBuilder.addLines(vertices));
            }
            
            void addLineStrip(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_LineStrips, m_vertexArrayBuilder.addLineStrip(vertices));
            }
            
            void addLineLoop(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_LineLoops, m_vertexArrayBuilder.addLineLoop(vertices));
            }
            
            void addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Triangles, m_vertexArrayBuilder.addTriangle(v1, v2, v3));
            }
            
            void addTriangles(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Triangles, m_vertexArrayBuilder.addTriangles(vertices));
            }
            
            void addTriangleFan(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_TriangleFans, m_vertexArrayBuilder.addTriangleFan(vertices));
            }

            void addTriangleStrip(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_TriangleStrips, m_vertexArrayBuilder.addTriangleStrip(vertices));
            }

            void addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Quads, m_vertexArrayBuilder.addQuad(v1, v2, v3, v4));
            }
            
            void addQuads(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Quads, m_vertexArrayBuilder.addQuads(vertices));
            }
            
            void addQuadStrip(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_QuadStrips, m_vertexArrayBuilder.addQuadStrip(vertices));
            }
            
            void addPolygon(const VertexList& vertices, const Key& key = 0) {
                addSpec(key, VertexRenderSpec::PT_Polygons, m_vertexArrayBuilder.addPolygon(vertices));
            }
        private:
            void addSpec(const Key& key, const VertexRenderSpec::PrimType primType, const IndexData& data) {
                m_renderSpec.add(key, primType, data.index, data.count);
            }
        };

        template <typename VertexSpec>
        class TexturedVertexRenderSpecBuilder : public VertexRenderSpecBuilder<VertexSpec, const Assets::Texture*, TextureFunc> {
        public:
            TexturedVertexRenderSpecBuilder(const size_t vertexCount, const TexturedVertexRenderSpec::Size& renderSpecSize) :
            VertexRenderSpecBuilder<VertexSpec, const Assets::Texture*, TextureFunc>(vertexCount, renderSpecSize) {}
        };
    }
}

#endif /* VertexRenderSpecBuilder_h */
