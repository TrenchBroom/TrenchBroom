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

#include "mdl/LinkSourceValidator.h"

#include "mdl/EntityLinkManager.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"

#include <string>

namespace tb::mdl
{
namespace
{
const auto Type = freeIssueType();
} // namespace

LinkSourceValidator::LinkSourceValidator(const EntityLinkManager& entityLinkManager)
  : Validator{Type, "Missing entity link source"}
  , m_entityLinkManager{entityLinkManager}
{
  addQuickFix(makeRemoveEntityPropertiesQuickFix(Type));
}

void LinkSourceValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  if (const auto& missingLinkKeys =
        m_entityLinkManager.getTargetPropertyKeysWithMissingSource(entityNode);
      !missingLinkKeys.empty())
  {
    for (const auto& key : missingLinkKeys)
    {
      issues.push_back(std::make_unique<EntityPropertyIssue>(
        Type,
        entityNode,
        key,
        entityNode.name() + " has unused link target key '" + key + "'"));
    }
  }
}

} // namespace tb::mdl
