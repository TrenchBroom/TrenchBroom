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

#include "render/PrimitiveRenderer.h"

#include "Color.h"
#include "gl/ActiveShader.h"
#include "gl/Shaders.h"
#include "mdl/BasicShapes.h"
#include "render/RenderContext.h"

#include "kd/contracts.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

namespace tb::render
{
std::partial_ordering PrimitiveRenderer::LineRenderAttributes::operator<=>(
  const LineRenderAttributes& other) const
{
  // compare by alpha first
  if (const auto cmp = m_color.to<RgbaF>().get<ColorChannel::a>()
                       <=> other.m_color.to<RgbaF>().get<ColorChannel::a>();
      cmp != 0)
  {
    return cmp;
  }

  // alpha is equal; continue with the regular comparison.
  if (const auto cmp = m_lineWidth <=> other.m_lineWidth; cmp != 0)
  {
    return cmp;
  }
  if (const auto cmp = m_color <=> other.m_color; cmp != 0)
  {
    return cmp;
  }
  return m_occlusionPolicy <=> other.m_occlusionPolicy;
}

bool PrimitiveRenderer::LineRenderAttributes::operator==(
  const LineRenderAttributes& other) const
{
  return *this <=> other == 0;
}

void PrimitiveRenderer::LineRenderAttributes::render(
  gl::IndexRangeRenderer& renderer, gl::ActiveShader& shader, const float dpiScale) const
{
  glAssert(glLineWidth(m_lineWidth * dpiScale));
  switch (m_occlusionPolicy)
  {
  case PrimitiveRendererOcclusionPolicy::Hide:
    shader.set("Color", m_color);
    renderer.render();
    break;
  case PrimitiveRendererOcclusionPolicy::Show:
    glAssert(glDisable(GL_DEPTH_TEST));
    shader.set("Color", m_color);
    renderer.render();
    glAssert(glEnable(GL_DEPTH_TEST));
    break;
  case PrimitiveRendererOcclusionPolicy::Transparent:
    glAssert(glDisable(GL_DEPTH_TEST));
    shader.set("Color", blendColor(m_color.to<RgbaF>(), 1.0f / 3.0f));
    renderer.render();
    glAssert(glEnable(GL_DEPTH_TEST));
    shader.set("Color", m_color);
    renderer.render();
    break;
  }
}

PrimitiveRenderer::TriangleRenderAttributes::TriangleRenderAttributes(
  Color color,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const PrimitiveRendererCullingPolicy cullingPolicy)
  : m_color{std::move(color)}
  , m_occlusionPolicy{occlusionPolicy}
  , m_cullingPolicy{cullingPolicy}
{
}

std::partial_ordering PrimitiveRenderer::TriangleRenderAttributes::operator<=>(
  const TriangleRenderAttributes& other) const
{
  // sort by alpha first
  if (const auto cmp = m_color.to<RgbaF>().get<ColorChannel::a>()
                       <=> other.m_color.to<RgbaF>().get<ColorChannel::a>();
      cmp != 0)
  {
    return cmp;
  }

  if (const auto cmp = m_color <=> other.m_color; cmp != 0)
  {
    return cmp;
  }
  if (const auto cmp = m_occlusionPolicy <=> other.m_occlusionPolicy; cmp != 0)
  {
    return cmp;
  }
  return m_cullingPolicy <=> other.m_cullingPolicy;
}

bool PrimitiveRenderer::TriangleRenderAttributes::operator==(
  const TriangleRenderAttributes& other) const
{
  return *this <=> other == 0;
}

void PrimitiveRenderer::TriangleRenderAttributes::render(
  gl::IndexRangeRenderer& renderer, gl::ActiveShader& shader) const
{
  if (m_cullingPolicy == PrimitiveRendererCullingPolicy::ShowBackfaces)
  {
    glAssert(glPushAttrib(GL_POLYGON_BIT));
    glAssert(glDisable(GL_CULL_FACE));
    glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  }

  // Disable depth writes if drawing something transparent
  if (m_color.to<RgbaF>().get<ColorChannel::a>() < 1.0f)
  {
    glAssert(glDepthMask(GL_FALSE));
  }

  switch (m_occlusionPolicy)
  {
  case PrimitiveRendererOcclusionPolicy::Hide:
    shader.set("Color", m_color);
    renderer.render();
    break;
  case PrimitiveRendererOcclusionPolicy::Show:
    glAssert(glDisable(GL_DEPTH_TEST));
    shader.set("Color", m_color);
    renderer.render();
    glAssert(glEnable(GL_DEPTH_TEST));
    break;
  case PrimitiveRendererOcclusionPolicy::Transparent:
    glAssert(glDisable(GL_DEPTH_TEST));
    shader.set("Color", blendColor(m_color.to<RgbaF>(), 1.0f / 2.0f));
    renderer.render();
    glAssert(glEnable(GL_DEPTH_TEST));
    shader.set("Color", m_color);
    renderer.render();
    break;
  }

  if (m_color.to<RgbaF>().get<ColorChannel::a>() < 1.0f)
  {
    glAssert(glDepthMask(GL_TRUE));
  }

  if (m_cullingPolicy == PrimitiveRendererCullingPolicy::ShowBackfaces)
  {
    glAssert(glPopAttrib());
  }
}

void PrimitiveRenderer::renderLine(
  const Color& color,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const vm::vec3f& start,
  const vm::vec3f& end)
{
  m_lineMeshes[LineRenderAttributes{color, lineWidth, occlusionPolicy}].addLine(
    Vertex{start}, Vertex{end});
}

void PrimitiveRenderer::renderLines(
  const Color& color,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_lineMeshes[LineRenderAttributes{color, lineWidth, occlusionPolicy}].addLines(
    Vertex::toList(positions.size(), positions.begin()));
}

void PrimitiveRenderer::renderLineStrip(
  const Color& color,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_lineMeshes[LineRenderAttributes{color, lineWidth, occlusionPolicy}].addLineStrip(
    Vertex::toList(positions.size(), positions.begin()));
}

void PrimitiveRenderer::renderCoordinateSystemXY(
  const Color& x,
  const Color& y,
  float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const vm::bbox3f& bounds)
{
  vm::vec3f start, end;

  mdl::coordinateSystemVerticesX(bounds, start, end);
  renderLine(x, lineWidth, occlusionPolicy, start, end);

  mdl::coordinateSystemVerticesY(bounds, start, end);
  renderLine(y, lineWidth, occlusionPolicy, start, end);
}

void PrimitiveRenderer::renderCoordinateSystemXZ(
  const Color& x,
  const Color& z,
  float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const vm::bbox3f& bounds)
{
  vm::vec3f start, end;

  mdl::coordinateSystemVerticesX(bounds, start, end);
  renderLine(x, lineWidth, occlusionPolicy, start, end);

  mdl::coordinateSystemVerticesZ(bounds, start, end);
  renderLine(z, lineWidth, occlusionPolicy, start, end);
}

void PrimitiveRenderer::renderCoordinateSystemYZ(
  const Color& y,
  const Color& z,
  float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const vm::bbox3f& bounds)
{
  vm::vec3f start, end;

  mdl::coordinateSystemVerticesY(bounds, start, end);
  renderLine(y, lineWidth, occlusionPolicy, start, end);

  mdl::coordinateSystemVerticesZ(bounds, start, end);
  renderLine(z, lineWidth, occlusionPolicy, start, end);
}

void PrimitiveRenderer::renderCoordinateSystem3D(
  const Color& x,
  const Color& y,
  const Color& z,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const vm::bbox3f& bounds)
{
  vm::vec3f start, end;

  mdl::coordinateSystemVerticesX(bounds, start, end);
  renderLine(x, lineWidth, occlusionPolicy, start, end);

  mdl::coordinateSystemVerticesY(bounds, start, end);
  renderLine(y, lineWidth, occlusionPolicy, start, end);

  mdl::coordinateSystemVerticesZ(bounds, start, end);
  renderLine(z, lineWidth, occlusionPolicy, start, end);
}

void PrimitiveRenderer::renderPolygon(
  const Color& color,
  float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_lineMeshes[LineRenderAttributes{color, lineWidth, occlusionPolicy}].addLineLoop(
    Vertex::toList(positions.size(), positions.begin()));
}

void PrimitiveRenderer::renderFilledPolygon(
  const Color& color,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const PrimitiveRendererCullingPolicy cullingPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_triangleMeshes[TriangleRenderAttributes{color, occlusionPolicy, cullingPolicy}]
    .addTriangleFan(Vertex::toList(positions.size(), positions.begin()));
}

void PrimitiveRenderer::renderCylinder(
  const Color& color,
  const float radius,
  const size_t segments,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const PrimitiveRendererCullingPolicy cullingPolicy,
  const vm::vec3f& start,
  const vm::vec3f& end)
{
  contract_pre(radius > 0.0f);
  contract_pre(segments > 2);

  const auto vec = end - start;
  const auto len = vm::length(vec);
  const auto dir = vec / len;

  const auto translation = vm::translation_matrix(start);
  const auto rotation = vm::rotation_matrix(vm::vec3f{0, 0, 1}, dir);
  const auto transform = translation * rotation;

  const auto cylinder = mdl::cylinder(radius, len, segments);
  const auto vertices = transform * cylinder.vertices;

  m_triangleMeshes[TriangleRenderAttributes{color, occlusionPolicy, cullingPolicy}]
    .addTriangleStrip(Vertex::toList(vertices.size(), vertices.begin()));
}

void PrimitiveRenderer::doPrepareVertices(gl::VboManager& vboManager)
{
  prepareLines(vboManager);
  prepareTriangles(vboManager);
}

void PrimitiveRenderer::prepareLines(gl::VboManager& vboManager)
{
  for (auto& [attributes, mesh] : m_lineMeshes)
  {
    auto& renderer =
      m_lineMeshRenderers.emplace(attributes, gl::IndexRangeRenderer{mesh}).first->second;
    renderer.prepare(vboManager);
  }
}

void PrimitiveRenderer::prepareTriangles(gl::VboManager& vboManager)
{
  for (auto& [attributes, mesh] : m_triangleMeshes)
  {
    auto& renderer =
      m_triangleMeshRenderers.emplace(attributes, gl::IndexRangeRenderer{mesh})
        .first->second;
    renderer.prepare(vboManager);
  }
}

void PrimitiveRenderer::doRender(RenderContext& renderContext)
{
  renderLines(renderContext);
  renderTriangles(renderContext);
}

void PrimitiveRenderer::renderLines(RenderContext& renderContext)
{
  auto shader =
    gl::ActiveShader{renderContext.shaderManager(), gl::Shaders::VaryingPUniformCShader};

  for (auto& [attributes, renderer] : m_lineMeshRenderers)
  {
    attributes.render(renderer, shader, renderContext.dpiScale());
  }
  glAssert(glLineWidth(renderContext.dpiScale()));
}

void PrimitiveRenderer::renderTriangles(RenderContext& renderContext)
{
  auto shader =
    gl::ActiveShader{renderContext.shaderManager(), gl::Shaders::VaryingPUniformCShader};

  for (auto& [attributes, renderer] : m_triangleMeshRenderers)
  {
    attributes.render(renderer, shader);
  }
}

} // namespace tb::render
