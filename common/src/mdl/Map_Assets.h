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

#pragma once

#include "mdl/EntityDefinitionFileSpec.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace tb::mdl
{
class Entity;
class Map;

std::optional<EntityDefinitionFileSpec> entityDefinitionFile(const Entity& entity);
std::optional<EntityDefinitionFileSpec> entityDefinitionFile(const Map& map);
void setEntityDefinitionFile(Map& map, const EntityDefinitionFileSpec& spec);

std::vector<std::filesystem::path> enabledMaterialCollections(const Map& map);
std::vector<std::filesystem::path> disabledMaterialCollections(const Map& map);

void setEnabledMaterialCollections(
  Map& map, const std::vector<std::filesystem::path>& enabledMaterialCollections);

void reloadMaterialCollections(Map& map);
void reloadEntityDefinitions(Map& map);

} // namespace tb::mdl
