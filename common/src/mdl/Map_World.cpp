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

#include "PreferenceManager.h"
#include "io/GameConfigParser.h"
#include "io/SystemPaths.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Game.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kd/contracts.h"

namespace tb::mdl
{
namespace
{

auto extractSoftMapBounds(const auto& entity, const auto& gameConfig)
{
  if (const auto* mapValue = entity.property(EntityPropertyKeys::SoftMapBounds))
  {
    return *mapValue == EntityPropertyValues::NoSoftMapBounds
             ? SoftMapBounds{SoftMapBoundsType::Map, std::nullopt}
             : SoftMapBounds{
                 SoftMapBoundsType::Map, io::parseSoftMapBoundsString(*mapValue)};
  }

  // Not set in map -> use Game value
  return SoftMapBounds{SoftMapBoundsType::Game, gameConfig.softMapBounds};
}

} // namespace

SoftMapBounds softMapBounds(const Map& map)
{
  return extractSoftMapBounds(map.worldNode().entity(), map.game()->config());
}

/**
 * Note if bounds.source is SoftMapBoundsType::Game, bounds.bounds is ignored.
 */
void setSoftMapBounds(Map& map, const SoftMapBounds& bounds)
{
  auto& worldNode = map.worldNode();
  auto entity = worldNode.entity();

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
    map, "Set Soft Map Bounds", {{&worldNode, NodeContents{std::move(entity)}}}, {});
}

std::vector<std::filesystem::path> externalSearchPaths(const Map& map)
{
  auto searchPaths = std::vector<std::filesystem::path>{};
  if (const auto& mapPath = map.path(); !mapPath.empty() && mapPath.is_absolute())
  {
    searchPaths.push_back(mapPath.parent_path());
  }

  if (const auto* game = map.game())
  {
    if (const auto gamePath = pref(game->info().gamePathPreference); !gamePath.empty())
    {
      searchPaths.push_back(gamePath);
    }
  }

  searchPaths.push_back(io::SystemPaths::appDirectory());
  return searchPaths;
}

std::vector<std::string> enabledMods(const Entity& entity)
{
  if (const auto* modStr = entity.property(EntityPropertyKeys::Mods))
  {
    return kdl::str_split(*modStr, ";");
  }

  return {};
}

std::vector<std::string> enabledMods(const Map& map)
{
  return enabledMods(map.worldNode().entity());
}

void setEnabledMods(Map& map, const std::vector<std::string>& mods)
{
  auto& worldNode = map.worldNode();
  auto entity = worldNode.entity();

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
    map, "Set Enabled Mods", {{&worldNode, NodeContents(std::move(entity))}}, {});
}

std::string defaultMod(const Map& map)
{
  contract_pre(map.game());

  return map.game()->config().fileSystemConfig.searchPath.string();
}

} // namespace tb::mdl
