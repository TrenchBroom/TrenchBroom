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

#include "EmptyPropertyKeyValidator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();

class EmptyPropertyKeyIssueQuickFix : public IssueQuickFix {
public:
  EmptyPropertyKeyIssueQuickFix()
    : IssueQuickFix{Type, "Delete property"} {}

private:
  void doApply(MapFacade& facade, const Issue& issue) const override {
    const auto pushSelection = PushSelection{facade};

    // If world node is affected, the selection will fail, but if nothing is selected,
    // the removeProperty call will correctly affect worldspawn either way.

    facade.deselectAll();
    facade.selectNodes({&issue.node()});
    facade.removeProperty("");
  }
};
} // namespace

EmptyPropertyKeyValidator::EmptyPropertyKeyValidator()
  : Validator{Type, "Empty property name"} {
  addQuickFix(std::make_unique<EmptyPropertyKeyIssueQuickFix>());
}

void EmptyPropertyKeyValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  if (entityNode.entity().hasProperty("")) {
    issues.push_back(std::make_unique<Issue>(
      Type, entityNode, entityNode.name() + " has a property with an empty name."));
  }
}
} // namespace Model
} // namespace TrenchBroom
