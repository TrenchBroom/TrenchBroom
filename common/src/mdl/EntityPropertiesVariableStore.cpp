/*
 Copyright (C) 2010 Kristian Duske

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

#include "EntityPropertiesVariableStore.h"

#include "el/Exceptions.h"
#include "el/Value.h"
#include "mdl/Entity.h"

#include <string>

namespace tb::mdl
{

EntityPropertiesVariableStore::EntityPropertiesVariableStore(const Entity& entity)
  : m_entity{entity}
{
}

el::VariableStore* EntityPropertiesVariableStore::clone() const
{
  return new EntityPropertiesVariableStore{m_entity};
}

size_t EntityPropertiesVariableStore::size() const
{
  return m_entity.properties().size();
}

el::Value EntityPropertiesVariableStore::value(const std::string& name) const
{
  const auto* value = m_entity.property(name);
  return value ? el::Value{*value} : el::Value{""};
}

std::vector<std::string> EntityPropertiesVariableStore::names() const
{
  return m_entity.propertyKeys();
}

void EntityPropertiesVariableStore::set(const std::string, const el::Value) {}

} // namespace tb::mdl
