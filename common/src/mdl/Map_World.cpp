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

#include "mdl/Map_World.h"

#include "io/GameConfigParser.h"
#include "io/SystemPaths.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Game.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

SoftMapBounds softMapBounds(const Map& map)
{
  if (const auto* worldNode = map.world())
  {
    return map.game()->extractSoftMapBounds(worldNode->entity());
  }
  return {SoftMapBoundsType::Game, std::nullopt};
}

/**
 * Note if bounds.source is SoftMapBoundsType::Game, bounds.bounds is ignored.
 */
void setSoftMapBounds(Map& map, const SoftMapBounds& bounds)
{
  auto* worldNode = map.world();
  ensure(worldNode, "world is set");

  auto entity = worldNode->entity();
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
    map, "Set Soft Map Bounds", {{worldNode, NodeContents(std::move(entity))}}, {});
}

std::vector<std::filesystem::path> externalSearchPaths(const Map& map)
{
  auto searchPaths = std::vector<std::filesystem::path>{};
  if (const auto& mapPath = map.path(); !mapPath.empty() && mapPath.is_absolute())
  {
    searchPaths.push_back(mapPath.parent_path());
  }

  if (const auto gamePath = map.game()->gamePath(); !gamePath.empty())
  {
    searchPaths.push_back(gamePath);
  }

  searchPaths.push_back(io::SystemPaths::appDirectory());
  return searchPaths;
}

std::vector<std::string> mods(const Map& map)
{
  return map.game()->extractEnabledMods(map.world()->entity());
}

void setMods(Map& map, const std::vector<std::string>& mods)
{
  auto* worldNode = map.world();
  auto entity = worldNode->entity();
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
    map, "Set Enabled Mods", {{worldNode, NodeContents(std::move(entity))}}, {});
}

std::string defaultMod(const Map& map)
{
  return map.game()->defaultMod();
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
