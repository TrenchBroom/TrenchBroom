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
#include "el/Types.h"
#include "ql/QueryDomainInference.h"

#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ql
{

namespace
{

el::ExpressionNode parse(const std::string& expression)
{
  return el::parseExpression(el::ParseMode::Strict, expression).value();
}

QueryDomain domainOf(const std::string& expression)
{
  return inferQueryDomain(parse(expression));
}

} // namespace

TEST_CASE("QueryDomainInference")
{
  using T = QueryObjectType;

  SECTION("queryFields")
  {
    const auto& fields = queryFields();

    CHECK(fields.at("classname").domain == QueryDomain{T::World, T::Entity});
    CHECK(fields.at("classname").valueType == el::ValueType::String);
    CHECK(fields.at("material").domain == QueryDomain{T::Face});
    CHECK(
      fields.at("visible").domain
      == QueryDomain{
        T::World, T::Layer, T::Group, T::Entity, T::Brush, T::Patch, T::Face});
    CHECK(fields.at("visible").valueType == el::ValueType::Boolean);
    CHECK(!fields.contains("missing"));
  }

  SECTION("inferQueryDomain")
  {
    const auto [expression, expected] = GENERATE(table<std::string, QueryDomain>({
      {R"(classname == "info_player_start")", {T::World, T::Entity}},
      {R"(materials like "*trigger*")", {T::Brush, T::Patch}},
      {R"(material like "*trigger*")", {T::Face}},
      {R"(entity.classname == "func_detail")", {T::Brush, T::Patch}},
      {R"(entity.classname == "func_detail" && type == "brush")", {T::Brush}},
      {R"(!(classname == "func_detail"))", {T::World, T::Entity}},
      {R"(classname != "func_detail")", {T::World, T::Entity}},
      {"visible == false", {T::World, T::Group, T::Entity, T::Brush, T::Patch}},
      {R"(type == "layer" && name like "Combat*")", {T::Layer}},
      {R"(name like "Combat*")", {T::Group}},
      {R"(bounds intersects bbox(vec(-128,-128,-128), vec(128,128,128)))",
       {T::Group, T::Entity, T::Brush, T::Patch}},
      {R"(tags contains "Detail")", {T::World, T::Entity, T::Brush, T::Patch}},
      {R"(type == "face" && bbox(vec(-128,-128,-128), vec(128,128,128)) contains bounds)",
       {T::Face}},
      {R"(classname == "a" || classname == "b")", {T::World, T::Entity}},
      {R"(classname == "a" || materials like "b")",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {R"(classname == "x" && materials like "y")", {}},
      {R"(!(classname == "a" && materials like "b"))",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {R"(!(classname == "a") || !(materials like "b"))",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {R"("world" == type)", {T::World}},
      {R"(type is "face")", {T::Face}},
      {R"("patch" is type)", {T::Patch}},
      {R"(type in ["brush", "patch"])", {T::Brush, T::Patch}},
      {R"(["layer", "group"] contains type)", {T::Group}},
      {R"(type in ["brush", "unknown"])", {T::Brush}},
      {R"(type in ["brush", "face"])", {T::Brush}},
      {R"(type in ["layer", "face"])", {}},
      {R"(properties.targetname == "door1")", {T::World, T::Entity}},
      {R"(name like "Hallway*" && type == "group")", {T::Group}},
      {R"(!(type == "brush"))", {T::World, T::Group, T::Entity, T::Patch}},
      {R"(!!(type == "brush"))", {T::Brush}},
      {R"(type != "brush")", {T::World, T::Group, T::Entity, T::Patch}},
      {R"("brush" != type)", {T::World, T::Group, T::Entity, T::Patch}},
      {R"(!(type != "face"))", {T::Face}},
      {R"(!(type != "layer"))", {T::Layer}},
      {R"(type != "brush" && type != "patch" && materials like "sky*")", {}},
      {R"(type in ["brush", "patch"] && !(type is "patch"))", {T::Brush}},
      {R"(!(type == "brush" || type == "patch"))", {T::World, T::Group, T::Entity}},
      {R"(type != "brush" && type != "patch")", {T::World, T::Group, T::Entity}},
      {R"(!(type == "brush" && visible))",
       {T::World, T::Group, T::Entity, T::Brush, T::Patch}},
      {R"(type != "brush" || !visible)",
       {T::World, T::Group, T::Entity, T::Brush, T::Patch}},
      {R"(type is "brush" == false)", {T::World, T::Group, T::Entity, T::Patch}},
      {R"((type == "brush") != true)", {T::World, T::Group, T::Entity, T::Patch}},
      {R"(!(type == "brush") == false)", {T::Brush}},
      {R"((type == "brush" && visible) == false)",
       {T::World, T::Group, T::Entity, T::Brush, T::Patch}},
      {R"({{ classname != "func_detail" -> true }})", {T::World, T::Entity}},
      {R"({{ type is "brush" -> true }})", {T::Brush}},
      {R"({{ type is "brush" -> visible }})",
       {T::World, T::Group, T::Entity, T::Brush, T::Patch}},
      {R"({{ classname == "light" -> true, materials like "sky*" }})",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {R"({{ classname == "light" -> materials like "sky*" }})",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {"{{}}", {}},
      {R"("func_door" in [classname, entity.classname])",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {R"({a: classname, b: materials}.a == "light")",
       {T::World, T::Entity, T::Brush, T::Patch}},
      {"materials == []", {T::Brush, T::Patch}},
    }));

    CAPTURE(expression);
    CHECK(domainOf(expression) == expected);
  }

  SECTION("specializeQuery")
  {
    const auto [expression, type, expected] =
      GENERATE(table<std::string, QueryObjectType, std::string>({
        {R"(classname == "a" || materials like "b")", T::Entity, R"(classname == "a")"},
        {R"(classname == "a" || materials like "b")", T::Brush, R"(materials like "b")"},
        {R"(!(classname is "a") || materials like "b")",
         T::Brush,
         R"(materials like "b")"},
        {R"(!(classname == "a" && materials like "b"))",
         T::Brush,
         R"(!(materials like "b"))"},
        {R"(!!(classname == "a"))", T::Brush, "false"},
        {R"(properties.light < 100 || materials like "b")",
         T::Brush,
         R"(materials like "b")"},
        {R"(type == "brush" || classname == "a")", T::Brush, "true"},
        {R"(type == "brush" || classname == "a")", T::Entity, R"(classname == "a")"},
        {R"(type == "brush" && visible)", T::Brush, "visible"},
        {R"((type is "brush") == false)", T::Brush, "false"},
        {R"((type is "brush") == false)", T::Entity, "true"},
        {R"(visible && classname == "a")", T::Brush, "false"},
        {R"((classname == "a") == false)", T::Brush, "false"},
        {R"(!((classname == "a") == false))", T::Brush, "false"},
        {R"("a" in [classname, entity.classname])",
         T::Brush,
         R"("a" in [classname, entity.classname])"},
        {R"({{ classname == "a" -> true, materials like "b" }})",
         T::Brush,
         R"({{ classname == "a" -> true, materials like "b" }})"},
        {R"({{ classname == "a" -> true }})", T::Brush, "false"},
      }));

    CAPTURE(expression, type);
    CHECK(specializeQuery(parse(expression), type) == parse(expected));
  }
}

} // namespace tb::ql
