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

#include "el/Expression.h"
#include "io/EntityDefinitionClassInfo.h"
#include "io/EntityDefinitionParser.h"
#include "io/TestParserStatus.h"
#include "mdl/EntityProperties.h"
#include "mdl/PropertyDefinition.h"

#include <vector>

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("resolveInheritance")
{
  SECTION("filterBaseClasses")
  {
    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("filterRedundantClasses")
  {
    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "a",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       1,
       "a",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       1,
       "b",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "b",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       1,
       "c",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       2,
       "c",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "c",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "d",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       1,
       "d",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "e",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       1,
       "e",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "f",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       1,
       "f",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BrushClass,
       0,
       1,
       "b",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       1,
       "c",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       2,
       "c",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "d",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "e",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 6u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("overrideMembersIfNotPresent")
  {
    const auto baseModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"abc"}}}};
    const auto baseDecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"def"}}}};

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       "description",
       Color{1, 2, 3},
       vm::bbox3d{-1, 1},
       baseModelDef,
       baseDecalDef,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "description",
       Color{1, 2, 3},
       vm::bbox3d{-1, 1},
       baseModelDef,
       baseDecalDef,
       {},
       {"base"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("skipMembersIfPresent")
  {
    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       "description",
       Color{1, 2, 3},
       vm::bbox3d{-1, 1},
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "blah blah",
       Color{2, 3, 4},
       vm::bbox3d{-2, 2},
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "blah blah",
       Color{2, 3, 4},
       vm::bbox3d{-2, 2},
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("mergeModelDefinitions")
  {
    const auto baseModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"abc"}}}};
    const auto pointModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"xyz"}}}};
    auto mergedModelDef = pointModelDef;
    mergedModelDef.append(baseModelDef);

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       baseModelDef,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       pointModelDef,
       std::nullopt,
       {},
       {"base"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       mergedModelDef,
       std::nullopt,
       {},
       {"base"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }


  SECTION("mergeDecalDefinitions")
  {
    const auto baseDecalDef = mdl::DecalDefinition{
      el::ExpressionNode{el::LiteralExpression{el::Value{"decal1"}}}};
    const auto pointDecalDef = mdl::DecalDefinition{
      el::ExpressionNode{el::LiteralExpression{el::Value{"decal2"}}}};
    auto mergedDecalDef = pointDecalDef;
    mergedDecalDef.append(baseDecalDef);

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       baseDecalDef,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       pointDecalDef,
       {},
       {"base"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       mergedDecalDef,
       {},
       {"base"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("inheritPropertyDefinitions")
  {
    using namespace mdl::PropertyValueTypes;

    const auto a1_1 = mdl::PropertyDefinition{"a1", String{}, "a1_1", ""};
    const auto a1_2 = mdl::PropertyDefinition{"a1", String{}, "a1_2", ""};
    const auto a2 = mdl::PropertyDefinition{"a2", String{}, "a2", ""};
    const auto a3 = mdl::PropertyDefinition{"a3", String{}, "a3", ""};

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a1_1, a2},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a1_2, a3},
       {"base"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a1_2, a3, a2},
       {"base"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("mergeSpawnflagsSimpleInheritance")
  {
    auto a1 = mdl::PropertyDefinition{
      mdl::EntityPropertyKeys::Spawnflags,
      mdl::PropertyValueTypes::Flags{
        {
          {1 << 1, "a1_1", ""},
          {1 << 2, "a1_2", ""},
        },
        1 << 1},
      "",
      ""};

    auto a2 = mdl::PropertyDefinition{
      mdl::EntityPropertyKeys::Spawnflags,
      mdl::PropertyValueTypes::Flags{
        {
          {1 << 2, "a2_2", ""},
          {1 << 4, "a2_4", ""},
        },
        1 << 2},
      "",
      ""};

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a1},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a2},
       {"base"}},
    };

    auto status = TestParserStatus{};
    const auto output = resolveInheritance(status, input);
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
    CHECK(output.size() == 1u);

    const auto& classInfo = output.front();
    CHECK(
      classInfo.propertyDefinitions
      == std::vector<mdl::PropertyDefinition>{
        {mdl::EntityPropertyKeys::Spawnflags,
         mdl::PropertyValueTypes::Flags{
           {
             {1 << 1, "a1_1", ""},
             {1 << 2, "a2_2", ""},
             {1 << 4, "a2_4", ""},
           },
           (1 << 1) | (1 << 2)},
         "",
         ""},
      });
  }

  SECTION("chainOfBaseClasses")
  {
    using namespace mdl::PropertyValueTypes;

    const auto a1_1 = mdl::PropertyDefinition{"a1", String{}, "a1_1", ""};
    const auto a1_2 = mdl::PropertyDefinition{"a1", String{}, "a1_2", ""};
    const auto a2 = mdl::PropertyDefinition{"a2", String{}, "a2", ""};
    const auto a3 = mdl::PropertyDefinition{"a3", String{}, "a3", ""};

    const auto base1ModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"abc"}}}};
    const auto base2ModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"def"}}}};
    const auto pointModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"xyz"}}}};
    auto mergedModelDef = pointModelDef;
    mergedModelDef.append(base2ModelDef);
    mergedModelDef.append(base1ModelDef);

    const auto base1DecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"dec1"}}}};
    const auto base2DecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"dec2"}}}};
    const auto pointDecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"dec3"}}}};
    auto mergedDecalDef = pointDecalDef;
    mergedDecalDef.append(base2DecalDef);
    mergedDecalDef.append(base1DecalDef);

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base1",
       "base1",
       std::nullopt,
       vm::bbox3d{-2, 2},
       base1ModelDef,
       base1DecalDef,
       {a1_1, a2},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base2",
       "base2",
       Color{1, 2, 3},
       std::nullopt,
       base2ModelDef,
       base2DecalDef,
       {a1_2, a3},
       {"base1"}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       pointModelDef,
       pointDecalDef,
       {},
       {"base2"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "base2",
       Color{1, 2, 3},
       vm::bbox3d{-2, 2},
       mergedModelDef,
       mergedDecalDef,
       {a1_2, a3, a2},
       {"base2"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("multipleBaseClasses")
  {
    using namespace mdl::PropertyValueTypes;

    const auto a1_1 = mdl::PropertyDefinition{"a1", String{}, "a1_1", ""};
    const auto a1_2 = mdl::PropertyDefinition{"a1", String{}, "a1_2", ""};
    const auto a2 = mdl::PropertyDefinition{"a2", String{}, "a2", ""};
    const auto a3 = mdl::PropertyDefinition{"a3", String{}, "a3", ""};

    const auto base1ModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"abc"}}}};
    const auto base2ModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"def"}}}};
    const auto pointModelDef =
      mdl::ModelDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"xyz"}}}};
    auto mergedModelDef = pointModelDef;
    mergedModelDef.append(base1ModelDef);
    mergedModelDef.append(base2ModelDef);

    const auto base1DecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"dec1"}}}};
    const auto base2DecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"dec2"}}}};
    const auto pointDecalDef =
      mdl::DecalDefinition{el::ExpressionNode{el::LiteralExpression{el::Value{"dec3"}}}};
    auto mergedDecalDef = pointDecalDef;
    mergedDecalDef.append(base1DecalDef);
    mergedDecalDef.append(base2DecalDef);

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base1",
       "base1",
       std::nullopt,
       vm::bbox3d{-2, 2},
       base1ModelDef,
       base1DecalDef,
       {a1_1, a2},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base2",
       "base2",
       Color{1, 2, 3},
       std::nullopt,
       base2ModelDef,
       base2DecalDef,
       {a1_2, a3},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       pointModelDef,
       pointDecalDef,
       {},
       {"base1", "base2"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "base1",
       Color{1, 2, 3},
       vm::bbox3d{-2, 2},
       mergedModelDef,
       mergedDecalDef,
       {a1_1, a2, a3},
       {"base1", "base2"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("diamondInheritance")
  {
    using namespace mdl::PropertyValueTypes;

    const auto a1 = mdl::PropertyDefinition{"a1", String{}, "a1", ""};
    const auto a2_1 = mdl::PropertyDefinition{"a2_1", String{}, "a2_1", ""};
    const auto a2_2 = mdl::PropertyDefinition{"a2_2", String{}, "a2_2", ""};
    const auto a3 = mdl::PropertyDefinition{"a3", String{}, "a3", ""};

    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base1",
       "base1",
       std::nullopt,
       vm::bbox3d{-2, 2},
       std::nullopt,
       std::nullopt,
       {a1},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base2_1",
       "base2_1",
       Color{1, 2, 3},
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a2_1},
       {"base1"}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "base2_2",
       "base2_2",
       std::nullopt,
       vm::bbox3d{-1, 1},
       std::nullopt,
       std::nullopt,
       {a2_2},
       {"base1"}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point1",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a3},
       {"base2_1", "base2_2"}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point2",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {a3},
       {"base2_2", "base2_1"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point1",
       "base2_1",
       Color{1, 2, 3},
       vm::bbox3d{-2, 2},
       std::nullopt,
       std::nullopt,
       {a3, a2_1, a1, a2_2},
       {"base2_1", "base2_2"}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point2",
       "base2_2",
       Color{1, 2, 3},
       vm::bbox3d{-1, 1},
       std::nullopt,
       std::nullopt,
       {a3, a2_2, a1, a2_1},
       {"base2_2", "base2_1"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("overloadedSuperClass")
  {
    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "base",
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "base",
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "base",
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "base",
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "brush",
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }

  SECTION("indirectOverloadedSuperClass")
  {
    const auto input = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "base",
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "base",
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BaseClass,
       0,
       0,
       "mid",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"base"}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"mid"}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"mid"}},
    };
    const auto expected = std::vector<EntityDefinitionClassInfo>{
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "base",
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "base",
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {}},
      {EntityDefinitionClassType::PointClass,
       0,
       0,
       "point",
       "point",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"mid"}},
      {EntityDefinitionClassType::BrushClass,
       0,
       0,
       "brush",
       "brush",
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
       {},
       {"mid"}},
    };

    auto status = TestParserStatus{};
    CHECK_THAT(
      resolveInheritance(status, input), Catch::Matchers::UnorderedEquals(expected));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }
}
} // namespace tb::io
