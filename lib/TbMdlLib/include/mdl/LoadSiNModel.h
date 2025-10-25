/*
 Copyright (C) 2025 Kristian Duske

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

#include "Result.h"
#include "fs/Reader.h"
#include "mdl/EntityModel.h"

#include <filesystem>
#include <string>

namespace tb
{
class Logger;

namespace fs
{
class FileSystem;
} // namespace fs

namespace mdl
{
class Palette;

bool canLoadSiNModel(const std::filesystem::path& path, fs::Reader reader);

Result<EntityModelData> loadSiNModel(
  std::string name,
  fs::Reader reader,
  const fs::FileSystem& fs,
  Logger& logger);

} // namespace mdl
} // namespace tb
