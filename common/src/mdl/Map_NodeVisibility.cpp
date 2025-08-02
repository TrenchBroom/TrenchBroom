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

#include "Map.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/SetVisibilityCommand.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"

#include <algorithm>

namespace tb::mdl
{

void Map::isolateSelectedNodes()
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

  world()->accept(kdl::overload(
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

  auto transaction = Transaction{*this, "Isolate Objects"};
  executeAndStore(SetVisibilityCommand::hide(unselectedNodes));
  executeAndStore(SetVisibilityCommand::show(selectedNodes));
  transaction.commit();
}

void Map::hideSelectedNodes()
{
  hideNodes(selection().nodes);
}

void Map::hideNodes(std::vector<Node*> nodes)
{
  auto transaction = Transaction{*this, "Hide Objects"};

  // Deselect any selected nodes inside `nodes`
  deselectNodes(collectSelectedNodes(nodes));

  // Reset visibility of any forced shown children of `nodes`
  downgradeShownToInherit(collectDescendants(nodes));

  executeAndStore(SetVisibilityCommand::hide(nodes));
  transaction.commit();
}

void Map::showAllNodes()
{
  resetNodeVisibility(mdl::collectDescendants(m_world->allLayers()));
}

void Map::showNodes(const std::vector<Node*>& nodes)
{
  executeAndStore(SetVisibilityCommand::show(nodes));
}

void Map::ensureNodesVisible(const std::vector<Node*>& nodes)
{
  executeAndStore(SetVisibilityCommand::ensureVisible(nodes));
}

void Map::resetNodeVisibility(const std::vector<Node*>& nodes)
{
  executeAndStore(SetVisibilityCommand::reset(nodes));
}

void Map::downgradeShownToInherit(const std::vector<Node*>& nodes)
{
  auto nodesToReset = std::vector<Node*>{};
  std::ranges::copy_if(nodes, std::back_inserter(nodesToReset), [](auto* node) {
    return node->visibilityState() == VisibilityState::Shown;
  });
  resetNodeVisibility(nodesToReset);
}

} // namespace tb::mdl
