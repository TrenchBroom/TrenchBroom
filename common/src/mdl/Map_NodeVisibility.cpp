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

#include "mdl/Map_NodeVisibility.h"

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/SetVisibilityCommand.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/ranges/to.h"

#include <ranges>

namespace tb::mdl
{

void isolateSelectedNodes(Map& map)
{
  auto selectedNodes = std::vector<Node*>{};
  auto unselectedNodes = std::vector<Node*>{};

  const auto collectNode = [&](auto* node) {
    if (node->transitivelySelected() || node->descendantSelected())
    {
      selectedNodes.push_back(node);
    }
    else
    {
      unselectedNodes.push_back(node);
    }
  };

  map.world()->accept(kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [&](auto&& thisLambda, GroupNode* group) {
      collectNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entity) {
      collectNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](BrushNode* brush) { collectNode(brush); },
    [&](PatchNode* patch) { collectNode(patch); }));

  auto transaction = Transaction{map, "Isolate Objects"};
  map.executeAndStore(SetVisibilityCommand::hide(unselectedNodes));
  map.executeAndStore(SetVisibilityCommand::show(selectedNodes));
  transaction.commit();
}

void hideSelectedNodes(Map& map)
{
  hideNodes(map, map.selection().nodes);
}

void hideNodes(Map& map, std::vector<Node*> nodes)
{
  auto transaction = Transaction{map, "Hide Objects"};

  // Deselect any selected nodes inside `nodes`
  deselectNodes(map, collectSelectedNodes(nodes));

  // Reset visibility of any forced shown children of `nodes`
  downgradeShownToInherit(map, collectDescendants(nodes));

  map.executeAndStore(SetVisibilityCommand::hide(nodes));
  transaction.commit();
}

void showAllNodes(Map& map)
{
  resetNodeVisibility(map, mdl::collectDescendants(map.world()->allLayers()));
}

void showNodes(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SetVisibilityCommand::show(nodes));
}

void ensureNodesVisible(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SetVisibilityCommand::ensureVisible(nodes));
}

void resetNodeVisibility(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SetVisibilityCommand::reset(nodes));
}

void downgradeShownToInherit(Map& map, const std::vector<Node*>& nodes)
{
  const auto nodesToReset = nodes | std::views::filter([](auto* node) {
                              return node->visibilityState() == VisibilityState::Shown;
                            })
                            | kdl::ranges::to<std::vector>();
  resetNodeVisibility(map, nodesToReset);
}

} // namespace tb::mdl
