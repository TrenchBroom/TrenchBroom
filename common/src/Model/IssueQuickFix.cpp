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

#include "IssueQuickFix.h"

#include "Model/Issue.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <cassert>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
IssueQuickFix::IssueQuickFix(std::string description, MultiIssueFix fix)
  : m_description{std::move(description)}
  , m_fix{std::move(fix)} {}

IssueQuickFix::IssueQuickFix(IssueType issueType, std::string description, SingleIssueFix fix)
  : IssueQuickFix{
      std::move(description), [=](MapFacade& facade, const std::vector<const Issue*>& issues) {
        for (const auto* issue : issues) {
          if (issue->type() == issueType) {
            fix(facade, *issue);
          }
        }
      }} {}

IssueQuickFix::~IssueQuickFix() = default;

const std::string& IssueQuickFix::description() const {
  return m_description;
}

void IssueQuickFix::apply(MapFacade& facade, const std::vector<const Issue*>& issues) const {
  m_fix(facade, issues);
}

IssueQuickFix makeDeleteNodesQuickFix() {
  return {"Delete Objects", [](MapFacade& facade, const std::vector<const Issue*>&) {
            facade.deleteObjects();
          }};
}

IssueQuickFix makeRemoveEntityPropertiesQuickFix(const IssueType type) {
  return {type, "Delete Property", [](MapFacade& facade, const Issue& issue) {
            const auto pushSelection = PushSelection{facade};

            const auto& entityPropertyIssue = static_cast<const EntityPropertyIssue&>(issue);

            // If world node is affected, the selection will fail, but if nothing is
            // selected, the removeProperty call will correctly affect worldspawn
            // either way.

            facade.deselectAll();
            facade.selectNodes({&issue.node()});
            facade.removeProperty(entityPropertyIssue.propertyKey());
          }};
}

IssueQuickFix makeTransformEntityPropertiesQuickFix(
  const IssueType type, std::string description,
  std::function<std::string(const std::string&)> keyTransform,
  std::function<std::string(const std::string&)> valueTransform) {
  return {type, std::move(description), [=](MapFacade& facade, const Issue& issue) {
            const auto pushSelection = PushSelection{facade};

            const auto& propIssue = static_cast<const EntityPropertyIssue&>(issue);
            const auto& oldkey = propIssue.propertyKey();
            const auto& oldValue = propIssue.propertyValue();
            const auto newKey = keyTransform(oldkey);
            const auto newValue = valueTransform(oldValue);

            // If world node is affected, the selection will fail, but if nothing is
            // selected, the removeProperty call will correctly affect worldspawn
            // either way.

            facade.deselectAll();
            facade.selectNodes({&issue.node()});

            if (newKey.empty()) {
              facade.removeProperty(propIssue.propertyKey());
            } else {
              if (newKey != oldkey) {
                facade.renameProperty(oldkey, newKey);
              }
              if (newValue != oldValue) {
                facade.setProperty(newKey, newValue);
              }
            }
          }};
}
} // namespace Model
} // namespace TrenchBroom
