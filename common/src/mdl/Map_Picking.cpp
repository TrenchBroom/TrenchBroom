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

#include "mdl/Map_Picking.h"

#include "mdl/Map.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

void pick(Map& map, const vm::ray3d& pickRay, PickResult& pickResult)
{
  map.worldNode().pick(map.editorContext(), pickRay, pickResult);
}

std::vector<Node*> findNodesContaining(Map& map, const vm::vec3d& point)
{
  auto result = std::vector<Node*>{};
  map.worldNode().findNodesContaining(point, result);
  return result;
}

} // namespace tb::mdl
