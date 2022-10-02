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
class EmptyPropertyKeyValidator::EmptyPropertyKeyIssue : public Issue {
public:
  static const IssueType Type;

public:
  explicit EmptyPropertyKeyIssue(EntityNodeBase* node)
    : Issue(node) {}

  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    const EntityNodeBase* entityNode = static_cast<EntityNodeBase*>(node());
    return entityNode->name() + " has a property with an empty name.";
  }
};

const IssueType EmptyPropertyKeyValidator::EmptyPropertyKeyIssue::Type = Issue::freeType();

class EmptyPropertyKeyValidator::EmptyPropertyKeyIssueQuickFix : public IssueQuickFix {
public:
  EmptyPropertyKeyIssueQuickFix()
    : IssueQuickFix(EmptyPropertyKeyIssue::Type, "Delete property") {}

private:
  void doApply(MapFacade* facade, const Issue* issue) const override {
    const PushSelection push(facade);

    // If world node is affected, the selection will fail, but if nothing is selected,
    // the removeProperty call will correctly affect worldspawn either way.

    facade->deselectAll();
    facade->selectNodes({issue->node()});
    facade->removeProperty("");
  }
};

EmptyPropertyKeyValidator::EmptyPropertyKeyValidator()
  : Validator(EmptyPropertyKeyIssue::Type, "Empty property name") {
  addQuickFix(new EmptyPropertyKeyIssueQuickFix());
}

void EmptyPropertyKeyValidator::doValidate(EntityNodeBase* node, IssueList& issues) const {
  if (node->entity().hasProperty(""))
    issues.push_back(new EmptyPropertyKeyIssue(node));
}
} // namespace Model
} // namespace TrenchBroom
