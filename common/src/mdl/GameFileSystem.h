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

#include "fs/VirtualFileSystem.h"

#include <filesystem>
#include <vector>

namespace tb
{
class Logger;
}

namespace tb::mdl
{
struct GameConfig;

class GameFileSystem : public io::VirtualFileSystem
{
private:
  std::vector<io::VirtualMountPointId> m_wadMountPoints;

public:
  void initialize(
    const GameConfig& config,
    const std::filesystem::path& gamePath,
    const std::vector<std::filesystem::path>& additionalSearchPaths,
    Logger& logger);
  void reloadWads(
    const std::filesystem::path& rootPath,
    const std::vector<std::filesystem::path>& wadSearchPaths,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger);

private:
  void addDefaultAssetPaths(const GameConfig& config, Logger& logger);
  void addGameFileSystems(
    const GameConfig& config,
    const std::filesystem::path& gamePath,
    const std::vector<std::filesystem::path>& additionalSearchPaths,
    Logger& logger);
  void addSearchPath(
    const GameConfig& config,
    const std::filesystem::path& gamePath,
    const std::filesystem::path& searchPath,
    Logger& logger);
  void addFileSystemPath(const std::filesystem::path& path, Logger& logger);
  void addFileSystemPackages(
    const GameConfig& config, const std::filesystem::path& searchPath, Logger& logger);

  void mountWads(
    const std::filesystem::path& rootPath,
    const std::vector<std::filesystem::path>& wadSearchPaths,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger);
  void unmountWads();
};
} // namespace tb::mdl
