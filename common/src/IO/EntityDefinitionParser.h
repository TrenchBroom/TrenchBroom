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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace TrenchBroom::Assets
{
class PropertyDefinition;
class EntityDefinition;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::IO
{
struct EntityDefinitionClassInfo;
class ParserStatus;

// exposed for testing
std::vector<EntityDefinitionClassInfo> resolveInheritance(
  ParserStatus& status, const std::vector<EntityDefinitionClassInfo>& classInfos);

class EntityDefinitionParser
{
private:
  Color m_defaultEntityColor;

public:
  explicit EntityDefinitionParser(const Color& defaultEntityColor);
  virtual ~EntityDefinitionParser();

  std::vector<Assets::EntityDefinition*> parseDefinitions(ParserStatus& status);

private:
  virtual std::vector<EntityDefinitionClassInfo> parseClassInfos(
    ParserStatus& status) = 0;
};

} // namespace TrenchBroom::IO
