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

#include "el/EvaluationContext.h"
#include "el/ParseExpression.h"
#include "mdl/CatchConfig.h"
#include "mdl/CompilationConfig.h"

#include <fmt/format.h>

#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

auto parse(const std::string_view str)
{
  return el::withEvaluationContext([&](auto& context) {
    return el::parseExpression(el::ParseMode::Strict, str)
           | kdl::transform(
             [&](const auto& expression) { return expression.evaluate(context); })
           | kdl::if_error([](const auto& e) { FAIL(e); });
  });
}

TEST_CASE("toValue")
{
  SECTION("empty config")
  {
    CHECK(toValue(CompilationConfig{}) == parse(R"({
      "version": 1.0,
      "profiles": []
    })"));
  }

  SECTION("no tasks")
  {
    CHECK(
      toValue(CompilationConfig{{
        {"name", "workDirSpec", {}},
      }})
      == parse(R"({
        "version": 1.0,
        "profiles": [
          {
            "name": "name",
            "workdir": "workDirSpec",
            "tasks": []
          }
        ]
      })"));
  }

  SECTION("export task")
  {
    const auto enabled = GENERATE(true, false);
    const auto stripTbProperties = GENERATE(true, false);

    CHECK(
      toValue(CompilationConfig{{
        {"name",
         "workDirSpec",
         {
           CompilationExportMap{
             enabled,
             stripTbProperties,
             "targetSpec",
           },
         }},
      }})
      == parse(fmt::format(
        R"({{
        "version": 1.0,
        "profiles": [
          {{
            "name": "name",
            "workdir": "workDirSpec",
            "tasks": [
              {{
                "type": "export",
                "enabled": {},
                "stripTbProperties": {},
                "target": "targetSpec"
              }}
            ]
          }}
        ]
      }})",
        enabled,
        stripTbProperties)));
  }

  SECTION("copy task")
  {
    const auto enabled = GENERATE(true, false);

    CHECK(
      toValue(CompilationConfig{{
        {"name",
         "workDirSpec",
         {
           CompilationCopyFiles{
             enabled,
             "sourceSpec",
             "targetSpec",
           },
         }},
      }})
      == parse(fmt::format(
        R"({{
        "version": 1.0,
        "profiles": [
          {{
            "name": "name",
            "workdir": "workDirSpec",
            "tasks": [
              {{
                "type": "copy",
                "enabled": {},
                "source": "sourceSpec",
                "target": "targetSpec"
              }}
            ]
          }}
        ]
      }})",
        enabled)));
  }

  SECTION("rename task")
  {
    const auto enabled = GENERATE(true, false);

    CHECK(
      toValue(CompilationConfig{{
        {"name",
         "workDirSpec",
         {
           CompilationRenameFile{
             enabled,
             "sourceSpec",
             "targetSpec",
           },
         }},
      }})
      == parse(fmt::format(
        R"({{
        "version": 1.0,
        "profiles": [
          {{
            "name": "name",
            "workdir": "workDirSpec",
            "tasks": [
              {{
                "type": "rename",
                "enabled": {},
                "source": "sourceSpec",
                "target": "targetSpec"
              }}
            ]
          }}
        ]
      }})",
        enabled)));
  }

  SECTION("delete task")
  {
    const auto enabled = GENERATE(true, false);

    CHECK(
      toValue(CompilationConfig{{
        {"name",
         "workDirSpec",
         {
           CompilationDeleteFiles{
             enabled,
             "targetSpec",
           },
         }},
      }})
      == parse(fmt::format(
        R"({{
        "version": 1.0,
        "profiles": [
          {{
            "name": "name",
            "workdir": "workDirSpec",
            "tasks": [
              {{
                "type": "delete",
                "enabled": {},
                "target": "targetSpec"
              }}
            ]
          }}
        ]
      }})",
        enabled)));
  }

  SECTION("tool task")
  {
    const auto enabled = GENERATE(true, false);
    const auto treatNonZeroResultCodeAsError = GENERATE(true, false);

    CHECK(
      toValue(CompilationConfig{{
        {"name",
         "workDirSpec",
         {
           CompilationRunTool{
             enabled,
             "toolSpec",
             "parameterSpec",
             treatNonZeroResultCodeAsError,
           },
         }},
      }})
      == parse(fmt::format(
        R"({{
        "version": 1.0,
        "profiles": [
          {{
            "name": "name",
            "workdir": "workDirSpec",
            "tasks": [
              {{
                "type": "tool",
                "enabled": {},
                "treatNonZeroResultCodeAsError": {},
                "tool": "toolSpec",
                "parameters": "parameterSpec"
              }}
            ]
          }}
        ]
      }})",
        enabled,
        treatNonZeroResultCodeAsError)));
  }
}

} // namespace tb::mdl
