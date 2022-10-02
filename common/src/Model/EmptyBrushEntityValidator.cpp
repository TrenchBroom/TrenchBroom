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
class EmptyBrushEntityValidator::EmptyBrushEntityIssue : public Issue {
public:
  static const IssueType Type;

public:
  explicit EmptyBrushEntityIssue(EntityNode& entity)
    : Issue(entity) {}

private:
  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    const auto& entity = static_cast<EntityNode&>(node());
    return "Entity '" + entity.name() + "' does not contain any brushes";
  }
};

const IssueType EmptyBrushEntityValidator::EmptyBrushEntityIssue::Type = Issue::freeType();

class EmptyBrushEntityValidator::EmptyBrushEntityIssueQuickFix : public IssueQuickFix {
public:
  EmptyBrushEntityIssueQuickFix()
    : IssueQuickFix(EmptyBrushEntityIssue::Type, "Delete entities") {}

private:
  void doApply(MapFacade* facade, const std::vector<const Issue*>& /* issues */) const override {
    facade->deleteObjects();
  }
};

EmptyBrushEntityValidator::EmptyBrushEntityValidator()
  : Validator(EmptyBrushEntityIssue::Type, "Empty brush entity") {
  addQuickFix(std::make_unique<EmptyBrushEntityIssueQuickFix>());
}

void EmptyBrushEntityValidator::doValidate(
  EntityNode& entityNode, std::vector<Issue*>& issues) const {
  const Assets::EntityDefinition* definition = entityNode.entity().definition();
  if (
    definition != nullptr && definition->type() == Assets::EntityDefinitionType::BrushEntity &&
    !entityNode.hasChildren())
    issues.push_back(new EmptyBrushEntityIssue(entityNode));
}
} // namespace Model
} // namespace TrenchBroom
