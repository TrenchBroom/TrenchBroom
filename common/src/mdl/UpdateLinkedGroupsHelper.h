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

#pragma once

#include "Result.h"

#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace tb::ui
{
class MapDocument;
}

namespace tb::mdl
{
class GroupNode;
class Node;

/**
 * Checks whether the given vector of linked group can be updated consistently.
 *
 * The given linked groups can be updated consistently if no two of them are in the same
 * linked set.
 */
bool checkLinkedGroupsToUpdate(const std::vector<GroupNode*>& changedLinkedGroups);

/**
 * A helper class to add support for updating linked groups to commands.
 *
 * The class is initialized with a vector of group nodes whose changes should be
 * propagated to the members of their respective link sets. When applyLinkedGroupUpdates
 * is first called, a replacement node is created for each linked group that needs to be
 * updated, and these linked groups are replaced with their replacements. Calling
 * applyLinkedGroupUpdates replaces the replacement nodes with their original
 * corresponding groups again, effectively undoing the change.
 */
class UpdateLinkedGroupsHelper
{
private:
  using ChangedLinkedGroups = std::vector<GroupNode*>;
  using LinkedGroupUpdates =
    std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>>;
  std::variant<ChangedLinkedGroups, LinkedGroupUpdates> m_state;

public:
  explicit UpdateLinkedGroupsHelper(ChangedLinkedGroups changedLinkedGroups);
  ~UpdateLinkedGroupsHelper();

  Result<void> applyLinkedGroupUpdates(ui::MapDocument& document);
  void undoLinkedGroupUpdates(ui::MapDocument& document);
  void collateWith(UpdateLinkedGroupsHelper& other);

private:
  Result<void> computeLinkedGroupUpdates(ui::MapDocument& document);
  static Result<LinkedGroupUpdates> computeLinkedGroupUpdates(
    const ChangedLinkedGroups& changedLinkedGroups, ui::MapDocument& document);

  void doApplyOrUndoLinkedGroupUpdates(ui::MapDocument& document);
};

} // namespace tb::mdl
