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

#include "EmptyBrushEntityValidator.h"

#include "Assets/EntityDefinition.h"
#include "Ensure.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <string>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();
} // namespace

EmptyBrushEntityValidator::EmptyBrushEntityValidator()
  : Validator{Type, "Empty brush entity"} {
  addQuickFix(makeDeleteNodesQuickFix());
}

void EmptyBrushEntityValidator::doValidate(
  EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  const auto* definition =
    dynamic_cast<const Assets::BrushEntityDefinition*>(entityNode.entity().definition());
  if (definition && !entityNode.hasChildren()) {
    issues.push_back(std::make_unique<Issue>(
      Type, entityNode, "Entity '" + entityNode.name() + "' does not contain any brushes"));
  }
}
} // namespace Model
} // namespace TrenchBroom
