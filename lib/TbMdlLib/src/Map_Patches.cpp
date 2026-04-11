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

#include "mdl/Map_Patches.h"

#include "mdl/CreatePatch.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/Transaction.h"

#include "kd/ranges/to.h"

#include <ranges>

namespace tb::mdl
{

void createPatches(Map& map, const size_t pointRowCount, const size_t pointColumnCount)
{
  const auto& selection = map.selection();
  contract_assert(selection.hasAnyBrushFaces());

  auto patchNodes =
    selection.allBrushFaces() | std::views::transform([&](const auto& faceHandle) {
      return createPatch(faceHandle.face(), pointRowCount, pointColumnCount);
    })
    | std::views::join | std::views::transform([](auto patch) {
        return static_cast<Node*>(
          std::make_unique<PatchNode>(std::move(patch)).release());
      })
    | kdl::ranges::to<std::vector>();

  auto transaction = Transaction{map, "Create Patches"};

  auto addedNodes = addNodes(map, {{parentForNodes(map), patchNodes}});
  deselectAll(map);
  selectNodes(map, addedNodes);

  transaction.commit();
}

} // namespace tb::mdl
