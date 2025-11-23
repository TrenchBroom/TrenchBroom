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

#include "Macros.h"
#include "Result.h"
#include "el/ELParser.h"
#include "mdl/GameConfig.h"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace tb
{
namespace mdl
{
class BrushFaceAttributes;
}

namespace io
{

class GameConfigParser
{
private:
  el::ELParser m_elParser;
  std::filesystem::path m_path;

public:
  explicit GameConfigParser(std::string_view str, std::filesystem::path path = {});

  Result<mdl::GameConfig> parse();

  deleteCopyAndMove(GameConfigParser);
};

std::optional<vm::bbox3d> parseSoftMapBoundsString(const std::string& string);
std::string serializeSoftMapBoundsString(const vm::bbox3d& bounds);

} // namespace io
} // namespace tb
