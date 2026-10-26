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

#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/Value.h"
#include "mdl/CatchConfig.h"
#include "mdl/ParseConfigExpression.h"

#include "kd/result.h"
#include "kd/result_io.h" // IWYU pragma: keep

#include <string_view>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

auto parseAndEvaluate(const std::string_view str, const el::IntegerType legacyVersion)
{
  return parseConfigExpression(str, legacyVersion)
         | kdl::and_then([](const auto& expression) {
             return el::withEvaluationContext(
               [&](auto& context) { return expression.evaluate(context); });
           });
}

} // namespace

TEST_CASE("parseConfigExpression")
{
  // the config files are not written inside CHECK because MSVC fails to stringize raw
  // string literals that contain a double quote followed by a backslash

  SECTION("legacy version uses legacy string escaping")
  {
    const auto config = R"({ "version": 1, "path": "\\\\server\\share" })";
    CHECK(
      parseAndEvaluate(config, 1)
      == el::Value{el::MapType{
        {"version", el::Value{1}},
        {"path", el::Value{R"(\\server\share)"}},
      }});
  }

  SECTION("legacy version given as a string uses legacy string escaping")
  {
    const auto config = R"({ "version": "1", "path": "\\\\server\\share" })";
    CHECK(
      parseAndEvaluate(config, 1)
      == el::Value{el::MapType{
        {"version", el::Value{"1"}},
        {"path", el::Value{R"(\\server\share)"}},
      }});
  }

  SECTION("older version uses legacy string escaping")
  {
    const auto config = R"({ "version": 0, "path": "\\\\server\\share" })";
    CHECK(
      parseAndEvaluate(config, 1)
      == el::Value{el::MapType{
        {"version", el::Value{0}},
        {"path", el::Value{R"(\\server\share)"}},
      }});
  }

  SECTION("newer version uses default string escaping")
  {
    const auto config = R"({ "version": 2, "path": "\\\\server\\share" })";
    CHECK(
      parseAndEvaluate(config, 1)
      == el::Value{el::MapType{
        {"version", el::Value{2}},
        {"path", el::Value{R"(\\\\server\\share)"}},
      }});
  }

  SECTION("missing version uses default string escaping")
  {
    const auto config = R"({ "path": "\\\\server\\share" })";
    CHECK(
      parseAndEvaluate(config, 1)
      == el::Value{el::MapType{
        {"path", el::Value{R"(\\\\server\\share)"}},
      }});
  }

  SECTION("root that is not a map uses default string escaping")
  {
    const auto config = R"([ "\\\\server\\share" ])";
    CHECK(
      parseAndEvaluate(config, 1)
      == el::Value{el::ArrayType{
        el::Value{R"(\\\\server\\share)"},
      }});
  }

  SECTION("syntax error")
  {
    CHECK(parseConfigExpression(R"({ "version": 1, )", 1).is_error());
  }
}

} // namespace tb::mdl
