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

#include "Compass.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/IndexRangeMapBuilder.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/Vertex.h"
#include "gl/VertexArray.h"
#include "gl/VertexType.h"
#include "render/Camera.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderUtils.h"
#include "render/Transformation.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <cassert>
#include <vector>

namespace tb::render
{
const size_t Compass::m_segments = 32;
const float Compass::m_shaftLength = 28.0f;
const float Compass::m_shaftRadius = 1.2f;
const float Compass::m_headLength = 7.0f;
const float Compass::m_headRadius = 3.5f;

Compass::Compass()
{
  makeArrows();
  makeBackground();
}

Compass::~Compass() = default;

void Compass::render(RenderBatch& renderBatch)
{
  renderBatch.add(this);
}

void Compass::doPrepareVertices(gl::VboManager& vboManager)
{
  if (!m_prepared)
  {
    m_arrowRenderer.prepare(vboManager);
    m_backgroundRenderer.prepare(vboManager);
    m_backgroundOutlineRenderer.prepare(vboManager);
    m_prepared = true;
  }
}

void Compass::doRender(RenderContext& renderContext)
{
  const auto& camera = renderContext.camera();
  const auto& viewport = camera.viewport();
  const auto viewWidth = static_cast<float>(viewport.width);
  const auto viewHeight = static_cast<float>(viewport.height);

  const auto projection = vm::ortho_matrix(
    0.0f,
    1000.0f,
    -viewWidth / 2.0f,
    viewHeight / 2.0f,
    viewWidth / 2.0f,
    -viewHeight / 2.0f);
  const auto view = vm::view_matrix(vm::vec3f{0, 1, 0}, vm::vec3f{0, 0, 1})
                    * vm::translation_matrix(500.0f * vm::vec3f{0, 1, 0});
  const auto ortho =
    ReplaceTransformation{renderContext.transformation(), projection, view};

  const auto translation = vm::translation_matrix(
    vm::vec3f{-viewWidth / 2.0f + 55.0f, 0.0f, -viewHeight / 2.0f + 55.0f});
  const auto scaling = vm::scaling_matrix(vm::vec3f::fill(2.0f));
  const auto compassTransformation = translation * scaling;
  const auto compass =
    MultiplyModelMatrix{renderContext.transformation(), compassTransformation};
  const auto cameraTransformation = cameraRotationMatrix(camera);

  glAssert(glClear(GL_DEPTH_BUFFER_BIT));
  renderBackground(renderContext);
  glAssert(glClear(GL_DEPTH_BUFFER_BIT));
  doRenderCompass(renderContext, cameraTransformation);
}

void Compass::makeArrows()
{
  const auto shaftOffset =
    vm::vec3f{0.0f, 0.0f, -(m_shaftLength + m_headLength) / 2.0f + 2.0f};
  const auto headOffset = vm::vec3f{0.0f, 0.0f, m_shaftLength} + shaftOffset;

  auto shaft = cylinder(m_shaftRadius, m_shaftLength, m_segments);
  for (size_t i = 0; i < shaft.vertices.size(); ++i)
  {
    shaft.vertices[i] = shaft.vertices[i] + shaftOffset;
  }

  auto head = cone(m_headRadius, m_headLength, m_segments);
  for (size_t i = 0; i < head.vertices.size(); ++i)
  {
    head.vertices[i] = head.vertices[i] + headOffset;
  }

  auto shaftCap = circle3D(m_shaftRadius, m_segments);
  for (size_t i = 0; i < shaftCap.vertices.size(); ++i)
  {
    shaftCap.vertices[i] = vm::mat4x4f::rot_180_x() * shaftCap.vertices[i] + shaftOffset;
    shaftCap.normals[i] = vm::mat4x4f::rot_180_x() * shaftCap.normals[i];
  }

  auto headCap = circle3D(m_headRadius, m_segments);
  for (size_t i = 0; i < headCap.vertices.size(); ++i)
  {
    headCap.vertices[i] = vm::mat4x4f::rot_180_x() * headCap.vertices[i] + headOffset;
    headCap.normals[i] = vm::mat4x4f::rot_180_x() * headCap.normals[i];
  }

  using Vertex = gl::VertexTypes::P3N::Vertex;
  auto shaftVertices = Vertex::toList(
    shaft.vertices.size(), std::begin(shaft.vertices), std::begin(shaft.normals));
  auto headVertices = Vertex::toList(
    head.vertices.size(), std::begin(head.vertices), std::begin(head.normals));
  auto shaftCapVertices = Vertex::toList(
    shaftCap.vertices.size(),
    std::begin(shaftCap.vertices),
    std::begin(shaftCap.normals));
  auto headCapVertices = Vertex::toList(
    headCap.vertices.size(), std::begin(headCap.vertices), std::begin(headCap.normals));

  const auto vertexCount = shaftVertices.size() + headVertices.size()
                           + shaftCapVertices.size() + headCapVertices.size();
  auto indexArraySize = gl::IndexRangeMap::Size{};
  indexArraySize.inc(gl::PrimType::TriangleStrip);
  indexArraySize.inc(gl::PrimType::TriangleFan, 2);
  indexArraySize.inc(gl::PrimType::Triangles, headVertices.size() / 3);

  auto builder = gl::IndexRangeMapBuilder<Vertex::Type>{vertexCount, indexArraySize};
  builder.addTriangleStrip(shaftVertices);
  builder.addTriangleFan(shaftCapVertices);
  builder.addTriangleFan(headCapVertices);
  builder.addTriangles(headVertices);

  m_arrowRenderer = gl::IndexRangeRenderer{builder};
}

void Compass::makeBackground()
{
  using Vertex = gl::VertexTypes::P2::Vertex;
  auto circ = circle2D(
    (m_shaftLength + m_headLength) / 2.0f + 5.0f, 0.0f, vm::Cf::two_pi(), m_segments);
  auto verts = Vertex::toList(circ.size(), std::begin(circ));

  auto backgroundSize = gl::IndexRangeMap::Size{};
  backgroundSize.inc(gl::PrimType::TriangleFan);

  auto backgroundBuilder =
    gl::IndexRangeMapBuilder<Vertex::Type>{verts.size(), backgroundSize};
  backgroundBuilder.addTriangleFan(verts);

  m_backgroundRenderer = gl::IndexRangeRenderer{backgroundBuilder};

  auto outlineSize = gl::IndexRangeMap::Size{};
  outlineSize.inc(gl::PrimType::LineLoop);

  auto outlineBuilder = gl::IndexRangeMapBuilder<Vertex::Type>{verts.size(), outlineSize};
  outlineBuilder.addLineLoop(verts);

  m_backgroundOutlineRenderer = gl::IndexRangeRenderer{outlineBuilder};
}

vm::mat4x4f Compass::cameraRotationMatrix(const Camera& camera) const
{
  auto rotation = vm::mat4x4f{};
  rotation[0] = vm::vec4f{camera.right()};
  rotation[1] = vm::vec4f{camera.direction()};
  rotation[2] = vm::vec4f{camera.up()};

  return *invert(rotation);
}

void Compass::renderBackground(RenderContext& renderContext)
{
  auto& prefs = PreferenceManager::instance();

  const auto rotate =
    MultiplyModelMatrix{renderContext.transformation(), vm::mat4x4f::rot_90_x_ccw()};
  auto shader =
    gl::ActiveShader{renderContext.shaderManager(), gl::Shaders::CompassBackgroundShader};

  shader.set("Color", prefs.get(Preferences::CompassBackgroundColor));
  m_backgroundRenderer.render();

  shader.set("Color", prefs.get(Preferences::CompassBackgroundOutlineColor));
  m_backgroundOutlineRenderer.render();
}

void Compass::renderSolidAxis(
  RenderContext& renderContext, const vm::mat4x4f& transformation, const Color& color)
{
  auto shader =
    gl::ActiveShader{renderContext.shaderManager(), gl::Shaders::CompassShader};
  shader.set("CameraPosition", vm::vec3f{0, 500, 0});
  shader.set("LightDirection", vm::normalize(vm::vec3f{0, 0.5, 1}));
  shader.set("LightDiffuse", RgbaF{1.0f, 1.0f, 1.0f, 1.0f});
  shader.set("LightSpecular", RgbaF{0.3f, 0.3f, 0.3f, 1.0f});
  shader.set("GlobalAmbient", RgbaF{0.2f, 0.2f, 0.2f, 1.0f});
  shader.set("MaterialShininess", 32.0f);

  shader.set("MaterialDiffuse", color);
  shader.set("MaterialAmbient", color);
  shader.set("MaterialSpecular", color);

  renderAxis(renderContext, transformation);
}

void Compass::renderAxisOutline(
  RenderContext& renderContext, const vm::mat4x4f& transformation, const Color& color)
{
  glAssert(glDepthMask(GL_FALSE));
  glAssert(glLineWidth(3.0f));
  glAssert(glPolygonMode(GL_FRONT, GL_LINE));

  auto shader =
    gl::ActiveShader{renderContext.shaderManager(), gl::Shaders::CompassOutlineShader};
  shader.set("Color", color);
  renderAxis(renderContext, transformation);

  glAssert(glDepthMask(GL_TRUE));
  glAssert(glLineWidth(1.0f));
  glAssert(glPolygonMode(GL_FRONT, GL_FILL));
}

void Compass::renderAxis(RenderContext& renderContext, const vm::mat4x4f& transformation)
{
  const auto apply = MultiplyModelMatrix{renderContext.transformation(), transformation};
  m_arrowRenderer.render();
}

} // namespace tb::render
