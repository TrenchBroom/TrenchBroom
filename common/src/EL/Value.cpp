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

#include "Value.h"

#include "EL/ELExceptions.h"

#include "kdl/map_utils.h"
#include "kdl/overload.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"
#include "kdl/vector_set.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>
#include <string>

namespace TrenchBroom::EL
{

NullType::NullType() = default;
const NullType NullType::Value = NullType{};

UndefinedType::UndefinedType() = default;
const UndefinedType UndefinedType::Value = UndefinedType{};

const Value Value::Null = Value{NullType::Value};
const Value Value::Undefined = Value{UndefinedType::Value};

Value::Value()
  : m_value{std::make_shared<VariantType>(NullType::Value)}
{
}

Value::Value(const BooleanType value)
  : m_value{std::make_shared<VariantType>(value)}
{
}

Value::Value(StringType value)
  : m_value{std::make_shared<VariantType>(std::move(value))}
{
}

Value::Value(const char* value)
  : m_value{std::make_shared<VariantType>(StringType(value))}
{
}

Value::Value(const NumberType value)
  : m_value{std::make_shared<VariantType>(value)}
{
}

Value::Value(const int value)
  : m_value{std::make_shared<VariantType>(static_cast<NumberType>(value))}
{
}

Value::Value(const long value)
  : m_value{std::make_shared<VariantType>(static_cast<NumberType>(value))}
{
}

Value::Value(const size_t value)
  : m_value{std::make_shared<VariantType>(static_cast<NumberType>(value))}
{
}

Value::Value(ArrayType value)
  : m_value{std::make_shared<VariantType>(std::move(value))}
{
}

Value::Value(MapType value)
  : m_value{std::make_shared<VariantType>(std::move(value))}
{
}

Value::Value(RangeType value)
  : m_value{std::make_shared<VariantType>(std::move(value))}
{
}

Value::Value(NullType value)
  : m_value{std::make_shared<VariantType>(value)}
{
}

Value::Value(UndefinedType value)
  : m_value{std::make_shared<VariantType>(value)}
{
}

ValueType Value::type() const
{
  return std::visit(
    kdl::overload(
      [](const BooleanType&) { return ValueType::Boolean; },
      [](const StringType&) { return ValueType::String; },
      [](const NumberType&) { return ValueType::Number; },
      [](const ArrayType&) { return ValueType::Array; },
      [](const MapType&) { return ValueType::Map; },
      [](const RangeType&) { return ValueType::Range; },
      [](const NullType&) { return ValueType::Null; },
      [](const UndefinedType&) { return ValueType::Undefined; }),
    *m_value);
}

bool Value::hasType(ValueType type) const
{
  return this->type() == type;
}

std::string Value::typeName() const
{
  return EL::typeName(type());
}

std::string Value::describe() const
{
  return asString(false);
}

const BooleanType& Value::booleanValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType& b) -> const BooleanType& { return b; },
      [&](const StringType&) -> const BooleanType& {
        throw DereferenceError{describe(), type(), ValueType::String};
      },
      [&](const NumberType&) -> const BooleanType& {
        throw DereferenceError{describe(), type(), ValueType::Number};
      },
      [&](const ArrayType&) -> const BooleanType& {
        throw DereferenceError{describe(), type(), ValueType::Array};
      },
      [&](const MapType&) -> const BooleanType& {
        throw DereferenceError{describe(), type(), ValueType::Map};
      },
      [&](const RangeType&) -> const BooleanType& {
        throw DereferenceError{describe(), type(), ValueType::Range};
      },
      [&](const NullType&) -> const BooleanType& {
        static const BooleanType b = false;
        return b;
      },
      [&](const UndefinedType&) -> const BooleanType& {
        throw DereferenceError{describe(), type(), ValueType::Undefined};
      }),
    *m_value);
}

