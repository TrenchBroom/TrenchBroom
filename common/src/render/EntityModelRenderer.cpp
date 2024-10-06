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

#include "EntityModelRenderer.h"

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "asset/AssetUtils.h"
#include "asset/EntityModel.h"
#include "asset/EntityModelManager.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "render/ActiveShader.h"
#include "render/Camera.h"
#include "render/MaterialIndexRangeRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderUtils.h"
#include "render/Shaders.h"
#include "render/Transformation.h"

#include "vm/mat.h"

#include <vector>

namespace tb::render
{

EntityModelRenderer::EntityModelRenderer(
  Logger& logger,
  asset::EntityModelManager& entityModelManager,
  const mdl::EditorContext& editorContext)
  : m_logger{logger}
  , m_entityModelManager{entityModelManager}
  , m_editorContext{editorContext}
{
}

EntityModelRenderer::~EntityModelRenderer()
{
  clear();
}

void EntityModelRenderer::addEntity(const mdl::EntityNode* entityNode)
{
  const auto modelSpec =
    asset::safeGetModelSpecification(m_logger, entityNode->entity().classname(), [&]() {
      return entityNode->entity().modelSpecification();
    });

  auto* renderer = m_entityModelManager.renderer(modelSpec);
  if (renderer != nullptr)
  {
    m_entities.emplace(entityNode, renderer);
  }
}

void EntityModelRenderer::removeEntity(const mdl::EntityNode* entityNode)
{
  m_entities.erase(entityNode);
}

void EntityModelRenderer::updateEntity(const mdl::EntityNode* entityNode)
{
  const auto modelSpec =
    asset::safeGetModelSpecification(m_logger, entityNode->entity().classname(), [&]() {
      return entityNode->entity().modelSpecification();
    });

  auto* renderer = m_entityModelManager.renderer(modelSpec);
  auto it = m_entities.find(entityNode);

  if (renderer == nullptr && it == std::end(m_entities))
  {
    return;
  }

  if (it == std::end(m_entities))
  {
    m_entities.emplace(entityNode, renderer);
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

void EntityModelRenderer::render(RenderBatch& renderBatch)
{
  renderBatch.add(this);
}

void EntityModelRenderer::doPrepareVertices(VboManager& vboManager)
{
  m_entityModelManager.prepare(vboManager);
}

void EntityModelRenderer::doRender(RenderContext& renderContext)
{
  if (!m_entities.empty())
  {
    auto& prefs = PreferenceManager::instance();

    glAssert(glEnable(GL_TEXTURE_2D));
    glAssert(glActiveTexture(GL_TEXTURE0));

    auto shader = ActiveShader{renderContext.shaderManager(), Shaders::EntityModelShader};
    shader.set("Brightness", prefs.get(Preferences::Brightness));
    shader.set("ApplyTinting", m_applyTinting);
    shader.set("TintColor", m_tintColor);
    shader.set("GrayScale", false);
    shader.set("Material", 0);
    shader.set("ShowSoftMapBounds", !renderContext.softMapBounds().is_empty());
    shader.set("SoftMapBoundsMin", renderContext.softMapBounds().min);
    shader.set("SoftMapBoundsMax", renderContext.softMapBounds().max);
    shader.set(
      "SoftMapBoundsColor",
      vm::vec4f{
        prefs.get(Preferences::SoftMapBoundsColor).r(),
        prefs.get(Preferences::SoftMapBoundsColor).g(),
        prefs.get(Preferences::SoftMapBoundsColor).b(),
        0.1f});

    shader.set("CameraPosition", renderContext.camera().position());
    shader.set("CameraDirection", renderContext.camera().direction());
    shader.set("CameraRight", renderContext.camera().right());
    shader.set("CameraUp", renderContext.camera().up());
    shader.set("ViewMatrix", renderContext.camera().viewMatrix());

    const auto& propertyConfig = m_entities.begin()->first->entityPropertyConfig();
    const auto& defaultModelScaleExpression = propertyConfig.defaultModelScaleExpression;

    for (const auto& [entityNode, renderer] : m_entities)
    {
      if (!m_showHiddenEntities && !m_editorContext.visible(entityNode))
      {
        continue;
      }

      const auto* model = entityNode->entity().model();
      const auto* modelData = model ? model->data() : nullptr;
      if (!modelData)
      {
        continue;
      }

      shader.set("Orientation", static_cast<int>(modelData->orientation()));

      const auto transformation = vm::mat4x4f{
        entityNode->entity().modelTransformation(defaultModelScaleExpression)};
      const auto multMatrix =
        MultiplyModelMatrix{renderContext.transformation(), transformation};

      shader.set("ModelMatrix", transformation);

      auto renderFunc = DefaultMaterialRenderFunc{
        renderContext.minFilterMode(), renderContext.magFilterMode()};
      renderer->render(renderFunc);
    }
  }
}

} // namespace tb::render
