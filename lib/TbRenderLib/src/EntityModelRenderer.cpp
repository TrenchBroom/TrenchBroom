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

#include "render/EntityModelRenderer.h"

#include "base/Logger.h"
#include "base/PreferenceManager.h"
#include "gl/ActiveShader.h"
#include "gl/Camera.h"
#include "gl/GlInterface.h"
#include "gl/Material.h"
#include "gl/MaterialIndexRangeRenderer.h"
#include "gl/MaterialRenderFunc.h"
#include "gl/Shaders.h"
#include "mdl/AssetUtils.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityModel.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "prefs/Preferences.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/Transformation.h"

#include "vm/mat.h"
#include "vm/vec.h"

#include <algorithm>
#include <ranges>
#include <vector>

namespace tb::render
{

namespace
{

bool hasTranslucentSkin(const mdl::EntityModelData& modelData)
{
  auto materials =
    modelData.surfaces() | std::views::transform([](const auto& surface) {
      return std::views::iota(0u, surface.skinCount())
             | std::views::transform([&](const auto i) { return surface.skin(i); });
    })
    | std::views::join
    | std::views::filter([](const auto* material) { return material != nullptr; })
    | std::views::transform(
      [](const auto* material) -> const gl::Material& { return *material; });

  return std::ranges::any_of(materials, [](const auto& material) {
    return material.effectiveBlendFunc().enable
           == gl::MaterialBlendFunc::Enable::UseFactors;
  });
}

bool isTransparentEntity(const mdl::EntityNode& entityNode)
{
  const auto* model = entityNode.entity().model();
  const auto* modelData = model ? model->data() : nullptr;
  return modelData && hasTranslucentSkin(*modelData);
}

} // namespace

EntityModelRenderer::Pass::Pass(EntityModelRenderer& owner, const bool transparent)
  : m_owner{owner}
  , m_transparent{transparent}
{
}

void EntityModelRenderer::Pass::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  m_owner.m_entityModelManager.prepare(gl, vboManager);
}

void EntityModelRenderer::Pass::render(RenderContext& context)
{
  m_owner.renderPass(context, m_transparent);
}

EntityModelRenderer::EntityModelRenderer(
  Logger& logger,
  mdl::EntityModelManager& entityModelManager,
  const mdl::EditorContext& editorContext)
  : m_logger{logger}
  , m_entityModelManager{entityModelManager}
  , m_editorContext{editorContext}
  , m_opaquePass{*this, false}
  , m_transparentPass{*this, true}
{
}

EntityModelRenderer::~EntityModelRenderer()
{
  clear();
}

void EntityModelRenderer::addEntity(const mdl::EntityNode& entityNode)
{
  const auto modelSpec =
    mdl::safeGetModelSpecification(m_logger, entityNode.entity().classname(), [&]() {
      return entityNode.entity().modelSpecification();
    });

  auto* renderer = m_entityModelManager.renderer(modelSpec);
  if (renderer != nullptr)
  {
    m_entities.emplace(&entityNode, renderer);
  }
}

void EntityModelRenderer::removeEntity(const mdl::EntityNode& entityNode)
{
  m_entities.erase(&entityNode);
}

void EntityModelRenderer::updateEntity(const mdl::EntityNode& entityNode)
{
  const auto modelSpec =
    mdl::safeGetModelSpecification(m_logger, entityNode.entity().classname(), [&]() {
      return entityNode.entity().modelSpecification();
    });

  auto* renderer = m_entityModelManager.renderer(modelSpec);
  auto it = m_entities.find(&entityNode);

  if (renderer == nullptr && it == std::end(m_entities))
  {
    return;
  }

  if (it == std::end(m_entities))
  {
    m_entities.emplace(&entityNode, renderer);
  }
  else
  {
    if (renderer == nullptr)
    {
      m_entities.erase(it);
    }
    else if (it->second != renderer)
    {
      it->second = renderer;
    }
  }
}

void EntityModelRenderer::clear()
{
  m_entities.clear();
}

bool EntityModelRenderer::applyTinting() const
{
  return m_applyTinting;
}

void EntityModelRenderer::setApplyTinting(const bool applyTinting)
{
  m_applyTinting = applyTinting;
}

const Color& EntityModelRenderer::tintColor() const
{
  return m_tintColor;
}

void EntityModelRenderer::setTintColor(const Color& tintColor)
{
  m_tintColor = tintColor;
}

