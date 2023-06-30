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

#include <filesystem>
#include <vector>

namespace TrenchBroom::IO::SystemPaths
{
/**
 * Returns the directory containing the TrenchBroom executable (this will be inside the
 * .app bundle on macOS).
 */
std::filesystem::path appDirectory();
/**
 * Returns the directory where configs should be written
 * e.g. `C:\\Users\\<user>\\AppData\\Roaming\\TrenchBroom`
 */
std::filesystem::path userDataDirectory();

std::filesystem::path logFilePath();

std::filesystem::path findResourceFile(const std::filesystem::path& file);
/**
 * Returns the possible search paths for the requested directory name.
 * They may or may not exist.
 */
std::vector<std::filesystem::path> findResourceDirectories(
  const std::filesystem::path& directory);
} // namespace TrenchBroom::IO::SystemPaths
