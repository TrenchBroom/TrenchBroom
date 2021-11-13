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

#include "LongPropertyValueIssueGenerator.h"

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
class LongPropertyValueIssueGenerator::LongPropertyValueIssue : public EntityPropertyIssue {
public:
  static const IssueType Type;

private:
  const std::string m_propertyKey;

public:
  LongPropertyValueIssue(EntityNodeBase* node, const std::string& propertyKey)
    : EntityPropertyIssue(node)
    , m_propertyKey(propertyKey) {}

  const std::string& propertyKey() const override { return m_propertyKey; }

private:
  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    return "The value of entity property '" + m_propertyKey + "' is too long.";
  }
};

const IssueType LongPropertyValueIssueGenerator::LongPropertyValueIssue::Type = Issue::freeType();

class LongPropertyValueIssueGenerator::TruncateLongPropertyValueIssueQuickFix
  : public IssueQuickFix {
private:
  size_t m_maxLength;

public:
  explicit TruncateLongPropertyValueIssueQuickFix(const size_t maxLength)
    : IssueQuickFix(LongPropertyValueIssue::Type, "Truncate property values")
    , m_maxLength(maxLength) {}

private:
  void doApply(MapFacade* facade, const Issue* issue) const override {
    const PushSelection push(facade);

    const LongPropertyValueIssue* propIssue = static_cast<const LongPropertyValueIssue*>(issue);
    const auto& propertyName = propIssue->propertyKey();
    const auto& propertyValue = propIssue->propertyValue();

    // If world node is affected, the selection will fail, but if nothing is selected,
    // the removeProperty call will correctly affect worldspawn either way.

    facade->deselectAll();
    facade->select(issue->node());
    facade->setProperty(propertyName, propertyValue.substr(0, m_maxLength));
  }
};

LongPropertyValueIssueGenerator::LongPropertyValueIssueGenerator(const size_t maxLength)
  : IssueGenerator(LongPropertyValueIssue::Type, "Long entity property value")
  , m_maxLength(maxLength) {
  addQuickFix(new RemoveEntityPropertiesQuickFix(LongPropertyValueIssue::Type));
  addQuickFix(new TruncateLongPropertyValueIssueQuickFix(m_maxLength));
}

void LongPropertyValueIssueGenerator::doGenerate(EntityNodeBase* node, IssueList& issues) const {
  for (const EntityProperty& property : node->entity().properties()) {
    const auto& propertyKey = property.key();
    const auto& propertyValue = property.value();
    if (propertyValue.size() >= m_maxLength) {
      issues.push_back(new LongPropertyValueIssue(node, propertyKey));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
