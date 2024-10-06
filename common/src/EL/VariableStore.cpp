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

#include "EL/ELExceptions.h"
#include "EL/Value.h"

#include "kdl/map_utils.h"

#include <fmt/format.h>

#include <algorithm>
#include <ostream>
#include <string>

namespace tb::EL
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
  return names == rhs.names()
         && std::all_of(std::begin(names), std::end(names), [&](const auto& name) {
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
  return kdl::map_keys(m_variables);
}

void VariableTable::declare(const std::string& name, const Value& value)
{
  if (!m_variables.try_emplace(name, value).second)
  {
    throw EvaluationError{fmt::format("Variable '{}' already declared", name)};
  }
}

void VariableTable::assign(const std::string& name, const Value& value)
{
  if (auto it = m_variables.find(name); it != std::end(m_variables))
  {
    it->second = value;
  }
  throw EvaluationError{fmt::format("Cannot assign to undeclared variable '{}'", name)};
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
  return std::vector<std::string>{};
}

void NullVariableStore::declare(const std::string& /* name */, const Value& /* value */)
{
}
void NullVariableStore::assign(const std::string& /* name */, const Value& /* value */) {}

} // namespace tb::EL
