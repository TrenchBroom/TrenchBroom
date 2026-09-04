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

#include "el/BoundValue.h"
#include "el/BoundValueStore.h"
#include "el/Value.h"
#include "el/VariableStore.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{

TEST_CASE("BoundValueStore")
{
  const auto fields = BoundValueFields<std::string>{
    {"a", [](const std::string& s) { return Value{s}; }},
  };

  SECTION("clone")
  {
    auto store = BoundValueStore{makeBoundValueOf(std::string{"1"}, fields)};
    const auto clone = std::unique_ptr<VariableStore>{store.clone()};

    CHECK(clone->size() == 1u);
    CHECK(clone->value("a") == Value{"1"});
  }

  SECTION("size")
  {
    CHECK(BoundValueStore{makeBoundValueOf(std::string{"1"}, fields)}.size() == 1u);
  }

  SECTION("value")
  {
    const auto store = BoundValueStore{makeBoundValueOf(std::string{"1"}, fields)};
    CHECK(store.value("a") == Value{"1"});
    CHECK(store.value("b") == Value::Undefined);
  }

  SECTION("names")
  {
    CHECK(
      BoundValueStore{makeBoundValueOf(std::string{"1"}, fields)}.names()
      == std::vector<std::string>{"a"});
  }

  SECTION("set")
  {
    // read-only, like EntityPropertiesVariableStore -- set() is silently ignored
    auto store = BoundValueStore{makeBoundValueOf(std::string{"1"}, fields)};
    store.set("a", Value{"2"});

    CHECK(store.value("a") == Value{"1"});
  }
}

} // namespace tb::el
