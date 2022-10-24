/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "PointEntityWithBrushesValidator.h"

#include "Assets/EntityDefinition.h"
#include "Ensure.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <kdl/vector_utils.h>

#include <map>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
namespace
{
static const auto Type = freeIssueType();

IssueQuickFix makeMoveBrushesToWorldQuickFix()
{
  return {
    "Move Brushes to World",
    [](MapFacade& facade, const std::vector<const Issue*>& issues) {
      auto affectedNodes = std::vector<Node*>{};
      auto nodesToReparent = std::map<Node*, std::vector<Node*>>{};

      for (const auto* issue : issues)
      {
        auto& node = issue->node();
        nodesToReparent[node.parent()] = node.children();

        affectedNodes.push_back(&node);
        affectedNodes = kdl::vec_concat(std::move(affectedNodes), node.children());
      }

      facade.deselectAll();
      facade.reparentNodes(nodesToReparent);
      facade.selectNodes(affectedNodes);
    }};
}
} // namespace

PointEntityWithBrushesValidator::PointEntityWithBrushesValidator()
  : Validator{Type, "Point entity with brushes"}
{
  addQuickFix(makeMoveBrushesToWorldQuickFix());
}

void PointEntityWithBrushesValidator::doValidate(
  EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  const auto* definition =
    dynamic_cast<const Assets::PointEntityDefinition*>(entityNode.entity().definition());
  if (definition && entityNode.hasChildren())
  {
    issues.push_back(
      std::make_unique<Issue>(Type, entityNode, entityNode.name() + " contains brushes"));
  }
}
} // namespace Model
} // namespace TrenchBroom
