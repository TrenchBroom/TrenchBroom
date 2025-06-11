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

#include <map>
#include <vector>

namespace tb::mdl
{
class Node;
} // namespace tb::mdl

namespace tb::ui
{
class MapDocument;

void addNodesAndNotify(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes, MapDocument& document);
void removeNodesAndNotify(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes, MapDocument& document);

} // namespace tb::ui
