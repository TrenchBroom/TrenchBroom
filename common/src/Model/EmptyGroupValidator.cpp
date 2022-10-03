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

#include "EmptyGroupValidator.h"

#include "Assets/EntityDefinition.h"
#include "Ensure.h"
#include "Model/GroupNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <vector>

namespace TrenchBroom {
namespace Model {
namespace {
class EmptyGroupIssue : public Issue {
public:
  static const IssueType Type;

public:
  explicit EmptyGroupIssue(GroupNode& groupNode)
    : Issue{groupNode} {}

private:
  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    const auto& groupNode = static_cast<GroupNode&>(node());
    return "Group '" + groupNode.name() + "' does not contain any objects";
  }
};

const IssueType EmptyGroupIssue::Type = Issue::freeType();

class EmptyGroupIssueQuickFix : public IssueQuickFix {
public:
  EmptyGroupIssueQuickFix()
    : IssueQuickFix{EmptyGroupIssue::Type, "Delete groups"} {}

private:
  void doApply(MapFacade* facade, const std::vector<const Issue*>&) const override {
    facade->deleteObjects();
  }
};
} // namespace

EmptyGroupValidator::EmptyGroupValidator()
  : Validator{EmptyGroupIssue::Type, "Empty group"} {
  addQuickFix(std::make_unique<EmptyGroupIssueQuickFix>());
}

void EmptyGroupValidator::doValidate(
  GroupNode& groupNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  if (!groupNode.hasChildren()) {
    issues.push_back(std::make_unique<EmptyGroupIssue>(groupNode));
  }
}
} // namespace Model
} // namespace TrenchBroom
