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

#include "GameFileSystem.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/DkPakFileSystem.h"
#include "IO/IdPakFileSystem.h"
#include "IO/PathInfo.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/SystemPaths.h"
#include "IO/WadFileSystem.h"
#include "IO/ZipFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <memory>

namespace TrenchBroom
{
namespace Model
{

void GameFileSystem::initialize(
  const GameConfig& config,
  const IO::Path& gamePath,
  const std::vector<IO::Path>& additionalSearchPaths,
  Logger& logger)
{
  unmountAll();
  m_shaderFS = nullptr;

  addDefaultAssetPaths(config, logger);

  if (!gamePath.empty() && IO::Disk::pathInfo(gamePath) == IO::PathInfo::Directory)
  {
    addGameFileSystems(config, gamePath, additionalSearchPaths, logger);
    addShaderFileSystem(config, logger);
  }
}

void GameFileSystem::reloadShaders()
{
  if (m_shaderFS)
  {
    m_shaderFS->reload();
  }
}

void GameFileSystem::reloadWads(
  const IO::Path& rootPath,
  const std::vector<IO::Path>& wadSearchPaths,
  const std::vector<IO::Path>& wadPaths,
  Logger& logger)
{
  unmountWads();
  mountWads(rootPath, wadSearchPaths, wadPaths, logger);
}

void GameFileSystem::addDefaultAssetPaths(const GameConfig& config, Logger& logger)
{
  // There are two ways of providing default assets: The 'defaults/assets' folder in
  // TrenchBroom's resources folder, and the 'assets' folder in the game configuration
  // folders. We add filesystems for both types here.

  auto defaultFolderPaths =
    IO::SystemPaths::findResourceDirectories(IO::Path("defaults"));
  if (!config.path.empty())
  {
    defaultFolderPaths.push_back(config.path.deleteLastComponent());
  }

  for (const auto& defaultFolderPath : defaultFolderPaths)
  {
    const auto defaultAssetsPath = defaultFolderPath / IO::Path("assets");
    auto exists = [](const auto& path) {
      try
      {
        return IO::Disk::pathInfo(path) == IO::PathInfo::Directory;
      }
      catch (const FileSystemException&)
      {
        return false;
      }
    };
    if (exists(defaultAssetsPath))
    {
      addFileSystemPath(defaultAssetsPath, logger);
    }
  }
}

void GameFileSystem::addGameFileSystems(
  const GameConfig& config,
  const IO::Path& gamePath,
  const std::vector<IO::Path>& additionalSearchPaths,
  Logger& logger)
{
  const auto& fileSystemConfig = config.fileSystemConfig;
  addFileSystemPath(gamePath / fileSystemConfig.searchPath, logger);
  addFileSystemPackages(config, gamePath / fileSystemConfig.searchPath, logger);

  for (const auto& searchPath : additionalSearchPaths)
  {
    addFileSystemPath(gamePath / searchPath, logger);
    addFileSystemPackages(config, gamePath / searchPath, logger);
  }
}

void GameFileSystem::addFileSystemPath(const IO::Path& path, Logger& logger)
{
  try
  {
    logger.info() << "Adding file system path " << path;
    mount(IO::Path{}, std::make_unique<IO::DiskFileSystem>(path));
  }
  catch (const FileSystemException& e)
  {
    logger.error() << "Could not add file system search path '" << path
                   << "': " << e.what();
  }
}

void GameFileSystem::addFileSystemPackages(
  const GameConfig& config, const IO::Path& searchPath, Logger& logger)
{
  const auto& fileSystemConfig = config.fileSystemConfig;
  const auto& packageFormatConfig = fileSystemConfig.packageFormat;

  const auto& packageExtensions = packageFormatConfig.extensions;
  const auto& packageFormat = packageFormatConfig.format;

  if (IO::Disk::pathInfo(searchPath) == IO::PathInfo::Directory)
  {
    const auto diskFS = IO::DiskFileSystem{searchPath};
    auto packages =
      diskFS.find(IO::Path{}, IO::makeExtensionPathMatcher(packageExtensions));
    packages = kdl::vec_sort(std::move(packages));

    for (const auto& packagePath : packages)
    {
      try
      {
        const auto absPackagePath = diskFS.makeAbsolute(packagePath);
        if (kdl::ci::str_is_equal(packageFormat, "idpak"))
        {
          logger.info() << "Adding file system package " << packagePath;
          mount(IO::Path{}, std::make_unique<IO::IdPakFileSystem>(absPackagePath));
        }
        else if (kdl::ci::str_is_equal(packageFormat, "dkpak"))
        {
          logger.info() << "Adding file system package " << packagePath;
          mount(IO::Path{}, std::make_unique<IO::DkPakFileSystem>(absPackagePath));
        }
        else if (kdl::ci::str_is_equal(packageFormat, "zip"))
        {
          logger.info() << "Adding file system package " << packagePath;
          mount(IO::Path{}, std::make_unique<IO::ZipFileSystem>(absPackagePath));
        }
      }
      catch (const std::exception& e)
      {
        logger.error() << e.what();
      }
    }
  }
}

void GameFileSystem::addShaderFileSystem(const GameConfig& config, Logger& logger)
{
  // To support Quake 3 shaders, we add a shader file system that loads the shaders
  // and makes them available as virtual files.
  const auto& textureConfig = config.textureConfig;
  if (!textureConfig.shaderSearchPath.empty())
  {
    logger.info() << "Adding shader file system";
    auto shaderSearchPath = textureConfig.shaderSearchPath;
    auto textureSearchPaths =
      std::vector<IO::Path>{textureConfig.root, IO::Path("models")};

    auto shaderFs = std::make_unique<IO::Quake3ShaderFileSystem>(
      *this, std::move(shaderSearchPath), std::move(textureSearchPaths), logger);
    m_shaderFS = shaderFs.get();
    mount(IO::Path{}, std::move(shaderFs));
  }
}

void GameFileSystem::mountWads(
  const IO::Path& rootPath,
  const std::vector<IO::Path>& wadSearchPaths,
  const std::vector<IO::Path>& wadPaths,
  Logger& logger)
{
  for (const auto& wadPath : wadPaths)
  {
    const auto mountPath = rootPath / wadPath.lastComponent();
    const auto resolvedWadPath = IO::Disk::resolvePath(wadSearchPaths, wadPath);
    try
    {
      m_wadMountPoints.push_back(
        mount(mountPath, std::make_unique<IO::WadFileSystem>(resolvedWadPath)));
    }
    catch (const Exception& e)
    {
      logger.error() << "Could not load wad file at '" << wadPath << "': " << e.what();
    }
  }
}

void GameFileSystem::unmountWads()
{
  for (const auto& id : m_wadMountPoints)
  {
    unmount(id);
  }
  m_wadMountPoints.clear();
}

} // namespace Model
} // namespace TrenchBroom
