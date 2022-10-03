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

#include "LongPropertyValueValidator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"
#include "Model/RemoveEntityPropertiesQuickFix.h"

#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();

class TruncateLongPropertyValueIssueQuickFix : public IssueQuickFix {
private:
  size_t m_maxLength;

public:
  explicit TruncateLongPropertyValueIssueQuickFix(const size_t maxLength)
    : IssueQuickFix{Type, "Truncate property values"}
    , m_maxLength{maxLength} {}

private:
  void doApply(MapFacade* facade, const Issue& issue) const override {
    const auto pushSelection = PushSelection{facade};

    const auto& propIssue = static_cast<const EntityPropertyIssue&>(issue);
    const auto& propertyName = propIssue.propertyKey();
    const auto& propertyValue = propIssue.propertyValue();

    // If world node is affected, the selection will fail, but if nothing is selected,
    // the removeProperty call will correctly affect worldspawn either way.

    facade->deselectAll();
    facade->selectNodes({&issue.node()});
    facade->setProperty(propertyName, propertyValue.substr(0, m_maxLength));
  }
};
} // namespace

LongPropertyValueValidator::LongPropertyValueValidator(const size_t maxLength)
  : Validator{Type, "Long entity property value"}
  , m_maxLength{maxLength} {
  addQuickFix(std::make_unique<RemoveEntityPropertiesQuickFix>(Type));
  addQuickFix(std::make_unique<TruncateLongPropertyValueIssueQuickFix>(m_maxLength));
}

void LongPropertyValueValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  for (const auto& property : entityNode.entity().properties()) {
    const auto& propertyKey = property.key();
    const auto& propertyValue = property.value();
    if (propertyValue.size() >= m_maxLength) {
      issues.push_back(std::make_unique<EntityPropertyIssue>(
        Type, entityNode, propertyKey,
        "Property value '" + propertyKey + "...' of " + entityNode.name() + " is too long."));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
