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

#include "EmptyBrushEntityValidator.h"

#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/MapFacade.h"

#include <string>

namespace tb::mdl
{
namespace
{
const auto Type = freeIssueType();
} // namespace

EmptyBrushEntityValidator::EmptyBrushEntityValidator()
  : Validator{Type, "Empty brush entity"}
{
  addQuickFix(makeDeleteNodesQuickFix());
}

void EmptyBrushEntityValidator::doValidate(
  EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  if (const auto* definition = entityNode.entity().definition();
      definition && getType(*definition) == EntityDefinitionType::Brush
      && !entityNode.hasChildren())
  {
    issues.push_back(std::make_unique<Issue>(
      Type,
      entityNode,
      "Entity '" + entityNode.name() + "' does not contain any brushes"));
  }
}

} // namespace tb::mdl
