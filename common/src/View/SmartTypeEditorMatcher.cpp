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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SmartTypeEditorMatcher.h"

#include "Assets/PropertyDefinition.h"
#include "Model/EntityNodeBase.h"

namespace TrenchBroom
{
namespace View
{
// SmartTypeEditorMatcher

SmartTypeEditorMatcher::SmartTypeEditorMatcher(const Assets::PropertyDefinitionType type)
  : m_type(type)
{
}

bool SmartTypeEditorMatcher::doMatches(
  const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const
{
  if (nodes.empty())
  {
    return false;
  }
  for (const auto* node : nodes)
  {
    const auto* propDef = Model::propertyDefinition(node, propertyKey);
    if (propDef == nullptr)
    {
      return false;
    }
    if (propDef->type() != m_type)
    {
      return false;
    }
  }
  return true;
}

// SmartTypeWithSameDefinitionEditorMatcher

SmartTypeWithSameDefinitionEditorMatcher::SmartTypeWithSameDefinitionEditorMatcher(
  const Assets::PropertyDefinitionType type)
  : m_type(type)
{
}

bool SmartTypeWithSameDefinitionEditorMatcher::doMatches(
  const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const
{
  const Assets::PropertyDefinition* propDef =
    Model::selectPropertyDefinition(propertyKey, nodes);
  return propDef != nullptr && propDef->type() == m_type;
}
} // namespace View
} // namespace TrenchBroom
