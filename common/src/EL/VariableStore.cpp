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

#include "VariableStore.h"

#include "EL/ELExceptions.h"
#include "EL/Value.h"

#include <kdl/map_utils.h>

#include <string>

namespace TrenchBroom {
namespace EL {
VariableStore* VariableStore::clone() const {
  return doClone();
}

size_t VariableStore::size() const {
  return doGetSize();
}

Value VariableStore::value(const std::string& name) const {
  return doGetValue(name);
}

const std::vector<std::string> VariableStore::names() const {
  return doGetNames();
}

void VariableStore::declare(const std::string& name, const Value& value) {
  doDeclare(name, value);
}

void VariableStore::assign(const std::string& name, const Value& value) {
  doAssign(name, value);
}

VariableTable::VariableTable() = default;

VariableTable::VariableTable(Table variables)
  : m_variables(std::move(variables)) {}

VariableStore* VariableTable::doClone() const {
  return new VariableTable(m_variables);
}

size_t VariableTable::doGetSize() const {
  return m_variables.size();
}

Value VariableTable::doGetValue(const std::string& name) const {
  auto it = m_variables.find(name);
  if (it != std::end(m_variables)) {
    return it->second;
  } else {
    return Value::Undefined;
  }
}

std::vector<std::string> VariableTable::doGetNames() const {
  return kdl::map_keys(m_variables);
}

void VariableTable::doDeclare(const std::string& name, const Value& value) {
  if (!m_variables.try_emplace(name, value).second) {
    throw EvaluationError("Variable '" + name + "' already declared");
  }
}

void VariableTable::doAssign(const std::string& name, const Value& value) {
  auto it = m_variables.find(name);
  if (it == std::end(m_variables)) {
    throw EvaluationError("Cannot assign to undeclared variable '" + name + "'");
  } else {
    it->second = value;
  }
}

NullVariableStore::NullVariableStore() = default;

VariableStore* NullVariableStore::doClone() const {
  return new NullVariableStore();
}

size_t NullVariableStore::doGetSize() const {
  return 0;
}

Value NullVariableStore::doGetValue(const std::string& /* name */) const {
  return Value::Null;
}

std::vector<std::string> NullVariableStore::doGetNames() const {
  return std::vector<std::string>();
}

void NullVariableStore::doDeclare(const std::string& /* name */, const Value& /* value */) {}
void NullVariableStore::doAssign(const std::string& /* name */, const Value& /* value */) {}
} // namespace EL
} // namespace TrenchBroom
