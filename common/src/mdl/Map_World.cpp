/*
 Copyright (C) 2025 Kristian Duske

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

#include "Map.h"
#include "io/GameConfigParser.h"
#include "io/SystemPaths.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Game.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

SoftMapBounds Map::softMapBounds() const
{
  if (!m_world)
  {
    return {SoftMapBoundsType::Game, std::nullopt};
  }
  return m_game->extractSoftMapBounds(m_world->entity());
}

/**
 * Note if bounds.source is SoftMapBoundsType::Game, bounds.bounds is ignored.
 */
void Map::setSoftMapBounds(const SoftMapBounds& bounds)
{
  auto entity = world()->entity();
  switch (bounds.source)
  {
  case SoftMapBoundsType::Map:
    if (!bounds.bounds.has_value())
    {
      // Set the worldspawn key EntityPropertyKeys::SoftMaxMapSize's value to the empty
      // string to indicate that we are overriding the Game's bounds with unlimited.
      entity.addOrUpdateProperty(
        EntityPropertyKeys::SoftMapBounds, EntityPropertyValues::NoSoftMapBounds);
    }
    else
    {
      entity.addOrUpdateProperty(
        EntityPropertyKeys::SoftMapBounds,
        io::serializeSoftMapBoundsString(*bounds.bounds));
    }
    break;
  case SoftMapBoundsType::Game:
    // Unset the map's setting
    entity.removeProperty(EntityPropertyKeys::SoftMapBounds);
    break;
    switchDefault();
  }
  updateNodeContents(
    "Set Soft Map Bounds", {{world(), NodeContents(std::move(entity))}}, {});
}

std::vector<std::filesystem::path> Map::externalSearchPaths() const
{
  std::vector<std::filesystem::path> searchPaths;
  if (!m_path.empty() && m_path.is_absolute())
  {
    searchPaths.push_back(m_path.parent_path());
  }

  const std::filesystem::path gamePath = m_game->gamePath();
  if (!gamePath.empty())
  {
    searchPaths.push_back(gamePath);
  }

  searchPaths.push_back(io::SystemPaths::appDirectory());
  return searchPaths;
}

void Map::updateGameSearchPaths()
{
  m_game->setAdditionalSearchPaths(
    kdl::vec_transform(
      mods(), [](const auto& mod) { return std::filesystem::path{mod}; }),
    m_logger);
}

std::vector<std::string> Map::mods() const
{
  return m_game->extractEnabledMods(m_world->entity());
}

void Map::setMods(const std::vector<std::string>& mods)
{
  auto entity = m_world->entity();
  if (mods.empty())
  {
    entity.removeProperty(EntityPropertyKeys::Mods);
  }
  else
  {
    const auto newValue = kdl::str_join(mods, ";");
    entity.addOrUpdateProperty(EntityPropertyKeys::Mods, newValue);
  }
  updateNodeContents(
    "Set Enabled Mods", {{world(), NodeContents(std::move(entity))}}, {});
}

std::string Map::defaultMod() const
{
  return m_game->defaultMod();
}

void Map::setWorld(
  const vm::bbox3d& worldBounds,
  std::unique_ptr<WorldNode> worldNode,
  std::unique_ptr<Game> game,
  const std::filesystem::path& path)
{
  m_worldBounds = worldBounds;
  m_world = std::move(worldNode);
  m_game = std::move(game);

  entityModelManager().setGame(m_game.get(), taskManager());
  editorContext().setCurrentLayer(world()->defaultLayer());

  updateGameSearchPaths();
  setPath(path);

  loadAssets();
  registerValidators();
  registerSmartTags();
}

void Map::clearWorld()
{
  m_world.reset();
  editorContext().reset();
}

} // namespace tb::mdl