bool EntityModelRenderer::showHiddenEntities() const
{
  return m_showHiddenEntities;
}

void EntityModelRenderer::setShowHiddenEntities(const bool showHiddenEntities)
{
  m_showHiddenEntities = showHiddenEntities;
}

void EntityModelRenderer::renderOpaque(RenderBatch& renderBatch)
{
  renderBatch.add(&m_opaquePass);
}

void EntityModelRenderer::renderTransparent(RenderBatch& renderBatch)
{
  renderBatch.add(&m_transparentPass);
}

void EntityModelRenderer::renderPass(RenderContext& renderContext, const bool transparent)
{
  if (m_entities.empty())
  {
    return;
  }

  auto& gl = renderContext.gl();

  gl.enable(GL_TEXTURE_2D);
  gl.activeTexture(GL_TEXTURE0);

  auto& prefs = PreferenceManager::instance();
  auto shader =
    gl::ActiveShader{gl, renderContext.shaderManager(), gl::Shaders::EntityModelShader};
  shader.set("Brightness", prefs.get(Preferences::Brightness));
  shader.set("ApplyTinting", m_applyTinting);
  shader.set("TintColor", m_tintColor);
  shader.set("GrayScale", false);
  shader.set("Material", 0);
  shader.set("EnableMasked", false);
  shader.set("AlphaFuncCompare", size_t{0});
  shader.set("AlphaFuncThreshold", 0.5f);
  shader.set("ShowSoftMapBounds", !renderContext.softMapBounds().is_empty());
  shader.set("SoftMapBoundsMin", renderContext.softMapBounds().min);
  shader.set("SoftMapBoundsMax", renderContext.softMapBounds().max);
  shader.set(
    "SoftMapBoundsColor",
    RgbaF{prefs.get(Preferences::SoftMapBoundsColor).to<RgbF>(), 0.1f});

  shader.set("CameraPosition", renderContext.camera().position());
  shader.set("CameraDirection", renderContext.camera().direction());
  shader.set("CameraRight", renderContext.camera().right());
  shader.set("CameraUp", renderContext.camera().up());
  shader.set("ViewMatrix", renderContext.camera().viewMatrix());

  const auto& propertyConfig = m_entities.begin()->first->entityPropertyConfig();
  const auto& defaultModelScaleExpression = propertyConfig.defaultModelScaleExpression;

  const auto renderEntity =
    [&](const mdl::EntityNode& entityNode, gl::MaterialRenderer& renderer) {
      if (!m_showHiddenEntities && !m_editorContext.visible(entityNode))
      {
        return;
      }

      const auto* model = entityNode.entity().model();
      const auto* modelData = model ? model->data() : nullptr;
      if (!modelData)
      {
        return;
      }

      shader.set("Orientation", static_cast<int>(modelData->orientation()));

      const auto transformation =
        vm::mat4x4f{entityNode.entity().modelTransformation(defaultModelScaleExpression)};
      const auto multMatrix =
        MultiplyModelMatrix{renderContext.transformation(), transformation};

      shader.set("ModelMatrix", transformation);

      auto renderFunc = gl::AlphaTestedMaterialRenderFunc{
        shader, renderContext.minFilterMode(), renderContext.magFilterMode()};
      renderer.render(gl, shader.program(), renderFunc);
    };

  if (transparent)
  {
    // Correct back-to-front blending requires draw order to follow distance to the
    // camera, which can change every frame, so the entities are re-sorted each time
    // rather than once when m_entities changes.
    m_transparentDrawOrder.clear();
    for (const auto& [entityNode, renderer] : m_entities)
    {
      if (isTransparentEntity(*entityNode))
      {
        m_transparentDrawOrder.push_back(entityNode);
      }
    }

    const auto cameraPosition = renderContext.camera().position();
    std::ranges::sort(m_transparentDrawOrder, [&](const auto* lhs, const auto* rhs) {
      return vm::squared_distance(vm::vec3f{lhs->entity().origin()}, cameraPosition)
             > vm::squared_distance(vm::vec3f{rhs->entity().origin()}, cameraPosition);
    });

    for (const auto* entityNode : m_transparentDrawOrder)
    {
      renderEntity(*entityNode, *m_entities.at(entityNode));
    }
  }
  else
  {
    for (const auto& [entityNode, renderer] : m_entities)
    {
      if (!isTransparentEntity(*entityNode))
      {
        renderEntity(*entityNode, *renderer);
      }
    }
  }
}

} // namespace tb::render
