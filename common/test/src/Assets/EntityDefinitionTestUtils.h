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

#pragma once

#include "Color.h"
#include "IO/Path.h"

#include <kdl/string_utils.h>

#include <string>

namespace TrenchBroom {
namespace IO {
class EntityDefinitionParser;
}

namespace Assets {
class EntityDefinition;
class ModelDefinition;
struct ModelSpecification;

void assertModelDefinition(
  const ModelSpecification& expected, IO::EntityDefinitionParser& parser,
  const std::string& entityPropertiesStr = "{}");
void assertModelDefinition(
  const ModelSpecification& expected, const EntityDefinition* definition,
  const std::string& entityPropertiesStr = "{}");
void assertModelDefinition(
  const ModelSpecification& expected, const ModelDefinition& actual,
  const std::string& entityPropertiesStr = "{}");

template <typename Parser>
void assertModelDefinition(
  const ModelSpecification& expected, const std::string& modelStr, const std::string& templateStr,
  const std::string& entityPropertiesStr = "{}") {
  const std::string defStr = kdl::str_replace_every(templateStr, "${MODEL}", modelStr);
  Parser parser(defStr, Color(1.0f, 1.0f, 1.0f, 1.0f));
  assertModelDefinition(expected, parser, entityPropertiesStr);
}
} // namespace Assets
} // namespace TrenchBroom
