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

#include "EntityModelManager.h"

#include "Exceptions.h"
#include "Logger.h"
#include "Macros.h"
#include "io/LoadEntityModel.h"
#include "io/LoadMaterialCollections.h"
#include "io/LoadShaders.h"
#include "io/MaterialUtils.h"
#include "mdl/EntityModel.h"
#include "mdl/Game.h"
#include "mdl/Quake3Shader.h"
#include "mdl/Resource.h"
#include "render/MaterialIndexRangeRenderer.h"

#include "kdl/range_to_vector.h"
#include "kdl/result.h"

namespace tb::mdl
{
EntityModelManager::EntityModelManager(
  CreateEntityModelDataResource createResource, Logger& logger)
  : m_createResource{std::move(createResource)}
  , m_logger{logger}
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

  m_unpreparedRenderers.clear();

  // Remove logging because it might fail when the document is already destroyed.
}

void EntityModelManager::reloadShaders(kdl::task_manager& taskManager)
{
  m_shaders.clear();

  if (m_game)
  {
    m_shaders =
      io::loadShaders(
        m_game->gameFileSystem(), m_game->config().materialConfig, taskManager, m_logger)
      | kdl::if_error(
        [&](const auto& e) { m_logger.error() << "Failed to reload shaders: " << e.msg; })
      | kdl::value_or(std::vector<Quake3Shader>{});
  }
}

void EntityModelManager::setGame(const mdl::Game* game, kdl::task_manager& taskManager)
{
  clear();
  m_game = game;
  reloadShaders(taskManager);
}

render::MaterialRenderer* EntityModelManager::renderer(
  const ModelSpecification& spec) const
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
      if (const auto* entityModelData = entityModel->data())
      {
        if (
          auto renderer = entityModelData->buildRenderer(spec.skinIndex, spec.frameIndex))
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
  }

  return nullptr;
}

const EntityModelFrame* EntityModelManager::frame(const ModelSpecification& spec) const
{
  if (auto* model = this->safeGetModel(spec.path))
  {
    if (const auto* entityModelData = model->data())
    {
      return entityModelData->frame(spec.frameIndex);
    }
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

             m_logger.debug() << "Loading entity model " << path;
             return &(pos->second);
           })
           | kdl::if_error([&](auto e) {
               m_logger.error() << e.msg;
               throw GameException{e.msg};
             })
           | kdl::value();
  }

  return nullptr;
}

const std::vector<const EntityModel*> EntityModelManager::
  findEntityModelsByTextureResourceId(const std::vector<ResourceId>& resourceIds) const
{
  using namespace std::ranges;

  const auto filterByResourceId =
    [resourceIdSet = std::unordered_set<ResourceId>{
       resourceIds.begin(), resourceIds.end()}](const auto& model) {
      return resourceIdSet.contains(model.dataResource().id());
    };

  const auto toPointer = [](const auto& model) { return &model; };

  return m_models | views::values | views::filter(filterByResourceId)
         | views::transform(toPointer) | kdl::to_vector;
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
      return io::loadMaterial(
               fs, materialConfig, materialPath, createResource, m_shaders, std::nullopt)
             | kdl::or_else(io::makeReadMaterialErrorHandler(fs, m_logger))
             | kdl::value();
    };

    return io::loadEntityModelAsync(
      fs, materialConfig, modelPath, loadMaterial, m_createResource, m_logger);
  }
  return Error{"Game is not set"};
}

void EntityModelManager::prepare(render::VboManager& vboManager)
{
  prepareRenderers(vboManager);
}

void EntityModelManager::prepareRenderers(render::VboManager& vboManager)
{
  for (auto* renderer : m_unpreparedRenderers)
  {
    renderer->prepare(vboManager);
  }
  m_unpreparedRenderers.clear();
}
} // namespace tb::mdl
