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

#include "WorldBoundsValidator.h"

#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PatchNode.h"

#include <string>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();

void validateInternal(
  const vm::bbox3& bounds, Node& node, std::vector<std::unique_ptr<Issue>>& issues) {
  if (!bounds.contains(node.logicalBounds())) {
    issues.push_back(std::make_unique<Issue>(Type, node, "Object is out of world bounds"));
  }
}
} // namespace

WorldBoundsValidator::WorldBoundsValidator(const vm::bbox3& bounds)
  : Validator{Type, "Objects out of world bounds"}
  , m_bounds{bounds} {
  addQuickFix(makeDeleteNodesQuickFix());
}

void WorldBoundsValidator::doValidate(
  EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  validateInternal(m_bounds, entityNode, issues);
}

void WorldBoundsValidator::doValidate(
  BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  validateInternal(m_bounds, brushNode, issues);
}

void WorldBoundsValidator::doValidate(
  PatchNode& patchNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  validateInternal(m_bounds, patchNode, issues);
}
} // namespace Model
} // namespace TrenchBroom
