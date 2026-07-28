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

#include "el/Value.h"
#include "el/VariableStore.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{

TEST_CASE("VariableTable")
{
  SECTION("constructor")
  {
    CHECK(VariableTable{}.size() == 0u);
    CHECK(VariableTable{{{"a", Value{1.0}}}}.size() == 1u);
  }

  SECTION("clone")
  {
    auto table = VariableTable{{{"a", Value{1.0}}}};
    const auto clone = std::unique_ptr<VariableStore>{table.clone()};

    CHECK(clone->size() == 1u);
    CHECK(clone->value("a") == Value{1.0});

    // the clone owns a copy of the table, so the original can move on without it
    table.set("b", Value{2.0});
    CHECK(clone->size() == 1u);
    CHECK(clone->value("b") == Value::Undefined);
  }

  SECTION("size")
  {
    auto table = VariableTable{};
    CHECK(table.size() == 0u);

    table.set("a", Value{1.0});
    CHECK(table.size() == 1u);

    table.set("a", Value{2.0});
    CHECK(table.size() == 1u);

    table.set("b", Value{3.0});
    CHECK(table.size() == 2u);
  }

  SECTION("value")
  {
    const auto table = VariableTable{{{"a", Value{1.0}}}};
    CHECK(table.value("a") == Value{1.0});

    // an unknown name is undefined rather than null
    CHECK(table.value("b") == Value::Undefined);
    CHECK(VariableTable{}.value("a") == Value::Undefined);
  }

  SECTION("names")
  {
    CHECK(VariableTable{}.names() == std::vector<std::string>{});
    CHECK(
      VariableTable{{{"b", Value{1.0}}, {"a", Value{2.0}}}}.names()
      == std::vector<std::string>{"a", "b"});
  }

  SECTION("set")
  {
    auto table = VariableTable{};

    table.set("a", Value{1.0});
    CHECK(table.value("a") == Value{1.0});

    table.set("a", Value{2.0});
    CHECK(table.value("a") == Value{2.0});
  }
}

TEST_CASE("NullVariableStore")
{
  SECTION("clone")
  {
    const auto store = NullVariableStore{};
    const auto clone = std::unique_ptr<VariableStore>{store.clone()};

    CHECK(clone->size() == 0u);
    CHECK(clone->value("a") == Value::Null);
  }

  SECTION("size")
  {
    CHECK(NullVariableStore{}.size() == 0u);
  }

  SECTION("value")
  {
    // unlike VariableTable, an unknown name is null rather than undefined
    CHECK(NullVariableStore{}.value("a") == Value::Null);
    CHECK(NullVariableStore{}.value("") == Value::Null);
  }

  SECTION("names")
  {
    CHECK(NullVariableStore{}.names() == std::vector<std::string>{});
  }

  SECTION("set")
  {
    auto store = NullVariableStore{};
    store.set("a", Value{1.0});

    CHECK(store.size() == 0u);
    CHECK(store.value("a") == Value::Null);
  }
}

TEST_CASE("VariableStore")
{
  SECTION("appendToStream")
  {
    const auto toString = [](const VariableStore& store) {
      auto str = std::ostringstream{};
      store.appendToStream(str);
      return str.str();
    };

    CHECK(toString(VariableTable{}) == "{\n}");
    CHECK(toString(NullVariableStore{}) == "{\n}");
    CHECK(toString(VariableTable{{{"a", Value{1.0}}}}) == "{\n  a: 1}");
    CHECK(toString(VariableTable{{{"a", Value{1.0}}, {"b", Value{"x"}}}}) == R"({
  a: 1,   b: "x"})");
  }

  SECTION("operator<<")
  {
    auto str = std::ostringstream{};
    str << VariableTable{{{"a", Value{1.0}}}};
    CHECK(str.str() == "{\n  a: 1}");
  }

  SECTION("operator==")
  {
    CHECK(VariableTable{} == VariableTable{});
    CHECK(VariableTable{{{"a", Value{1.0}}}} == VariableTable{{{"a", Value{1.0}}}});

    CHECK_FALSE(VariableTable{{{"a", Value{1.0}}}} == VariableTable{{{"a", Value{2.0}}}});
    CHECK_FALSE(VariableTable{{{"a", Value{1.0}}}} == VariableTable{{{"b", Value{1.0}}}});
    CHECK_FALSE(VariableTable{{{"a", Value{1.0}}}} == VariableTable{});

    // stores are compared by their contents, not by their type
    CHECK(VariableTable{} == NullVariableStore{});
    CHECK_FALSE(VariableTable{{{"a", Value{1.0}}}} == NullVariableStore{});
  }

  SECTION("operator!=")
  {
    CHECK_FALSE(VariableTable{} != VariableTable{});
    CHECK(VariableTable{{{"a", Value{1.0}}}} != VariableTable{});
  }
}

} // namespace tb::el
