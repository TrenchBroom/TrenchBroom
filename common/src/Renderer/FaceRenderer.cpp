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

namespace TrenchBroom::Renderer
{

namespace
{

class RenderFunc : public TextureRenderFunc
{
private:
  ActiveShader& m_shader;
  bool m_applyTexture;
  Color m_defaultColor;

public:
  RenderFunc(ActiveShader& shader, const bool applyTexture, const Color& defaultColor)
    : m_shader{shader}
    , m_applyTexture{applyTexture}
    , m_defaultColor{defaultColor}
  {
  }

  void before(const Assets::Texture* texture) override
  {
    if (texture)
    {
      texture->activate();
      m_shader.set("ApplyTexture", m_applyTexture);
      m_shader.set("Color", texture->averageColor());
    }
    else
    {
      m_shader.set("ApplyTexture", false);
      m_shader.set("Color", m_defaultColor);
    }
  }

  void after(const Assets::Texture* texture) override
  {
    if (texture)
    {
      texture->deactivate();
    }
  }
};

} // namespace

FaceRenderer::FaceRenderer() = default;

FaceRenderer::FaceRenderer(
  std::shared_ptr<BrushVertexArray> vertexArray,
  std::shared_ptr<TextureToBrushIndicesMap> indexArrayMap,
  const Color& faceColor)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArrayMap{std::move(indexArrayMap)}
  , m_faceColor{faceColor}
{
}

void FaceRenderer::setGrayscale(const bool grayscale)
{
  m_grayscale = grayscale;
}

void FaceRenderer::setTint(const bool tint)
{
  m_tint = tint;
}

void FaceRenderer::setTintColor(const Color& color)
{
  m_tintColor = color;
}

void FaceRenderer::setAlpha(const float alpha)
{
  m_alpha = alpha;
}

void FaceRenderer::render(RenderBatch& renderBatch)
{
  renderBatch.add(this);
}

void FaceRenderer::prepareVerticesAndIndices(VboManager& vboManager)
{
  m_vertexArray->prepare(vboManager);

  for (const auto& [texture, brushIndexHolderPtr] : *m_indexArrayMap)
  {
    brushIndexHolderPtr->prepare(vboManager);
  }
}

void FaceRenderer::doRender(RenderContext& context)
{
  if (!m_indexArrayMap->empty() && m_vertexArray->setupVertices())
  {
    auto& shaderManager = context.shaderManager();
    auto shader = ActiveShader{shaderManager, Shaders::FaceShader};
    auto& prefs = PreferenceManager::instance();

    const auto applyTexture = context.showTextures();
    const auto shadeFaces = context.shadeFaces();
    const auto showFog = context.showFog();

    glAssert(glEnable(GL_TEXTURE_2D));
    glAssert(glActiveTexture(GL_TEXTURE0));
    shader.set("Brightness", prefs.get(Preferences::Brightness));
    shader.set("RenderGrid", context.showGrid());
    shader.set("GridSize", static_cast<float>(context.gridSize()));
    shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
    shader.set("ApplyTexture", applyTexture);
    shader.set("Texture", 0);
    shader.set("ApplyTinting", m_tint);
    if (m_tint)
    {
      shader.set("TintColor", m_tintColor);
    }
    shader.set("GrayScale", m_grayscale);
    shader.set("CameraPosition", context.camera().position());
    shader.set("ShadeFaces", shadeFaces);
    shader.set("ShowFog", showFog);
    shader.set("Alpha", m_alpha);
    shader.set("EnableMasked", false);
    shader.set("ShowSoftMapBounds", !context.softMapBounds().is_empty());
    shader.set("SoftMapBoundsMin", context.softMapBounds().min);
    shader.set("SoftMapBoundsMax", context.softMapBounds().max);
    shader.set(
      "SoftMapBoundsColor",
      vm::vec4f{prefs.get(Preferences::SoftMapBoundsColor).xyz(), 0.1f});

    auto func = RenderFunc{shader, applyTexture, m_faceColor};
    if (m_alpha < 1.0f)
    {
      glAssert(glDepthMask(GL_FALSE));
    }
    for (const auto& [texture, brushIndexHolderPtr] : *m_indexArrayMap)
    {
      if (brushIndexHolderPtr->hasValidIndices())
      {
        const auto enableMasked = texture && texture->masked();

        // set any per-texture uniforms
        shader.set("GridColor", gridColorForTexture(texture));
        shader.set("EnableMasked", enableMasked);

        func.before(texture);
        brushIndexHolderPtr->setupIndices();
        brushIndexHolderPtr->render(PrimType::Triangles);
        brushIndexHolderPtr->cleanupIndices();
        func.after(texture);
      }
    }
    if (m_alpha < 1.0f)
    {
      glAssert(glDepthMask(GL_TRUE));
    }
    m_vertexArray->cleanupVertices();
  }
}

} // namespace TrenchBroom::Renderer
