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
#include "mdl/QueryDomainInference.h"

#include "kd/result.h"

#include <string>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{

QueryDomain domainOf(const std::string& expression)
{
  const auto node = el::parseExpression(el::ParseMode::Strict, expression) | kdl::value();
  return inferQueryDomain(node);
}

} // namespace

TEST_CASE("QueryDomainInference")
{
  using T = std::tuple<std::string, QueryDomain>;

  // clang-format off
  const auto
  [expression, expectedDomain] = GENERATE(values<T>({
    // fields exclusive to one kind narrow straight down to it
    {R"(classname == "info_player_start")",                     {QueryKind::Entity}},
    {R"(properties.targetname == "door1")",                     {QueryKind::Entity}},

    // fields shared by brush/patch
    {R"(materials like "*trigger*")",                           {QueryKind::Brush, QueryKind::Patch}},

    // face-only fields infer face domain with no explicit hint needed
    {R"(material like "*trigger*")",                            {QueryKind::Face}},
    {R"(normal.z > 0.9)",                                       {QueryKind::Face}},

    // entity (owning entity) is bound on entity/brush/patch, never face or world/layer/group
    {R"(entity.classname == "func_detail")",                    {QueryKind::Entity, QueryKind::Brush, QueryKind::Patch}},
    {R"(entity.classname == "func_detail" && type == "brush")", {QueryKind::Brush}},

    // negation does not change the inferred domain of its operand
    {R"(!(classname == "func_detail"))",                        {QueryKind::Entity}},

    // no domain-narrowing field at all -> every node kind, never face
    {R"(visible == false)",
      {QueryKind::World, QueryKind::Layer, QueryKind::Group,
       QueryKind::Entity, QueryKind::Brush, QueryKind::Patch}},

    // name spans entity/layer/group; an explicit type hint narrows further
    {R"(type == "layer" && name like "Combat*")",               {QueryKind::Layer}},
    {R"(name like "Hallway*" && type == "group")",               {QueryKind::Group}},

    // tags is bound on entity/brush/patch
    {R"(tags contains "Detail")",                                {QueryKind::Entity, QueryKind::Brush, QueryKind::Patch}},

    // an explicit `type == "face"` hint combines correctly with domain-agnostic fields
    {R"(type == "face" && bbox(vec(-128,-128,-128), vec(128,128,128)) contains bounds)",
      {QueryKind::Face}},

    // a field exclusive to entity AND-combined with a field exclusive to brush/patch
    // is provably unsatisfiable
    {R"(classname == "x" && materials like "y")",                {}},

    // disjunction unions rather than intersects
    {R"(type == "entity" || type == "brush")",                   {QueryKind::Entity, QueryKind::Brush}},

    // a literal with no field reference at all carries no domain information
    {R"(true)",
      {QueryKind::World, QueryKind::Layer, QueryKind::Group,
       QueryKind::Entity, QueryKind::Brush, QueryKind::Patch}},
  }));
  // clang-format on

  CAPTURE(expression);
  CHECK(domainOf(expression) == expectedDomain);
}

} // namespace tb::mdl
