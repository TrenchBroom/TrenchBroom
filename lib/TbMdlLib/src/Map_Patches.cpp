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

#include "mdl/ApplyAndSwap.h"
#include "mdl/ControlPointCommand.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchUtils.h"
#include "mdl/Selection.h"
#include "mdl/Transaction.h"

#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/string_format.h"
#include "kd/vector_utils.h"

#include <ranges>

namespace tb::mdl
{

bool resamplePatches(Map& map, const size_t pointRowCount, const size_t pointColumnCount)
{
  const auto allSelectedHandlePositions =
    map.nodeHandles().selectedHandles<ControlPointHandle>()
    | std::views::transform([](const auto& handle) { return handle.position; })
    | kdl::ranges::to<std::vector>();

  auto newNodes = applyToNodeContents(
    map.selection().patches,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [](Brush&) { return true; },
      [&](BezierPatch& patch) {
        patch = resamplePatch(patch, pointRowCount, pointColumnCount);
        return true;
      }));

  if (!newNodes)
  {
    return false;
  }

  auto transaction = Transaction{map, "Resample Patch"};

  const auto changedLinkedGroups = collectContainingGroups(
    *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

  auto command = std::make_unique<ControlPointCommand>(
    "Resample Patch",
    std::move(*newNodes),
    allSelectedHandlePositions,
    std::vector<ControlPointHandle::Position>{});

  if (!map.executeAndStore(std::move(command)))
  {
    transaction.cancel();
    return false;
  }

  setHasPendingChanges(changedLinkedGroups, true);

  return transaction.commit();
}

bool transformControlPoints(
  Map& map,
  const std::vector<vm::vec3d>& controlPointPositions,
  const vm::mat4x4d& transform)
{
  const auto controlPointPositionSet =
    std::set<vm::vec3d>{controlPointPositions.begin(), controlPointPositions.end()};

  auto newNodes = applyToNodeContents(
    map.selection().patches,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [](Brush&) { return true; },
      [&](BezierPatch& patch) {
        patch.transformControlPoints(controlPointPositionSet, transform);
        return true;
      }));

  if (!newNodes)
  {
    return false;
  }

  auto newControlPointPositions = transform * controlPointPositions;
  kdl::vec_sort_and_remove_duplicates(newControlPointPositions);

  auto commandName = kdl::str_plural(
    controlPointPositions.size(), "Move Control Point", "Move Control Points");
  auto transaction = Transaction{map, commandName};

  const auto changedLinkedGroups = collectContainingGroups(
    *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

  auto command = std::make_unique<ControlPointCommand>(
    std::move(commandName),
    std::move(*newNodes),
    controlPointPositions,
    newControlPointPositions);

  if (!map.executeAndStore(std::move(command)))
  {
    transaction.cancel();
    return false;
  }

  setHasPendingChanges(changedLinkedGroups, true);

  return transaction.commit();
}

} // namespace tb::mdl
