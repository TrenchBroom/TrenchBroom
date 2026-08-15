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

#include "base/PreferenceManager.h"
#include "gl/ActiveShader.h"
#include "gl/Camera.h"
#include "gl/GlInterface.h"
#include "gl/Material.h"
#include "gl/MaterialRenderFunc.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/Texture.h"
#include "prefs/Preferences.h"
#include "render/BrushRendererArrays.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"

#include "vm/vec.h"

#include <algorithm>

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

  void before(gl::Gl& gl, const gl::Material* material) override
  {
    if (const auto* texture = gl::getTexture(material))
    {
      material->activate(gl, m_minFilter, m_magFilter);
      m_shader.set("ApplyMaterial", m_applyMaterial);
      m_shader.set("Color", texture->averageColor());
    }
    else
    {
      m_shader.set("ApplyMaterial", false);
      m_shader.set("Color", m_defaultColor);
    }
  }

  void after(gl::Gl& gl, const gl::Material* material) override
  {
    if (material)
    {
      material->deactivate(gl);
    }
  }
};

} // namespace

FaceRenderer::FaceRenderer() = default;

FaceRenderer::FaceRenderer(
  std::shared_ptr<BrushVertexArray> vertexArray,
  std::shared_ptr<MaterialToBrushIndicesMap> indexArrayMap,
  Color faceColor,
  std::shared_ptr<const std::vector<TransparentDrawItem>> sortedDrawItems)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArrayMap{std::move(indexArrayMap)}
  , m_sortedDrawItems{std::move(sortedDrawItems)}
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

void FaceRenderer::setDisableDepthWrite(const bool disableDepthWrite)
{
  m_disableDepthWrite = disableDepthWrite;
}

void FaceRenderer::render(RenderBatch& renderBatch)
{
  renderBatch.add(this);
}

void FaceRenderer::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  m_vertexArray->prepare(gl, vboManager);

  for (const auto& [material, brushIndexHolderPtr] : *m_indexArrayMap)
  {
    brushIndexHolderPtr->prepare(gl, vboManager);
  }
}

void FaceRenderer::render(RenderContext& context)
{
  auto& gl = context.gl();

  auto& shaderManager = context.shaderManager();
  auto shader = gl::ActiveShader{gl, shaderManager, gl::Shaders::FaceShader};

  if (!m_indexArrayMap->empty() && m_vertexArray->setup(gl, shader.program()))
  {
    auto& prefs = PreferenceManager::instance();

    const auto applyMaterial = context.showMaterials();
    const auto shadeFaces = context.shadeFaces();
    const auto showFog = context.showFog();

    gl.enable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE0);
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
    shader.set("AlphaFuncCompare", size_t{0});
    shader.set("AlphaFuncThreshold", 0.5f);
    shader.set("ShowSoftMapBounds", !context.softMapBounds().is_empty());
    shader.set("SoftMapBoundsMin", context.softMapBounds().min);
    shader.set("SoftMapBoundsMax", context.softMapBounds().max);
    shader.set(
      "SoftMapBoundsColor",
      RgbaF{prefs.get(Preferences::SoftMapBoundsColor).to<RgbF>(), 0.1f});

    auto renderFunc = RenderFunc{
      shader,
      applyMaterial,
      m_faceColor,
      context.minFilterMode(),
      context.magFilterMode()};

    const auto setMaterialUniforms = [&](const gl::Material* material) {
      const auto isRealBlend = material
                               && material->effectiveBlendFunc().enable
                                    == gl::MaterialBlendFunc::Enable::UseFactors;

      gl::setAlphaFuncUniforms(shader, material);
      // A material with real per-pixel blending renders with its own true alpha,
      // independent of the whole-batch X-ray/hidden-brush fade.
      shader.set("Alpha", isRealBlend ? 1.0f : m_alpha);
    };

    if (m_disableDepthWrite)
    {
      gl.depthMask(GL_FALSE);
    }

    if (m_sortedDrawItems)
    {
      renderTransparentItems(context, renderFunc, setMaterialUniforms);
    }
    else
    {
      renderOpaqueItems(context, renderFunc, setMaterialUniforms);
    }

    if (m_disableDepthWrite)
    {
      gl.depthMask(GL_TRUE);
    }

    m_vertexArray->cleanup(gl, shader.program());
  }
}

void FaceRenderer::renderTransparentItems(
  RenderContext& context,
  gl::MaterialRenderFunc& renderFunc,
  const std::function<void(const gl::Material*)>& setMaterialUniforms)
{
  auto& gl = context.gl();

  // Correct back-to-front blending requires draw order to follow distance to the
  // camera, which is fundamentally incompatible with batching by material -- so unlike
  // renderOpaqueItems, this draws one item at a time in sorted order instead of one
  // whole material buffer at a time.
  //
  // Sort a reused array of pointers into m_sortedDrawItems rather than a copy of the
  // items themselves -- the camera (and therefore the sort order) can change every
  // frame, but the items themselves only change when BrushRenderer::validate() reruns.
  m_transparentDrawOrder.resize(m_sortedDrawItems->size());
  std::ranges::transform(
    *m_sortedDrawItems, m_transparentDrawOrder.begin(), [](const auto& item) {
      return &item;
    });
  std::ranges::sort(m_transparentDrawOrder, [&](const auto* lhs, const auto* rhs) {
    return vm::squared_distance(lhs->sortPosition, context.camera().position())
           > vm::squared_distance(rhs->sortPosition, context.camera().position());
  });

  for (const auto* item : m_transparentDrawOrder)
  {
    if (const auto it = m_indexArrayMap->find(item->material);
        it != m_indexArrayMap->end())
    {
      const auto& brushIndexHolderPtr = it->second;

      setMaterialUniforms(item->material);

      renderFunc.before(gl, item->material);
      brushIndexHolderPtr->setup(gl);
      brushIndexHolderPtr->render(
        gl, gl::PrimType::Triangles, item->indexPos, item->indexCount);
      brushIndexHolderPtr->cleanup(gl);
      renderFunc.after(gl, item->material);
    }
  }
}

void FaceRenderer::renderOpaqueItems(
  RenderContext& context,
  gl::MaterialRenderFunc& renderFunc,
  const std::function<void(const gl::Material*)>& setMaterialUniforms) const
{
  auto& gl = context.gl();

  for (const auto& [material, brushIndexHolderPtr] : *m_indexArrayMap)
  {
    if (brushIndexHolderPtr->hasValidIndices())
    {
      setMaterialUniforms(material);

      renderFunc.before(gl, material);
      brushIndexHolderPtr->setup(gl);
      brushIndexHolderPtr->render(gl, gl::PrimType::Triangles);
      brushIndexHolderPtr->cleanup(gl);
      renderFunc.after(gl, material);
    }
  }
}

} // namespace tb::render
