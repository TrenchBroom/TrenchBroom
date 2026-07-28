/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Types.h"

#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{
namespace
{

const auto allTypes = std::vector{
  ValueType::Boolean,
  ValueType::String,
  ValueType::Number,
  ValueType::Array,
  ValueType::Map,
  ValueType::Range,
  ValueType::Null,
  ValueType::Undefined,
};

template <typename R, typename... Args>
std::vector<long> visited(const R& range, const Args... args)
{
  auto result = std::vector<long>{};
  range.forEach([&](const auto i) { result.push_back(i); }, args...);
  return result;
}

} // namespace

TEST_CASE("LeftBoundedRange")
{
  SECTION("length")
  {
    CHECK(LeftBoundedRange{0}.length(3) == 3u);
    CHECK(LeftBoundedRange{1}.length(3) == 2u);
    CHECK(LeftBoundedRange{2}.length(3) == 1u);

    // the range extends backwards if it starts past the end of the indexable
    CHECK(LeftBoundedRange{5}.length(3) == 4u);

    // an empty indexable clamps to index 0, so the range still has one element
    CHECK(LeftBoundedRange{0}.length(0) == 1u);
  }

  SECTION("forEach")
  {
    CHECK(visited(LeftBoundedRange{0}, size_t(3)) == std::vector<long>{0, 1, 2});
    CHECK(visited(LeftBoundedRange{2}, size_t(3)) == std::vector<long>{2});
    CHECK(visited(LeftBoundedRange{3}, size_t(2)) == std::vector<long>{3, 2, 1});
  }
}

TEST_CASE("RightBoundedRange")
{
  SECTION("length")
  {
    CHECK(RightBoundedRange{0}.length(3) == 3u);
    CHECK(RightBoundedRange{1}.length(3) == 2u);
    CHECK(RightBoundedRange{2}.length(3) == 1u);

    // the range extends forwards if it ends past the end of the indexable
    CHECK(RightBoundedRange{5}.length(3) == 4u);

    CHECK(RightBoundedRange{0}.length(0) == 1u);
  }

  SECTION("forEach")
  {
    CHECK(visited(RightBoundedRange{0}, size_t(3)) == std::vector<long>{2, 1, 0});
    CHECK(visited(RightBoundedRange{2}, size_t(3)) == std::vector<long>{2});
    CHECK(visited(RightBoundedRange{3}, size_t(2)) == std::vector<long>{1, 2, 3});
  }
}

TEST_CASE("BoundedRange")
{
  SECTION("length")
  {
    CHECK(BoundedRange{0, 2}.length() == 3u);
    CHECK(BoundedRange{2, 2}.length() == 1u);

    // a descending range has the same length as its ascending counterpart
    CHECK(BoundedRange{2, 0}.length() == 3u);

    CHECK(BoundedRange{-2, 2}.length() == 5u);
  }

  SECTION("forEach")
  {
    CHECK(visited(BoundedRange{0, 2}) == std::vector<long>{0, 1, 2});
    CHECK(visited(BoundedRange{2, 2}) == std::vector<long>{2});
    CHECK(visited(BoundedRange{2, 0}) == std::vector<long>{2, 1, 0});
  }
}

TEST_CASE("Types")
{
  SECTION("operator<<")
  {
    const auto toString = [](const RangeType& range) {
      auto str = std::ostringstream{};
      str << range;
      return str.str();
    };

    CHECK(toString(RangeType{LeftBoundedRange{2}}) == "LeftBoundedRange{first: 2}");
    CHECK(toString(RangeType{RightBoundedRange{5}}) == "RightBoundedRange{last: 5}");
    CHECK(toString(RangeType{BoundedRange{1, 3}}) == "BoundedRange{first: 1, last: 3}");
  }

  SECTION("typeName")
  {
    CHECK(typeName(ValueType::Boolean) == "Boolean");
    CHECK(typeName(ValueType::String) == "String");
    CHECK(typeName(ValueType::Number) == "Number");
    CHECK(typeName(ValueType::Array) == "Array");
    CHECK(typeName(ValueType::Map) == "Map");
    CHECK(typeName(ValueType::Range) == "Range");
    CHECK(typeName(ValueType::Null) == "Null");
    CHECK(typeName(ValueType::Undefined) == "Undefined");
  }

  SECTION("typeForName")
  {
    CHECK(typeForName("Boolean") == ValueType::Boolean);
    CHECK(typeForName("String") == ValueType::String);
    CHECK(typeForName("Number") == ValueType::Number);
    CHECK(typeForName("Array") == ValueType::Array);
    CHECK(typeForName("Map") == ValueType::Map);
    CHECK(typeForName("Range") == ValueType::Range);
    CHECK(typeForName("Null") == ValueType::Null);
    CHECK(typeForName("Undefined") == ValueType::Undefined);

    // every type name round trips
    for (const auto type : allTypes)
    {
      CHECK(typeForName(typeName(type)) == type);
    }
  }
}

} // namespace tb::el
