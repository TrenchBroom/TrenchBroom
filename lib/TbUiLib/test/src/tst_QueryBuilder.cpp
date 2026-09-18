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
#include "ui/QueryBuilder.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

namespace
{

bool conditionsEqual(const QueryCondition& lhs, const QueryCondition& rhs)
{
  return lhs.fieldId == rhs.fieldId && lhs.propertyKey == rhs.propertyKey
         && lhs.op == rhs.op && lhs.value == rhs.value;
}

bool conditionListsEqual(
  const std::vector<QueryCondition>& lhs, const std::vector<QueryCondition>& rhs)
{
  return lhs.size() == rhs.size()
         && std::equal(lhs.begin(), lhs.end(), rhs.begin(), conditionsEqual);
}

std::optional<ImportedQuery> importText(const std::string& text)
{
  return tryImportQuery(el::parseExpression(el::ParseMode::Strict, text).value());
}

// buildQueryExpression's own contract is "" only in the sense that there's nothing to
// run (nullopt); this mirrors that as a plain string for readable test assertions.
std::string textOf(
  const std::vector<QueryCondition>& conditions,
  const QueryCombinator combinator,
  const std::optional<ql::QueryKind> hint)
{
  const auto expression = buildQueryExpression(conditions, combinator, hint);
  return expression ? expression->asString() : std::string{};
}

} // namespace