const StringType& Value::stringValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType&) -> const StringType& {
        throw DereferenceError{describe(), type(), ValueType::Boolean};
      },
      [&](const StringType& s) -> const StringType& { return s; },
      [&](const NumberType&) -> const StringType& {
        throw DereferenceError{describe(), type(), ValueType::Number};
      },
      [&](const ArrayType&) -> const StringType& {
        throw DereferenceError{describe(), type(), ValueType::Array};
      },
      [&](const MapType&) -> const StringType& {
        throw DereferenceError{describe(), type(), ValueType::Map};
      },
      [&](const RangeType&) -> const StringType& {
        throw DereferenceError{describe(), type(), ValueType::Range};
      },
      [&](const NullType&) -> const StringType& {
        static const StringType s;
        return s;
      },
      [&](const UndefinedType&) -> const StringType& {
        throw DereferenceError{describe(), type(), ValueType::Undefined};
      }),
    *m_value);
}

const NumberType& Value::numberValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType&) -> const NumberType& {
        throw DereferenceError{describe(), type(), ValueType::Boolean};
      },
      [&](const StringType&) -> const NumberType& {
        throw DereferenceError{describe(), type(), ValueType::String};
      },
      [&](const NumberType& n) -> const NumberType& { return n; },
      [&](const ArrayType&) -> const NumberType& {
        throw DereferenceError{describe(), type(), ValueType::Array};
      },
      [&](const MapType&) -> const NumberType& {
        throw DereferenceError{describe(), type(), ValueType::Map};
      },
      [&](const RangeType&) -> const NumberType& {
        throw DereferenceError{describe(), type(), ValueType::Range};
      },
      [&](const NullType&) -> const NumberType& {
        static const NumberType n = 0.0;
        return n;
      },
      [&](const UndefinedType&) -> const NumberType& {
        throw DereferenceError{describe(), type(), ValueType::Undefined};
      }),
    *m_value);
}

IntegerType Value::integerValue() const
{
  return static_cast<IntegerType>(numberValue());
}

const ArrayType& Value::arrayValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType&) -> const ArrayType& {
        throw DereferenceError{describe(), type(), ValueType::Boolean};
      },
      [&](const StringType&) -> const ArrayType& {
        throw DereferenceError{describe(), type(), ValueType::String};
      },
      [&](const NumberType&) -> const ArrayType& {
        throw DereferenceError{describe(), type(), ValueType::Number};
      },
      [&](const ArrayType& a) -> const ArrayType& { return a; },
      [&](const MapType&) -> const ArrayType& {
        throw DereferenceError{describe(), type(), ValueType::Map};
      },
      [&](const RangeType&) -> const ArrayType& {
        throw DereferenceError{describe(), type(), ValueType::Range};
      },
      [&](const NullType&) -> const ArrayType& {
        static const ArrayType a(0);
        return a;
      },
      [&](const UndefinedType&) -> const ArrayType& {
        throw DereferenceError{describe(), type(), ValueType::Undefined};
      }),
    *m_value);
}

const MapType& Value::mapValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType&) -> const MapType& {
        throw DereferenceError{describe(), type(), ValueType::Boolean};
      },
      [&](const StringType&) -> const MapType& {
        throw DereferenceError{describe(), type(), ValueType::String};
      },
      [&](const NumberType&) -> const MapType& {
        throw DereferenceError{describe(), type(), ValueType::Number};
      },
      [&](const ArrayType&) -> const MapType& {
        throw DereferenceError{describe(), type(), ValueType::Array};
      },
      [&](const MapType& m) -> const MapType& { return m; },
      [&](const RangeType&) -> const MapType& {
        throw DereferenceError{describe(), type(), ValueType::Range};
      },
      [&](const NullType&) -> const MapType& {
        static const MapType m;
        return m;
      },
      [&](const UndefinedType&) -> const MapType& {
        throw DereferenceError{describe(), type(), ValueType::Undefined};
      }),
    *m_value);
}

const RangeType& Value::rangeValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::Boolean};
      },
      [&](const StringType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::String};
      },
      [&](const NumberType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::Number};
      },
      [&](const ArrayType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::Array};
      },
      [&](const MapType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::Map};
      },
      [&](const RangeType& r) -> const RangeType& { return r; },
      [&](const NullType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::Null};
      },
      [&](const UndefinedType&) -> const RangeType& {
        throw DereferenceError{describe(), type(), ValueType::Undefined};
      }),
    *m_value);
}

const std::vector<std::string> Value::asStringList() const
{
  const ArrayType& array = arrayValue();
  auto result = std::vector<std::string>{};
  result.reserve(array.size());

  for (const auto& entry : array)
  {
    result.push_back(entry.convertTo(ValueType::String).stringValue());
  }

  return result;
}

