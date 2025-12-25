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

#include "TriangleRenderer.h"

#include "gl/ActiveShader.h"
#include "gl/Camera.h"
#include "gl/Shaders.h"
#include "render/RenderContext.h"

#include <utility>

namespace tb::render
{
TriangleRenderer::TriangleRenderer() = default;

TriangleRenderer::TriangleRenderer(
  gl::VertexArray vertexArray, gl::IndexRangeMap indexArray)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArray{std::move(indexArray)}
{
}

TriangleRenderer::TriangleRenderer(
  gl::VertexArray vertexArray, const gl::PrimType primType)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArray{primType, 0, m_vertexArray.vertexCount()}
{
}

void TriangleRenderer::setUseColor(const bool useColor)
{
  m_useColor = useColor;
}

void TriangleRenderer::setColor(const Color& color)
{
  m_color = color;
}

void TriangleRenderer::setApplyTinting(const bool applyTinting)
{
  m_applyTinting = applyTinting;
}

void TriangleRenderer::setTintColor(const Color& tintColor)
{
  m_tintColor = tintColor;
}

void TriangleRenderer::doPrepareVertices(gl::VboManager& vboManager)
{
  m_vertexArray.prepare(vboManager);
}

void TriangleRenderer::doRender(RenderContext& context)
{
  if (m_vertexArray.vertexCount() != 0)
  {
    auto shader = gl::ActiveShader{context.shaderManager(), gl::Shaders::TriangleShader};
    shader.set("ApplyTinting", m_applyTinting);
    shader.set("TintColor", m_tintColor);
    shader.set("UseColor", m_useColor);
    shader.set("Color", m_color);
    shader.set("CameraPosition", context.camera().position());
    m_indexArray.render(m_vertexArray);
  }
}

} // namespace tb::render
