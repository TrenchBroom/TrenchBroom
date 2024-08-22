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

#include "PropertyKeyWithDoubleQuotationMarksValidator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"

#include "kdl/string_utils.h"

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
namespace
{
static const auto Type = freeIssueType();
} // namespace

PropertyKeyWithDoubleQuotationMarksValidator::
  PropertyKeyWithDoubleQuotationMarksValidator()
  : Validator{Type, "Invalid entity property keys"}
{
  addQuickFix(makeRemoveEntityPropertiesQuickFix(Type));
  addQuickFix(makeTransformEntityPropertiesQuickFix(
    Type,
    "Replace \" with '",
    [](const std::string& key) { return kdl::str_replace_every(key, "\"", "'"); },
    [](const std::string& value) { return value; }));
}

void PropertyKeyWithDoubleQuotationMarksValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  for (const auto& property : entityNode.entity().properties())
  {
    const auto& propertyKey = property.key();
    if (propertyKey.find('"') != std::string::npos)
    {
      issues.push_back(std::make_unique<EntityPropertyIssue>(
        Type,
        entityNode,
        propertyKey,
        "Property key '" + propertyKey + "' of " + entityNode.name()
          + " contains double quotation marks."));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
