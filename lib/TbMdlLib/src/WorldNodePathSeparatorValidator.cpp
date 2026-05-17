/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/WorldNodePathSeparatorValidator.h"

#include "mdl/Entity.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityProperties.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"

#include "kd/string_utils.h"

#include <fmt/format.h>

#include <string>
#include <vector>

namespace tb::mdl
{
namespace
{
const auto Type = freeIssueType();

std::unique_ptr<Issue> validateEntityPropertyPath(
  EntityNodeBase& entityNode, const std::string_view key, const std::string_view value)
{

  if (value.find('\\') != std::string::npos)
  {
    return std::make_unique<EntityPropertyIssue>(
      Type,
      entityNode,
      std::string{key},
      fmt::format(
        "Property '{}' of {} contains backslashes in paths.", key, entityNode.name()));
  }

  return {};
}

} // namespace

WorldNodePathSeparatorValidator::WorldNodePathSeparatorValidator()
  : Validator{Type, "Paths must use forward slashes"}
{
  addQuickFix(makeTransformEntityPropertiesQuickFix(
    Type,
    "Replace \\ with /",
    [](const std::string& key) { return key; },
    [](const std::string& value) { return kdl::str_replace_every(value, "\\", "/"); }));
}

void WorldNodePathSeparatorValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  if (entityNode.entity().classname() != EntityPropertyValues::WorldspawnClassname)
  {
    return;
  }

  for (const auto& key :
       {EntityPropertyKeys::Wad, EntityPropertyKeys::TbEntityDefinitions})
  {
    if (const auto* value = entityNode.entity().property(key))
    {
      if (auto issue = validateEntityPropertyPath(entityNode, key, *value))
      {
        issues.push_back(std::move(issue));
      }
    }
  }
}

} // namespace tb::mdl
