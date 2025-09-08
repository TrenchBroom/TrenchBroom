/*
 Copyright (C) 2023 Daniel Walder

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

#include "el/VariableStore.h"
#include "io/ELParser.h"
#include "mdl/DecalDefinition.h"

#include <map>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

namespace
{
DecalDefinition makeDecalDefinition(const std::string& expression)
{
  auto parser = io::ELParser{io::ELParser::Mode::Strict, expression};
  return DecalDefinition{parser.parse().value()};
}
} // namespace

TEST_CASE("DecalDefinitionTest.append")
{
  auto d1 = makeDecalDefinition(R"("decal1")");
  REQUIRE(d1.decalSpecification(el::NullVariableStore{}) == DecalSpecification{"decal1"});

  d1.append(makeDecalDefinition(R"("decal2")"));
  CHECK(d1.decalSpecification(el::NullVariableStore{}) == DecalSpecification{"decal1"});
}

TEST_CASE("DecalDefinitionTest.decalSpecification")
{
  using T = std::tuple<std::string, std::map<std::string, el::Value>, DecalSpecification>;

  // clang-format off
  const auto 
  [expression,                                 variables, expectedDecalSpecification] = GENERATE(values<T>({
  {R"("decal1")",                              {},        {"decal1"}},
  {R"({ texture: "decal2" })",                 {},        {"decal2"}},

  {R"({ texture: texture })",                  {{"texture", el::Value{"decal3"}}},
                                                          {"decal3"}},
  
  }));
  // clang-format on

  CAPTURE(expression, variables);

  const auto decalDefinition = makeDecalDefinition(expression);
  CHECK(
    decalDefinition.decalSpecification(el::VariableTable{variables})
    == expectedDecalSpecification);
}

TEST_CASE("DecalDefinitionTest.defaultDecalSpecification")
{
  using T = std::tuple<std::string, DecalSpecification>;

  // clang-format off
  const auto 
  [expression,                 expectedDecalSpecification] = GENERATE(values<T>({
  {R"("decal1")",              {"decal1"}},
  {R"({ texture: "decal2" })", {"decal2"}},

  {R"({ texture: texture })",  {}},
  
  }));
  // clang-format on

  CAPTURE(expression);

  const auto decalDefinition = makeDecalDefinition(expression);
  CHECK(decalDefinition.defaultDecalSpecification() == expectedDecalSpecification);
}

} // namespace tb::mdl
