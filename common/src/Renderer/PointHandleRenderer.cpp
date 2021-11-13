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

#include "PointHandleRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/VboManager.h"

#include <vecmath/forward.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
namespace Renderer {
PointHandleRenderer::PointHandleRenderer()
  : m_handle(pref(Preferences::HandleRadius), 16, true)
  , m_highlight(2.0f * pref(Preferences::HandleRadius), 16, false) {}

void PointHandleRenderer::addPoint(const Color& color, const vm::vec3f& position) {
  m_pointHandles[color].push_back(position);
}

void PointHandleRenderer::addHighlight(const Color& color, const vm::vec3f& position) {
  m_highlights[color].push_back(position);
}

void PointHandleRenderer::doPrepareVertices(VboManager& vboManager) {
  m_handle.prepare(vboManager);
  m_highlight.prepare(vboManager);
}

void PointHandleRenderer::doRender(RenderContext& renderContext) {
  const Camera& camera = renderContext.camera();
  const Camera::Viewport& viewport = camera.viewport();
  const vm::mat4x4f projection = vm::ortho_matrix(
    0.0f, 1.0f, static_cast<float>(viewport.x), static_cast<float>(viewport.height),
    static_cast<float>(viewport.width), static_cast<float>(viewport.y));
  const vm::mat4x4f view = vm::view_matrix(vm::vec3f::neg_z(), vm::vec3f::pos_y());
  ReplaceTransformation ortho(renderContext.transformation(), projection, view);

  if (renderContext.render3D()) {
    // Un-occluded handles: use depth test, draw fully opaque
    renderHandles(renderContext, m_pointHandles, m_handle, 1.0f);
    renderHandles(renderContext, m_highlights, m_highlight, 1.0f);

    // Occluded handles: don't use depth test, but draw translucent
    glAssert(glDisable(GL_DEPTH_TEST));
    renderHandles(renderContext, m_pointHandles, m_handle, 0.33f);
    renderHandles(renderContext, m_highlights, m_highlight, 0.33f);
    glAssert(glEnable(GL_DEPTH_TEST));
  } else {
    // In 2D views, render fully opaque without depth test
    glAssert(glDisable(GL_DEPTH_TEST));
    renderHandles(renderContext, m_pointHandles, m_handle, 1.0f);
    renderHandles(renderContext, m_highlights, m_highlight, 1.0f);
    glAssert(glEnable(GL_DEPTH_TEST));
  }

  clear();
}

void PointHandleRenderer::renderHandles(
  RenderContext& renderContext, const HandleMap& map, Circle& circle, const float opacity) {
  const Camera& camera = renderContext.camera();
  ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);

  for (const auto& [color, positions] : map) {
    shader.set("Color", mixAlpha(color, opacity));

    for (const vm::vec3f& position : positions) {
      vm::vec3f nudgeTowardsCamera;

      // In 3D view, nudge towards camera by the handle radius, to prevent lines (brush edges, etc.)
      // from clipping into the handle
      if (renderContext.render3D()) {
        nudgeTowardsCamera =
          vm::normalize(camera.position() - position) * pref(Preferences::HandleRadius);
      }

      const vm::vec3f offset =
        camera.project(position + nudgeTowardsCamera) * vm::vec3f(1.0f, 1.0f, -1.0f);
      MultiplyModelMatrix translate(renderContext.transformation(), vm::translation_matrix(offset));
      circle.render();
    }
  }
}

void PointHandleRenderer::clear() {
  m_pointHandles.clear();
  m_highlights.clear();
}
} // namespace Renderer
} // namespace TrenchBroom
