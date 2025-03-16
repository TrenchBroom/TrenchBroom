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
#include "Result.h"

#include <memory>
#include <vector>

namespace tb::mdl
{
class PropertyDefinition;
class EntityDefinition;
} // namespace tb::mdl

namespace tb::io
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

  Result<std::vector<std::unique_ptr<mdl::EntityDefinition>>> parseDefinitions(
    ParserStatus& status);

private:
  virtual std::vector<EntityDefinitionClassInfo> parseClassInfos(
    ParserStatus& status) = 0;
};

} // namespace tb::io
