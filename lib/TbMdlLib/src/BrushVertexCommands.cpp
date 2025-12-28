/*
 Copyright (C) 2020 Kristian Duske

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

#include "mdl/BrushVertexCommands.h"

#include "mdl/BrushNode.h"

#include "kd/ranges/to.h"

#include <ranges>


namespace tb::mdl::detail
{

std::vector<BrushNode*> collectBrushNodes(
  const std::vector<std::pair<Node*, NodeContents>>& nodes)
{
  return nodes | std::views::filter([](const auto& pair) {
           return dynamic_cast<BrushNode*>(pair.first) != nullptr;
         })
         | std::views::transform(
           [](const auto& pair) { return static_cast<BrushNode*>(pair.first); })
         | kdl::ranges::to<std::vector>();
}

} // namespace tb::mdl::detail
