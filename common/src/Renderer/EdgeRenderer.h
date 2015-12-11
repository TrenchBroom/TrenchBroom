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

#ifndef TrenchBroom_EdgeRenderer
#define TrenchBroom_EdgeRenderer

#include "Color.h"
#include "Reference.h"
#include "Renderer/IndexArray.h"
#include "Renderer/IndexArrayMap.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
        class Vbo;

        class EdgeRenderer {
        public:
            struct Params {
                float width;
                float offset;
                bool onTop;
                bool useColor;
                Color color;
                Params(float i_width, float i_offset, bool i_onTop);
                Params(float i_width, float i_offset, bool i_onTop, const Color& i_color);
                Params(float i_width, float i_offset, bool i_onTop, bool i_useColor, const Color& i_color);
            };

            class RenderBase {
            private:
                const Params m_params;
            public:
                RenderBase(const Params& params);
                virtual ~RenderBase();
            protected:
                void renderEdges(RenderContext& renderContext);
            private:
                virtual void doRenderVertices(RenderContext& renderContext) = 0;
            };
        public:
            virtual ~EdgeRenderer();
            
            void render(RenderBatch& renderBatch, float width = 1.0f, float offset = 0.0f);
            void render(RenderBatch& renderBatch, const Color& color, float width = 1.0f, float offset = 0.0f);
            void render(RenderBatch& renderBatch, bool useColor, const Color& color, float width = 1.0f, float offset = 0.0f);
            void renderOnTop(RenderBatch& renderBatch, float width = 1.0f, float offset = 0.2f);
            void renderOnTop(RenderBatch& renderBatch, const Color& color, float width = 1.0f, float offset = 0.2f);
            void renderOnTop(RenderBatch& renderBatch, bool useColor, const Color& color, float width = 1.0f, float offset = 0.2f);
            void render(RenderBatch& renderBatch, bool useColor, const Color& color, bool onTop, float width, float offset);
        private:
            virtual void doRender(RenderBatch& renderBatch, const Params& params) = 0;
        };
        
        class DirectEdgeRenderer : public EdgeRenderer {
        private:
            class Render : public RenderBase, public DirectRenderable {
            private:
                VertexArray m_vertexArray;
                IndexRangeMap m_indexRanges;
            public:
                Render(const Params& params, VertexArray& vertexArray, IndexRangeMap& indexRanges);
            private:
                void doPrepareVertices(Vbo& vertexVbo);
                void doRender(RenderContext& renderContext);
                void doRenderVertices(RenderContext& renderContext);
            };
        private:
            VertexArray m_vertexArray;
            IndexRangeMap m_indexRanges;
        public:
            DirectEdgeRenderer();
            DirectEdgeRenderer(const VertexArray& vertexArray, const IndexRangeMap& indexRanges);
            DirectEdgeRenderer(const VertexArray& vertexArray, PrimType primType);

            DirectEdgeRenderer(const DirectEdgeRenderer& other);
            DirectEdgeRenderer& operator=(DirectEdgeRenderer other);
            
            friend void swap(DirectEdgeRenderer& left, DirectEdgeRenderer& right);
        private:
            void doRender(RenderBatch& renderBatch, const EdgeRenderer::Params& params);
        };
        
        class IndexedEdgeRenderer : public EdgeRenderer {
        private:
            class Render : public RenderBase, public IndexedRenderable {
            private:
                VertexArray m_vertexArray;
                IndexArray m_indexArray;
                IndexArrayMap m_indexRanges;
            public:
                Render(const Params& params, VertexArray& vertexArray, IndexArray& indexArray, IndexArrayMap& indexRanges);
            private:
                void doPrepareVertices(Vbo& vertexVbo);
                void doPrepareIndices(Vbo& indexVbo);
                void doRender(RenderContext& renderContext);
                void doRenderVertices(RenderContext& renderContext);
            };
        private:
            VertexArray m_vertexArray;
            IndexArray m_indexArray;
            IndexArrayMap m_indexRanges;
        public:
            IndexedEdgeRenderer();
            IndexedEdgeRenderer(const VertexArray& vertexArray, const IndexArray& indexArray, const IndexArrayMap& indexRanges);

            IndexedEdgeRenderer(const IndexedEdgeRenderer& other);
            IndexedEdgeRenderer& operator=(IndexedEdgeRenderer other);
            
            friend void swap(IndexedEdgeRenderer& left, IndexedEdgeRenderer& right);
        private:
            void doRender(RenderBatch& renderBatch, const EdgeRenderer::Params& params);
        };
    }
}

#endif /* defined(TrenchBroom_EdgeRenderer) */
