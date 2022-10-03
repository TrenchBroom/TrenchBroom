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

#include "LinkTargetValidator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();

void validateInternal(
  EntityNodeBase& entityNode, const std::vector<std::string>& propertyKeys,
  std::vector<std::unique_ptr<Issue>>& issues) {
  issues.reserve(issues.size() + propertyKeys.size());
  for (const auto& key : propertyKeys) {
    issues.push_back(std::make_unique<EntityPropertyIssue>(
      Type, entityNode, key, entityNode.name() + " has missing target for key '" + key + "'"));
  }
}
} // namespace

LinkTargetValidator::LinkTargetValidator()
  : Validator{Type, "Missing entity link target"} {
  addQuickFix(makeRemoveEntityPropertiesQuickFix(Type));
}

void LinkTargetValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  validateInternal(entityNode, entityNode.findMissingLinkTargets(), issues);
  validateInternal(entityNode, entityNode.findMissingKillTargets(), issues);
}

} // namespace Model
} // namespace TrenchBroom
