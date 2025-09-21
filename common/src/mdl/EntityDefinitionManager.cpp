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

#include "EntityDefinitionManager.h"

#include "Ensure.h"
#include "io/EntityDefinitionLoader.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityNodeBase.h"

#include "kdl/result.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace tb::mdl
{

EntityDefinitionManager::~EntityDefinitionManager()
{
  clear();
}

Result<void> EntityDefinitionManager::loadDefinitions(
  const std::filesystem::path& path,
  const io::EntityDefinitionLoader& loader,
  io::ParserStatus& status)
{
  return loader.loadEntityDefinitions(status, path)
         | kdl::transform(
           [&](auto entityDefinitions) { setDefinitions(std::move(entityDefinitions)); });
}

void EntityDefinitionManager::setDefinitions(std::vector<EntityDefinition> newDefinitions)
{
  clear();

  m_definitions = std::move(newDefinitions);

  updateIndices();
  updateGroups();
}

void EntityDefinitionManager::clear()
{
  clearGroups();
}

const EntityDefinition* EntityDefinitionManager::definition(
  const EntityNodeBase* node) const
{
  ensure(node != nullptr, "node is null");
  return definition(node->entity().classname());
}

const EntityDefinition* EntityDefinitionManager::definition(
  const std::string_view classname) const
{
  if (const auto it = std::ranges::find_if(
        m_definitions,
        [&](const auto& definition) { return definition.name == classname; });
      it != m_definitions.end())
  {
    return &*it;
  }

  return nullptr;
}

std::vector<const EntityDefinition*> EntityDefinitionManager::definitions(
  const EntityDefinitionType type, const EntityDefinitionSortOrder order) const
{
  return filterAndSort(
    definitions() | std::views::transform([](const auto& d) { return &d; }), type, order);
}

const std::vector<EntityDefinition>& EntityDefinitionManager::definitions() const
{
  return m_definitions;
}

const std::vector<EntityDefinitionGroup>& EntityDefinitionManager::groups() const
{
  return m_groups;
}

void EntityDefinitionManager::updateIndices()
{
  for (size_t i = 0; i < m_definitions.size(); ++i)
  {
    m_definitions[i].index = i + 1;
  }
}

void EntityDefinitionManager::updateGroups()
{
  clearGroups();

  auto groupMap =
    std::unordered_map<std::string_view, std::vector<const EntityDefinition*>>{};

  for (const auto& definition : m_definitions)
  {
    auto groupName = getGroupName(definition);
    groupMap[std::move(groupName)].push_back(&definition);
  }

  for (auto& [groupName, definitions] : groupMap)
  {
    m_groups.push_back(
      EntityDefinitionGroup{std::string{groupName}, std::move(definitions)});
  }
}

void EntityDefinitionManager::clearGroups()
{
  m_groups.clear();
}

} // namespace tb::mdl
