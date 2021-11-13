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

#include "PropertyValueWithDoubleQuotationMarksIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/RemoveEntityPropertiesQuickFix.h"
#include "Model/TransformEntityPropertiesQuickFix.h"

#include <kdl/string_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class PropertyValueWithDoubleQuotationMarksIssueGenerator::
  PropertyValueWithDoubleQuotationMarksIssue : public EntityPropertyIssue {
public:
  static const IssueType Type;

private:
  const std::string m_propertyKey;

public:
  PropertyValueWithDoubleQuotationMarksIssue(EntityNodeBase* node, const std::string& propertyKey)
    : EntityPropertyIssue(node)
    , m_propertyKey(propertyKey) {}

  const std::string& propertyKey() const override { return m_propertyKey; }

private:
  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    return "The value of entity property '" + m_propertyKey +
           "' contains double quotation marks. This may cause errors during compilation or in the "
           "game.";
  }
};

const IssueType PropertyValueWithDoubleQuotationMarksIssueGenerator::
  PropertyValueWithDoubleQuotationMarksIssue::Type = Issue::freeType();

PropertyValueWithDoubleQuotationMarksIssueGenerator::
  PropertyValueWithDoubleQuotationMarksIssueGenerator()
  : IssueGenerator(
      PropertyValueWithDoubleQuotationMarksIssue::Type, "Invalid entity property values") {
  addQuickFix(new RemoveEntityPropertiesQuickFix(PropertyValueWithDoubleQuotationMarksIssue::Type));
  addQuickFix(new TransformEntityPropertiesQuickFix(
    PropertyValueWithDoubleQuotationMarksIssue::Type, "Replace \" with '",
    [](const std::string& key) {
      return key;
    },
    [](const std::string& value) {
      return kdl::str_replace_every(value, "\"", "'");
    }));
}

void PropertyValueWithDoubleQuotationMarksIssueGenerator::doGenerate(
  EntityNodeBase* node, IssueList& issues) const {
  for (const EntityProperty& property : node->entity().properties()) {
    const std::string& propertyKey = property.key();
    const std::string& propertyValue = property.value();
    if (propertyValue.find('"') != std::string::npos) {
      issues.push_back(new PropertyValueWithDoubleQuotationMarksIssue(node, propertyKey));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
