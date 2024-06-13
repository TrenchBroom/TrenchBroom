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

#pragma once

#include "Assets/EntityModel.h"
#include "Result.h"

#include "kdl/vector_set.h"

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom
{
class Logger;

namespace Model
{
class EntityNode;
class Game;
} // namespace Model

namespace Renderer
{
class MaterialRenderer;
class VboManager;
} // namespace Renderer

namespace Assets
{
class EntityModelFrame;
struct ModelSpecification;
enum class Orientation;

class EntityModelManager
{
private:
  using ModelCache = std::map<std::filesystem::path, EntityModel>;
  using ModelMismatches = kdl::vector_set<std::filesystem::path>;
  using ModelList = std::vector<EntityModel*>;

  using RendererCache =
    std::map<ModelSpecification, std::unique_ptr<Renderer::MaterialRenderer>>;
  using RendererMismatches = kdl::vector_set<ModelSpecification>;
  using RendererList = std::vector<Renderer::MaterialRenderer*>;

  Logger& m_logger;

  int m_minFilter;
  int m_magFilter;
  bool m_resetFilterMode = false;
  const Model::Game* m_game = nullptr;

  mutable ModelCache m_models;
  mutable ModelMismatches m_modelMismatches;
  mutable RendererCache m_renderers;
  mutable RendererMismatches m_rendererMismatches;

  mutable ModelList m_unpreparedModels;
  mutable RendererList m_unpreparedRenderers;

public:
  EntityModelManager(int magFilter, int minFilter, Logger& logger);
  ~EntityModelManager();

  void clear();

  void setFilterMode(int minFilter, int magFilter);
  void setGame(const Model::Game* game);

  Renderer::MaterialRenderer* renderer(const ModelSpecification& spec) const;

  const EntityModelFrame* frame(const ModelSpecification& spec) const;

private:
  EntityModel* model(const std::filesystem::path& path) const;
  EntityModel* safeGetModel(const std::filesystem::path& path) const;
  Result<EntityModel> loadModel(const std::filesystem::path& path) const;

public:
  void prepare(Renderer::VboManager& vboManager);

private:
  void resetFilterMode();
  void prepareModels();
  void prepareRenderers(Renderer::VboManager& vboManager);
};
} // namespace Assets
} // namespace TrenchBroom