const std::vector<std::string> Value::asStringSet() const
{
  const ArrayType& array = arrayValue();
  auto result = kdl::vector_set<std::string>(array.size());

  for (const auto& entry : array)
  {
    result.insert(entry.convertTo(ValueType::String).stringValue());
  }

  return result.release_data();
}

size_t Value::length() const
{
  return std::visit(
    kdl::overload(
      [](const BooleanType&) -> size_t { return 1u; },
      [](const StringType& s) -> size_t { return s.length(); },
      [](const NumberType&) -> size_t { return 1u; },
      [](const ArrayType& a) -> size_t { return a.size(); },
      [](const MapType& m) -> size_t { return m.size(); },
      [](const RangeType&) -> size_t { return 2u; },
      [](const NullType&) -> size_t { return 0u; },
      [](const UndefinedType&) -> size_t { return 0u; }),
    *m_value);
}

bool Value::convertibleTo(const ValueType toType) const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType&) {
        switch (toType)
        {
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
          return true;
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Undefined:
        case ValueType::Null:
          break;
        }

        return false;
      },
      [&](const StringType& s) {
        switch (toType)
        {
        case ValueType::Boolean:
        case ValueType::String:
          return true;
        case ValueType::Number: {
          if (kdl::str_is_blank(s))
          {
            return true;
          }
          const char* begin = s.c_str();
          char* end;
          const NumberType value = std::strtod(begin, &end);
          if (value == 0.0 && end == begin)
          {
            return false;
          }
          return true;
        }
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const NumberType&) {
        switch (toType)
        {
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
          return true;
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const ArrayType&) {
        switch (toType)
        {
        case ValueType::Array:
          return true;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const MapType&) {
        switch (toType)
        {
        case ValueType::Map:
          return true;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const RangeType&) {
        switch (toType)
        {
        case ValueType::Range:
          return true;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const NullType&) {
        switch (toType)
        {
        case ValueType::Boolean:
        case ValueType::Null:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
          return true;
        case ValueType::Range:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const UndefinedType&) {
        switch (toType)
        {
        case ValueType::Undefined:
          return true;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
          break;
        }

        return false;
      }),
    *m_value);
}

Value Value::convertTo(const ValueType toType) const
{
  return std::visit(
    kdl::overload(
      [&](const BooleanType& b) -> Value {
        switch (toType)
        {
        case ValueType::Boolean:
          return *this;
        case ValueType::String:
          return Value{b ? "true" : "false"};
        case ValueType::Number:
          return Value{b ? 1.0 : 0.0};
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Undefined:
        case ValueType::Null:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const StringType& s) -> Value {
        switch (toType)
        {
        case ValueType::Boolean:
          return Value{!kdl::cs::str_is_equal(s, "false") && !s.empty()};
        case ValueType::String:
          return *this;
        case ValueType::Number: {
          if (kdl::str_is_blank(s))
          {
            return Value{0.0};
          }
          const char* begin = s.c_str();
          char* end;
          const NumberType value = std::strtod(begin, &end);
          if (value == 0.0 && end == begin)
          {
            throw ConversionError{describe(), type(), toType};
          }
          return Value{value};
        }
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const NumberType& n) -> Value {
        switch (toType)
        {
        case ValueType::Boolean:
          return Value{n != 0.0};
        case ValueType::String:
          return Value{describe()};
        case ValueType::Number:
          return *this;
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const ArrayType&) -> Value {
        switch (toType)
        {
        case ValueType::Array:
          return *this;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const MapType&) -> Value {
        switch (toType)
        {
        case ValueType::Map:
          return *this;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Range:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const RangeType&) -> Value {
        switch (toType)
        {
        case ValueType::Range:
          return *this;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const NullType&) -> Value {
        switch (toType)
        {
        case ValueType::Boolean:
          return Value{false};
        case ValueType::Null:
          return *this;
        case ValueType::Number:
          return Value{0.0};
        case ValueType::String:
          return Value{""};
        case ValueType::Array:
          return Value{ArrayType{0}};
        case ValueType::Map:
          return Value{MapType{}};
        case ValueType::Range:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      },
      [&](const UndefinedType&) -> Value {
        switch (toType)
        {
        case ValueType::Undefined:
          return *this;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Null:
          break;
        }

        throw ConversionError{describe(), type(), toType};
      }),
    *m_value);
}

std::optional<Value> Value::tryConvertTo(const ValueType toType) const
{
  try
  {
    return convertTo(toType);
  }
  catch (const ConversionError&)
  {
    return std::nullopt;
  }
}

std::string Value::asString(const bool multiline) const
{
  std::stringstream str;
  appendToStream(str, multiline);
  return str.str();
}

void Value::appendToStream(
  std::ostream& str, const bool multiline, const std::string& indent) const
{
  std::visit(
    kdl::overload(
      [&](const BooleanType& b) { str << (b ? "true" : "false"); },
      [&](const StringType& s) {
        // Unescaping happens in IO::ELParser::parseLiteral
        str << "\"" << kdl::str_escape(s, "\\\"") << "\"";
      },
      [&](const NumberType& n) {
        static constexpr auto RoundingThreshold = 0.00001;
        if (std::abs(n - std::round(n)) < RoundingThreshold)
        {
          str.precision(0);
          str.setf(std::ios::fixed);
        }
        else
        {
          str.precision(17);
          str.unsetf(std::ios::fixed);
        }
        str << n;
      },
      [&](const ArrayType& a) {
        if (a.empty())
        {
          str << "[]";
        }
        else
        {
          const std::string childIndent = multiline ? indent + "\t" : "";
          str << "[";
          if (multiline)
          {
            str << "\n";
          }
          else
          {
            str << " ";
          }
          for (size_t i = 0; i < a.size(); ++i)
          {
            str << childIndent;
            a[i].appendToStream(str, multiline, childIndent);
            if (i < a.size() - 1)
            {
              str << ",";
              if (!multiline)
              {
                str << " ";
              }
            }
            if (multiline)
            {
              str << "\n";
            }
          }
          if (multiline)
          {
            str << indent;
          }
          else
          {
            str << " ";
          }
          str << "]";
        }
      },
      [&](const MapType& m) {
        if (m.empty())
        {
          str << "{}";
        }
        else
        {
          const std::string childIndent = multiline ? indent + "\t" : "";
          str << "{";
          if (multiline)
          {
            str << "\n";
          }
          else
          {
            str << " ";
          }

          size_t i = 0;
          for (const auto& [key, value] : m)
          {
            str << childIndent << "\"" << key << "\""
                << ": ";
            value.appendToStream(str, multiline, childIndent);
            if (i++ < m.size() - 1)
            {
              str << ",";
              if (!multiline)
              {
                str << " ";
              }
            }
            if (multiline)
            {
              str << "\n";
            }
          }
          if (multiline)
          {
            str << indent;
          }
          else
          {
            str << " ";
          }
          str << "}";
        }
      },
      [&](const RangeType& r) {
        str << "[";
        std::visit(
          kdl::overload(
            [&](const LeftBoundedRange& lbr) { str << lbr.first << ".."; },
            [&](const RightBoundedRange& rbr) { str << ".." << rbr.last; },
            [&](const BoundedRange& br) { str << br.first << ".." << br.last; }),
          r);
        str << "]";
      },
      [&](const NullType&) { str << "null"; },
      [&](const UndefinedType&) { str << "undefined"; }),
    *m_value);
}

namespace
{

size_t computeIndex(const long index, const size_t indexableSize)
{
  const auto size = static_cast<long>(indexableSize);
  return (index >= 0 && index < size) || (index < 0 && index >= -size)
           ? static_cast<size_t>((size + index % size) % size)
           : static_cast<size_t>(size);
}

size_t computeIndex(const Value& indexValue, const size_t indexableSize)
{
  return computeIndex(
    static_cast<long>(indexValue.convertTo(ValueType::Number).numberValue()),
    indexableSize);
}

void computeIndexArray(
  const LeftBoundedRange& range, const size_t indexableSize, std::vector<size_t>& result)
{
  result.reserve(result.size() + range.length(indexableSize));
  range.forEach(
    [&](const auto i) { result.push_back(computeIndex(i, indexableSize)); },
    indexableSize);
}

void computeIndexArray(
  const RightBoundedRange& range, const size_t indexableSize, std::vector<size_t>& result)
{
  result.reserve(result.size() + range.length(indexableSize));
  range.forEach(
    [&](const auto i) { result.push_back(computeIndex(i, indexableSize)); },
    indexableSize);
}

void computeIndexArray(
  const BoundedRange& range, const size_t indexableSize, std::vector<size_t>& result)
{
  result.reserve(result.size() + range.length());
  range.forEach([&](const auto i) { result.push_back(computeIndex(i, indexableSize)); });
}

void computeIndexArray(
  const Value& indexValue, const size_t indexableSize, std::vector<size_t>& result)
{
  switch (indexValue.type())
  {
  case ValueType::Array: {
    const ArrayType& indexArray = indexValue.arrayValue();
    result.reserve(result.size() + indexArray.size());
    for (size_t i = 0; i < indexArray.size(); ++i)
    {
      computeIndexArray(indexArray[i], indexableSize, result);
    }
    break;
  }
  case ValueType::Range: {
    std::visit(
      [&](const auto& x) { computeIndexArray(x, indexableSize, result); },
      indexValue.rangeValue());
    break;
  }
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::String:
  case ValueType::Map:
  case ValueType::Null:
  case ValueType::Undefined:
    result.push_back(computeIndex(indexValue, indexableSize));
    break;
  }
}

std::vector<size_t> computeIndexArray(const Value& indexValue, const size_t indexableSize)
{
  auto result = std::vector<size_t>{};
  computeIndexArray(indexValue, indexableSize, result);
  return result;
}

} // namespace

bool Value::contains(const Value& indexValue) const
{
  switch (type())
  {
  case ValueType::String: {
    switch (indexValue.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const size_t index = computeIndex(indexValue, length());
      return index < length();
    }
    case ValueType::Array:
    case ValueType::Range: {
      const std::vector<size_t> indices = computeIndexArray(indexValue, length());
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const size_t index = indices[i];
        if (index >= length())
        {
          return false;
        }
      }
      return true;
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  }
  case ValueType::Array:
    switch (indexValue.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const size_t index = computeIndex(indexValue, length());
      return index < length();
    }
    case ValueType::Array:
    case ValueType::Range: {
      const std::vector<size_t> indices = computeIndexArray(indexValue, length());
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const size_t index = indices[i];
        if (index >= length())
        {
          return false;
        }
      }
      return true;
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Map:
    switch (indexValue.type())
    {
    case ValueType::String: {
      const MapType& map = mapValue();
      const std::string& key = indexValue.stringValue();
      const auto it = map.find(key);
      return it != std::end(map);
    }
    case ValueType::Array: {
      const MapType& map = mapValue();
      const ArrayType& keys = indexValue.arrayValue();
      for (size_t i = 0; i < keys.size(); ++i)
      {
        const Value& keyValue = keys[i];
        if (keyValue.type() != ValueType::String)
        {
          throw ConversionError{keyValue.describe(), keyValue.type(), ValueType::String};
        }
        const std::string& key = keyValue.stringValue();
        const auto it = map.find(key);
        if (it == std::end(map))
        {
          return false;
        }
      }
      return true;
    }
    case ValueType::Boolean:
    case ValueType::Number:
    case ValueType::Map:
    case ValueType::Range:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  return false;
}

bool Value::contains(const size_t index) const
{
  switch (type())
  {
  case ValueType::String:
  case ValueType::Array:
    return index < length();
  case ValueType::Map:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  return false;
}

bool Value::contains(const std::string& key) const
{
  const MapType& map = mapValue();
  const auto it = map.find(key);
  return it != std::end(map);
}

std::vector<std::string> Value::keys() const
{
  return kdl::map_keys(mapValue());
}

Value Value::operator[](const Value& indexValue) const
{
  switch (type())
  {
  case ValueType::String:
    switch (indexValue.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const StringType& str = stringValue();
      const size_t index = computeIndex(indexValue, str.length());
      std::stringstream result;
      if (index < str.length())
      {
        result << str[index];
      }
      return Value{result.str()};
    }
    case ValueType::Array:
    case ValueType::Range: {
      const StringType& str = stringValue();
      const std::vector<size_t> indices = computeIndexArray(indexValue, str.length());
      std::stringstream result;
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const size_t index = indices[i];
        if (index < str.length())
        {
          result << str[index];
        }
      }
      return Value{result.str()};
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Array:
    switch (indexValue.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const ArrayType& array = arrayValue();
      const size_t index = computeIndex(indexValue, array.size());
      if (index >= array.size())
      {
        throw IndexOutOfBoundsError{*this, indexValue, index};
      }
      return array[index];
    }
    case ValueType::Array:
    case ValueType::Range: {
      const ArrayType& array = arrayValue();
      const std::vector<size_t> indices = computeIndexArray(indexValue, array.size());
      auto result = ArrayType{};
      result.reserve(indices.size());
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const size_t index = indices[i];
        if (index >= array.size())
        {
          throw IndexOutOfBoundsError{*this, indexValue, index};
        }
        result.push_back(array[index]);
      }
      return Value{std::move(result)};
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Map:
    switch (indexValue.type())
    {
    case ValueType::String: {
      const MapType& map = mapValue();
      const std::string& key = indexValue.stringValue();
      const auto it = map.find(key);
      if (it == std::end(map))
      {
        return Value{UndefinedType::Value};
      }
      return it->second;
    }
    case ValueType::Array: {
      const MapType& map = mapValue();
      const ArrayType& keys = indexValue.arrayValue();
      MapType result;
      for (size_t i = 0; i < keys.size(); ++i)
      {
        const Value& keyValue = keys[i];
        if (keyValue.type() != ValueType::String)
        {
          throw ConversionError{keyValue.describe(), keyValue.type(), ValueType::String};
        }
        const std::string& key = keyValue.stringValue();
        const auto it = map.find(key);
        if (it != std::end(map))
        {
          result.insert(std::make_pair(key, it->second));
        }
      }
      return Value{std::move(result)};
    }
    case ValueType::Boolean:
    case ValueType::Number:
    case ValueType::Map:
    case ValueType::Range:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{*this, indexValue};
}

Value Value::operator[](const size_t index) const
{
  switch (type())
  {
  case ValueType::String: {
    const StringType& str = stringValue();
    std::stringstream result;
    if (index < str.length())
    {
      result << str[index];
    }
    return Value{result.str()};
  }
  case ValueType::Array: {
    const ArrayType& array = arrayValue();
    if (index >= array.size())
    {
      throw IndexOutOfBoundsError{*this, index};
    }
    return array[index];
  }
  case ValueType::Map:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{*this, index};
}

Value Value::operator[](const int index) const
{
  assert(index >= 0);
  return this->operator[](static_cast<size_t>(index));
}

Value Value::operator[](const std::string& key) const
{
  return this->operator[](key.c_str());
}

Value Value::operator[](const char* key) const
{
  switch (type())
  {
  case ValueType::Map: {
    const MapType& map = mapValue();
    const auto it = map.find(key);
    if (it == std::end(map))
    {
      return Value{NullType::Value};
    }
    else
    {
      return it->second;
    }
  }
  case ValueType::String:
  case ValueType::Array:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{*this, key};
}

bool operator==(const Value& lhs, const Value& rhs)
{
  return lhs.m_value == rhs.m_value
         || std::visit(
           kdl::overload(
             [](const BooleanType& lhsBool, const BooleanType& rhsBool) {
               return lhsBool == rhsBool;
             },
             [](const StringType& lhsString, const StringType& rhsString) {
               return lhsString == rhsString;
             },
             [](const NumberType& lhsNumber, const NumberType& rhsNumber) {
               return lhsNumber == rhsNumber;
             },
             [](const ArrayType& lhsArray, const ArrayType& rhsArray) {
               return lhsArray == rhsArray;
             },
             [](
               const MapType& lhsMap, const MapType& rhsMap) { return lhsMap == rhsMap; },
             [](const RangeType& lhsRange, const RangeType& rhsRange) {
               return lhsRange == rhsRange;
             },
             [](const NullType&, const NullType&) { return true; },
             [](const UndefinedType&, const UndefinedType&) { return true; },
             [](const auto&, const auto&) { return false; }),
           *lhs.m_value,
           *rhs.m_value);
}

bool operator!=(const Value& lhs, const Value& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const Value& rhs)
{
  rhs.appendToStream(lhs);
  return lhs;
}

} // namespace TrenchBroom::EL
