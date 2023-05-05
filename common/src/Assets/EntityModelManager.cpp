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

#include "EntityModelManager.h"

#include "Assets/EntityModel.h"
#include "Assets/ModelDefinition.h"
#include "Exceptions.h"
#include "IO/EntityModelLoader.h"
#include "Logger.h"
#include "Macros.h"
#include "Model/EntityNode.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

namespace TrenchBroom
{
namespace Assets
{
EntityModelManager::EntityModelManager(
  const int magFilter, const int minFilter, Logger& logger)
  : m_logger(logger)
  , m_loader(nullptr)
  , m_minFilter(minFilter)
  , m_magFilter(magFilter)
  , m_resetTextureMode(false)
{
}

EntityModelManager::~EntityModelManager()
{
  clear();
}

void EntityModelManager::clear()
{
  m_renderers.clear();
  m_models.clear();
  m_rendererMismatches.clear();
  m_modelMismatches.clear();

  m_unpreparedModels.clear();
  m_unpreparedRenderers.clear();

  // Remove logging because it might fail when the document is already destroyed.
}

void EntityModelManager::setTextureMode(const int minFilter, const int magFilter)
{
  m_minFilter = minFilter;
  m_magFilter = magFilter;
  m_resetTextureMode = true;
}

void EntityModelManager::setLoader(const IO::EntityModelLoader* loader)
{
  clear();
  m_loader = loader;
}

Renderer::TexturedRenderer* EntityModelManager::renderer(
  const Assets::ModelSpecification& spec) const
{
  auto* entityModel = safeGetModel(spec.path);

  if (entityModel == nullptr)
  {
    return nullptr;
  }

  auto it = m_renderers.find(spec);
  if (it != std::end(m_renderers))
  {
    return it->second.get();
  }

  if (m_rendererMismatches.count(spec) > 0)
  {
    return nullptr;
  }

  auto renderer = entityModel->buildRenderer(spec.skinIndex, spec.frameIndex);
  if (renderer != nullptr)
  {
    const auto [pos, success] = m_renderers.emplace(spec, std::move(renderer));
    assert(success);
    unused(success);

    auto* result = pos->second.get();
    m_unpreparedRenderers.push_back(result);
    m_logger.debug() << "Constructed entity model renderer for " << spec;
    return result;
  }
  else
  {
    m_rendererMismatches.insert(spec);
    m_logger.error() << "Failed to construct entity model renderer for " << spec
                     << ", check the skin and frame indices";
    return nullptr;
  }
}

const EntityModelFrame* EntityModelManager::frame(
  const Assets::ModelSpecification& spec) const
{
  auto* model = this->safeGetModel(spec.path);
  if (model == nullptr)
  {
    return nullptr;
  }
  else if (spec.frameIndex >= model->frameCount())
  {
    return nullptr;
  }
  else
  {
    if (!model->frame(spec.frameIndex)->loaded())
    {
      loadFrame(spec, *model);
    }
    return model->frame(spec.frameIndex);
  }
}

EntityModel* EntityModelManager::model(const std::filesystem::path& path) const
{
  if (path.empty())
  {
    return nullptr;
  }

  auto it = m_models.find(path);
  if (it != std::end(m_models))
  {
    return it->second.get();
  }

  if (m_modelMismatches.count(path) > 0)
  {
    return nullptr;
  }

  try
  {
    const auto [pos, success] = m_models.emplace(path, loadModel(path));
    assert(success);
    unused(success);

    auto* model = pos->second.get();
    m_unpreparedModels.push_back(model);

    m_logger.debug() << "Loaded entity model " << path;

    return model;
  }
  catch (const GameException& e)
  {
    m_logger.error() << e.what();
    m_modelMismatches.insert(path);
    throw;
  }
}

EntityModel* EntityModelManager::safeGetModel(const std::filesystem::path& path) const
{
  try
  {
    return model(path);
  }
  catch (const GameException&)
  {
    return nullptr;
  }
}

std::unique_ptr<EntityModel> EntityModelManager::loadModel(
  const std::filesystem::path& path) const
{
  ensure(m_loader != nullptr, "loader is null");
  return m_loader->initializeModel(path, m_logger);
}

void EntityModelManager::loadFrame(
  const Assets::ModelSpecification& spec, Assets::EntityModel& model) const
{
  try
  {
    ensure(m_loader != nullptr, "loader is null");
    m_loader->loadFrame(spec.path, spec.frameIndex, model, m_logger);
  }
  catch (const Exception& e)
  {
    // FIXME: be specific about which exceptions to catch here
    m_logger.error() << "Could not load entity model frame " << spec << ": " << e.what();
  }
}

void EntityModelManager::prepare(Renderer::VboManager& vboManager)
{
  resetTextureMode();
  prepareModels();
  prepareRenderers(vboManager);
}

void EntityModelManager::resetTextureMode()
{
  if (m_resetTextureMode)
  {
    for (const auto& [path, model] : m_models)
    {
      model->setTextureMode(m_minFilter, m_magFilter);
    }
    m_resetTextureMode = false;
  }
}

void EntityModelManager::prepareModels()
{
  for (auto* model : m_unpreparedModels)
  {
    model->prepare(m_minFilter, m_magFilter);
  }
  m_unpreparedModels.clear();
}

void EntityModelManager::prepareRenderers(Renderer::VboManager& vboManager)
{
  for (auto* renderer : m_unpreparedRenderers)
  {
    renderer->prepare(vboManager);
  }
  m_unpreparedRenderers.clear();
}
} // namespace Assets
} // namespace TrenchBroom
