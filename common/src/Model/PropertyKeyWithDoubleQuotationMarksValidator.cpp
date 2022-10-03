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

#include "PropertyKeyWithDoubleQuotationMarksValidator.h"

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
namespace {
class PropertyKeyWithDoubleQuotationMarksIssue : public EntityPropertyIssue {
public:
  static const IssueType Type;

private:
  const std::string m_propertyKey;

public:
  PropertyKeyWithDoubleQuotationMarksIssue(EntityNodeBase& entityNode, std::string propertyKey)
    : EntityPropertyIssue{entityNode}
    , m_propertyKey{std::move(propertyKey)} {}

  const std::string& propertyKey() const override { return m_propertyKey; }

private:
  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    return "The key of entity property '" + m_propertyKey +
           "' contains double quotation marks. This may cause errors during compilation or in the "
           "game.";
  }
};

const IssueType PropertyKeyWithDoubleQuotationMarksIssue::Type = Issue::freeType();
} // namespace

PropertyKeyWithDoubleQuotationMarksValidator::PropertyKeyWithDoubleQuotationMarksValidator()
  : Validator{PropertyKeyWithDoubleQuotationMarksIssue::Type, "Invalid entity property keys"} {
  addQuickFix(std::make_unique<RemoveEntityPropertiesQuickFix>(
    PropertyKeyWithDoubleQuotationMarksIssue::Type));
  addQuickFix(std::make_unique<TransformEntityPropertiesQuickFix>(
    PropertyKeyWithDoubleQuotationMarksIssue::Type, "Replace \" with '",
    [](const std::string& key) {
      return kdl::str_replace_every(key, "\"", "'");
    },
    [](const std::string& value) {
      return value;
    }));
}

void PropertyKeyWithDoubleQuotationMarksValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  for (const auto& property : entityNode.entity().properties()) {
    const auto& propertyKey = property.key();
    if (propertyKey.find('"') != std::string::npos) {
      issues.push_back(
        std::make_unique<PropertyKeyWithDoubleQuotationMarksIssue>(entityNode, propertyKey));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
