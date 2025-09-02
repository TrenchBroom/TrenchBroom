/*
 Copyright (C) 2010 Kristian Duske

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

#include "SoftMapBoundsValidator.h"

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include <optional>
#include <string>

namespace tb::mdl
{
namespace
{
const auto Type = freeIssueType();

void validateInternal(
  const Game& game,
  const WorldNode& worldNode,
  Node& node,
  std::vector<std::unique_ptr<Issue>>& issues)
{
  const auto bounds = game.extractSoftMapBounds(worldNode.entity());

  if (bounds.bounds && !bounds.bounds->contains(node.logicalBounds()))
  {
    issues.push_back(
      std::make_unique<Issue>(Type, node, "Object is out of soft map bounds"));
  }
}
} // namespace

SoftMapBoundsValidator::SoftMapBoundsValidator(const Game& game, const WorldNode& world)
  : Validator(Type, "Objects out of soft map bounds")
  , m_game{game}
  , m_world{world}
{
  addQuickFix(makeDeleteNodesQuickFix());
}

void SoftMapBoundsValidator::doValidate(
  EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  validateInternal(m_game, m_world, entityNode, issues);
}

void SoftMapBoundsValidator::doValidate(
  BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  validateInternal(m_game, m_world, brushNode, issues);
}

void SoftMapBoundsValidator::doValidate(
  PatchNode& patchNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  validateInternal(m_game, m_world, patchNode, issues);
}

} // namespace tb::mdl
