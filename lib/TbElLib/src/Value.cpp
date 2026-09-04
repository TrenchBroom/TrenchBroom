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

#include "el/Value.h"

#include "el/Exceptions.h"
#include "el/Expression.h" // IWYU pragma: keep

#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/string_compare.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

#include "vm/vec_io.h"

#include <cmath>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>

namespace tb::el
{
namespace
{

void appendNumber(std::ostream& str, const NumberType n)
{
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
}

} // namespace

NullType::NullType() = default;
const NullType NullType::Value = NullType{};

UndefinedType::UndefinedType() = default;
const UndefinedType UndefinedType::Value = UndefinedType{};

const Value Value::Null = Value{NullType::Value};
const Value Value::Undefined = Value{UndefinedType::Value};

Value::Value()
  : m_value{NullType::Value}
{
}

Value::Value(const BooleanType value)
  : m_value{value}
{
}

Value::Value(StringType value)
  : m_value{std::make_shared<const StringType>(std::move(value))}
{
}

Value::Value(const char* value)
  : m_value{std::make_shared<const StringType>(value)}
{
}

Value::Value(const NumberType value)
  : m_value{value}
{
}

Value::Value(const int value)
  : m_value{static_cast<NumberType>(value)}
{
}

Value::Value(const long value)
  : m_value{static_cast<NumberType>(value)}
{
}

Value::Value(const size_t value)
  : m_value{static_cast<NumberType>(value)}
{
}

Value::Value(ArrayType value)
  : m_value{std::make_shared<const ArrayType>(std::move(value))}
{
}

Value::Value(MapType value)
  : m_value{std::make_shared<const MapType>(std::move(value))}
{
}

Value::Value(RangeType value)
  : m_value{std::move(value)}
{
}

Value::Value(Vec3Type value)
  : m_value{value}
{
}

Value::Value(BBoxType value)
  : m_value{value}
{
}

Value::Value(LazyMapType value)
  : m_value{std::make_shared<const LazyMapType>(std::move(value))}
{
}

Value::Value(NullType value)
  : m_value{value}
{
}

Value::Value(UndefinedType value)
  : m_value{value}
{
}

ValueType Value::type() const
{
  return std::visit(
    kdl::overload(
      [](const BooleanType&) { return ValueType::Boolean; },
      [](const std::shared_ptr<const StringType>&) { return ValueType::String; },
      [](const NumberType&) { return ValueType::Number; },
      [](const std::shared_ptr<const ArrayType>&) { return ValueType::Array; },
      [](const std::shared_ptr<const MapType>&) { return ValueType::Map; },
      [](const RangeType&) { return ValueType::Range; },
      [](const Vec3Type&) { return ValueType::Vec3; },
      [](const BBoxType&) { return ValueType::BBox; },
      [](const std::shared_ptr<const LazyMapType>&) { return ValueType::LazyMap; },
      [](const NullType&) { return ValueType::Null; },
      [](const UndefinedType&) { return ValueType::Undefined; }),
    m_value);
}

bool Value::hasType(ValueType type) const
{
  return this->type() == type;
}

std::string Value::typeName() const
{
  return el::typeName(type());
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
      [&](const NullType&) -> const BooleanType& {
        static const BooleanType b = false;
        return b;
      },
      [&](const auto&) -> const BooleanType& {
        throw DereferenceError{location(), describe(), type(), ValueType::Boolean};
      }),
    m_value);
}

const StringType& Value::stringValue() const
{
  return std::visit(
    kdl::overload(
      [&](const std::shared_ptr<const StringType>& s) -> const StringType& { return *s; },
      [&](const NullType&) -> const StringType& {
        static const StringType s;
        return s;
      },
      [&](const auto&) -> const StringType& {
        throw DereferenceError{location(), describe(), type(), ValueType::String};
      }),
    m_value);
}

