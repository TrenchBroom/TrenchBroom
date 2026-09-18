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

namespace tb::mdl
{
class Map;
class Node;
} // namespace tb::mdl

namespace tb::ql
{

/**
 * Value-producing helpers for the `layerName`/`groupName`/`linked` fields, which apply
 * identically to every node kind's BoundValue (WorldNodeBoundValue, LayerNodeBoundValue,
 * ...).
 */

el::Value layerNameValue(const mdl::Node& node);
el::Value groupNameValue(const mdl::Node& node);
bool isLinked(const mdl::Map& map, const mdl::Node& node);

} // namespace tb::ql
