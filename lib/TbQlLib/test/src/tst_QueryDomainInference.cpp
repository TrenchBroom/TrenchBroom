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
#include "el/ParseExpression.h"
#include "ql/QueryDomainInference.h"

#include <string>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ql
{

namespace
{

QueryDomain domainOf(const std::string& expression)
{
  return inferQueryDomain(el::parseExpression(el::ParseMode::Strict, expression).value());
}

} // namespace

TEST_CASE("QueryDomainInference")
{
  using K = QueryKind;

  const auto [expression, expected] = GENERATE(table<std::string, QueryDomain>({
    {R"(classname == "info_player_start")", {K::World, K::Entity}},
    {R"(materials like "*trigger*")", {K::Brush, K::Patch}},
    {R"(material like "*trigger*")", {K::Face}},
    {R"(entity.classname == "func_detail")", {K::Brush, K::Patch, K::Face}},
    {R"(entity.classname == "func_detail" && type == "brush")", {K::Brush}},
    {R"(!(classname == "func_detail"))", {K::World, K::Entity}},
    {"visible == false", {K::World, K::Layer, K::Group, K::Entity, K::Brush, K::Patch}},
    {R"(type == "layer" && name like "Combat*")", {K::Layer}},
    {R"(tags contains "Detail")", {K::World, K::Entity, K::Brush, K::Patch, K::Face}},
    {R"(type == "face" && bbox(vec(-128,-128,-128), vec(128,128,128)) contains bounds)",
     {K::Face}},
    {R"(classname == "a" || classname == "b")", {K::World, K::Entity}},
    {R"(classname == "a" || materials like "b")",
     {K::World, K::Entity, K::Brush, K::Patch}},
    {R"(classname == "x" && materials like "y")", {}},
    {R"("world" == type)", {K::World}},
    {R"(properties.targetname == "door1")", {K::World, K::Entity}},
    {R"(name like "Hallway*" && type == "group")", {K::Group}},
  }));

  CAPTURE(expression);
  CHECK(domainOf(expression) == expected);
}

} // namespace tb::ql
