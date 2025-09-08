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

#include "mdl/SoftMapBounds.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb::mdl
{
class Entity;
class Map;

SoftMapBounds softMapBounds(const Map& map);
void setSoftMapBounds(Map& map, const SoftMapBounds& bounds);

std::vector<std::filesystem::path> externalSearchPaths(const Map& map);

std::vector<std::string> enabledMods(const Entity& entity);
std::vector<std::string> enabledMods(const Map& map);
void setEnabledMods(Map& map, const std::vector<std::string>& mods);
std::string defaultMod(const Map& map);

} // namespace tb::mdl
