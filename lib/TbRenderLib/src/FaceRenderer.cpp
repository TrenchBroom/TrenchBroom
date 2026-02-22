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

#include "render/FaceRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/Camera.h"
#include "gl/Material.h"
#include "gl/MaterialRenderFunc.h"
#include "gl/PrimType.h"
#include "gl/ShaderManager.h"
#include "gl/Shaders.h"
#include "gl/Texture.h"
#include "render/BrushRendererArrays.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"

namespace tb::render
{

namespace
{

class RenderFunc : public gl::MaterialRenderFunc
{
private:
  gl::ActiveShader& m_shader;
  bool m_applyMaterial;
  Color m_defaultColor;
  int m_minFilter;
  int m_magFilter;

public:
  RenderFunc(
    gl::ActiveShader& shader,
    const bool applyMaterial,
    Color defaultColor,
    const int minFilter,
    const int magFilter)
    : m_shader{shader}
    , m_applyMaterial{applyMaterial}
    , m_defaultColor{std::move(defaultColor)}
    , m_minFilter{minFilter}
    , m_magFilter{magFilter}
  {
  }

  void before(const gl::Material* material) override
  {
    if (const auto* texture = gl::getTexture(material))
    {
      material->activate(m_minFilter, m_magFilter);
      m_shader.set("ApplyMaterial", m_applyMaterial);
      m_shader.set("Color", texture->averageColor());
    }
    else
    {
      m_shader.set("ApplyMaterial", false);
      m_shader.set("Color", m_defaultColor);
    }
  }

  void after(const gl::Material* material) override
  {
    if (material)
    {
      material->deactivate();
    }
  }
};

} // namespace

FaceRenderer::FaceRenderer() = default;

FaceRenderer::FaceRenderer(
  std::shared_ptr<BrushVertexArray> vertexArray,
  std::shared_ptr<MaterialToBrushIndicesMap> indexArrayMap,
  Color faceColor)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArrayMap{std::move(indexArrayMap)}
  , m_faceColor{std::move(faceColor)}
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

void FaceRenderer::prepare(gl::VboManager& vboManager)
{
  m_vertexArray->prepare(vboManager);

  for (const auto& [material, brushIndexHolderPtr] : *m_indexArrayMap)
  {
    brushIndexHolderPtr->prepare(vboManager);
  }
}

void FaceRenderer::render(RenderContext& context)
{
  auto* currentProgram = context.shaderManager().currentProgram();
  contract_assert(currentProgram);

  if (!m_indexArrayMap->empty() && m_vertexArray->setup(*currentProgram))
  {
    auto& shaderManager = context.shaderManager();
    auto shader = gl::ActiveShader{shaderManager, gl::Shaders::FaceShader};
    auto& prefs = PreferenceManager::instance();

    const auto applyMaterial = context.showMaterials();
    const auto shadeFaces = context.shadeFaces();
    const auto showFog = context.showFog();

    glAssert(glEnable(GL_TEXTURE_2D));
    glAssert(glActiveTexture(GL_TEXTURE0));
    shader.set("Brightness", prefs.get(Preferences::Brightness));
    shader.set("RenderGrid", context.showGrid());
    shader.set("GridSize", static_cast<float>(context.gridSize()));
    shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
    shader.set("ApplyMaterial", applyMaterial);
    shader.set("Material", 0);
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
      RgbaF{prefs.get(Preferences::SoftMapBoundsColor).to<RgbF>(), 0.1f});

    auto func = RenderFunc{
      shader,
      applyMaterial,
      m_faceColor,
      context.minFilterMode(),
      context.magFilterMode()};

    if (m_alpha < 1.0f)
    {
      glAssert(glDepthMask(GL_FALSE));
    }
    for (const auto& [material, brushIndexHolderPtr] : *m_indexArrayMap)
    {
      if (brushIndexHolderPtr->hasValidIndices())
      {
        const auto* texture = getTexture(material);
        const auto enableMasked = texture && texture->mask() == gl::TextureMask::On;

        // set any per-material uniforms
        shader.set("GridColor", material);
        shader.set("EnableMasked", enableMasked);

        func.before(material);
        brushIndexHolderPtr->setup();
        brushIndexHolderPtr->render(gl::PrimType::Triangles);
        brushIndexHolderPtr->cleanup();
        func.after(material);
      }
    }
    if (m_alpha < 1.0f)
    {
      glAssert(glDepthMask(GL_TRUE));
    }
    m_vertexArray->cleanup(*currentProgram);
  }
}

} // namespace tb::render
