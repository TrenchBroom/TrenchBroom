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

#include "EntityDefinitionTestUtils.h"

#include "Assets/EntityDefinition.h"
#include "EL/EvaluationContext.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "EL/VariableStore.h"
#include "IO/ELParser.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/TestParserStatus.h"

#include <kdl/vector_utils.h>

#include <string>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom::Assets
{
void assertModelDefinition(
  const ModelSpecification& expected,
  IO::EntityDefinitionParser& parser,
  const std::string& entityPropertiesStr)
{
  auto status = IO::TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == EntityDefinitionType::PointEntity);

  assertModelDefinition(expected, definition, entityPropertiesStr);

  kdl::vec_clear_and_delete(definitions);
}

void assertModelDefinition(
  const ModelSpecification& expected,
  const EntityDefinition* definition,
  const std::string& entityPropertiesStr)
{
  assert(definition->type() == EntityDefinitionType::PointEntity);

  const auto* pointDefinition = dynamic_cast<const PointEntityDefinition*>(definition);
  const auto& modelDefinition = pointDefinition->modelDefinition();
  assertModelDefinition(expected, modelDefinition, entityPropertiesStr);
}

void assertModelDefinition(
  const ModelSpecification& expected,
  const ModelDefinition& actual,
  const std::string& entityPropertiesStr)
{
  const auto entityPropertiesMap = IO::ELParser::parseStrict(entityPropertiesStr)
                                     .evaluate(EL::EvaluationContext{})
                                     .mapValue();
  const auto variableStore = EL::VariableTable{entityPropertiesMap};
  CHECK(actual.modelSpecification(variableStore) == expected);
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  IO::EntityDefinitionParser& parser,
  const std::string& entityPropertiesStr)
{
  auto status = IO::TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == EntityDefinitionType::PointEntity);

  assertDecalDefinition(expected, definition, entityPropertiesStr);

  kdl::vec_clear_and_delete(definitions);
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  const EntityDefinition* definition,
  const std::string& entityPropertiesStr)
{
  assert(definition->type() == EntityDefinitionType::PointEntity);

  const auto* pointDefinition = dynamic_cast<const PointEntityDefinition*>(definition);
  const auto& modelDefinition = pointDefinition->decalDefinition();
  assertDecalDefinition(expected, modelDefinition, entityPropertiesStr);
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  const DecalDefinition& actual,
  const std::string& entityPropertiesStr)
{
  const auto entityPropertiesMap = IO::ELParser::parseStrict(entityPropertiesStr)
                                     .evaluate(EL::EvaluationContext{})
                                     .mapValue();
  const auto variableStore = EL::VariableTable{entityPropertiesMap};
  CHECK(actual.decalSpecification(variableStore) == expected);
}
} // namespace TrenchBroom::Assets
