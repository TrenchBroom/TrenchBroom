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

#include "mdl/EntityDefinitionTestUtils.h"

#include "TestParserStatus.h"
#include "el/EvaluationContext.h"
#include "el/ParseExpression.h"
#include "el/VariableStore.h"
#include "mdl/CatchConfig.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionParser.h"

#include "kd/contracts.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

ModelSpecification getModelSpecification(
  EntityDefinitionParser& parser, const std::string& entityPropertiesStr)
{
  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  REQUIRE(definitions);
  CHECK(definitions.value().size() == 1u);

  const auto& definition = definitions.value()[0];
  CHECK(getType(definition) == EntityDefinitionType::Point);

  return getModelSpecification(definition, entityPropertiesStr);
}

ModelSpecification getModelSpecification(
  const EntityDefinition& definition, const std::string& entityPropertiesStr)
{
  contract_pre(getType(definition) == EntityDefinitionType::Point);

  const auto& pointDefinition = *definition.pointEntityDefinition;
  const auto& modelDefinition = pointDefinition.modelDefinition;
  return getModelSpecification(modelDefinition, entityPropertiesStr);
}

ModelSpecification getModelSpecification(
  const ModelDefinition& modelDefinition, const std::string& entityPropertiesStr)
{
  return el::withEvaluationContext([&](auto& context) {
           const auto entityPropertiesMap =
             el::parseExpression(el::ParseMode::Strict, entityPropertiesStr)
               .value()
               .evaluate(context)
               .mapValue(context);
           const auto variableStore = el::VariableTable{entityPropertiesMap};
           return modelDefinition.modelSpecification(variableStore);
         })
         | kdl::value();
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  EntityDefinitionParser& parser,
  const std::string& entityPropertiesStr)
{
  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  REQUIRE(definitions);
  CHECK(definitions.value().size() == 1u);

  const auto& definition = definitions.value()[0];
  CHECK(getType(definition) == EntityDefinitionType::Point);

  assertDecalDefinition(expected, definition, entityPropertiesStr);
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  const EntityDefinition& definition,
  const std::string& entityPropertiesStr)
{
  contract_pre(getType(definition) == EntityDefinitionType::Point);

  const auto& pointDefinition = *definition.pointEntityDefinition;
  const auto& modelDefinition = pointDefinition.decalDefinition;
  assertDecalDefinition(expected, modelDefinition, entityPropertiesStr);
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  const DecalDefinition& actual,
  const std::string& entityPropertiesStr)
{
  el::withEvaluationContext([&](auto& context) {
    const auto entityPropertiesMap =
      el::parseExpression(el::ParseMode::Strict, entityPropertiesStr)
        .value()
        .evaluate(context)
        .mapValue(context);
    const auto variableStore = el::VariableTable{entityPropertiesMap};
    CHECK(actual.decalSpecification(variableStore) == expected);
  }).ignore();
}

} // namespace tb::mdl
