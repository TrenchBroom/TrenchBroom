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

#include "el/EL_Forward.h"

#include "kdl/reflection_decl.h"

#include <iosfwd>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace tb::el
{

namespace detail
{
size_t length(long first, long last);

template <typename F>
void forEach(const long first, const long last, const F& f)
{
  if (first <= last)
  {
    for (auto i = first; i <= last; ++i)
    {
      f(i);
    }
  }
  else
  {
    for (auto i = first; i >= last; --i)
    {
      f(i);
    }
  }
}
} // namespace detail

struct LeftBoundedRange
{
  long first;

  size_t length(size_t indexableSize) const;

  template <typename F>
  void forEach(const F& f, const size_t indexableSize) const
  {
    detail::forEach(first, static_cast<long>(indexableSize) - 1, f);
  }

  kdl_reflect_decl(LeftBoundedRange, first);
};

struct RightBoundedRange
{
  long last;

  size_t length(size_t indexableSize) const;

  template <typename F>
  void forEach(const F& f, const size_t indexableSize) const
  {
    detail::forEach(static_cast<long>(indexableSize) - 1, last, f);
  }

  kdl_reflect_decl(RightBoundedRange, last);
};

struct BoundedRange
{
  long first;
  long last;

  size_t length() const;

  template <typename F>
  void forEach(const F& f) const
  {
    detail::forEach(first, last, f);
  }

  kdl_reflect_decl(BoundedRange, first, last);
};

using BooleanType = bool;
using StringType = std::string;
using NumberType = double;
using IntegerType = long;
using ArrayType = std::vector<Value>;
using MapType = std::map<std::string, Value>;
using RangeType = std::variant<LeftBoundedRange, RightBoundedRange, BoundedRange>;

std::ostream& operator<<(std::ostream& lhs, const RangeType& rhs);

enum class ValueType
{
  Boolean,
  String,
  Number,
  Array,
  Map,
  Range,
  Null,
  Undefined
};

std::string typeName(ValueType type);
ValueType typeForName(const std::string& type);

} // namespace tb::el
