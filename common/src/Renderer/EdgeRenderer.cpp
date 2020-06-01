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

#include "EdgeRenderer.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/BrushRendererArrays.h"
#include "Renderer/RenderBatch.h"

namespace TrenchBroom {
    namespace Renderer {
        EdgeRenderer::Params::Params(const float i_width, const double i_offset, const bool i_onTop) :
        width(i_width),
        offset(i_offset),
        onTop(i_onTop),
        useColor(false) {}

        EdgeRenderer::Params::Params(const float i_width, const double i_offset,const  bool i_onTop, const Color& i_color) :
        width(i_width),
        offset(i_offset),
        onTop(i_onTop),
        useColor(true),
        color(i_color) {}

        EdgeRenderer::Params::Params(const float i_width, const double i_offset, const bool i_onTop, const bool i_useColor, const Color& i_color) :
        width(i_width),
        offset(i_offset),
        onTop(i_onTop),
        useColor(i_useColor),
        color(i_color) {}

        EdgeRenderer::RenderBase::RenderBase(const Params& params) :
        m_params(params) {}

        EdgeRenderer::RenderBase::~RenderBase() {}

        void EdgeRenderer::RenderBase::renderEdges(RenderContext& renderContext) {
            if (m_params.offset != 0.0)
                glSetEdgeOffset(m_params.offset);

            if (m_params.width != 1.0f)
                glAssert(glLineWidth(m_params.width))

            if (m_params.onTop)
                glAssert(glDisable(GL_DEPTH_TEST))

            {
                PreferenceManager& prefs = PreferenceManager::instance();
                ActiveShader shader(renderContext.shaderManager(), Shaders::BrushEdgeShader);
                shader.set("ShowSoftMapBounds", !renderContext.softMapBounds().is_empty());
                shader.set("SoftMapBoundsMin", renderContext.softMapBounds().min);
                shader.set("SoftMapBoundsMax", renderContext.softMapBounds().max);
                shader.set("SoftMapBoundsColor", vm::vec4f(prefs.get(Preferences::SoftMapBoundsColor).r(),
                                                           prefs.get(Preferences::SoftMapBoundsColor).g(),
                                                           prefs.get(Preferences::SoftMapBoundsColor).b(),
                                                           0.33f)); // NOTE: heavier tint than FaceRenderer, since these are lines
                if (m_params.useColor) {
                    shader.set("UseUniformColor", true);
                    shader.set("Color", m_params.color);
                }
                doRenderVertices(renderContext);
            }

            if (m_params.onTop)
                glAssert(glEnable(GL_DEPTH_TEST))

            if (m_params.width != 1.0f)
                glAssert(glLineWidth(1.0f))

            if (m_params.offset != 0.0)
                glResetEdgeOffset();
        }

        EdgeRenderer::~EdgeRenderer() {}

        void EdgeRenderer::render(RenderBatch& renderBatch, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, false));
        }

        void EdgeRenderer::render(RenderBatch& renderBatch, const Color& color, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, false, color));
        }

        void EdgeRenderer::render(RenderBatch& renderBatch, const bool useColor, const Color& color, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, false, useColor, color));
        }

        void EdgeRenderer::renderOnTop(RenderBatch& renderBatch, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, true));
        }

        void EdgeRenderer::renderOnTop(RenderBatch& renderBatch, const Color& color, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, true, color));
        }

        void EdgeRenderer::renderOnTop(RenderBatch& renderBatch, const bool useColor, const Color& color, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, true, useColor, color));
        }

        void EdgeRenderer::render(RenderBatch& renderBatch, const bool useColor, const Color& color, const bool onTop, const float width, const double offset) {
            doRender(renderBatch, Params(width, offset, onTop, useColor, color));
        }

        DirectEdgeRenderer::Render::Render(const EdgeRenderer::Params& params, VertexArray& vertexArray, IndexRangeMap& indexRanges) :
        RenderBase(params),
        m_vertexArray(vertexArray),
        m_indexRanges(indexRanges) {}

        void DirectEdgeRenderer::Render::doPrepareVertices(VboManager& vboManager) {
            m_vertexArray.prepare(vboManager);
        }

        void DirectEdgeRenderer::Render::doRender(RenderContext& renderContext) {
            if (m_vertexArray.vertexCount() == 0)
                return;
            renderEdges(renderContext);
        }

        void DirectEdgeRenderer::Render::doRenderVertices(RenderContext&) {
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

        // IndexedEdgeRenderer::Render

        IndexedEdgeRenderer::Render::Render(const EdgeRenderer::Params& params, std::shared_ptr<BrushVertexArray> vertexArray, std::shared_ptr<BrushIndexArray> indexArray) :
        RenderBase(params),
        m_vertexArray(std::move(vertexArray)),
        m_indexArray(std::move(indexArray)) {}

        void IndexedEdgeRenderer::Render::prepareVerticesAndIndices(VboManager& vboManager) {
            m_vertexArray->prepare(vboManager);
            m_indexArray->prepare(vboManager);
        }

        void IndexedEdgeRenderer::Render::doRender(RenderContext& renderContext) {
            if (!m_indexArray->hasValidIndices()) {
                return;
            }
            renderEdges(renderContext);
        }

        void IndexedEdgeRenderer::Render::doRenderVertices(RenderContext&) {
            m_vertexArray->setupVertices();
            m_indexArray->setupIndices();
            m_indexArray->render(PrimType::Lines);
            m_vertexArray->cleanupVertices();
            m_indexArray->cleanupIndices();
        }

        // IndexedEdgeRenderer

        IndexedEdgeRenderer::IndexedEdgeRenderer() {}

        IndexedEdgeRenderer::IndexedEdgeRenderer(std::shared_ptr<BrushVertexArray> vertexArray, std::shared_ptr<BrushIndexArray> indexArray) :
        m_vertexArray(std::move(vertexArray)),
        m_indexArray(std::move(indexArray)) {}

        IndexedEdgeRenderer::IndexedEdgeRenderer(const IndexedEdgeRenderer& other) :
        m_vertexArray(other.m_vertexArray),
        m_indexArray(other.m_indexArray) {}

        IndexedEdgeRenderer& IndexedEdgeRenderer::operator=(IndexedEdgeRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(IndexedEdgeRenderer& left, IndexedEdgeRenderer& right) {
            using std::swap;
            swap(left.m_vertexArray, right.m_vertexArray);
            swap(left.m_indexArray, right.m_indexArray);
        }

        void IndexedEdgeRenderer::doRender(RenderBatch& renderBatch, const EdgeRenderer::Params& params) {
            renderBatch.addOneShot(new Render(params, m_vertexArray, m_indexArray));
        }
    }
}
