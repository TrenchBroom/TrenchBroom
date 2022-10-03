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

#include "LongPropertyKeyValidator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/MapFacade.h"
#include "Model/RemoveEntityPropertiesQuickFix.h"

#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
namespace {
class LongPropertyKeyIssue : public EntityPropertyIssue {
public:
  static const IssueType Type;

private:
  const std::string m_propertyKey;

public:
  LongPropertyKeyIssue(EntityNodeBase& entityNode, const std::string& propertyKey)
    : EntityPropertyIssue{entityNode}
    , m_propertyKey{propertyKey} {}

  const std::string& propertyKey() const override { return m_propertyKey; }

private:
  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override {
    return "Entity property key '" + m_propertyKey.substr(0, 8) + "...' is too long.";
  }
};

const IssueType LongPropertyKeyIssue::Type = Issue::freeType();
} // namespace

LongPropertyKeyValidator::LongPropertyKeyValidator(const size_t maxLength)
  : Validator{LongPropertyKeyIssue::Type, "Long entity property keys"}
  , m_maxLength{maxLength} {
  addQuickFix(std::make_unique<RemoveEntityPropertiesQuickFix>(LongPropertyKeyIssue::Type));
}

void LongPropertyKeyValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  for (const auto& property : entityNode.entity().properties()) {
    const auto& propertyKey = property.key();
    if (propertyKey.size() >= m_maxLength) {
      issues.push_back(std::make_unique<LongPropertyKeyIssue>(entityNode, propertyKey));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
