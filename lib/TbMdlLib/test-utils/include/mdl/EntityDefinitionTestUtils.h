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

#pragma once

#include "Color.h"
#include "mdl/ModelSpecification.h"

#include "kd/string_utils.h"

#include <string>

namespace tb::mdl
{
class DecalDefinition;
class EntityDefinitionParser;
class ModelDefinition;

struct DecalSpecification;
struct EntityDefinition;
struct ModelSpecification;

ModelSpecification getModelSpecification(
  EntityDefinitionParser& parser, const std::string& entityPropertiesStr = "{}");
ModelSpecification getModelSpecification(
  const EntityDefinition& definition, const std::string& entityPropertiesStr = "{}");
ModelSpecification getModelSpecification(
  const ModelDefinition& modelDefinition, const std::string& entityPropertiesStr = "{}");

template <typename Parser>
ModelSpecification getModelSpecification(
  const std::string& modelStr,
  const std::string& templateStr,
  const std::string& entityPropertiesStr = "{}")
{
  const auto defStr = kdl::str_replace_every(templateStr, "${MODEL}", modelStr);
  auto parser = Parser{defStr, RgbaF{1.0f, 1.0f, 1.0f, 1.0f}};
  return getModelSpecification(parser, entityPropertiesStr);
}

void assertDecalDefinition(
  const DecalSpecification& expected,
  EntityDefinitionParser& parser,
  const std::string& entityPropertiesStr = "{}");
void assertDecalDefinition(
  const DecalSpecification& expected,
  const EntityDefinition& definition,
  const std::string& entityPropertiesStr = "{}");
void assertDecalDefinition(
  const DecalSpecification& expected,
  const DecalDefinition& actual,
  const std::string& entityPropertiesStr = "{}");

template <typename Parser>
void assertDecalDefinition(
  const DecalSpecification& expected,
  const std::string& decalStr,
  const std::string& templateStr,
  const std::string& entityPropertiesStr = "{}")
{
  const auto defStr = kdl::str_replace_every(templateStr, "${DECAL}", decalStr);
  auto parser = Parser{defStr, RgbaF{1.0f, 1.0f, 1.0f, 1.0f}};
  assertDecalDefinition(expected, parser, entityPropertiesStr);
}

} // namespace tb::mdl
