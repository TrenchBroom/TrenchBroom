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

#pragma once

#include "ExpressionNode.h"
#include "Types.h"
#include "base/FileLocation.h"

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace tb::el
{

class Value
{
private:
  using VariantType = std::variant<
    BooleanType,
    std::shared_ptr<const StringType>,
    NumberType,
    std::shared_ptr<const ArrayType>,
    std::shared_ptr<const MapType>,
    RangeType,
    Vec3Type,
    BBoxType,
    std::shared_ptr<const LazyMapType>,
    NullType,
    UndefinedType>;
  VariantType m_value;

  std::shared_ptr<Expression> m_producedByExpression;
  std::optional<FileLocation> m_producedByLocation;

public:
  static const Value Null;
  static const Value Undefined;

  Value();

  explicit Value(BooleanType value);
  explicit Value(StringType value);
  explicit Value(const char* value);
  explicit Value(NumberType value);
  explicit Value(int value);
  explicit Value(long value);
  explicit Value(size_t value);
  explicit Value(ArrayType value);
  explicit Value(MapType value);
  explicit Value(RangeType value);
  explicit Value(Vec3Type value);
  explicit Value(BBoxType value);
  explicit Value(LazyMapType value);
  explicit Value(NullType value);
  explicit Value(UndefinedType value);

  ValueType type() const;

  bool hasType(ValueType type) const;

  template <typename... T>
  bool hasType(const T... types) const
  {
    return (... || hasType(types));
  }

  std::string typeName() const;
  std::string describe() const;

  const BooleanType& booleanValue() const;
  const StringType& stringValue() const;
  const NumberType& numberValue() const;
  IntegerType integerValue() const;
  const ArrayType& arrayValue() const;
  const MapType& mapValue() const;
  const RangeType& rangeValue() const;
  const Vec3Type& vec3Value() const;
  const BBoxType& bboxValue() const;
  const LazyMapType& lazyMapValue() const;

  std::vector<std::string> asStringList() const;
  std::vector<std::string> asStringSet() const;

  size_t length() const;
  bool convertibleTo(ValueType toType) const;
  Value convertTo(ValueType toType) const;
  std::optional<Value> tryConvertTo(ValueType toType) const;

  std::string asString(bool multiline = false) const;
  void appendToStream(
    std::ostream& str, bool multiline = true, const std::string& indent = "") const;

  bool contains(size_t index) const;
  bool contains(const std::string& key) const;

  std::vector<std::string> keys() const;

  Value at(size_t index) const;
  Value atOrDefault(size_t index, Value defaultValue = Null) const;

  Value at(const std::string& key) const;
  Value atOrDefault(const std::string& key, Value defaultValue = Null) const;

  std::optional<ExpressionNode> expression() const;
  std::optional<FileLocation> location() const;

  Value producedBy(const ExpressionNode& expressionNode) const;
  Value producedBy(const Value& original) const;

  friend bool operator==(const Value& lhs, const Value& rhs);
  friend bool operator!=(const Value& lhs, const Value& rhs);

  friend std::ostream& operator<<(std::ostream& lhs, const Value& rhs);
};

} // namespace tb::el
