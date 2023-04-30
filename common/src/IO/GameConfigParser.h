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

#include "EL/EL_Forward.h"
#include "EL/Value.h"
#include "FloatType.h"
#include "IO/ConfigParserBase.h"
#include "Macros.h"
#include "Model/GameConfig.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom::Model
{
class BrushFaceAttributes;
}

namespace TrenchBroom::IO
{
class Path;

class GameConfigParser : public ConfigParserBase
{
private:
  EL::IntegerType m_version;

public:
  explicit GameConfigParser(std::string_view str, const Path& path = Path{});

  Model::GameConfig parse();

  deleteCopyAndMove(GameConfigParser);
};

std::optional<vm::bbox3> parseSoftMapBoundsString(const std::string& string);
std::string serializeSoftMapBoundsString(const vm::bbox3& bounds);
} // namespace TrenchBroom::IO
