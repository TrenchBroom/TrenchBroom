/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Types.h"

#include <string>
#include <vector>

namespace tb
{
namespace mdl
{
class Map;
class WorldNode;
} // namespace mdl

namespace ql
{

/**
 * A lazy map exposing the world node's data.
 */
el::LazyMap makeWorldNodeLazyMap(const mdl::Map& map, const mdl::WorldNode& node);

/**
 * The field names makeWorldNodeLazyMap's result exposes.
 */
std::vector<std::string> worldNodeFieldNames();

} // namespace ql
} // namespace tb
