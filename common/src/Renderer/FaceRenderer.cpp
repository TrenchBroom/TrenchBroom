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

#include "FaceRenderer.h"

#include "Assets/Texture.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/BrushRendererArrays.h"
#include "Renderer/Camera.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"

namespace TrenchBroom {
namespace Renderer {
struct FaceRenderer::RenderFunc : public TextureRenderFunc {
  ActiveShader& shader;
  bool applyTexture;
  const Color& defaultColor;

  RenderFunc(ActiveShader& i_shader, const bool i_applyTexture, const Color& i_defaultColor)
    : shader(i_shader)
    , applyTexture(i_applyTexture)
    , defaultColor(i_defaultColor) {}

  void before(const Assets::Texture* texture) override {
    if (texture != nullptr) {
      texture->activate();
      shader.set("ApplyTexture", applyTexture);
      shader.set("Color", texture->averageColor());
    } else {
      shader.set("ApplyTexture", false);
      shader.set("Color", defaultColor);
    }
  }

  void after(const Assets::Texture* texture) override {
    if (texture != nullptr) {
      texture->deactivate();
    }
  }
};

FaceRenderer::FaceRenderer()
  : m_grayscale(false)
  , m_tint(false)
  , m_alpha(1.0f) {}

FaceRenderer::FaceRenderer(
  std::shared_ptr<BrushVertexArray> vertexArray,
  std::shared_ptr<TextureToBrushIndicesMap> indexArrayMap, const Color& faceColor)
  : m_vertexArray(std::move(vertexArray))
  , m_indexArrayMap(std::move(indexArrayMap))
  , m_faceColor(faceColor)
  , m_grayscale(false)
  , m_tint(false)
  , m_alpha(1.0f) {}

FaceRenderer::FaceRenderer(const FaceRenderer& other)
  : IndexedRenderable(other)
  , m_vertexArray(other.m_vertexArray)
  , m_indexArrayMap(other.m_indexArrayMap)
  , m_faceColor(other.m_faceColor)
  , m_grayscale(other.m_grayscale)
  , m_tint(other.m_tint)
  , m_tintColor(other.m_tintColor)
  , m_alpha(other.m_alpha) {}

FaceRenderer& FaceRenderer::operator=(FaceRenderer other) {
  using std::swap;
  swap(*this, other);
  return *this;
}

void swap(FaceRenderer& left, FaceRenderer& right) {
  using std::swap;
  swap(left.m_vertexArray, right.m_vertexArray);
  swap(left.m_indexArrayMap, right.m_indexArrayMap);
  swap(left.m_faceColor, right.m_faceColor);
  swap(left.m_grayscale, right.m_grayscale);
  swap(left.m_tint, right.m_tint);
  swap(left.m_tintColor, right.m_tintColor);
  swap(left.m_alpha, right.m_alpha);
}

void FaceRenderer::setGrayscale(const bool grayscale) {
  m_grayscale = grayscale;
}

void FaceRenderer::setTint(const bool tint) {
  m_tint = tint;
}

void FaceRenderer::setTintColor(const Color& color) {
  m_tintColor = color;
}

void FaceRenderer::setAlpha(const float alpha) {
  m_alpha = alpha;
}

void FaceRenderer::render(RenderBatch& renderBatch) {
  renderBatch.add(this);
}

void FaceRenderer::prepareVerticesAndIndices(VboManager& vboManager) {
  m_vertexArray->prepare(vboManager);

  for (const auto& [texture, brushIndexHolderPtr] : *m_indexArrayMap) {
    brushIndexHolderPtr->prepare(vboManager);
  }
}

void FaceRenderer::doRender(RenderContext& context) {
  if (m_indexArrayMap->empty())
    return;

  ShaderManager& shaderManager = context.shaderManager();
  ActiveShader shader(shaderManager, Shaders::FaceShader);

  if (m_vertexArray->setupVertices()) {
    PreferenceManager& prefs = PreferenceManager::instance();

    const bool applyTexture = context.showTextures();
    const bool shadeFaces = context.shadeFaces();
    const bool showFog = context.showFog();

    glAssert(glEnable(GL_TEXTURE_2D));
    glAssert(glActiveTexture(GL_TEXTURE0));
    shader.set("Brightness", prefs.get(Preferences::Brightness));
    shader.set("RenderGrid", context.showGrid());
    shader.set("GridSize", static_cast<float>(context.gridSize()));
    shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
    shader.set("ApplyTexture", applyTexture);
    shader.set("Texture", 0);
    // shader.set("ApplyTinting", m_tint);
    // if (m_tint)
    //     shader.set("TintColor", m_tintColor);
    // shader.set("GrayScale", m_grayscale);
    shader.set("CameraPosition", context.camera().position());
    shader.set("ShadeFaces", shadeFaces);
    shader.set("ShowFog", showFog);
    shader.set("Alpha", m_alpha);
    shader.set("EnableMasked", false);
    shader.set("ShowSoftMapBounds", !context.softMapBounds().is_empty());
    shader.set("SoftMapBoundsMin", context.softMapBounds().min);
    shader.set("SoftMapBoundsMax", context.softMapBounds().max);
    shader.set(
      "SoftMapBoundsColor", vm::vec4f(
                              prefs.get(Preferences::SoftMapBoundsColor).r(),
                              prefs.get(Preferences::SoftMapBoundsColor).g(),
                              prefs.get(Preferences::SoftMapBoundsColor).b(), 0.1f));

    RenderFunc func(shader, applyTexture, m_faceColor);
    if (m_alpha < 1.0f) {
      glAssert(glDepthMask(GL_FALSE));
    }
    for (const auto& [texture, brushIndexHolderPtr] : *m_indexArrayMap) {
      if (!brushIndexHolderPtr->hasValidIndices()) {
        continue;
      }

      const bool enableMasked = texture != nullptr && texture->masked();

      // set any per-texture uniforms
      shader.set("GridColor", gridColorForTexture(texture));
      shader.set("EnableMasked", enableMasked);

      func.before(texture);
      brushIndexHolderPtr->setupIndices();
      brushIndexHolderPtr->render(PrimType::Triangles);
      brushIndexHolderPtr->cleanupIndices();
      func.after(texture);
    }
    if (m_alpha < 1.0f) {
      glAssert(glDepthMask(GL_TRUE));
    }
    m_vertexArray->cleanupVertices();
  }
}
} // namespace Renderer
} // namespace TrenchBroom
