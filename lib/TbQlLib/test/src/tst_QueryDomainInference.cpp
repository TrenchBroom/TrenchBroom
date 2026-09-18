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

#include "el/ParseExpression.h"
#include "ql/QueryDomainInference.h"

#include <string>

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
  using T = QueryObjectType;

  const auto [expression, expected] = GENERATE(table<std::string, QueryDomain>({
    {R"(classname == "info_player_start")", {T::World, T::Entity}},
    {R"(materials like "*trigger*")", {T::Brush, T::Patch}},
    {R"(material like "*trigger*")", {T::Face}},
    {R"(entity.classname == "func_detail")", {T::Brush, T::Patch, T::Face}},
    {R"(entity.classname == "func_detail" && type == "brush")", {T::Brush}},
    {R"(!(classname == "func_detail"))", {T::World, T::Entity}},
    {"visible == false", {T::World, T::Layer, T::Group, T::Entity, T::Brush, T::Patch}},
    {R"(type == "layer" && name like "Combat*")", {T::Layer}},
    {R"(tags contains "Detail")", {T::World, T::Entity, T::Brush, T::Patch, T::Face}},
    {R"(type == "face" && bbox(vec(-128,-128,-128), vec(128,128,128)) contains bounds)",
     {T::Face}},
    {R"(classname == "a" || classname == "b")", {T::World, T::Entity}},
    {R"(classname == "a" || materials like "b")",
     {T::World, T::Entity, T::Brush, T::Patch}},
    {R"(classname == "x" && materials like "y")", {}},
    {R"("world" == type)", {T::World}},
    {R"(properties.targetname == "door1")", {T::World, T::Entity}},
    {R"(name like "Hallway*" && type == "group")", {T::Group}},
  }));

  CAPTURE(expression);
  CHECK(domainOf(expression) == expected);
}

} // namespace tb::ql
