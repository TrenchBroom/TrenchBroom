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

#include "mdl/Map_Assets.h"

#include "Logger.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Game.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Map_World.h"
#include "mdl/MaterialManager.h"
#include "mdl/PushSelection.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kdl/ranges/to.h"
#include "kdl/string_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <ranges>
#include <vector>

namespace tb::mdl
{

EntityDefinitionFileSpec entityDefinitionFile(const Map& map)
{
  const auto* worldNode = map.world();
  return worldNode ? map.game()->extractEntityDefinitionFile(worldNode->entity())
                   : EntityDefinitionFileSpec{};
}

void setEntityDefinitionFile(Map& map, const EntityDefinitionFileSpec& spec)
{
  // to avoid backslashes being misinterpreted as escape sequences
  const auto formatted = kdl::str_replace_every(spec.asString(), "\\", "/");

  auto entity = map.world()->entity();
  entity.addOrUpdateProperty(EntityPropertyKeys::EntityDefinitions, formatted);
  updateNodeContents(
    map, "Set Entity Definitions", {{map.world(), NodeContents{std::move(entity)}}}, {});
}

std::vector<std::filesystem::path> enabledMaterialCollections(const Map& map)
{
  if (const auto* worldNode = map.world())
  {
    if (
      const auto* materialCollectionStr =
        worldNode->entity().property(EntityPropertyKeys::EnabledMaterialCollections))
    {
      const auto strs = kdl::str_split(*materialCollectionStr, ";");
      return kdl::vec_sort_and_remove_duplicates(
        strs | std::views::transform([](const auto& str) {
          return std::filesystem::path{str};
        })
        | kdl::ranges::to<std::vector>());
    }

    // Otherwise, enable all material collections
    return kdl::vec_sort_and_remove_duplicates(
      map.materialManager().collections()
      | std::views::transform([](const auto& collection) { return collection.path(); })
      | kdl::ranges::to<std::vector>());
  }
  return {};
}

std::vector<std::filesystem::path> disabledMaterialCollections(const Map& map)
{
  if (map.world())
  {
    auto materialCollections = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
      map.materialManager().collections(),
      [](const auto& collection) { return collection.path(); }));

    return kdl::set_difference(materialCollections, enabledMaterialCollections(map));
  }
  return {};
}

void setEnabledMaterialCollections(
  Map& map, const std::vector<std::filesystem::path>& enabledMaterialCollections)
{
  const auto enabledMaterialCollectionStr = kdl::str_join(
    kdl::vec_transform(
      kdl::vec_sort_and_remove_duplicates(enabledMaterialCollections),
      [](const auto& path) { return path.string(); }),
    ";");

  auto transaction = Transaction{map, "Set enabled material collections"};

  const auto pushSelection = PushSelection{map};
  deselectAll(map);

  const auto success = setEntityProperty(
    map, EntityPropertyKeys::EnabledMaterialCollections, enabledMaterialCollectionStr);
  transaction.finish(success);
}

void reloadMaterialCollections(Map& map)
{
  const auto nodes = std::vector<Node*>{map.world()};
  const auto notifyNodes =
    NotifyBeforeAndAfter{map.nodesWillChangeNotifier, map.nodesDidChangeNotifier, nodes};
  const auto notifyMaterialCollections = NotifyBeforeAndAfter{
    map.materialCollectionsWillChangeNotifier, map.materialCollectionsDidChangeNotifier};

  map.logger().info() << "Reloading material collections";
  // materialCollectionsDidChange will load the collections again
}

void reloadEntityDefinitions(Map& map)
{
  const auto nodes = std::vector<Node*>{map.world()};
  const auto notifyNodes =
    NotifyBeforeAndAfter{map.nodesWillChangeNotifier, map.nodesDidChangeNotifier, nodes};
  const auto notifyEntityDefinitions = NotifyBeforeAndAfter{
    map.entityDefinitionsWillChangeNotifier, map.entityDefinitionsDidChangeNotifier};

  map.logger().info() << "Reloading entity definitions";
}

} // namespace tb::mdl
