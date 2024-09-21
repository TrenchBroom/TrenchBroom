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

#include "Types.h"

#include "Macros.h"

#include "kdl/reflection_impl.h"

#include <string>

namespace TrenchBroom::EL
{

kdl_reflect_impl(LeftBoundedRange);
kdl_reflect_impl(RightBoundedRange);
kdl_reflect_impl(BoundedRange);

namespace detail
{
size_t length(const long first, const long last)
{
  return first <= last ? static_cast<size_t>(last - first + 1)
                       : static_cast<size_t>(first - last + 1);
}
} // namespace detail

size_t LeftBoundedRange::length(const size_t indexableSize) const
{
  return detail::length(first, std::max(static_cast<long>(indexableSize) - 1, 0l));
}

size_t RightBoundedRange::length(const size_t indexableSize) const
{
  return detail::length(std::max(static_cast<long>(indexableSize) - 1, 0l), last);
}

size_t BoundedRange::length() const
{
  return detail::length(first, last);
}

std::ostream& operator<<(std::ostream& lhs, const RangeType& rhs)
{
  std::visit([&lhs](const auto& value) { lhs << value; }, rhs);
  return lhs;
}

std::string typeName(const ValueType type)
{
  switch (type)
  {
  case ValueType::Boolean:
    return "Boolean";
  case ValueType::String:
    return "String";
  case ValueType::Number:
    return "Number";
  case ValueType::Array:
    return "Array";
  case ValueType::Map:
    return "Map";
  case ValueType::Range:
    return "Range";
  case ValueType::Null:
    return "Null";
  case ValueType::Undefined:
    return "Undefined";
    switchDefault();
  }
}

ValueType typeForName(const std::string& type)
{
  if (type == "Boolean")
  {
    return ValueType::Boolean;
  }
  if (type == "String")
  {
    return ValueType::String;
  }
  if (type == "Number")
  {
    return ValueType::Number;
  }
  if (type == "Array")
  {
    return ValueType::Array;
  }
  if (type == "Map")
  {
    return ValueType::Map;
  }
  if (type == "Range")
  {
    return ValueType::Range;
  }
  if (type == "Undefined")
  {
    return ValueType::Undefined;
  }

  assert(false);
  return ValueType::Null;
}
} // namespace TrenchBroom::EL
