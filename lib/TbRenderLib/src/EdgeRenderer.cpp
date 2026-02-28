/*
 Copyright (C) 2010 Kristian Duske

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

#include "render/EdgeRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/GlInterface.h"
#include "gl/PrimType.h"
#include "gl/ShaderManager.h"
#include "gl/Shaders.h"
#include "render/BrushRendererArrays.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"

namespace tb::render
{

EdgeRenderer::Params::Params(
  const float i_width, const double i_offset, const bool i_onTop)
  : width{i_width}
  , offset{i_offset}
  , onTop{i_onTop}
  , useColor{false}
{
}

EdgeRenderer::Params::Params(
  const float i_width, const double i_offset, const bool i_onTop, Color i_color)
  : width{i_width}
  , offset{i_offset}
  , onTop{i_onTop}
  , useColor{true}
  , color{std::move(i_color)}
{
}

EdgeRenderer::Params::Params(
  const float i_width,
  const double i_offset,
  const bool i_onTop,
  const bool i_useColor,
  Color i_color)
  : width{i_width}
  , offset{i_offset}
  , onTop{i_onTop}
  , useColor{i_useColor}
  , color{std::move(i_color)}
{
}

EdgeRenderer::RenderBase::RenderBase(Params params)
  : m_params{std::move(params)}
{
}

EdgeRenderer::RenderBase::~RenderBase() = default;

void EdgeRenderer::RenderBase::renderEdges(RenderContext& renderContext)
{
  auto& gl = renderContext.gl();

  if (m_params.offset != 0.0)
  {
    gl::glSetEdgeOffset(gl, m_params.offset);
  }

  gl.lineWidth(m_params.width * renderContext.dpiScale());

  if (m_params.onTop)
  {
    gl.disable(GL_DEPTH_TEST);
  }

  {
    auto shader =
      gl::ActiveShader{gl, renderContext.shaderManager(), gl::Shaders::EdgeShader};
    shader.set("ShowSoftMapBounds", !renderContext.softMapBounds().is_empty());
    shader.set("SoftMapBoundsMin", renderContext.softMapBounds().min);
    shader.set("SoftMapBoundsMax", renderContext.softMapBounds().max);
    shader.set(
      "SoftMapBoundsColor",
      RgbaF{
        pref(Preferences::SoftMapBoundsColor).to<RgbF>(),
        0.33f}); // NOTE: heavier tint than FaceRenderer, since these are lines
    shader.set("UseUniformColor", m_params.useColor);
    shader.set("Color", m_params.color);

    doRenderVertices(renderContext);
  }

  if (m_params.onTop)
  {
    gl.enable(GL_DEPTH_TEST);
  }

  gl.lineWidth(renderContext.dpiScale());

  if (m_params.offset != 0.0)
  {
    gl::glResetEdgeOffset(gl);
  }
}

EdgeRenderer::~EdgeRenderer() = default;

void EdgeRenderer::render(
  RenderBatch& renderBatch, const float width, const double offset)
{
  doRender(renderBatch, {width, offset, false});
}

void EdgeRenderer::render(
  RenderBatch& renderBatch, const Color& color, const float width, const double offset)
{
  doRender(renderBatch, {width, offset, false, color});
}

void EdgeRenderer::render(
  RenderBatch& renderBatch,
  const bool useColor,
  const Color& color,
  const float width,
  const double offset)
{
  doRender(renderBatch, {width, offset, false, useColor, color});
}

void EdgeRenderer::renderOnTop(
  RenderBatch& renderBatch, const float width, const double offset)
{
  doRender(renderBatch, {width, offset, true});
}

void EdgeRenderer::renderOnTop(
  RenderBatch& renderBatch, const Color& color, const float width, const double offset)
{
  doRender(renderBatch, Params(width, offset, true, color));
}

void EdgeRenderer::renderOnTop(
  RenderBatch& renderBatch,
  const bool useColor,
  const Color& color,
  const float width,
  const double offset)
{
  doRender(renderBatch, {width, offset, true, useColor, color});
}

void EdgeRenderer::render(
  RenderBatch& renderBatch,
  const bool useColor,
  const Color& color,
  const bool onTop,
  const float width,
  const double offset)
{
  doRender(renderBatch, {width, offset, onTop, useColor, color});
}

DirectEdgeRenderer::Render::Render(
  const EdgeRenderer::Params& params,
  gl::VertexArray& vertexArray,
  gl::IndexRangeMap& indexRanges)
  : RenderBase{params}
  , m_vertexArray{vertexArray}
  , m_indexRanges{indexRanges}
{
}

void DirectEdgeRenderer::Render::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  m_vertexArray.prepare(gl, vboManager);
}

void DirectEdgeRenderer::Render::render(RenderContext& renderContext)
{
  if (m_vertexArray.vertexCount() > 0)
  {
    renderEdges(renderContext);
  }
}

void DirectEdgeRenderer::Render::doRenderVertices(RenderContext& renderContext)
{
  auto& gl = renderContext.gl();

  auto* currentProgram = renderContext.shaderManager().currentProgram();
  contract_assert(currentProgram);

  if (m_vertexArray.setup(gl, *currentProgram))
  {
    m_indexRanges.render(gl, m_vertexArray);
    m_vertexArray.cleanup(gl, *currentProgram);
  }
}

DirectEdgeRenderer::DirectEdgeRenderer() {}

DirectEdgeRenderer::DirectEdgeRenderer(
  gl::VertexArray vertexArray, gl::IndexRangeMap indexRanges)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexRanges{std::move(indexRanges)}
{
}

DirectEdgeRenderer::DirectEdgeRenderer(
  gl::VertexArray vertexArray, const gl::PrimType primType)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexRanges{gl::IndexRangeMap{primType, 0, m_vertexArray.vertexCount()}}
{
}

void DirectEdgeRenderer::doRender(
  RenderBatch& renderBatch, const EdgeRenderer::Params& params)
{
  renderBatch.addOneShot(new Render{params, m_vertexArray, m_indexRanges});
}

// IndexedEdgeRenderer::Render

IndexedEdgeRenderer::Render::Render(
  const EdgeRenderer::Params& params,
  std::shared_ptr<BrushVertexArray> vertexArray,
  std::shared_ptr<BrushIndexArray> indexArray)
  : RenderBase{params}
  , m_vertexArray{std::move(vertexArray)}
  , m_indexArray{std::move(indexArray)}
{
}

void IndexedEdgeRenderer::Render::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  m_vertexArray->prepare(gl, vboManager);
  m_indexArray->prepare(gl, vboManager);
}

void IndexedEdgeRenderer::Render::render(RenderContext& renderContext)
{
  if (m_indexArray->hasValidIndices())
  {
    renderEdges(renderContext);
  }
}

void IndexedEdgeRenderer::Render::doRenderVertices(RenderContext& renderContext)
{
  auto* currentProgram = renderContext.shaderManager().currentProgram();
  contract_assert(currentProgram);

  auto& gl = renderContext.gl();
  if (m_vertexArray->setup(gl, *currentProgram))
  {
    m_indexArray->setup(gl);
    m_indexArray->render(gl, gl::PrimType::Lines);
    m_vertexArray->cleanup(gl, *currentProgram);
    m_indexArray->cleanup(gl);
  }
}

// IndexedEdgeRenderer

IndexedEdgeRenderer::IndexedEdgeRenderer() = default;

IndexedEdgeRenderer::IndexedEdgeRenderer(
  std::shared_ptr<BrushVertexArray> vertexArray,
  std::shared_ptr<BrushIndexArray> indexArray)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArray{std::move(indexArray)}
{
}

void IndexedEdgeRenderer::doRender(
  RenderBatch& renderBatch, const EdgeRenderer::Params& params)
{
  renderBatch.addOneShot(new Render{params, m_vertexArray, m_indexArray});
}

} // namespace tb::render
