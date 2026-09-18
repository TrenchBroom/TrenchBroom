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

#include "el/Value.h"

#include <string>
#include <vector>

namespace tb::mdl
{
class Map;
class WorldNode;
} // namespace tb::mdl

namespace tb::ql
{

/**
 * The search/filter query language's Binding for a WorldNode -- the same shape as an
 * EntityNode's (`type`/`classname`/`properties`/`tags`), minus the fields that don't
 * apply to the map's single, implicit root: `bounds`/`center` (there is no sensible
 * bounding box for the whole map), `layerName`/`groupName` (a WorldNode has neither),
 * and `linked` (it can't belong to a linked group).
 */
el::BoundValue makeWorldNodeBinding(const mdl::Map& map, const mdl::WorldNode& node);

/**
 * The field names makeWorldNodeBinding's result exposes, without needing an actual
 * node instance.
 */
std::vector<std::string> worldNodeFieldNames();

} // namespace tb::ql
