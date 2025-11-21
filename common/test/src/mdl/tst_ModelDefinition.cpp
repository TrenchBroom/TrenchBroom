/*
 Copyright (C) 2021 Kristian Duske

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

#include "el/ELParser.h"
#include "el/Expression.h"
#include "el/VariableStore.h"
#include "mdl/ModelDefinition.h"

#include <map>
#include <tuple>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{

ModelDefinition makeModelDefinition(const std::string& expression)
{
  auto parser = io::ELParser{io::ELParser::Mode::Strict, expression};
  return ModelDefinition{parser.parse().value()};
}

} // namespace

TEST_CASE("ModelDefinition")
{
  SECTION("append")
  {
    auto d1 = makeModelDefinition(R"("maps/b_shell0.bsp")");
    REQUIRE(
      d1.modelSpecification(el::NullVariableStore{})
      == ModelSpecification{"maps/b_shell0.bsp", 0, 0});

    d1.append(makeModelDefinition(R"("maps/b_shell1.bsp")"));
    CHECK(
      d1.modelSpecification(el::NullVariableStore{})
      == ModelSpecification{"maps/b_shell0.bsp", 0, 0});
  }

  SECTION("modelSpecification")
  {
    using T =
      std::tuple<std::string, std::map<std::string, el::Value>, ModelSpecification>;

    // clang-format off
    const auto 
    [expression,                                            variables, expectedModelSpecification] = GENERATE(values<T>({
    {R"("maps/b_shell0.bsp")",                              {},        {"maps/b_shell0.bsp", 0, 0}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2 })", {},        {"maps/b_shell0.bsp", 1, 2}},
    
    {R"({{
        spawnflags == 1 -> "maps/b_shell0.bsp",
                            "maps/b_shell1.bsp"
    }})",                                                   {},
                                                                        {"maps/b_shell1.bsp", 0, 0}},
    
    {R"({{
        spawnflags == 1 -> "maps/b_shell0.bsp",
                            "maps/b_shell1.bsp"
    }})",                                                   {{"spawnflags", el::Value{1}}},
                                                                        {"maps/b_shell0.bsp", 0, 0}},

    {R"({path: model, skin: skin, frame: frame})",          {{"model", el::Value{"maps/b_shell0.bsp"}},
                                                              {"skin",  el::Value{1}},
                                                              {"frame", el::Value{2}}},
                                                                        {"maps/b_shell0.bsp", 1, 2}},
    
    }));
    // clang-format on

    CAPTURE(expression, variables);

    const auto modelDefinition = makeModelDefinition(expression);
    CHECK(
      modelDefinition.modelSpecification(el::VariableTable{variables})
      == expectedModelSpecification);
  }

  SECTION("defaultModelSpecification")
  {
    using T = std::tuple<std::string, ModelSpecification>;

    // clang-format off
    const auto 
    [expression,                                            expectedModelSpecification] = GENERATE(values<T>({
    {R"("maps/b_shell0.bsp")",                              {"maps/b_shell0.bsp", 0, 0}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2 })", {"maps/b_shell0.bsp", 1, 2}},
    
    {R"({{
        spawnflags == 1 -> "maps/b_shell0.bsp",
                            "maps/b_shell1.bsp"
    }})",                                                   {"maps/b_shell1.bsp", 0, 0}},

    {R"({path: model, skin: skin, frame: frame})",          {}},
    
    {R"({ path: 123, skin: 1, frame: 2 })",                 {"", 1, 2}},
    {R"({ 
        path: "maps/b_shell0.bsp", 
        scale : radius * 64 })", 
                                                            {"maps/b_shell0.bsp", 0, 0}},
    }));
    // clang-format on

    CAPTURE(expression);

    const auto modelDefinition = makeModelDefinition(expression);
    CHECK(modelDefinition.defaultModelSpecification() == expectedModelSpecification);
  }

  SECTION("scale")
  {
    using T = std::tuple<std::string, std::optional<std::string>, vm::vec3d>;

    // clang-format off
    const auto
    [expression,                                                                                 globalScaleExpressionStr, expectedScale] = GENERATE(values<T>({
    {R"("maps/b_shell0.bsp")",                                                                   std::nullopt,             vm::vec3d{1, 1, 1}},
    {R"("maps/b_shell0.bsp")",                                                                   R"(2)",                   vm::vec3d{2, 2, 2}},
    {R"("maps/b_shell0.bsp")",                                                                   R"(modelscale)",          vm::vec3d{4, 4, 4}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: 1.5 })",                          std::nullopt,             vm::vec3d{1.5, 1.5, 1.5}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: 1.5 })",                          R"(modelscale)",          vm::vec3d{1.5, 1.5, 1.5}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: '1.5' })",                        std::nullopt,             vm::vec3d{1.5, 1.5, 1.5}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: '1 2 3' })",                      std::nullopt,             vm::vec3d{1, 2, 3}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: modelscale })",                   std::nullopt,             vm::vec3d{4, 4, 4}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: [modelscale, modelscale_vec] })", std::nullopt,             vm::vec3d{4, 4, 4}},
    {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: [modelscale_vec, modelscale] })", std::nullopt,             vm::vec3d{5, 6, 7}},
    }));
    // clang-format on

    CAPTURE(expression, globalScaleExpressionStr);

    const auto modelDefinition = makeModelDefinition(expression);
    const auto variables = el::VariableTable{{
      {"modelscale", el::Value{4}},
      {"modelscale_vec", el::Value{"5, 6, 7"}},
    }};

    const auto defaultScaleExpression =
      globalScaleExpressionStr
        ? std::optional<el::ExpressionNode>{io::ELParser::parseStrict(
                                              *globalScaleExpressionStr)
                                              .value()}
        : std::nullopt;

    CHECK(modelDefinition.scale(variables, defaultScaleExpression) == expectedScale);
  }
}

} // namespace tb::mdl
