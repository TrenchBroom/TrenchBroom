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

#include "LongPropertyKeyValidator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

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

LongPropertyKeyValidator::LongPropertyKeyValidator(const size_t maxLength)
  : Validator{Type, "Long entity property keys"}
  , m_maxLength{maxLength}
{
  addQuickFix(makeRemoveEntityPropertiesQuickFix(Type));
}

void LongPropertyKeyValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  for (const auto& property : entityNode.entity().properties())
  {
    const auto& propertyKey = property.key();
    if (propertyKey.size() >= m_maxLength)
    {
      issues.push_back(std::make_unique<EntityPropertyIssue>(
        Type,
        entityNode,
        propertyKey,
        "Property key '" + propertyKey.substr(0, 8) + "...' of " + entityNode.name()
          + " is too long."));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
