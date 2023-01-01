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

#include "EL/Expression.h"
#include "EL/Types.h"

// FIXME: try to remove some of these headers
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom
{
namespace EL
{
class NullType
{
private:
  NullType();

public:
  static const NullType Value;
};

class UndefinedType
{
private:
  UndefinedType();

public:
  static const UndefinedType Value;
};

class Value
{
private:
  using VariantType = std::variant<
    BooleanType,
    StringType,
    NumberType,
    ArrayType,
    MapType,
    RangeType,
    NullType,
    UndefinedType>;
  std::shared_ptr<VariantType> m_value;
  std::optional<Expression> m_expression;

public:
  static const Value Null;
  static const Value Undefined;

  Value();

  explicit Value(BooleanType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(StringType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(const char* value, std::optional<Expression> expression = std::nullopt);
  explicit Value(NumberType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(int value, std::optional<Expression> expression = std::nullopt);
  explicit Value(long value, std::optional<Expression> expression = std::nullopt);
  explicit Value(size_t value, std::optional<Expression> expression = std::nullopt);
  explicit Value(ArrayType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(MapType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(RangeType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(NullType value, std::optional<Expression> expression = std::nullopt);
  explicit Value(
    UndefinedType value, std::optional<Expression> expression = std::nullopt);

  Value(Value value, std::optional<Expression> expression);

  ValueType type() const;

  bool hasType(ValueType type) const;

  template <typename... T>
  bool hasType(const T... types) const
  {
    return (... || hasType(types));
  }

  std::string typeName() const;
  std::string describe() const;

  const std::optional<Expression>& expression() const;

  size_t line() const;
  size_t column() const;

  const BooleanType& booleanValue() const;
  const StringType& stringValue() const;
  const NumberType& numberValue() const;
  IntegerType integerValue() const;
  const ArrayType& arrayValue() const;
  const MapType& mapValue() const;
  const RangeType& rangeValue() const;

  const std::vector<std::string> asStringList() const;
  const std::vector<std::string> asStringSet() const;

  size_t length() const;
  bool convertibleTo(ValueType toType) const;
  Value convertTo(ValueType toType) const;
  std::optional<Value> tryConvertTo(ValueType toType) const;

  std::string asString(bool multiline = false) const;
  void appendToStream(
    std::ostream& str, bool multiline = true, const std::string& indent = "") const;

  bool contains(const Value& indexValue) const;
  bool contains(size_t index) const;
  bool contains(const std::string& key) const;
  std::vector<std::string> keys() const;

  Value operator[](const Value& indexValue) const;
  Value operator[](size_t index) const;
  Value operator[](int index) const;
  Value operator[](const std::string& key) const;
  Value operator[](const char* key) const;

  friend bool operator==(const Value& lhs, const Value& rhs);
  friend bool operator!=(const Value& lhs, const Value& rhs);

  friend std::ostream& operator<<(std::ostream& lhs, const Value& rhs);
};
} // namespace EL
} // namespace TrenchBroom
