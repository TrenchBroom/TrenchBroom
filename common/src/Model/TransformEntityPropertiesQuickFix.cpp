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

#include "TransformEntityPropertiesQuickFix.h"

#include "Model/Issue.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>

namespace TrenchBroom {
namespace Model {
TransformEntityPropertiesQuickFix::TransformEntityPropertiesQuickFix(
  const IssueType issueType, const std::string& description, const KeyTransform& keyTransform,
  const ValueTransform& valueTransform)
  : IssueQuickFix(issueType, description)
  , m_keyTransform(keyTransform)
  , m_valueTransform(valueTransform) {}

void TransformEntityPropertiesQuickFix::doApply(MapFacade* facade, const Issue* issue) const {
  const PushSelection push(facade);

  const auto* propIssue = static_cast<const EntityPropertyIssue*>(issue);
  const auto& oldkey = propIssue->propertyKey();
  const auto& oldValue = propIssue->propertyValue();
  const auto newKey = m_keyTransform(oldkey);
  const auto newValue = m_valueTransform(oldValue);

  // If world node is affected, the selection will fail, but if nothing is selected,
  // the removeProperty call will correctly affect worldspawn either way.

  facade->deselectAll();
  facade->select(issue->node());

  if (newKey.empty()) {
    facade->removeProperty(propIssue->propertyKey());
  } else {
    if (newKey != oldkey)
      facade->renameProperty(oldkey, newKey);
    if (newValue != oldValue)
      facade->setProperty(newKey, newValue);
  }
}
} // namespace Model
} // namespace TrenchBroom