TEST_CASE("QueryBuilder::buildQueryExpression")
{
  SECTION("one condition, per operator and field")
  {
    struct TestCase
    {
      QueryCondition condition;
      std::string expected;
    };

    const auto testCase = GENERATE(
      TestCase{
        {"classname", "", QueryConditionOperator::Equals, "func_detail"},
        R"(classname == "func_detail")"},
      TestCase{
        {"layerName", "", QueryConditionOperator::NotEquals, "Combat"},
        R"(layerName != "Combat")"},
      TestCase{
        {"classname", "", QueryConditionOperator::Matches, "func_*"},
        R"(classname like "func_*")"},
      TestCase{
        {"tags", "", QueryConditionOperator::Contains, "Detail"},
        R"(tags contains "Detail")"},
      TestCase{
        {"materials", "", QueryConditionOperator::Matches, "*trigger*"},
        R"(materials like "*trigger*")"},
      TestCase{{"visible", "", QueryConditionOperator::Is, "true"}, "visible == true"},
      TestCase{{"locked", "", QueryConditionOperator::Is, "false"}, "locked == false"},
      TestCase{
        {"entityClassname", "", QueryConditionOperator::Equals, "func_detail"},
        R"(entity.classname == "func_detail")"},
      TestCase{
        {"property", "targetname", QueryConditionOperator::Equals, "door1"},
        R"(properties["targetname"] == "door1")"},
      TestCase{
        {"entityProperty", "angle", QueryConditionOperator::NotEquals, "90"},
        R"(entity.properties["angle"] != "90")"},
      TestCase{
        {"classname", "", QueryConditionOperator::Equals, R"(say "hi")"},
        R"(classname == "say \"hi\"")"});

    CAPTURE(testCase.condition.fieldId, testCase.condition.op, testCase.condition.value);
    CHECK(
      textOf({testCase.condition}, QueryCombinator::All, std::nullopt)
      == testCase.expected);
  }

  SECTION("a condition with an empty value is skipped")
  {
    const auto condition =
      QueryCondition{"classname", "", QueryConditionOperator::Equals, ""};
    CHECK(textOf({condition}, QueryCombinator::All, std::nullopt).empty());
  }

  SECTION("a property condition with an empty key is skipped even with a value")
  {
    const auto condition =
      QueryCondition{"property", "", QueryConditionOperator::Equals, "x"};
    CHECK(textOf({condition}, QueryCombinator::All, std::nullopt).empty());
  }

  SECTION("multiple conditions are joined by the combinator")
  {
    const auto conditions = std::vector<QueryCondition>{
      {"classname", "", QueryConditionOperator::Equals, "x"},
      {"visible", "", QueryConditionOperator::Is, "true"},
    };
    CHECK(
      textOf(conditions, QueryCombinator::All, std::nullopt)
      == R"(classname == "x" && visible == true)");
    CHECK(
      textOf(conditions, QueryCombinator::Any, std::nullopt)
      == R"(classname == "x" || visible == true)");
  }

  SECTION("a hint with no complete rows is the bare type check")
  {
    CHECK(textOf({}, QueryCombinator::All, ql::QueryKind::Layer) == R"(type == "layer")");

    const auto emptyCondition =
      QueryCondition{"classname", "", QueryConditionOperator::Equals, ""};
    CHECK(
      textOf({emptyCondition}, QueryCombinator::All, ql::QueryKind::Layer)
      == R"(type == "layer")");
  }

  SECTION("a hint is ANDed onto a single row without extra parens")
  {
    const auto condition =
      QueryCondition{"classname", "", QueryConditionOperator::Equals, "x"};
    CHECK(
      textOf({condition}, QueryCombinator::Any, ql::QueryKind::Entity)
      == R"(type == "entity" && classname == "x")");
  }

  SECTION("a hint is ANDed onto an All-chain without extra parens")
  {
    const auto conditions = std::vector<QueryCondition>{
      {"classname", "", QueryConditionOperator::Equals, "x"},
      {"visible", "", QueryConditionOperator::Is, "true"},
    };
    CHECK(
      textOf(conditions, QueryCombinator::All, ql::QueryKind::Entity)
      == R"(type == "entity" && classname == "x" && visible == true)");
  }

  SECTION("a hint parenthesizes an Any-chain of more than one row")
  {
    const auto conditions = std::vector<QueryCondition>{
      {"classname", "", QueryConditionOperator::Equals, "x"},
      {"visible", "", QueryConditionOperator::Is, "true"},
    };
    CHECK(
      textOf(conditions, QueryCombinator::Any, ql::QueryKind::Entity)
      == R"(type == "entity" && ( classname == "x" || visible == true ))");
  }
}

TEST_CASE("QueryBuilder::tryImportQuery")
{
  SECTION("round-trips every shape buildQueryExpression can produce")
  {
    const auto conditions = std::vector<QueryCondition>{
      {"classname", "", QueryConditionOperator::Equals, "x"},
      {"visible", "", QueryConditionOperator::Is, "true"},
      {"entityProperty", "angle", QueryConditionOperator::NotEquals, "90"},
    };

    struct TestCase
    {
      QueryCombinator combinator;
      std::optional<ql::QueryKind> hint;
    };

    const auto testCase = GENERATE(
      TestCase{QueryCombinator::All, std::nullopt},
      TestCase{QueryCombinator::Any, std::nullopt},
      TestCase{QueryCombinator::All, std::optional{ql::QueryKind::Entity}},
      TestCase{QueryCombinator::Any, std::optional{ql::QueryKind::Entity}});

    const auto text = textOf(conditions, testCase.combinator, testCase.hint);
    CAPTURE(text);

    const auto imported = importText(text);
    REQUIRE(imported.has_value());
    CHECK(imported->hint == testCase.hint);
    CHECK(imported->combinator == testCase.combinator);
    CHECK(conditionListsEqual(imported->conditions, conditions));
  }

  SECTION("round-trips a single row with a hint")
  {
    const auto condition =
      QueryCondition{"classname", "", QueryConditionOperator::Equals, "x"};
    const auto text = textOf({condition}, QueryCombinator::All, ql::QueryKind::Entity);

    const auto imported = importText(text);
    REQUIRE(imported.has_value());
    CHECK(imported->hint == ql::QueryKind::Entity);
    CHECK(conditionListsEqual(imported->conditions, {condition}));
  }

  SECTION("round-trips a bare hint with no rows")
  {
    const auto imported =
      importText(textOf({}, QueryCombinator::All, ql::QueryKind::Layer));
    REQUIRE(imported.has_value());
    CHECK(imported->hint == ql::QueryKind::Layer);
    CHECK(imported->conditions.empty());
  }

  SECTION("rejects a negation")
  {
    CHECK(importText(R"(!(classname == "x"))") == std::nullopt);
  }

  SECTION("rejects a mix of && and ||")
  {
    CHECK(
      importText(R"(classname == "x" && (visible == true || locked == true))")
      == std::nullopt);
  }

  SECTION("rejects a function call that isn't like/contains")
  {
    CHECK(importText(R"(distanceTo(center, vec(0,0,0)) <= 256)") == std::nullopt);
  }

  SECTION("rejects a field outside the builder's catalog")
  {
    CHECK(importText(R"(foo == "bar")") == std::nullopt);
  }

  SECTION("rejects a comparison operator the builder never generates")
  {
    CHECK(importText(R"(properties.light > 200)") == std::nullopt);
  }

  SECTION("rejects != on a Boolean field, which the builder never generates")
  {
    CHECK(importText("visible != true") == std::nullopt);
  }
}

} // namespace tb::ui
