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

#include "Compass.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>

#include <cassert>
#include <vector>

namespace TrenchBroom {
namespace Renderer {
const size_t Compass::m_segments = 32;
const float Compass::m_shaftLength = 28.0f;
const float Compass::m_shaftRadius = 1.2f;
const float Compass::m_headLength = 7.0f;
const float Compass::m_headRadius = 3.5f;

Compass::Compass()
  : m_prepared(false) {
  makeArrows();
  makeBackground();
}

Compass::~Compass() = default;

void Compass::render(RenderBatch& renderBatch) {
  renderBatch.add(this);
}

void Compass::doPrepareVertices(VboManager& vboManager) {
  if (!m_prepared) {
    m_arrowRenderer.prepare(vboManager);
    m_backgroundRenderer.prepare(vboManager);
    m_backgroundOutlineRenderer.prepare(vboManager);
    m_prepared = true;
  }
}

void Compass::doRender(RenderContext& renderContext) {
  const auto& camera = renderContext.camera();
  const auto& viewport = camera.viewport();
  const auto viewWidth = static_cast<float>(viewport.width);
  const auto viewHeight = static_cast<float>(viewport.height);

  const auto projection = vm::ortho_matrix(
    0.0f, 1000.0f, -viewWidth / 2.0f, viewHeight / 2.0f, viewWidth / 2.0f, -viewHeight / 2.0f);
  const auto view = vm::view_matrix(vm::vec3f::pos_y(), vm::vec3f::pos_z()) *
                    vm::translation_matrix(500.0f * vm::vec3f::pos_y());
  const ReplaceTransformation ortho(renderContext.transformation(), projection, view);

  const auto translation =
    vm::translation_matrix(vm::vec3f(-viewWidth / 2.0f + 55.0f, 0.0f, -viewHeight / 2.0f + 55.0f));
  const auto scaling = vm::scaling_matrix(vm::vec3f::fill(2.0f));
  const auto compassTransformation = translation * scaling;
  const MultiplyModelMatrix compass(renderContext.transformation(), compassTransformation);
  const auto cameraTransformation = cameraRotationMatrix(camera);

  glAssert(glClear(GL_DEPTH_BUFFER_BIT)) renderBackground(renderContext);
  glAssert(glClear(GL_DEPTH_BUFFER_BIT)) doRenderCompass(renderContext, cameraTransformation);
}

void Compass::makeArrows() {
  const vm::vec3f shaftOffset(0.0f, 0.0f, -(m_shaftLength + m_headLength) / 2.0f + 2.0f);
  const vm::vec3f headOffset = vm::vec3f(0.0f, 0.0f, m_shaftLength) + shaftOffset;

  VertsAndNormals shaft = cylinder3D(m_shaftRadius, m_shaftLength, m_segments);
  for (size_t i = 0; i < shaft.vertices.size(); ++i) {
    shaft.vertices[i] = shaft.vertices[i] + shaftOffset;
  }

  VertsAndNormals head = cone3D(m_headRadius, m_headLength, m_segments);
  for (size_t i = 0; i < head.vertices.size(); ++i) {
    head.vertices[i] = head.vertices[i] + headOffset;
  }

  VertsAndNormals shaftCap = circle3D(m_shaftRadius, m_segments);
  for (size_t i = 0; i < shaftCap.vertices.size(); ++i) {
    shaftCap.vertices[i] = vm::mat4x4f::rot_180_x() * shaftCap.vertices[i] + shaftOffset;
    shaftCap.normals[i] = vm::mat4x4f::rot_180_x() * shaftCap.normals[i];
  }

  VertsAndNormals headCap = circle3D(m_headRadius, m_segments);
  for (size_t i = 0; i < headCap.vertices.size(); ++i) {
    headCap.vertices[i] = vm::mat4x4f::rot_180_x() * headCap.vertices[i] + headOffset;
    headCap.normals[i] = vm::mat4x4f::rot_180_x() * headCap.normals[i];
  }

  using Vertex = GLVertexTypes::P3N::Vertex;
  std::vector<Vertex> shaftVertices =
    Vertex::toList(shaft.vertices.size(), std::begin(shaft.vertices), std::begin(shaft.normals));
  std::vector<Vertex> headVertices =
    Vertex::toList(head.vertices.size(), std::begin(head.vertices), std::begin(head.normals));
  std::vector<Vertex> shaftCapVertices = Vertex::toList(
    shaftCap.vertices.size(), std::begin(shaftCap.vertices), std::begin(shaftCap.normals));
  std::vector<Vertex> headCapVertices = Vertex::toList(
    headCap.vertices.size(), std::begin(headCap.vertices), std::begin(headCap.normals));

  const size_t vertexCount =
    shaftVertices.size() + headVertices.size() + shaftCapVertices.size() + headCapVertices.size();
  IndexRangeMap::Size indexArraySize;
  indexArraySize.inc(PrimType::TriangleStrip);
  indexArraySize.inc(PrimType::TriangleFan, 2);
  indexArraySize.inc(PrimType::Triangles, headVertices.size() / 3);

  IndexRangeMapBuilder<Vertex::Type> builder(vertexCount, indexArraySize);
  builder.addTriangleStrip(shaftVertices);
  builder.addTriangleFan(shaftCapVertices);
  builder.addTriangleFan(headCapVertices);
  builder.addTriangles(headVertices);

  m_arrowRenderer = IndexRangeRenderer(builder);
}

void Compass::makeBackground() {
  using Vertex = GLVertexTypes::P2::Vertex;
  std::vector<vm::vec2f> circ =
    circle2D((m_shaftLength + m_headLength) / 2.0f + 5.0f, 0.0f, vm::Cf::two_pi(), m_segments);
  auto verts = Vertex::toList(circ.size(), std::begin(circ));

  IndexRangeMap::Size backgroundSize;
  backgroundSize.inc(PrimType::TriangleFan);

  IndexRangeMapBuilder<Vertex::Type> backgroundBuilder(verts.size(), backgroundSize);
  backgroundBuilder.addTriangleFan(verts);

  m_backgroundRenderer = IndexRangeRenderer(backgroundBuilder);

  IndexRangeMap::Size outlineSize;
  outlineSize.inc(PrimType::LineLoop);

  IndexRangeMapBuilder<Vertex::Type> outlineBuilder(verts.size(), outlineSize);
  outlineBuilder.addLineLoop(verts);

  m_backgroundOutlineRenderer = IndexRangeRenderer(outlineBuilder);
}

vm::mat4x4f Compass::cameraRotationMatrix(const Camera& camera) const {
  vm::mat4x4f rotation;
  rotation[0] = vm::vec4f(camera.right());
  rotation[1] = vm::vec4f(camera.direction());
  rotation[2] = vm::vec4f(camera.up());

  const auto [invertible, inverseRotation] = invert(rotation);
  assert(invertible);
  unused(invertible);
  return inverseRotation;
}

void Compass::renderBackground(RenderContext& renderContext) {
  PreferenceManager& prefs = PreferenceManager::instance();

  const MultiplyModelMatrix rotate(renderContext.transformation(), vm::mat4x4f::rot_90_x_ccw());
  ActiveShader shader(renderContext.shaderManager(), Shaders::CompassBackgroundShader);
  shader.set("Color", prefs.get(Preferences::CompassBackgroundColor));
  m_backgroundRenderer.render();
  shader.set("Color", prefs.get(Preferences::CompassBackgroundOutlineColor));
  m_backgroundOutlineRenderer.render();
}

void Compass::renderSolidAxis(
  RenderContext& renderContext, const vm::mat4x4f& transformation, const Color& color) {
  ActiveShader shader(renderContext.shaderManager(), Shaders::CompassShader);
  shader.set("CameraPosition", vm::vec3f(0.0f, 500.0f, 0.0f));
  shader.set("LightDirection", vm::normalize(vm::vec3f(0.0f, 0.5f, 1.0f)));
  shader.set("LightDiffuse", Color(1.0f, 1.0f, 1.0f, 1.0f));
  shader.set("LightSpecular", Color(0.3f, 0.3f, 0.3f, 1.0f));
  shader.set("GlobalAmbient", Color(0.2f, 0.2f, 0.2f, 1.0f));
  shader.set("MaterialShininess", 32.0f);

  shader.set("MaterialDiffuse", color);
  shader.set("MaterialAmbient", color);
  shader.set("MaterialSpecular", color);

  renderAxis(renderContext, transformation);
}

void Compass::renderAxisOutline(
  RenderContext& renderContext, const vm::mat4x4f& transformation, const Color& color) {
  glAssert(glDepthMask(GL_FALSE)) glAssert(glLineWidth(3.0f))
    glAssert(glPolygonMode(GL_FRONT, GL_LINE))

      ActiveShader shader(renderContext.shaderManager(), Shaders::CompassOutlineShader);
  shader.set("Color", color);
  renderAxis(renderContext, transformation);

  glAssert(glDepthMask(GL_TRUE)) glAssert(glLineWidth(1.0f))
    glAssert(glPolygonMode(GL_FRONT, GL_FILL))
}

void Compass::renderAxis(RenderContext& renderContext, const vm::mat4x4f& transformation) {
  const MultiplyModelMatrix apply(renderContext.transformation(), transformation);
  m_arrowRenderer.render();
}
} // namespace Renderer
} // namespace TrenchBroom
