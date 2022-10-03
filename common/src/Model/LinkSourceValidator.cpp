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

#include "LinkSourceValidator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();
} // namespace

LinkSourceValidator::LinkSourceValidator()
  : Validator{Type, "Missing entity link source"} {
  addQuickFix(makeRemoveEntityPropertiesQuickFix(Type));
}

void LinkSourceValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  if (entityNode.hasMissingSources()) {
    issues.push_back(std::make_unique<EntityPropertyIssue>(
      Type, entityNode, EntityPropertyKeys::Targetname,
      entityNode.name() + " has unused targetname key"));
  }
}
} // namespace Model
} // namespace TrenchBroom