const NumberType& Value::numberValue() const
{
  return std::visit(
    kdl::overload(
      [&](const NumberType& n) -> const NumberType& { return n; },
      [&](const NullType&) -> const NumberType& {
        static const NumberType n = 0.0;
        return n;
      },
      [&](const auto&) -> const NumberType& {
        throw DereferenceError{location(), describe(), type(), ValueType::Number};
      }),
    m_value);
}

IntegerType Value::integerValue() const
{
  return static_cast<IntegerType>(numberValue());
}

const ArrayType& Value::arrayValue() const
{
  return std::visit(
    kdl::overload(
      [&](const std::shared_ptr<const ArrayType>& a) -> const ArrayType& { return *a; },
      [&](const NullType&) -> const ArrayType& {
        static const ArrayType a(0);
        return a;
      },
      [&](const auto&) -> const ArrayType& {
        throw DereferenceError{location(), describe(), type(), ValueType::Array};
      }),
    m_value);
}

const MapType& Value::mapValue() const
{
  return std::visit(
    kdl::overload(
      [&](const std::shared_ptr<const MapType>& m) -> const MapType& { return *m; },
      [&](const NullType&) -> const MapType& {
        static const MapType m;
        return m;
      },
      [&](const auto&) -> const MapType& {
        throw DereferenceError{location(), describe(), type(), ValueType::Map};
      }),
    m_value);
}

const RangeType& Value::rangeValue() const
{
  return std::visit(
    kdl::overload(
      [&](const RangeType& r) -> const RangeType& { return r; },
      [&](const auto&) -> const RangeType& {
        throw DereferenceError{location(), describe(), type(), ValueType::Range};
      }),
    m_value);
}

const Vec3Type& Value::vec3Value() const
{
  return std::visit(
    kdl::overload(
      [&](const Vec3Type& v) -> const Vec3Type& { return v; },
      [&](const auto&) -> const Vec3Type& {
        throw DereferenceError{location(), describe(), type(), ValueType::Vec3};
      }),
    m_value);
}

const BBoxType& Value::bboxValue() const
{
  return std::visit(
    kdl::overload(
      [&](const BBoxType& b) -> const BBoxType& { return b; },
      [&](const auto&) -> const BBoxType& {
        throw DereferenceError{location(), describe(), type(), ValueType::BBox};
      }),
    m_value);
}

const LazyMapType& Value::lazyMapValue() const
{
  return std::visit(
    kdl::overload(
      [&](const std::shared_ptr<const LazyMapType>& b) -> const LazyMapType& {
        return *b;
      },
      [&](const auto&) -> const LazyMapType& {
        throw DereferenceError{location(), describe(), type(), ValueType::LazyMap};
      }),
    m_value);
}

std::vector<std::string> Value::asStringList() const
{
  return arrayValue()
         | std::views::transform([&](const auto& entry) { return entry.stringValue(); })
         | kdl::ranges::to<std::vector>();
}

std::vector<std::string> Value::asStringSet() const
{
  return kdl::vec_sort_and_remove_duplicates(asStringList());
}

