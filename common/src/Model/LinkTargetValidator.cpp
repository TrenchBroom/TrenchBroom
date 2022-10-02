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

#include "LinkTargetValidator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class LinkTargetValidator::LinkTargetIssue : public Issue {
public:
  friend class LinkTargetIssueQuickFix;

private:
  const std::string m_name;

public:
  static const IssueType Type;

public:
  LinkTargetIssue(EntityNodeBase* node, const std::string& name)
    : Issue(node)
    , m_name(name) {}

  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    const EntityNodeBase* propertyNode = static_cast<EntityNodeBase*>(node());
    return propertyNode->name() + " has missing target for key '" + m_name + "'";
  }
};

const IssueType LinkTargetValidator::LinkTargetIssue::Type = Issue::freeType();

class LinkTargetValidator::LinkTargetIssueQuickFix : public IssueQuickFix {
public:
  LinkTargetIssueQuickFix()
    : IssueQuickFix(LinkTargetIssue::Type, "Delete property") {}

private:
  void doApply(MapFacade* facade, const Issue* issue) const override {
    const PushSelection push(facade);

    const LinkTargetIssue* targetIssue = static_cast<const LinkTargetIssue*>(issue);
    const std::string& propertyKey = targetIssue->m_name;

    // If world node is affected, the selection will fail, but if nothing is selected,
    // the removeProperty call will correctly affect worldspawn either way.

    facade->deselectAll();
    facade->selectNodes({issue->node()});
    facade->removeProperty(propertyKey);
  }
};

LinkTargetValidator::LinkTargetValidator()
  : Validator(LinkTargetIssue::Type, "Missing entity link target") {
  addQuickFix(new LinkTargetIssueQuickFix());
}

void LinkTargetValidator::doValidate(EntityNodeBase* node, IssueList& issues) const {
  processKeys(node, node->findMissingLinkTargets(), issues);
  processKeys(node, node->findMissingKillTargets(), issues);
}

void LinkTargetValidator::processKeys(
  EntityNodeBase* node, const std::vector<std::string>& keys, IssueList& issues) const {
  issues.reserve(issues.size() + keys.size());
  for (const std::string& key : keys) {
    issues.push_back(new LinkTargetIssue(node, key));
  }
}
} // namespace Model
} // namespace TrenchBroom
