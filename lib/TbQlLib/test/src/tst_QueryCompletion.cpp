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

#include "el/Expression.h"
#include "ql/QueryCompletion.h"

#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ql
{

namespace
{

// completes the text at the position of the `$` in it
QueryCompletion completeQueryAtCursor(
  const std::string_view textWithCursor,
  const std::vector<std::string>& propertyKeys = {})
{
  const auto cursorPosition = textWithCursor.find('$');
  auto inputText = std::string{textWithCursor};
  inputText.erase(cursorPosition, 1);
  return completeQuery(inputText, cursorPosition, propertyKeys);
}

} // namespace

TEST_CASE("QueryCompletion")
{
  using namespace std::string_view_literals;

  SECTION("completeQuery")
  {
    using Words = std::vector<std::string>;

    const auto infixFunctionNames =
      Words{"contains", "distanceTo", "in", "intersects", "is", "like"};

    SECTION("at the beginning, every field, literal and function is offered")
    {
      CHECK(
        completeQueryAtCursor("$")
        == QueryCompletion{
          0,
          0,
          {
            "bbox",      "bounds",    "center",     "classname", "entity",    "false",
            "groupName", "layerName", "locked",     "material",  "materials", "name",
            "normal",    "null",      "properties", "tags",      "true",      "type",
            "undefined", "vec",       "visible",
          }});
    }

    SECTION("where a term can begin, fields, literals and functions are offered")
    {
      const auto [inputText, expectedWords] = GENERATE(table<std::string, Words>({
        {"c$", {"center", "classname"}},
        {"classname is 'light' && v$", {"vec", "visible"}},
        {"!l$", {"layerName", "locked"}},
        {"(t$", {"tags", "true", "type"}},
        {"[b$", {"bbox", "bounds"}},
        {"center == vec(1, 2, n$", {"name", "normal", "null"}},
        {"properties[n$", {"name", "normal", "null"}},
        {"{key: f$", {"false"}},
        {"{{ v$", {"vec", "visible"}},
        {"visible -> m$", {"material", "materials"}},
        // type names are only offered where they are compared with type
        {"classname is b$", {"bbox", "bounds"}},
      }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words == expectedWords);
    }

    SECTION("after a term, infix functions are offered")
    {
      const auto [inputText, expectedWords] = GENERATE_COPY(table<std::string, Words>({
        {"classname $", infixFunctionNames},
        {"classname i$", {"in", "intersects", "is"}},
        {"(visible) i$", {"in", "intersects", "is"}},
        {"center d$", {"distanceTo"}},
        {"vec(0, 0, 0) d$", {"distanceTo"}},
        {"properties.target l$", {"like"}},
        {R"("light"l$)", {"like"}},
      }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words == expectedWords);
    }

    SECTION("where a type name doesn't need quotes, only type names are offered")
    {
      const auto [inputText, expectedWords] = GENERATE(table<std::string, Words>({
        {"type is $", {"brush", "entity", "face", "group", "layer", "patch", "world"}},
        {"type == b$", {"brush"}},
        {"type != p$", {"patch"}},
        {"type in [brush, p$", {"patch"}},
        {"type contains w$", {"world"}},
        {"!(type is f$", {"face"}},
        {"visible && type is e$", {"entity"}},
      }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words == expectedWords);
    }

    SECTION("after a dot, the keys of the value before it are offered")
    {
      const auto [inputText, expectedWords] = GENERATE(table<std::string, Words>({
        {"center.$", {"x", "y", "z"}},
        {"normal.y$", {"y"}},
        {"center . $", {"x", "y", "z"}},
        {"bounds.$", {"max", "min"}},
        {"bounds.min.$", {"x", "y", "z"}},
        {"entity.$", {"classname", "properties"}},
        {"entity.c$", {"classname"}},
        {"!(center.$", {"x", "y", "z"}},
        {"classname is 'light' && bounds.max.z$", {"z"}},
        {"vec(1, 2, 3).$", {"x", "y", "z"}},
        {"bbox(center, center).$", {"max", "min"}},
        {"bbox(center, center).min.$", {"x", "y", "z"}},
        {"(center).$", {"x", "y", "z"}},
        {"[1, center.$", {"x", "y", "z"}},
        // the entries of a map are ordered by key rather than as written
        {"{b: 1, a: center.$", {"x", "y", "z"}},
        {"vec(center.$", {"x", "y", "z"}},
        {"properties[center.$", {"x", "y", "z"}},
        {"{{ center.$", {"x", "y", "z"}},
      }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words == expectedWords);
    }

    SECTION("after properties, the given property keys that are names are offered")
    {
      const auto propertyKeys = Words{
        "",
        "1key",
        "_tb_id",
        "light",
        "light-color",
        "target",
        "targetname",
        "true",
      };

      const auto [inputText, expectedWords] = GENERATE(table<std::string, Words>({
        {"properties.$", {"_tb_id", "light", "target", "targetname"}},
        {"properties.t$", {"target", "targetname"}},
        {"entity.properties.l$", {"light"}},
        {"classname is 'light' && properties.TARGET$", {"target", "targetname"}},
        // a property value is a string, which has no keys
        {"properties.target.$", {}},
      }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText, propertyKeys).words == expectedWords);
    }

    SECTION("after a dot, nothing is offered if the keys of the value are unknown")
    {
      const auto inputText = GENERATE(
        "classname.$"sv,
        "center.x.$"sv,
        "bounds.mid.$"sv,
        "foo.$"sv,
        "foo(1).$"sv,
        "vec(1, 2, 3).x.$"sv,
        "(center + center).$"sv,
        "properties['a'].$"sv,
        "classname center.$"sv,
        "{center.$"sv);
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words.empty());
    }

    SECTION("names are matched ignoring case")
    {
      const auto [inputText, expectedWords] = GENERATE(table<std::string, Words>({
        {"LAYERN$", {"layerName"}},
        {"center DIST$", {"distanceTo"}},
        {"type is BR$", {"brush"}},
      }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words == expectedWords);
    }

    SECTION("the name around the cursor is replaced")
    {
      const auto [inputText, expectedCompletion] =
        GENERATE_COPY(table<std::string, QueryCompletion>({
          {"clas$sname", {0, 9, {"classname"}}},
          {"visible && lo$cked", {11, 17, {"locked"}}},
          {"classname i$s 'light'", {10, 12, {"in", "intersects", "is"}}},
          {"visible $&& locked", {8, 8, infixFunctionNames}},
        }));
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText) == expectedCompletion);
    }

    SECTION("nothing is offered where no name can be inserted")
    {
      const auto inputText = GENERATE(
        "{t$"sv,
        "{a: 1, t$"sv,
        "'t$"sv,
        R"("type is $)"sv,
        "classname // t$"sv,
        "classname visible $"sv,
        "classname == == $"sv,
        ") $"sv,
        "(] $"sv,
        "#t$"sv,
        "1$"sv,
        "1.5$"sv,
        "classname == 2a$"sv);
      CAPTURE(inputText);

      CHECK(completeQueryAtCursor(inputText).words.empty());
    }

    SECTION("every infix function that is offered takes two arguments")
    {
      for (const auto& word : completeQueryAtCursor("classname $").words)
      {
        CAPTURE(word);
        CHECK(el::builtinFunctions().at(word).arity == 2u);
      }
    }
  }
}

} // namespace tb::ql
