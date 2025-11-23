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

#include "io/EntityModelLoader.h"

#include <filesystem>
#include <string>

namespace tb
{
namespace fs
{
class FileSystem;
class Reader;
} // namespace fs

namespace io
{

// see http://tfc.duke.free.fr/coding/md2-specs-en.html
class DkmLoader : public EntityModelLoader
{
private:
  std::string m_name;
  const fs::Reader& m_reader;
  const fs::FileSystem& m_fs;

public:
  DkmLoader(std::string name, const fs::Reader& reader, const fs::FileSystem& fs);

  static bool canParse(const std::filesystem::path& path, fs::Reader reader);

  Result<mdl::EntityModelData> load(Logger& logger) override;
};

} // namespace io
} // namespace tb
