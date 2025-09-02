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

#include "LongPropertyValueValidator.h"

#include "mdl/Entity.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Selection.h"
#include "mdl/PushSelection.h"

#include <string>
#include <vector>

namespace tb::mdl
{
namespace
{
const auto Type = freeIssueType();

IssueQuickFix makeTruncatePropertyValueQuickFix(const size_t maxLength)
{
  return {Type, "Truncate Property Values", [=](Map& map, const Issue& issue) {
            const auto pushSelection = PushSelection{map};

            const auto& propIssue = static_cast<const EntityPropertyIssue&>(issue);
            const auto& propertyName = propIssue.propertyKey();
            const auto& propertyValue = propIssue.propertyValue();

            // If world node is affected, the selection will fail, but if nothing is
            // selected, the removeProperty call will correctly affect worldspawn either
            // way.

            deselectAll(map);
            selectNodes(map, {&issue.node()});
            setEntityProperty(map, propertyName, propertyValue.substr(0, maxLength));
          }};
}
} // namespace

LongPropertyValueValidator::LongPropertyValueValidator(const size_t maxLength)
  : Validator{Type, "Long entity property value"}
  , m_maxLength{maxLength}
{
  addQuickFix(makeRemoveEntityPropertiesQuickFix(Type));
  addQuickFix(makeTruncatePropertyValueQuickFix(m_maxLength));
}

void LongPropertyValueValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  for (const auto& property : entityNode.entity().properties())
  {
    const auto& propertyKey = property.key();
    const auto& propertyValue = property.value();
    if (propertyValue.size() >= m_maxLength)
    {
      issues.push_back(std::make_unique<EntityPropertyIssue>(
        Type,
        entityNode,
        propertyKey,
        "Property value '" + propertyKey + "...' of " + entityNode.name()
          + " is too long."));
    }
  }
}

} // namespace tb::mdl
