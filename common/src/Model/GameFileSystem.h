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

#include "IO/VirtualFileSystem.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
class Logger;

namespace IO
{
class Path;
class Quake3ShaderFileSystem;
} // namespace IO

namespace Model
{
struct GameConfig;

class GameFileSystem : public IO::VirtualFileSystem
{
private:
  IO::Quake3ShaderFileSystem* m_shaderFS = nullptr;
  std::vector<IO::VirtualMountPointId> m_wadMountPoints;

public:
  void initialize(
    const GameConfig& config,
    const IO::Path& gamePath,
    const std::vector<IO::Path>& additionalSearchPaths,
    Logger& logger);
  void reloadShaders();
  void reloadWads(
    const IO::Path& rootPath,
    const std::vector<IO::Path>& wadSearchPaths,
    const std::vector<IO::Path>& wadPaths,
    Logger& logger);

private:
  void addDefaultAssetPaths(const GameConfig& config, Logger& logger);
  void addGameFileSystems(
    const GameConfig& config,
    const IO::Path& gamePath,
    const std::vector<IO::Path>& additionalSearchPaths,
    Logger& logger);
  void addShaderFileSystem(const GameConfig& config, Logger& logger);
  void addFileSystemPath(const IO::Path& path, Logger& logger);
  void addFileSystemPackages(
    const GameConfig& config, const IO::Path& searchPath, Logger& logger);

  void mountWads(
    const IO::Path& rootPath,
    const std::vector<IO::Path>& wadSearchPaths,
    const std::vector<IO::Path>& wadPaths,
    Logger& logger);
  void unmountWads();
};
} // namespace Model
} // namespace TrenchBroom
