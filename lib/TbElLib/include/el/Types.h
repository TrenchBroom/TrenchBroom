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

#include "Forward.h"

#include "kd/reflection_decl.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <functional>
#include <iosfwd>
#include <map>
#include <optional>
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
using Vec3Type = vm::vec3d;
using BBoxType = vm::bbox3d;

/**
 * A lazy, read-only, Map-like value: `at` looks up a key, returning nullopt if the key
 * doesn't apply at all (as opposed to applying but currently evaluating to Undefined --
 * the same distinction a real Map's `at`/`contains` already make between "key absent"
 * and "key present with an Undefined value"); `keys` enumerates every key `at` can
 * currently resolve. Each closure captures whatever object it's exposing directly, with
 * its real type -- never a `void*` -- so building one is just constructing two closures,
 * with no field ever materialized into a real `Value` until it's actually looked up.
 */
struct BoundValue
{
  std::function<std::optional<Value>(const std::string& key)> at;
  std::function<std::vector<std::string>()> keys;
};

using BoundValueType = BoundValue;

std::ostream& operator<<(std::ostream& lhs, const RangeType& rhs);

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

enum class ValueType
{
  Boolean,
  String,
  Number,
  Array,
  Map,
  Range,
  Vec3,
  BBox,
  BoundValue,
  Null,
  Undefined
};

std::string typeName(ValueType type);
ValueType typeForName(const std::string& type);

} // namespace tb::el
