/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <catch2/catch.hpp>

#include "Assets/AttributeDefinition.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/TestParserStatus.h"
#include "Model/EntityAttributes.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("resolveInheritance.filterBaseClasses", "[resolveInheritance]") {
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description   color         size          modelDef      attributes superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "brush", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},        {} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "brush", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},        {} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.filterRedundantClasses", "[resolveInheritance]") {
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description   color         size          modelDef      attributes superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "a", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 1, "a", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 1, "b", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BaseClass,  0, 0, "b", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 1, "c", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 2, "c", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BaseClass,  0, 0, "c", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 0, "d", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 1, "d", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "e", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 1, "e", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BaseClass,  0, 0, "f", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BaseClass,  0, 1, "f", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::BrushClass, 0, 1, "b", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 1, "c", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 2, "c", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::PointClass, 0, 0, "d", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "e", std::nullopt, std::nullopt, std::nullopt, std::nullopt,     {},        {} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 6u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.overrideMembersIfNotPresent", "[resolveInheritance]") {
            const auto baseModelDef = Assets::ModelDefinition(EL::Expression(EL::LiteralExpression(EL::Value("abc")), 0, 0));
            
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description    color           size              modelDef      attributes superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base",  "description", Color(1, 2, 3), vm::bbox3(-1, 1), baseModelDef, {},        {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt,  std::nullopt,   std::nullopt,     std::nullopt, {},        {"base"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point", "description", Color(1, 2, 3), vm::bbox3(-1, 1), baseModelDef, {},        {"base"} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.skipMembersIfPresent", "[resolveInheritance]") {
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description    color           size              modelDef      attributes superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base",  "description", Color(1, 2, 3), vm::bbox3(-1, 1), std::nullopt, {},        {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", "blah blah",   Color(2, 3, 4), vm::bbox3(-2, 2), std::nullopt, {},        {"base"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point", "blah blah",   Color(2, 3, 4), vm::bbox3(-2, 2), std::nullopt, {},        {"base"} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.mergeModelDefinitions", "[resolveInheritance]") {
            const auto baseModelDef = Assets::ModelDefinition(EL::Expression(EL::LiteralExpression(EL::Value("abc")), 0, 0));
            const auto pointModelDef = Assets::ModelDefinition(EL::Expression(EL::LiteralExpression(EL::Value("xyz")), 0, 0));
            auto mergedModelDef = pointModelDef;
            mergedModelDef.append(baseModelDef);
            
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description   color         size          modelDef        attributes superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base",  std::nullopt, std::nullopt, std::nullopt, baseModelDef,   {},        {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, pointModelDef,  {},        {"base"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, mergedModelDef, {},        {"base"} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.inheritAttributes", "[resolveInheritance]") {
            const auto a1_1 = std::make_shared<Assets::StringAttributeDefinition>("a1", "", "", false);
            const auto a1_2 = std::make_shared<Assets::StringAttributeDefinition>("a1", "", "", false);
            const auto a2 = std::make_shared<Assets::StringAttributeDefinition>("a2", "", "", false);
            const auto a3 = std::make_shared<Assets::StringAttributeDefinition>("a3", "", "", false);
        
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description   color         size          modelDef      attributes      superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {a1_1, a2},     {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {a1_2, a3},     {"base"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {a1_2, a3, a2}, {"base"} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.mergeSpawnflagsSimpleInheritance", "[resolveInheritance]") {
            auto a1 = std::make_shared<Assets::FlagsAttributeDefinition>(Model::AttributeNames::Spawnflags);
            a1->addOption(1 << 1, "a1_1", "", true);
            a1->addOption(1 << 2, "a1_2", "", false);
            
            auto a2 = std::make_shared<Assets::FlagsAttributeDefinition>(Model::AttributeNames::Spawnflags);
            a2->addOption(1 << 2, "a2_2", "", true);
            a2->addOption(1 << 4, "a2_4", "", false);
        
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description   color         size          modelDef      attributes  superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {a1},       {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {a2},       {"base"} },
            });

            TestParserStatus status;
            const auto output = resolveInheritance(status, input);
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
            CHECK(output.size() == 1u);
            
            const auto& classInfo = output.front();
            CHECK(classInfo.attributes.size() == 1u);
            
            const auto attribute = classInfo.attributes.front();
            CHECK(attribute->type() == Assets::AttributeDefinitionType::FlagsAttribute);
            
            const auto& flagsAttribute = static_cast<const Assets::FlagsAttributeDefinition&>(*attribute.get());
            CHECK(flagsAttribute.name() == Model::AttributeNames::Spawnflags);
            
            const auto& options = flagsAttribute.options();
            CHECK_THAT(options, Catch::Equals(std::vector<Assets::FlagsAttributeOption>({
                { 1 << 1, "a1_1", "", true },
                { 1 << 2, "a2_2", "", true },
                { 1 << 4, "a2_4", "", false },
            })));
        }

        TEST_CASE("resolveInheritance.multipleBaseClasses", "[resolveInheritance]") {
            const auto a1_1 = std::make_shared<Assets::StringAttributeDefinition>("a1", "", "", false);
            const auto a1_2 = std::make_shared<Assets::StringAttributeDefinition>("a1", "", "", false);
            const auto a2 = std::make_shared<Assets::StringAttributeDefinition>("a2", "", "", false);
            const auto a3 = std::make_shared<Assets::StringAttributeDefinition>("a3", "", "", false);

            const auto base1ModelDef = Assets::ModelDefinition(EL::Expression(EL::LiteralExpression(EL::Value("abc")), 0, 0));
            const auto base2ModelDef = Assets::ModelDefinition(EL::Expression(EL::LiteralExpression(EL::Value("def")), 0, 0));
            const auto pointModelDef = Assets::ModelDefinition(EL::Expression(EL::LiteralExpression(EL::Value("xyz")), 0, 0));
            auto mergedModelDef = pointModelDef;
            mergedModelDef.append(base1ModelDef);
            mergedModelDef.append(base2ModelDef);
            
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name     description   color         size              modelDef        attributes      superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base1", "base1",      std::nullopt, vm::bbox3(-2, 2), base1ModelDef,  {a1_1, a2},     {}                 },
                { EntityDefinitionClassType::BaseClass,  0, 0, "base2", "base2",      Color(1,2,3), std::nullopt,     base2ModelDef,  {a1_2, a3},     {}                 },
                { EntityDefinitionClassType::PointClass, 0, 0, "point", std::nullopt, std::nullopt, std::nullopt,     pointModelDef,  {},             {"base1", "base2"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point", "base1",      Color(1,2,3), vm::bbox3(-2, 2), mergedModelDef, {a1_1, a2, a3}, {"base1", "base2"} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }

        TEST_CASE("resolveInheritance.diamondInheritance", "[resolveInheritance]") {
            const auto a1 = std::make_shared<Assets::StringAttributeDefinition>("a1", "", "", false);
            const auto a2_1 = std::make_shared<Assets::StringAttributeDefinition>("a2_1", "", "", false);
            const auto a2_2 = std::make_shared<Assets::StringAttributeDefinition>("a2_2", "", "", false);
            const auto a3 = std::make_shared<Assets::StringAttributeDefinition>("a3", "", "", false);

            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name       description    color         size              modelDef      attributes      superclasses
                { EntityDefinitionClassType::BaseClass,  0, 0, "base1",   "base1",       std::nullopt, vm::bbox3(-2, 2), std::nullopt, {a1},   {}                     },
                { EntityDefinitionClassType::BaseClass,  0, 0, "base2_1", "base2_1",     Color(1,2,3), std::nullopt,     std::nullopt, {a2_1}, {"base1"}              },
                { EntityDefinitionClassType::BaseClass,  0, 0, "base2_2", "base2_2",     std::nullopt, vm::bbox3(-1, 1), std::nullopt, {a2_2}, {"base1"}              },
                { EntityDefinitionClassType::PointClass, 0, 0, "point1",   std::nullopt, std::nullopt, std::nullopt,     std::nullopt, {a3},   {"base2_1", "base2_2"} },
                { EntityDefinitionClassType::PointClass, 0, 0, "point2",   std::nullopt, std::nullopt, std::nullopt,     std::nullopt, {a3},   {"base2_2", "base2_1"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "point1", "base2_1", Color(1,2,3), vm::bbox3(-2, 2), std::nullopt, {a3, a2_1, a1, a2_2}, {"base2_1", "base2_2"} },
                { EntityDefinitionClassType::PointClass, 0, 0, "point2", "base2_2", Color(1,2,3), vm::bbox3(-1, 1), std::nullopt, {a3, a2_2, a1, a2_1}, {"base2_2", "base2_1"} },
            });
            
            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }
        
        TEST_CASE("resolveInheritance.overloadedSuperClass", "[resolveInheritance]") {
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name      description   color         size          modelDef      attributes      superclasses
                { EntityDefinitionClassType::PointClass, 0, 0, "base",   "point",      std::nullopt, std::nullopt, std::nullopt, {},             {}       },
                { EntityDefinitionClassType::BrushClass, 0, 0, "base",   "brush",      std::nullopt, std::nullopt, std::nullopt, {},             {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},             {"base"} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "brush",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},             {"base"} },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "base",   "point",      std::nullopt, std::nullopt, std::nullopt, {},             {}       },
                { EntityDefinitionClassType::BrushClass, 0, 0, "base",   "brush",      std::nullopt, std::nullopt, std::nullopt, {},             {}       },
                { EntityDefinitionClassType::PointClass, 0, 0, "point",  "point",      std::nullopt, std::nullopt, std::nullopt, {},             {"base"} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "brush",  "brush",      std::nullopt, std::nullopt, std::nullopt, {},             {"base"} },
            });

            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }
        
        TEST_CASE("resolveInheritance.indirectOverloadedSuperClass", "[resolveInheritance]") {
            const auto input = std::vector<EntityDefinitionClassInfo>({
                //type                                   l  c  name      description   color         size          modelDef      attributes      superclasses
                { EntityDefinitionClassType::PointClass, 0, 0, "base",   "point",      std::nullopt, std::nullopt, std::nullopt, {},             {}       },
                { EntityDefinitionClassType::BrushClass, 0, 0, "base",   "brush",      std::nullopt, std::nullopt, std::nullopt, {},             {}       },
                { EntityDefinitionClassType::BaseClass,  0, 0, "mid",    std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},             {"base"} },
                { EntityDefinitionClassType::PointClass, 0, 0, "point",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},             {"mid"}  },
                { EntityDefinitionClassType::BrushClass, 0, 0, "brush",  std::nullopt, std::nullopt, std::nullopt, std::nullopt, {},             {"mid"}  },
            });
            const auto expected = std::vector<EntityDefinitionClassInfo>({
                { EntityDefinitionClassType::PointClass, 0, 0, "base",   "point",      std::nullopt, std::nullopt, std::nullopt, {},             {}      },
                { EntityDefinitionClassType::BrushClass, 0, 0, "base",   "brush",      std::nullopt, std::nullopt, std::nullopt, {},             {}      },
                { EntityDefinitionClassType::PointClass, 0, 0, "point",  "point",      std::nullopt, std::nullopt, std::nullopt, {},             {"mid"} },
                { EntityDefinitionClassType::BrushClass, 0, 0, "brush",  "brush",      std::nullopt, std::nullopt, std::nullopt, {},             {"mid"} },
            });

            TestParserStatus status;
            CHECK_THAT(resolveInheritance(status, input), Catch::UnorderedEquals(expected));
            CHECK(status.countStatus(LogLevel::Warn) == 0u);
            CHECK(status.countStatus(LogLevel::Error) == 0u);
        }
    }
}
