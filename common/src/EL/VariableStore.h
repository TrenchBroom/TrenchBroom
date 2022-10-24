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

#pragma once

#include "EL/Value.h" // required by VariableTable::Table declaration

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace EL
{
class VariableStore
{
public:
  VariableStore() = default;
  VariableStore(const VariableStore& other) = default;
  VariableStore(VariableStore&& other) noexcept = default;
  virtual ~VariableStore() = default;

  VariableStore& operator=(const VariableStore& other) = default;
  VariableStore& operator=(VariableStore&& other) noexcept = default;

  virtual VariableStore* clone() const = 0;
  virtual size_t size() const = 0;
  virtual Value value(const std::string& name) const = 0;
  virtual std::vector<std::string> names() const = 0;
  virtual void declare(const std::string& name, const Value& value) = 0;
  virtual void assign(const std::string& name, const Value& value) = 0;
  virtual void appendToStream(std::ostream& str) const;
};

std::ostream& operator<<(std::ostream& lhs, const VariableStore& rhs);
bool operator==(const VariableStore& lhs, const VariableStore& rhs);
bool operator!=(const VariableStore& lhs, const VariableStore& rhs);

class VariableTable : public VariableStore
{
private:
  using Table = std::map<std::string, Value>;
  Table m_variables;

public:
  VariableTable();
  explicit VariableTable(Table variables);

  VariableStore* clone() const override;
  size_t size() const override;
  Value value(const std::string& name) const override;
  std::vector<std::string> names() const override;
  void declare(const std::string& name, const Value& value) override;
  void assign(const std::string& name, const Value& value) override;
};

class NullVariableStore : public VariableStore
{
public:
  NullVariableStore();

  VariableStore* clone() const override;
  size_t size() const override;
  Value value(const std::string& name) const override;
  std::vector<std::string> names() const override;
  void declare(const std::string& name, const Value& value) override;
  void assign(const std::string& name, const Value& value) override;
};
} // namespace EL
} // namespace TrenchBroom
