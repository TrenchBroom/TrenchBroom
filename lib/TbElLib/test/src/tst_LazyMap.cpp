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

#include "el/LazyMap.h"
#include "el/Types.h"
#include "el/Value.h"

#include "kd/flat_map.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{

TEST_CASE("LazyMap")
{
  const auto fields = LazyMapFields<std::string>{
    {"name", {ValueType::String, [](const std::string& s) { return Value{s}; }}},
    {"length",
     {ValueType::Number, [](const std::string& s) { return Value{double(s.size())}; }}},
  };

  SECTION("lazyMapFieldTypes")
  {
    CHECK(
      lazyMapFieldTypes(fields)
      == kdl::flat_map<std::string, ValueType>{
        {"length", ValueType::Number},
        {"name", ValueType::String},
      });
  }

  SECTION("makeLazyMapOf")
  {
    const auto lazyMap = makeLazyMapOf(std::string{"door"}, fields);

    CHECK(lazyMap.keys() == std::vector<std::string>{"length", "name"});
    CHECK(lazyMap.at("name") == Value{"door"});
    CHECK(lazyMap.at("length") == Value{4.0});
    CHECK(lazyMap.at("missing") == std::nullopt);
  }

  SECTION("makeLazyMap")
  {
    const auto value = makeLazyMap(std::string{"door"}, fields);

    CHECK(value.hasType(ValueType::LazyMap));
  }
}

} // namespace tb::el
