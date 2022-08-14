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

#include "UpdateLinkedGroupsHelper.h"

#include "Model/BrushError.h"
#include "Model/GroupNode.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Model/UpdateLinkedGroupsError.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_set>

namespace TrenchBroom {
namespace View {
bool checkLinkedGroupsToUpdate(const std::vector<const Model::GroupNode*>& linkedGroupsToUpdate) {
  const auto linkedGroupIds =
    kdl::vec_sort(kdl::vec_transform(linkedGroupsToUpdate, [](const auto* groupNode) {
      return groupNode->group().linkedGroupId();
    }));

  return std::adjacent_find(std::begin(linkedGroupIds), std::end(linkedGroupIds)) ==
         std::end(linkedGroupIds);
}

// Order groups so that descendants will be updated before their ancestors
const auto compareByAncestry = [](const auto* lhs, const auto* rhs) {
  return rhs->isAncestorOf(lhs);
};

UpdateLinkedGroupsHelper::UpdateLinkedGroupsHelper(LinkedGroupsToUpdate linkedGroupsToUpdate)
  : m_state{kdl::vec_sort(std::move(linkedGroupsToUpdate), compareByAncestry)} {}

UpdateLinkedGroupsHelper::~UpdateLinkedGroupsHelper() = default;

kdl::result<void, Model::UpdateLinkedGroupsError> UpdateLinkedGroupsHelper::applyLinkedGroupUpdates(
  MapDocumentCommandFacade& document) {
  return computeLinkedGroupUpdates(document).and_then([&]() {
    doApplyOrUndoLinkedGroupUpdates(document);
  });
}

void UpdateLinkedGroupsHelper::undoLinkedGroupUpdates(MapDocumentCommandFacade& document) {
  doApplyOrUndoLinkedGroupUpdates(document);
}

void UpdateLinkedGroupsHelper::collateWith(UpdateLinkedGroupsHelper& other) {
  // Both helpers have already applied their changes at this point, so in both helpers,
  // m_linkedGroups contains pairs p where
  // - p.first is the group node to update
  // - p.second is a vector containing the group node's original children
  //
  // Let p_o be an update from the other helper. If p_o is an update for a linked group node that
  // was updated by this helper, then there is a pair p_t in this helper such that p_t.first ==
  // p_o.first. In this case, we want to keep the old children of the linked group node stored in
  // this helper and discard those in the other helper. If p_o is not an update for a linked group
  // node that was updated by this helper, then we will add p_o to our updates and remove it from
  // the other helper's updates to prevent the replaced node to be deleted with the other helper.

  auto& myLinkedGroupUpdates = std::get<LinkedGroupUpdates>(m_state);
  auto& theirLinkedGroupUpdates = std::get<LinkedGroupUpdates>(other.m_state);

  for (auto& [theirGroupNodeToUpdate, theirOldChildren] : theirLinkedGroupUpdates) {
    const auto myIt = std::find_if(
      std::begin(myLinkedGroupUpdates), std::end(myLinkedGroupUpdates),
      [theirGroupNodeToUpdate = theirGroupNodeToUpdate](const auto& p) {
        return p.first == theirGroupNodeToUpdate;
      });
    if (myIt == std::end(myLinkedGroupUpdates)) {
      myLinkedGroupUpdates.emplace_back(theirGroupNodeToUpdate, std::move(theirOldChildren));
    }
  }
}

kdl::result<void, Model::UpdateLinkedGroupsError> UpdateLinkedGroupsHelper::
  computeLinkedGroupUpdates(MapDocumentCommandFacade& document) {
  return std::visit(
    kdl::overload(
      [&](const LinkedGroupsToUpdate& linkedGroupsToUpdate) {
        return computeLinkedGroupUpdates(linkedGroupsToUpdate, document)
          .and_then([&](auto&& linkedGroupUpdates) {
            m_state = std::move(linkedGroupUpdates);
          });
      },
      [](const LinkedGroupUpdates&) -> kdl::result<void, Model::UpdateLinkedGroupsError> {
        return kdl::void_success;
      }),
    m_state);
}

kdl::result<UpdateLinkedGroupsHelper::LinkedGroupUpdates, Model::UpdateLinkedGroupsError>
UpdateLinkedGroupsHelper::computeLinkedGroupUpdates(
  const LinkedGroupsToUpdate& linkedGroupsToUpdate, MapDocumentCommandFacade& document) {
  if (!checkLinkedGroupsToUpdate(linkedGroupsToUpdate)) {
    return Model::UpdateLinkedGroupsError::UpdateIsInconsistent;
  }

  const auto& worldBounds = document.worldBounds();
  return kdl::for_each_result(
           linkedGroupsToUpdate,
           [&](const auto* groupNode) {
             const auto groupNodesToUpdate = kdl::vec_erase(
               Model::findLinkedGroups(*document.world(), *groupNode->group().linkedGroupId()),
               groupNode);

             return Model::updateLinkedGroups(*groupNode, groupNodesToUpdate, worldBounds);
           })
    .and_then(
      [&](auto&& nestedUpdateLists)
        -> kdl::result<LinkedGroupUpdates, Model::UpdateLinkedGroupsError> {
        return kdl::vec_flatten(std::move(nestedUpdateLists));
      });
}

void UpdateLinkedGroupsHelper::doApplyOrUndoLinkedGroupUpdates(MapDocumentCommandFacade& document) {
  std::visit(
    kdl::overload(
      [](const LinkedGroupsToUpdate&) {},
      [&](LinkedGroupUpdates&& linkedGroupUpdates) {
        m_state = document.performReplaceChildren(std::move(linkedGroupUpdates));
      }),
    std::move(m_state));
}
} // namespace View
} // namespace TrenchBroom
