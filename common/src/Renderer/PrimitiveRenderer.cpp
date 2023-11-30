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

#include "PrimitiveRenderer.h"

#include "Color.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/IndexRangeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>

namespace TrenchBroom
{
namespace Renderer
{
PrimitiveRenderer::LineRenderAttributes::LineRenderAttributes(
  const Color& color,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy)
  : m_color(color)
  , m_lineWidth(lineWidth)
  , m_occlusionPolicy(occlusionPolicy)
{
}

bool PrimitiveRenderer::LineRenderAttributes::operator<(
  const LineRenderAttributes& other) const
{
  // As a special exception, sort by descending alpha so opaque batches render first.
  if (m_color.a() < other.m_color.a())
    return false;
  if (m_color.a() > other.m_color.a())
    return true;
  // alpha is equal; continue with the regular comparison.

  if (m_lineWidth < other.m_lineWidth)
    return true;
  if (m_lineWidth > other.m_lineWidth)
    return false;
  if (m_color < other.m_color)
    return true;
  if (m_color > other.m_color)
    return false;
  if (m_occlusionPolicy < other.m_occlusionPolicy)
    return true;
  if (m_occlusionPolicy > other.m_occlusionPolicy)
    return false;
  return false;
}

void PrimitiveRenderer::LineRenderAttributes::render(
  IndexRangeRenderer& renderer, ActiveShader& shader, const float dpiScale) const
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
    shader.set("Color", Color(m_color, m_color.a() / 3.0f));
    renderer.render();
    glAssert(glEnable(GL_DEPTH_TEST));
    shader.set("Color", m_color);
    renderer.render();
    break;
  }
}

PrimitiveRenderer::TriangleRenderAttributes::TriangleRenderAttributes(
  const Color& color,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const PrimitiveRendererCullingPolicy cullingPolicy)
  : m_color(color)
  , m_occlusionPolicy(occlusionPolicy)
  , m_cullingPolicy(cullingPolicy)
{
}

bool PrimitiveRenderer::TriangleRenderAttributes::operator<(
  const TriangleRenderAttributes& other) const
{
  // As a special exception, sort by descending alpha so opaque batches render first.
  if (m_color.a() < other.m_color.a())
    return false;
  if (m_color.a() > other.m_color.a())
    return true;
  // alpha is equal; continue with the regular comparison.

  if (m_color < other.m_color)
    return true;
  if (m_color > other.m_color)
    return false;
  if (m_occlusionPolicy < other.m_occlusionPolicy)
    return true;
  if (m_occlusionPolicy > other.m_occlusionPolicy)
    return false;
  if (m_cullingPolicy < other.m_cullingPolicy)
    return true;
  if (m_cullingPolicy > other.m_cullingPolicy)
    return false;
  return false;
}

void PrimitiveRenderer::TriangleRenderAttributes::render(
  IndexRangeRenderer& renderer, ActiveShader& shader) const
{
  if (m_cullingPolicy == PrimitiveRendererCullingPolicy::ShowBackfaces)
  {
    glAssert(glPushAttrib(GL_POLYGON_BIT));
    glAssert(glDisable(GL_CULL_FACE));
    glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  }

  // Disable depth writes if drawing something transparent
  if (m_color.a() < 1.0f)
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
    shader.set("Color", Color(m_color, m_color.a() / 2.0f));
    renderer.render();
    glAssert(glEnable(GL_DEPTH_TEST));
    shader.set("Color", m_color);
    renderer.render();
    break;
  }

  if (m_color.a() < 1.0f)
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
  m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLine(
    Vertex(start), Vertex(end));
}

void PrimitiveRenderer::renderLines(
  const Color& color,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLines(
    Vertex::toList(positions.size(), std::begin(positions)));
}

void PrimitiveRenderer::renderLineStrip(
  const Color& color,
  const float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLineStrip(
    Vertex::toList(positions.size(), std::begin(positions)));
}

