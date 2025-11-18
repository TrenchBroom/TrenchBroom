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

#include "VariableStore.h"

#include "el/ELExceptions.h"
#include "el/Value.h"

#include "kd/ranges/to.h"

#include <fmt/format.h>

#include <algorithm>
#include <ostream>
#include <ranges>
#include <string>

namespace tb::el
{

void VariableStore::appendToStream(std::ostream& str) const
{
  str << "{\n";

  const auto& allNames = names();
  for (size_t i = 0; i < allNames.size(); ++i)
  {
    const auto& name = allNames[i];
    str << "  " << name << ": " << value(name);
    if (i < allNames.size() - 1)
    {
      str << ", ";
    }
  }

  str << "}";
}

std::ostream& operator<<(std::ostream& lhs, const VariableStore& rhs)
{
  rhs.appendToStream(lhs);
  return lhs;
}

bool operator==(const VariableStore& lhs, const VariableStore& rhs)
{
  const auto names = lhs.names();
  return names == rhs.names() && std::ranges::all_of(names, [&](const auto& name) {
           return lhs.value(name) == rhs.value(name);
         });
}

bool operator!=(const VariableStore& lhs, const VariableStore& rhs)
{
  return !(lhs == rhs);
}

VariableTable::VariableTable() = default;

VariableTable::VariableTable(Table variables)
  : m_variables{std::move(variables)}
{
}

VariableStore* VariableTable::clone() const
{
  return new VariableTable(m_variables);
}

size_t VariableTable::size() const
{
  return m_variables.size();
}

Value VariableTable::value(const std::string& name) const
{
  if (const auto it = m_variables.find(name); it != std::end(m_variables))
  {
    return it->second;
  }
  return Value::Undefined;
}

std::vector<std::string> VariableTable::names() const
{
  return m_variables | std::views::keys | kdl::ranges::to<std::vector>();
}

void VariableTable::set(std::string name, Value value)
{
  m_variables[std::move(name)] = std::move(value);
}

NullVariableStore::NullVariableStore() = default;

VariableStore* NullVariableStore::clone() const
{
  return new NullVariableStore{};
}

size_t NullVariableStore::size() const
{
  return 0;
}

Value NullVariableStore::value(const std::string& /* name */) const
{
  return Value::Null;
}

std::vector<std::string> NullVariableStore::names() const
{
  return {};
}

void NullVariableStore::set(const std::string, const Value) {}

} // namespace tb::el
