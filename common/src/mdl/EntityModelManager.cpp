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

#include "Logger.h"
#include "gl/CreateResource.h"
#include "gl/MaterialIndexRangeRenderer.h"
#include "mdl/EntityModel.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/LoadEntityModel.h"
#include "mdl/LoadMaterialCollections.h"
#include "mdl/LoadShaders.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Quake3Shader.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/result.h"

namespace tb::mdl
{
EntityModelManager::EntityModelManager(
  const GameInfo& gameInfo,
  const fs::FileSystem& gameFileSystem,
  CreateEntityModelDataResource createResource,
  Logger& logger)
  : m_gameInfo{gameInfo}
  , m_gameFileSystem{gameFileSystem}
  , m_createResource{std::move(createResource)}
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

  m_shaders =
    loadShaders(
      m_gameFileSystem, m_gameInfo.gameConfig.materialConfig, taskManager, m_logger)
    | kdl::if_error(
      [&](const auto& e) { m_logger.error() << "Failed to reload shaders: " << e.msg; })
    | kdl::value_or(std::vector<Quake3Shader>{});
}

gl::MaterialRenderer* EntityModelManager::renderer(const ModelSpecification& spec) const
{
  if (auto* entityModel = model(spec.path))
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
          contract_assert(success);

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
  if (auto* entityModel = model(spec.path))
  {
    if (const auto* entityModelData = entityModel->data())
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
             contract_assert(success);

             m_logger.debug() << "Loading entity model " << path;
             return &(pos->second);
           })
           | kdl::transform_error([&](auto e) {
               m_logger.error() << e.msg;
               return nullptr;
             })
           | kdl::value();
  }

  return nullptr;
}

const std::vector<const EntityModel*> EntityModelManager::
  findEntityModelsByTextureResourceId(
    const std::vector<gl::ResourceId>& resourceIds) const
{
  using namespace std::ranges;

  const auto filterByResourceId =
    [resourceIdSet = std::unordered_set<gl::ResourceId>{
       resourceIds.begin(), resourceIds.end()}](const auto& model) {
      return resourceIdSet.contains(model.dataResource().id());
    };

  const auto toPointer = [](const auto& model) { return &model; };

  return m_models | views::values | views::filter(filterByResourceId)
         | views::transform(toPointer) | kdl::ranges::to<std::vector>();
}

Result<EntityModel> EntityModelManager::loadModel(
  const std::filesystem::path& modelPath) const
{
  const auto& materialConfig = m_gameInfo.gameConfig.materialConfig;

  const auto createResource = [](auto resourceLoader) {
    return gl::createResourceSync(std::move(resourceLoader));
  };

  const auto loadMaterial = [&](const auto& materialPath) {
    return mdl::loadMaterial(
             m_gameFileSystem,
             materialConfig,
             materialPath,
             createResource,
             m_shaders,
             std::nullopt)
           | kdl::or_else(makeReadMaterialErrorHandler(m_gameFileSystem, m_logger))
           | kdl::value();
  };

  return mdl::loadEntityModelAsync(
    m_gameFileSystem,
    materialConfig,
    modelPath,
    loadMaterial,
    m_createResource,
    m_logger);
}

void EntityModelManager::prepare(gl::VboManager& vboManager)
{
  prepareRenderers(vboManager);
}

void EntityModelManager::prepareRenderers(gl::VboManager& vboManager)
{
  for (auto* renderer : m_unpreparedRenderers)
  {
    renderer->prepare(vboManager);
  }
  m_unpreparedRenderers.clear();
}
} // namespace tb::mdl