size_t Value::length() const
{
  return std::visit(
    kdl::overload(
      [](const BooleanType&) -> size_t { return 1u; },
      [](const std::shared_ptr<const StringType>& s) -> size_t { return s->length(); },
      [](const NumberType&) -> size_t { return 1u; },
      [](const std::shared_ptr<const ArrayType>& a) -> size_t { return a->size(); },
      [](const std::shared_ptr<const MapType>& m) -> size_t { return m->size(); },
      [](const RangeType&) -> size_t { return 2u; },
      [](const Vec3Type&) -> size_t { return 3u; },
      [](const BBoxType&) -> size_t { return 2u; },
      [](const std::shared_ptr<const LazyMapType>& b) -> size_t {
        return b->keys().size();
      },
      [](const NullType&) -> size_t { return 0u; },
      [](const UndefinedType&) -> size_t { return 0u; }),
    m_value);
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Undefined:
        case ValueType::Null:
          break;
        }

        return false;
      },
      [&](const std::shared_ptr<const StringType>& sp) {
        const auto& s = *sp;
        switch (toType)
        {
        case ValueType::Boolean:
        case ValueType::String:
          return true;
        case ValueType::Number:
          return kdl::str_is_blank(s) || kdl::str_to_double(s) != std::nullopt;
        case ValueType::Vec3:
          return vm::parse<double, 3>(s).has_value();
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::BBox:
        case ValueType::LazyMap:
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const std::shared_ptr<const ArrayType>&) {
        switch (toType)
        {
        case ValueType::Array:
          return true;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const std::shared_ptr<const MapType>&) {
        switch (toType)
        {
        case ValueType::Map:
          return true;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const Vec3Type&) {
        switch (toType)
        {
        case ValueType::Vec3:
        case ValueType::String:
          return true;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const BBoxType&) {
        switch (toType)
        {
        case ValueType::BBox:
          return true;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        return false;
      },
      [&](const std::shared_ptr<const LazyMapType>&) {
        switch (toType)
        {
        case ValueType::LazyMap:
          return true;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
          break;
        }

        return false;
      }),
    m_value);
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
          return Value{b ? "true" : "false"}.producedBy(*this);
        case ValueType::Number:
          return Value{b ? 1.0 : 0.0}.producedBy(*this);
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Undefined:
        case ValueType::Null:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const std::shared_ptr<const StringType>& sp) -> Value {
        const auto& s = *sp;
        switch (toType)
        {
        case ValueType::Boolean:
          return Value{!kdl::cs::str_is_equal(s, "false") && !s.empty()}.producedBy(
            *this);
        case ValueType::String:
          return *this;
        case ValueType::Number: {
          if (kdl::str_is_blank(s))
          {
            return Value{0.0}.producedBy(*this);
          }
          if (const auto x = kdl::str_to_double(s))
          {
            return Value{*x}.producedBy(*this);
          }
          throw ConversionError{location(), describe(), type(), toType};
        }
        case ValueType::Vec3: {
          if (const auto v = vm::parse<double, 3>(s))
          {
            return Value{*v}.producedBy(*this);
          }
          throw ConversionError{location(), describe(), type(), toType};
        }
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const NumberType& n) -> Value {
        switch (toType)
        {
        case ValueType::Boolean:
          return Value{n != 0.0}.producedBy(*this);
        case ValueType::String:
          return Value{describe()}.producedBy(*this);
        case ValueType::Number:
          return *this;
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const std::shared_ptr<const ArrayType>&) -> Value {
        switch (toType)
        {
        case ValueType::Array:
          return *this;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const std::shared_ptr<const MapType>&) -> Value {
        switch (toType)
        {
        case ValueType::Map:
          return *this;
        case ValueType::Boolean:
        case ValueType::String:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const Vec3Type& v) -> Value {
        switch (toType)
        {
        case ValueType::Vec3:
          return *this;
        case ValueType::String: {
          auto str = std::ostringstream{};
          str << v;
          return Value{str.str()}.producedBy(*this);
        }
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const BBoxType&) -> Value {
        switch (toType)
        {
        case ValueType::BBox:
          return *this;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::LazyMap:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const std::shared_ptr<const LazyMapType>&) -> Value {
        switch (toType)
        {
        case ValueType::LazyMap:
          return *this;
        case ValueType::Boolean:
        case ValueType::Number:
        case ValueType::String:
        case ValueType::Array:
        case ValueType::Map:
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::Null:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      },
      [&](const NullType&) -> Value {
        switch (toType)
        {
        case ValueType::Boolean:
          return Value{false}.producedBy(*this);
        case ValueType::Null:
          return *this;
        case ValueType::Number:
          return Value{0.0}.producedBy(*this);
        case ValueType::String:
          return Value{""}.producedBy(*this);
        case ValueType::Array:
          return Value{ArrayType{0}}.producedBy(*this);
        case ValueType::Map:
          return Value{MapType{}}.producedBy(*this);
        case ValueType::Range:
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Undefined:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
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
        case ValueType::Vec3:
        case ValueType::BBox:
        case ValueType::LazyMap:
        case ValueType::Null:
          break;
        }

        throw ConversionError{location(), describe(), type(), toType};
      }),
    m_value);
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
      [&](const std::shared_ptr<const StringType>& sp) {
        const auto& s = *sp;
        // Unescaping happens in Parser::parseLiteral
        str << "\"" << kdl::str_escape(s, "\\\"") << "\"";
      },
      [&](const NumberType& n) { appendNumber(str, n); },
      [&](const std::shared_ptr<const ArrayType>& ap) {
        const auto& a = *ap;
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
          str << "]";
        }
      },
      [&](const std::shared_ptr<const MapType>& mp) {
        const auto& m = *mp;
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
      [&](const Vec3Type& v) {
        str << "vec(";
        appendNumber(str, v.x());
        str << ", ";
        appendNumber(str, v.y());
        str << ", ";
        appendNumber(str, v.z());
        str << ")";
      },
      [&](const BBoxType& b) {
        str << "bbox(";
        Value{b.min}.appendToStream(str, multiline, indent);
        str << ", ";
        Value{b.max}.appendToStream(str, multiline, indent);
        str << ")";
      },
      [&](const std::shared_ptr<const LazyMapType>& bp) {
        const auto& b = *bp;
        const auto keys = b.keys();
        if (keys.empty())
        {
          str << "{}";
        }
        else
        {
          const auto childIndent = multiline ? indent + "\t" : "";
          str << "{";
          str << (multiline ? "\n" : " ");

          for (size_t i = 0; i < keys.size(); ++i)
          {
            const auto& key = keys[i];
            str << childIndent << "\"" << key << "\""
                << ": ";
            b.at(key)
              .value_or(Value::Undefined)
              .appendToStream(str, multiline, childIndent);
            if (i < keys.size() - 1)
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
          str << (multiline ? indent : " ");
          str << "}";
        }
      },
      [&](const NullType&) { str << "null"; },
      [&](const UndefinedType&) { str << "undefined"; }),
    m_value);
}

bool Value::contains(const size_t index) const
{
  switch (type())
  {
  case ValueType::String:
  case ValueType::Array:
    return index < length();
  case ValueType::Map:
  case ValueType::LazyMap:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Vec3:
  case ValueType::BBox:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  return false;
}

bool Value::contains(const std::string& key) const
{
  if (type() == ValueType::LazyMap)
  {
    return lazyMapValue().at(key) != std::nullopt;
  }

  const MapType& map = mapValue();
  const auto it = map.find(key);
  return it != std::end(map);
}

std::vector<std::string> Value::keys() const
{
  if (type() == ValueType::LazyMap)
  {
    return lazyMapValue().keys();
  }

  return mapValue() | std::views::keys | kdl::ranges::to<std::vector>();
}

Value Value::at(const size_t index) const
{
  switch (type())
  {
  case ValueType::String: {
    const auto& str = stringValue();
    if (index < str.length())
    {
      return Value{str.substr(index, 1)};
    }
    throw IndexOutOfBoundsError{location(), *this, index};
  }
  case ValueType::Array: {
    const auto& array = arrayValue();
    if (index < array.size())
    {
      return array[index];
    }
    throw IndexOutOfBoundsError{location(), *this, index};
  }
  case ValueType::Map:
  case ValueType::LazyMap:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Vec3:
  case ValueType::BBox:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{location(), *this, index};
}

Value Value::atOrDefault(const size_t index, Value defaultValue) const
{
  switch (type())
  {
  case ValueType::String: {
    const auto& str = stringValue();
    if (index < str.length())
    {
      return Value{str.substr(index, 1)};
    }
    return defaultValue;
  }
  case ValueType::Array: {
    const auto& array = arrayValue();
    if (index < array.size())
    {
      return array[index];
    }
    return defaultValue;
  }
  case ValueType::Map:
  case ValueType::LazyMap:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Vec3:
  case ValueType::BBox:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{location(), *this, index};
}

Value Value::at(const std::string& key) const
{
  switch (type())
  {
  case ValueType::Map: {
    const auto& map = mapValue();
    if (const auto it = map.find(key); it != map.end())
    {
      return it->second;
    }
    throw IndexOutOfBoundsError{location(), *this, key};
  }
  case ValueType::LazyMap: {
    if (auto value = lazyMapValue().at(key))
    {
      return *value;
    }
    throw IndexOutOfBoundsError{location(), *this, key};
  }
  case ValueType::String:
  case ValueType::Array:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Vec3:
  case ValueType::BBox:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{location(), *this, key};
}

Value Value::atOrDefault(const std::string& key, Value defaultValue) const
{
  switch (type())
  {
  case ValueType::Map: {
    const auto& map = mapValue();
    if (const auto it = map.find(key); it != map.end())
    {
      return it->second;
    }
    return defaultValue;
  }
  case ValueType::LazyMap:
    return lazyMapValue().at(key).value_or(std::move(defaultValue));
  case ValueType::String:
  case ValueType::Array:
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Vec3:
  case ValueType::BBox:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{location(), *this, key};
}

std::optional<ExpressionNode> Value::expression() const
{
  return m_producedByExpression
           ? std::optional{ExpressionNode{m_producedByExpression, m_producedByLocation}}
           : std::nullopt;
}

std::optional<FileLocation> Value::location() const
{
  return m_producedByLocation;
}

Value Value::producedBy(const ExpressionNode& expressionNode) const
{
  if (m_producedByExpression)
  {
    return *this;
  }

  auto result = *this;
  result.m_producedByExpression = expressionNode.m_expression;
  result.m_producedByLocation = expressionNode.m_location;
  return result;
}

Value Value::producedBy(const Value& original) const
{
  if (m_producedByExpression || !original.m_producedByExpression)
  {
    return *this;
  }

  auto result = *this;
  result.m_producedByExpression = original.m_producedByExpression;
  result.m_producedByLocation = original.m_producedByLocation;
  return result;
}

bool operator==(const Value& lhs, const Value& rhs)
{
  return std::visit(
    kdl::overload(
      [](const BooleanType& lhsBool, const BooleanType& rhsBool) {
        return lhsBool == rhsBool;
      },
      [](
        const std::shared_ptr<const StringType>& lhsString,
        const std::shared_ptr<const StringType>& rhsString) {
        return lhsString == rhsString || *lhsString == *rhsString;
      },
      [](const NumberType& lhsNumber, const NumberType& rhsNumber) {
        return lhsNumber == rhsNumber;
      },
      [](
        const std::shared_ptr<const ArrayType>& lhsArray,
        const std::shared_ptr<const ArrayType>& rhsArray) {
        return lhsArray == rhsArray || *lhsArray == *rhsArray;
      },
      [](
        const std::shared_ptr<const MapType>& lhsMap,
        const std::shared_ptr<const MapType>& rhsMap) {
        return lhsMap == rhsMap || *lhsMap == *rhsMap;
      },
      [](const RangeType& lhsRange, const RangeType& rhsRange) {
        return lhsRange == rhsRange;
      },
      [](const Vec3Type& lhsVec3, const Vec3Type& rhsVec3) { return lhsVec3 == rhsVec3; },
      [](const BBoxType& lhsBBox, const BBoxType& rhsBBox) { return lhsBBox == rhsBBox; },
      [](
        const std::shared_ptr<const LazyMapType>& lhsLazyMap,
        const std::shared_ptr<const LazyMapType>& rhsLazyMap) {
        return lhsLazyMap == rhsLazyMap;
      },
      [](const NullType&, const NullType&) { return true; },
      [](const UndefinedType&, const UndefinedType&) { return true; },
      [](const auto&, const auto&) { return false; }),
    lhs.m_value,
    rhs.m_value);
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

} // namespace tb::el
