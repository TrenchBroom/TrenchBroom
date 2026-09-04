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

#include "el/LazyMapVariableStore.h"

#include <utility>

namespace tb::el
{

LazyMapVariableStore::LazyMapVariableStore(LazyMap lazyMap)
  : m_lazyMap{std::move(lazyMap)}
{
}

VariableStore* LazyMapVariableStore::clone() const
{
  return new LazyMapVariableStore{m_lazyMap};
}

size_t LazyMapVariableStore::size() const
{
  return m_lazyMap.keys().size();
}

Value LazyMapVariableStore::value(const std::string& name) const
{
  return m_lazyMap.at(name).value_or(Value::Undefined);
}

std::vector<std::string> LazyMapVariableStore::names() const
{
  return m_lazyMap.keys();
}

void LazyMapVariableStore::set(std::string, Value) {}

} // namespace tb::el
