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

#include "EdgeRenderer.h"

#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        EdgeRenderer::Params::Params(const float i_width, const float i_offset, const bool i_onTop) :
        width(i_width),
        offset(i_offset),
        onTop(i_onTop),
        useColor(false) {}
        
        EdgeRenderer::Params::Params(float i_width, float i_offset, bool i_onTop, const Color& i_color) :
        width(i_width),
        offset(i_offset),
        onTop(i_onTop),
        useColor(true),
        color(i_color) {}

        EdgeRenderer::Params::Params(float i_width, float i_offset, bool i_onTop, bool i_useColor, const Color& i_color) :
        width(i_width),
        offset(i_offset),
        onTop(i_onTop),
        useColor(i_useColor),
        color(i_color) {}

        EdgeRenderer::RenderBase::RenderBase(const Params& params) :
        m_params(params) {}
        
        EdgeRenderer::RenderBase::~RenderBase() {}

        void EdgeRenderer::RenderBase::renderEdges(RenderContext& renderContext) {
            if (m_params.offset != 0.0f)
                glSetEdgeOffset(m_params.offset);
            
            if (m_params.width != 1.0f)
                glAssert(glLineWidth(m_params.width));
            
            if (m_params.onTop)
                glAssert(glDisable(GL_DEPTH_TEST));
            
            if (m_params.useColor) {
                ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPUniformCShader);
                shader.set("Color", m_params.color);
                doRenderVertices(renderContext);
            } else {
                ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPCShader);
                doRenderVertices(renderContext);
            }
            
            if (m_params.onTop)
                glAssert(glEnable(GL_DEPTH_TEST));
            
            if (m_params.width != 1.0f)
                glAssert(glLineWidth(1.0f));
            
            if (m_params.offset != 0.0f)
                glResetEdgeOffset();
        }

        EdgeRenderer::~EdgeRenderer() {}
        
        void EdgeRenderer::render(RenderBatch& renderBatch, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, false));
        }
        
        void EdgeRenderer::render(RenderBatch& renderBatch, const Color& color, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, false, color));
        }
        
        void EdgeRenderer::render(RenderBatch& renderBatch, const bool useColor, const Color& color, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, false, useColor, color));
        }
        
        void EdgeRenderer::renderOnTop(RenderBatch& renderBatch, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, true));
        }
        
        void EdgeRenderer::renderOnTop(RenderBatch& renderBatch, const Color& color, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, true, color));
        }
        
        void EdgeRenderer::renderOnTop(RenderBatch& renderBatch, const bool useColor, const Color& color, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, true, useColor, color));
        }

        void EdgeRenderer::render(RenderBatch& renderBatch, const bool useColor, const Color& color, const bool onTop, const float width, const float offset) {
            doRender(renderBatch, Params(width, offset, onTop, useColor, color));
        }

        DirectEdgeRenderer::Render::Render(const EdgeRenderer::Params& params, VertexArray& vertexArray, IndexRangeMap& indexRanges) :
        RenderBase(params),
        m_vertexArray(vertexArray),
        m_indexRanges(indexRanges) {}

        void DirectEdgeRenderer::Render::doPrepareVertices(Vbo& vertexVbo) {
            m_vertexArray.prepare(vertexVbo);
        }
        
        void DirectEdgeRenderer::Render::doRender(RenderContext& renderContext) {
            if (m_vertexArray.vertexCount() == 0)
                return;
            renderEdges(renderContext);
        }

        void DirectEdgeRenderer::Render::doRenderVertices(RenderContext& renderContext) {
            m_indexRanges.render(m_vertexArray);
        }

        DirectEdgeRenderer::DirectEdgeRenderer() {}

        DirectEdgeRenderer::DirectEdgeRenderer(const VertexArray& vertexArray, const IndexRangeMap& indexRanges) :
        m_vertexArray(vertexArray),
        m_indexRanges(indexRanges) {}
        
        DirectEdgeRenderer::DirectEdgeRenderer(const VertexArray& vertexArray, const PrimType primType) :
        m_vertexArray(vertexArray),
        m_indexRanges(IndexRangeMap(primType, 0, vertexArray.vertexCount())) {}
        
        DirectEdgeRenderer::DirectEdgeRenderer(const DirectEdgeRenderer& other) :
        m_vertexArray(other.m_vertexArray),
        m_indexRanges(other.m_indexRanges) {}
        
        DirectEdgeRenderer& DirectEdgeRenderer::operator=(DirectEdgeRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(DirectEdgeRenderer& left, DirectEdgeRenderer& right) {
            using std::swap;
            swap(left.m_vertexArray, right.m_vertexArray);
            swap(left.m_indexRanges, right.m_indexRanges);
        }

        void DirectEdgeRenderer::doRender(RenderBatch& renderBatch, const EdgeRenderer::Params& params) {
            renderBatch.addOneShot(new Render(params, m_vertexArray, m_indexRanges));
        }
        
        IndexedEdgeRenderer::Render::Render(const EdgeRenderer::Params& params, VertexArray& vertexArray, IndexArray& indexArray, IndexArrayMap& indexRanges) :
        RenderBase(params),
        m_vertexArray(vertexArray),
        m_indexArray(indexArray),
        m_indexRanges(indexRanges) {}
        
        void IndexedEdgeRenderer::Render::doPrepareVertices(Vbo& vertexVbo) {
            m_vertexArray.prepare(vertexVbo);
        }
        
        void IndexedEdgeRenderer::Render::doPrepareIndices(Vbo& indexVbo) {
            m_indexArray.prepare(indexVbo);
        }

        void IndexedEdgeRenderer::Render::doRender(RenderContext& renderContext) {
            if (m_vertexArray.vertexCount() == 0)
                return;
            renderEdges(renderContext);
        }
        
        void IndexedEdgeRenderer::Render::doRenderVertices(RenderContext& renderContext) {
            m_vertexArray.setup();
            m_indexRanges.render(m_indexArray);
            m_vertexArray.cleanup();
        }
        
        IndexedEdgeRenderer::IndexedEdgeRenderer() {}
        
        IndexedEdgeRenderer::IndexedEdgeRenderer(const VertexArray& vertexArray, const IndexArray& indexArray, const IndexArrayMap& indexRanges) :
        m_vertexArray(vertexArray),
        m_indexArray(indexArray),
        m_indexRanges(indexRanges) {}
        
        IndexedEdgeRenderer::IndexedEdgeRenderer(const IndexedEdgeRenderer& other) :
        m_vertexArray(other.m_vertexArray),
        m_indexArray(other.m_indexArray),
        m_indexRanges(other.m_indexRanges) {}
        
        IndexedEdgeRenderer& IndexedEdgeRenderer::operator=(IndexedEdgeRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(IndexedEdgeRenderer& left, IndexedEdgeRenderer& right) {
            using std::swap;
            swap(left.m_vertexArray, right.m_vertexArray);
            swap(left.m_indexArray, right.m_indexArray);
            swap(left.m_indexRanges, right.m_indexRanges);
        }
        
        void IndexedEdgeRenderer::doRender(RenderBatch& renderBatch, const EdgeRenderer::Params& params) {
            renderBatch.addOneShot(new Render(params, m_vertexArray, m_indexArray, m_indexRanges));
        }
    }
}
