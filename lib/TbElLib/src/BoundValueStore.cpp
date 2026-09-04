/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/BoundValueStore.h"

#include <utility>

namespace tb::el
{

BoundValueStore::BoundValueStore(BoundValue boundValue)
  : m_boundValue{std::move(boundValue)}
{
}

VariableStore* BoundValueStore::clone() const
{
  return new BoundValueStore{m_boundValue};
}

size_t BoundValueStore::size() const
{
  return m_boundValue.keys().size();
}

Value BoundValueStore::value(const std::string& name) const
{
  return m_boundValue.at(name).value_or(Value::Undefined);
}

std::vector<std::string> BoundValueStore::names() const
{
  return m_boundValue.keys();
}

void BoundValueStore::set(std::string, Value) {}

} // namespace tb::el
