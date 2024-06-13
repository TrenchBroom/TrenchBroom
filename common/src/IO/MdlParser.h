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

#include "Assets/EntityModel_Forward.h"
#include "IO/EntityModelParser.h"

#include "vm/forward.h"
#include "vm/vec.h"

#include <filesystem>
#include <string>
#include <vector>

namespace TrenchBroom::Assets
{
class Palette;
}

namespace TrenchBroom::IO
{
class Reader;

class MdlParser : public EntityModelParser
{
private:
  std::string m_name;
  const Reader& m_reader;
  const Assets::Palette& m_palette;

public:
  MdlParser(std::string name, const Reader& reader, const Assets::Palette& palette);

  static bool canParse(const std::filesystem::path& path, Reader reader);

  std::unique_ptr<Assets::EntityModel> initializeModel(Logger& logger) override;
};

} // namespace TrenchBroom::IO
