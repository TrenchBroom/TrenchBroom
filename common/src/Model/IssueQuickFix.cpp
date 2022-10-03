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
IssueQuickFix::IssueQuickFix(const IssueType issueType, std::string description)
  : m_issueType{issueType}
  , m_description{std::move(description)} {}

IssueQuickFix::~IssueQuickFix() = default;

const std::string& IssueQuickFix::description() const {
  return m_description;
}

void IssueQuickFix::apply(MapFacade& facade, const std::vector<const Issue*>& issues) const {
  doApply(facade, issues);
}

void IssueQuickFix::doApply(MapFacade& facade, const std::vector<const Issue*>& issues) const {
  for (const Issue* issue : issues) {
    if (issue->type() == m_issueType) {
      doApply(facade, *issue);
    }
  }
}

void IssueQuickFix::doApply(MapFacade&, const Issue&) const {
  assert(false);
}

RemoveEntityPropertiesQuickFix::RemoveEntityPropertiesQuickFix(const IssueType issueType)
  : IssueQuickFix{issueType, "Delete properties"} {}

void RemoveEntityPropertiesQuickFix::doApply(MapFacade& facade, const Issue& issue) const {
  const auto pushSelection = PushSelection{facade};

  const auto& entityPropertyIssue = static_cast<const EntityPropertyIssue&>(issue);

  // If world node is affected, the selection will fail, but if nothing is selected,
  // the removeProperty call will correctly affect worldspawn either way.

  facade.deselectAll();
  facade.selectNodes({&issue.node()});
  facade.removeProperty(entityPropertyIssue.propertyKey());
}

TransformEntityPropertiesQuickFix::TransformEntityPropertiesQuickFix(
  const IssueType issueType, std::string description, KeyTransform keyTransform,
  ValueTransform valueTransform)
  : IssueQuickFix{issueType, std::move(description)}
  , m_keyTransform{std::move(keyTransform)}
  , m_valueTransform{std::move(valueTransform)} {}

void TransformEntityPropertiesQuickFix::doApply(MapFacade& facade, const Issue& issue) const {
  const auto pushSelection = PushSelection{facade};

  const auto& propIssue = static_cast<const EntityPropertyIssue&>(issue);
  const auto& oldkey = propIssue.propertyKey();
  const auto& oldValue = propIssue.propertyValue();
  const auto newKey = m_keyTransform(oldkey);
  const auto newValue = m_valueTransform(oldValue);

  // If world node is affected, the selection will fail, but if nothing is selected,
  // the removeProperty call will correctly affect worldspawn either way.

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
}
} // namespace Model
} // namespace TrenchBroom
