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
#include "Assets/Material.h"
#include "Assets/Palette.h"
#include "Assets/Quake3Shader.h"
#include "Assets/Resource.h"
#include "Error.h"
#include "Exceptions.h"
#include "IO/LoadEntityModel.h"
#include "IO/LoadMaterialCollections.h"
#include "IO/LoadShaders.h"
#include "IO/MaterialUtils.h"
#include "Logger.h"
#include "Macros.h"
#include "Model/Game.h"
#include "Renderer/MaterialIndexRangeRenderer.h"

#include "kdl/result.h"

namespace TrenchBroom::Assets
{
EntityModelManager::EntityModelManager(
  const int magFilter, const int minFilter, Logger& logger)
  : m_logger{logger}
  , m_minFilter{minFilter}
  , m_magFilter{magFilter}
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

  m_unpreparedModels.clear();
  m_unpreparedRenderers.clear();

  // Remove logging because it might fail when the document is already destroyed.
}

void EntityModelManager::reloadShaders()
{
  m_shaders.clear();

  if (m_game)
  {
    m_shaders =
      IO::loadShaders(m_game->gameFileSystem(), m_game->config().materialConfig, m_logger)
      | kdl::if_error(
        [&](const auto& e) { m_logger.error() << "Failed to reload shaders: " << e.msg; })
      | kdl::value_or(std::vector<Quake3Shader>{});
  }
}


void EntityModelManager::setFilterMode(const int minFilter, const int magFilter)
{
  m_minFilter = minFilter;
  m_magFilter = magFilter;
  m_resetFilterMode = true;
}

void EntityModelManager::setGame(const Model::Game* game)
{
  clear();
  m_game = game;
  reloadShaders();
}

Renderer::MaterialRenderer* EntityModelManager::renderer(
  const Assets::ModelSpecification& spec) const
{
  if (auto* entityModel = safeGetModel(spec.path))
  {
    auto it = m_renderers.find(spec);
    if (it != std::end(m_renderers))
    {
      return it->second.get();
    }

    if (!m_rendererMismatches.contains(spec))
    {
      const auto& entityModelData = entityModel->data();
      if (auto renderer = entityModelData.buildRenderer(spec.skinIndex, spec.frameIndex))
      {
        const auto [pos, success] = m_renderers.emplace(spec, std::move(renderer));
        assert(success);
        unused(success);

        auto* result = pos->second.get();
        m_unpreparedRenderers.push_back(result);
        m_logger.debug() << "Constructed entity model renderer for " << spec;
        return result;
      }

      m_rendererMismatches.insert(spec);
      m_logger.error() << "Failed to construct entity model renderer for " << spec
                       << ", check the skin and frame indices";
    }
  }

  return nullptr;
}

const EntityModelFrame* EntityModelManager::frame(
  const Assets::ModelSpecification& spec) const
{
  if (auto* model = this->safeGetModel(spec.path))
  {
    const auto& entityModelData = model->data();
    return entityModelData.frame(spec.frameIndex);
  }

  return nullptr;
}

const EntityModel* EntityModelManager::model(const std::filesystem::path& path) const
{
  if (!path.empty())
  {
    auto it = m_models.find(path);
    if (it != std::end(m_models))
    {
      return &it->second;
    }

    return loadModel(path) | kdl::transform([&](auto model) {
             const auto [pos, success] = m_models.emplace(path, std::move(model));
             assert(success);
             unused(success);

             auto* modelPtr = &(pos->second);
             m_unpreparedModels.push_back(modelPtr);
             m_logger.debug() << "Loaded entity model " << path;

             return modelPtr;
           })
           | kdl::if_error([&](auto e) {
               m_logger.error() << e.msg;
               throw GameException{e.msg};
             })
           | kdl::value();
  }

  return nullptr;
}

const EntityModel* EntityModelManager::safeGetModel(
  const std::filesystem::path& path) const
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

Result<EntityModel> EntityModelManager::loadModel(
  const std::filesystem::path& modelPath) const
{
  if (m_game)
  {
    const auto& fs = m_game->gameFileSystem();
    const auto& materialConfig = m_game->config().materialConfig;

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return IO::loadMaterial(
               fs, materialConfig, materialPath, createResource, m_shaders, std::nullopt)
             | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, m_logger))
             | kdl::value();
    };

    return IO::loadEntityModel(fs, materialConfig, modelPath, loadMaterial, m_logger);
  }
  return Error{"Game is not set"};
}

void EntityModelManager::prepare(Renderer::VboManager& vboManager)
{
  resetFilterMode();
  prepareModels();
  prepareRenderers(vboManager);
}

void EntityModelManager::resetFilterMode()
{
  if (m_resetFilterMode)
  {
    for (auto& [path, model] : m_models)
    {
      auto& entityModelData = model.data();
      entityModelData.setFilterMode(m_minFilter, m_magFilter);
    }
    m_resetFilterMode = false;
  }
}

void EntityModelManager::prepareModels()
{
  for (auto* model : m_unpreparedModels)
  {
    auto& entityModelData = model->data();
    entityModelData.prepare(m_minFilter, m_magFilter);
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
} // namespace TrenchBroom::Assets