void PrimitiveRenderer::renderCoordinateSystemXY(
  const Color& x,
  const Color& y,
  float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const vm::bbox3f& bounds)
{
  vm::vec3f start, end;

  coordinateSystemVerticesX(bounds, start, end);
  renderLine(x, lineWidth, occlusionPolicy, start, end);

  coordinateSystemVerticesY(bounds, start, end);
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

  coordinateSystemVerticesX(bounds, start, end);
  renderLine(x, lineWidth, occlusionPolicy, start, end);

  coordinateSystemVerticesZ(bounds, start, end);
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

  coordinateSystemVerticesY(bounds, start, end);
  renderLine(y, lineWidth, occlusionPolicy, start, end);

  coordinateSystemVerticesZ(bounds, start, end);
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

  coordinateSystemVerticesX(bounds, start, end);
  renderLine(x, lineWidth, occlusionPolicy, start, end);

  coordinateSystemVerticesY(bounds, start, end);
  renderLine(y, lineWidth, occlusionPolicy, start, end);

  coordinateSystemVerticesZ(bounds, start, end);
  renderLine(z, lineWidth, occlusionPolicy, start, end);
}

void PrimitiveRenderer::renderPolygon(
  const Color& color,
  float lineWidth,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_lineMeshes[LineRenderAttributes(color, lineWidth, occlusionPolicy)].addLineLoop(
    Vertex::toList(positions.size(), std::begin(positions)));
}

void PrimitiveRenderer::renderFilledPolygon(
  const Color& color,
  const PrimitiveRendererOcclusionPolicy occlusionPolicy,
  const PrimitiveRendererCullingPolicy cullingPolicy,
  const std::vector<vm::vec3f>& positions)
{
  m_triangleMeshes[TriangleRenderAttributes(color, occlusionPolicy, cullingPolicy)]
    .addTriangleFan(Vertex::toList(positions.size(), std::begin(positions)));
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
  assert(radius > 0.0f);
  assert(segments > 2);

  const vm::vec3f vec = end - start;
  const float len = vm::length(vec);
  const vm::vec3f dir = vec / len;

  const vm::mat4x4f translation = vm::translation_matrix(start);
  const vm::mat4x4f rotation = vm::rotation_matrix(vm::vec3f::pos_z(), dir);
  const vm::mat4x4f transform = translation * rotation;

  const VertsAndNormals cylinder = cylinder3D(radius, len, segments);
  const std::vector<vm::vec3f> vertices = transform * cylinder.vertices;

  m_triangleMeshes[TriangleRenderAttributes(color, occlusionPolicy, cullingPolicy)]
    .addTriangleStrip(Vertex::toList(vertices.size(), std::begin(vertices)));
}

void PrimitiveRenderer::doPrepareVertices(VboManager& vboManager)
{
  prepareLines(vboManager);
  prepareTriangles(vboManager);
}

void PrimitiveRenderer::prepareLines(VboManager& vboManager)
{
  for (auto& [attributes, mesh] : m_lineMeshes)
  {
    IndexRangeRenderer& renderer =
      m_lineMeshRenderers.insert(std::make_pair(attributes, IndexRangeRenderer(mesh)))
        .first->second;
    renderer.prepare(vboManager);
  }
}

void PrimitiveRenderer::prepareTriangles(VboManager& vboManager)
{
  for (auto& [attributes, mesh] : m_triangleMeshes)
  {
    IndexRangeRenderer& renderer =
      m_triangleMeshRenderers.insert(std::make_pair(attributes, IndexRangeRenderer(mesh)))
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
  ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPUniformCShader);

  for (auto& [attributes, renderer] : m_lineMeshRenderers)
  {
    attributes.render(renderer, shader, renderContext.dpiScale());
  }
  glAssert(glLineWidth(renderContext.dpiScale()));
}

void PrimitiveRenderer::renderTriangles(RenderContext& renderContext)
{
  ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPUniformCShader);

  for (auto& [attributes, renderer] : m_triangleMeshRenderers)
  {
    attributes.render(renderer, shader);
  }
}
} // namespace Renderer
} // namespace TrenchBroom
