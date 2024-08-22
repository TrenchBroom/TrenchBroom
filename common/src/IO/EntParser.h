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

#include "IO/EntityDefinitionParser.h"

#include <string_view>
#include <vector>

namespace TrenchBroom::IO
{
struct EntityDefinitionClassInfo;
class ParserStatus;

class EntParser : public EntityDefinitionParser
{
private:
  std::string_view m_str;

public:
  EntParser(std::string_view str, const Color& defaultEntityColor);

private:
  std::vector<EntityDefinitionClassInfo> parseClassInfos(ParserStatus& status) override;
};

} // namespace TrenchBroom::IO
