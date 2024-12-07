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

#pragma once

#include "Result.h"
#include "mdl/EntityModel.h"
#include "mdl/ModelSpecification.h"

#include "kdl/path_hash.h"

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
class Logger;
}

namespace tb::render
{
class MaterialRenderer;
class VboManager;
} // namespace tb::render

namespace tb::mdl
{
class EntityModelFrame;
class EntityNode;
class Game;
enum class Orientation;
class Quake3Shader;

class EntityModelManager
{
private:
  CreateEntityModelDataResource m_createResource;
  Logger& m_logger;

  const mdl::Game* m_game = nullptr;

  // Cache Quake 3 shaders to use when loading models
  std::vector<Quake3Shader> m_shaders;

  mutable std::unordered_map<std::filesystem::path, EntityModel, kdl::path_hash> m_models;
  mutable std::
    unordered_map<ModelSpecification, std::unique_ptr<render::MaterialRenderer>>
      m_renderers;
  mutable std::unordered_set<ModelSpecification> m_rendererMismatches;

  mutable std::vector<render::MaterialRenderer*> m_unpreparedRenderers;

public:
  EntityModelManager(CreateEntityModelDataResource createResource, Logger& logger);
  ~EntityModelManager();

  void clear();
  void reloadShaders(kdl::task_manager& taskManager);

  void setGame(const mdl::Game* game, kdl::task_manager& taskManager);

  render::MaterialRenderer* renderer(const ModelSpecification& spec) const;

  const EntityModelFrame* frame(const ModelSpecification& spec) const;
  const EntityModel* model(const std::filesystem::path& path) const;

  const std::vector<const EntityModel*> findEntityModelsByTextureResourceId(
    const std::vector<ResourceId>& resourceIds) const;

private:
  const EntityModel* safeGetModel(const std::filesystem::path& path) const;
  Result<EntityModel> loadModel(const std::filesystem::path& path) const;

public:
  void prepare(render::VboManager& vboManager);

private:
  void prepareRenderers(render::VboManager& vboManager);
};

} // namespace tb::mdl
