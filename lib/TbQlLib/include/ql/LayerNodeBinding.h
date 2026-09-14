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
class LayerNode;
class Map;
} // namespace tb::mdl

namespace tb::ql
{

/**
 * The search/filter query language's Binding for a LayerNode -- the fields common to
 * every node kind, plus `name` (the layer's own stored name).
 */
el::BoundValue makeLayerNodeBinding(const mdl::Map& map, const mdl::LayerNode& node);

/**
 * The field names makeLayerNodeBinding's result exposes, without needing an actual
 * node instance.
 */
std::vector<std::string> layerNodeFieldNames();

} // namespace tb::ql
